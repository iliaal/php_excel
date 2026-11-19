/*
  +----------------------------------------------------------------------+
  | Copyright (c) 1997-2026 The PHP Group                                |
  +----------------------------------------------------------------------+
  | This source file is subject to version 3.01 of the PHP license,      |
  | that is bundled with this package in the file LICENSE, and is        |
  | available through the world-wide-web at the following url:           |
  | http://www.php.net/license/3_01.txt                                  |
  | If you did not receive a copy of the PHP license and are unable to   |
  | obtain it through the world-wide-web, please send a note to          |
  | license@php.net so we can mail you a copy immediately.               |
  +----------------------------------------------------------------------+
  | Author: Ilia Alshanetsky <ilia@ilia.ws>                              |
  +----------------------------------------------------------------------+
*/

/* Private to excel.c. LibXL calls setlocale() around book creation, release,
 * setLocale(), load and save, which changes the locale of every thread in the
 * process. On glibc the LibXL relocations for setlocale() become per-thread
 * bookkeeping: each thread records the categories LibXL set, on top of "C",
 * and builds a locale_t from those names alone. LibXL's relocations for the
 * locale-dependent libc functions it imports run that function under the
 * thread's LibXL locale and return to the previous one. Outside LibXL code a
 * thread stays on LC_GLOBAL_LOCALE, and none of this reads the global locale,
 * which a concurrent setlocale() may be freeing. The file uses no Zend API so
 * a standalone harness can include it. */

#ifndef PHP_EXCEL_LIBXL_LOCALE_H
#define PHP_EXCEL_LIBXL_LOCALE_H 1

#include <locale.h>
#include <stdbool.h>

#if defined(__linux__) && defined(__GLIBC__) && (defined(__x86_64__) || defined(__aarch64__))
# include <langinfo.h>
/* struct dl_phdr_info needs _GNU_SOURCE before the first system header. */
# if defined(__USE_GNU) && defined(_NL_LOCALE_NAME)
#  define PHP_EXCEL_LOCALE_ISOLATION 1
# else
#  define PHP_EXCEL_LOCALE_ISOLATION_OFF "disabled (built without GNU extensions)"
# endif
#endif

#ifndef PHP_EXCEL_LOCALE_ISOLATION_OFF
# define PHP_EXCEL_LOCALE_ISOLATION_OFF "disabled (unsupported platform)"
#endif

#ifdef PHP_EXCEL_LOCALE_ISOLATION

#include <elf.h>
#include <errno.h>
#include <link.h>
#include <pthread.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>
#include <wchar.h>
#include <wctype.h>
#include <ctype.h>
#include <strings.h>

#define PHP_EXCEL_LC_COUNT (LC_IDENTIFICATION + 1)
#define PHP_EXCEL_LOCALE_MAX_SLOTS 64

#if defined(__x86_64__)
# define PHP_EXCEL_R_JUMP_SLOT R_X86_64_JUMP_SLOT
# define PHP_EXCEL_R_GLOB_DAT R_X86_64_GLOB_DAT
#else
# define PHP_EXCEL_R_JUMP_SLOT R_AARCH64_JUMP_SLOT
# define PHP_EXCEL_R_GLOB_DAT R_AARCH64_GLOB_DAT
#endif

typedef struct php_excel_locale_name {
	struct php_excel_locale_name *next;
	char value[];
} php_excel_locale_name;

/* names[c] is the category LibXL last set on this thread, or NULL for "C".
 * loc is built from names alone, or 0 when every category is "C". */
typedef struct {
	locale_t loc;
	const char *names[PHP_EXCEL_LC_COUNT];
	const unsigned short *ctype_b;
	struct lconv conv;
	size_t books;
} php_excel_thread_locale;

static struct {
	bool active;
	pthread_key_t key;
	locale_t c_locale;
	size_t nslots;
	void **slots[PHP_EXCEL_LOCALE_MAX_SLOTS];
	void *saved[PHP_EXCEL_LOCALE_MAX_SLOTS];
	int prot[PHP_EXCEL_LOCALE_MAX_SLOTS];
	bool localeconv;
	pthread_mutex_t names_lock;
	php_excel_locale_name *names;
	char status[128];
} php_excel_locale_isolation = {
	.names_lock = PTHREAD_MUTEX_INITIALIZER,
	.status = "disabled (not installed)",
};

static const char *const php_excel_lc_names[PHP_EXCEL_LC_COUNT] = {
	[LC_CTYPE] = "LC_CTYPE",
	[LC_NUMERIC] = "LC_NUMERIC",
	[LC_TIME] = "LC_TIME",
	[LC_COLLATE] = "LC_COLLATE",
	[LC_MONETARY] = "LC_MONETARY",
	[LC_MESSAGES] = "LC_MESSAGES",
	[LC_PAPER] = "LC_PAPER",
	[LC_NAME] = "LC_NAME",
	[LC_ADDRESS] = "LC_ADDRESS",
	[LC_TELEPHONE] = "LC_TELEPHONE",
	[LC_MEASUREMENT] = "LC_MEASUREMENT",
	[LC_IDENTIFICATION] = "LC_IDENTIFICATION",
};

static char php_excel_locale_c_name[] = "C";

/* LibXL keeps the pointer setlocale(cat, NULL) returned and passes it back
 * later, so every name handed out lives until the isolation is removed. The
 * set of distinct locale names a process meets is small. */
static inline const char *php_excel_locale_intern(const char *name)
{
	php_excel_locale_name *entry;
	size_t len;

	pthread_mutex_lock(&php_excel_locale_isolation.names_lock);
	for (entry = php_excel_locale_isolation.names; entry; entry = entry->next) {
		if (strcmp(entry->value, name) == 0) {
			pthread_mutex_unlock(&php_excel_locale_isolation.names_lock);
			return entry->value;
		}
	}
	len = strlen(name);
	entry = malloc(sizeof(*entry) + len + 1);
	if (entry) {
		memcpy(entry->value, name, len + 1);
		entry->next = php_excel_locale_isolation.names;
		php_excel_locale_isolation.names = entry;
	}
	pthread_mutex_unlock(&php_excel_locale_isolation.names_lock);
	return entry ? entry->value : NULL;
}

/* Shared by every thread with nothing recorded; install creates it, and
 * code that drives the emulation without installing it gets one here. */
static inline locale_t php_excel_locale_c(void)
{
	locale_t c = __atomic_load_n(&php_excel_locale_isolation.c_locale, __ATOMIC_ACQUIRE);
	locale_t fresh;

	if (c) {
		return c;
	}
	fresh = newlocale(LC_ALL_MASK, "C", (locale_t) 0);
	if (!fresh) {
		return (locale_t) 0;
	}
	if (!__atomic_compare_exchange_n(&php_excel_locale_isolation.c_locale, &c, fresh, false, __ATOMIC_ACQ_REL, __ATOMIC_ACQUIRE)) {
		freelocale(fresh);
		return c;
	}
	return fresh;
}

static inline locale_t php_excel_thread_locale_obj(const php_excel_thread_locale *tl)
{
	return tl && tl->loc ? tl->loc : php_excel_locale_c();
}

static inline void php_excel_thread_locale_set_obj(php_excel_thread_locale *tl, locale_t loc)
{
	locale_t effective;

	if (tl->loc) {
		freelocale(tl->loc);
	}
	tl->loc = loc;
	effective = php_excel_thread_locale_obj(tl);
	tl->ctype_b = effective ? effective->__ctype_b : NULL;
}

static inline void php_excel_thread_locale_clear(php_excel_thread_locale *tl)
{
	php_excel_thread_locale_set_obj(tl, (locale_t) 0);
	memset(tl->names, 0, sizeof(tl->names));
}

static inline void php_excel_thread_locale_dtor(void *ptr)
{
	php_excel_thread_locale_clear(ptr);
	free(ptr);
}

static inline php_excel_thread_locale *php_excel_thread_locale_get(void)
{
	php_excel_thread_locale *tl = pthread_getspecific(php_excel_locale_isolation.key);

	if (!tl) {
		tl = calloc(1, sizeof(*tl));
		if (tl && pthread_setspecific(php_excel_locale_isolation.key, tl) != 0) {
			free(tl);
			tl = NULL;
		}
		if (tl) {
			php_excel_thread_locale_set_obj(tl, (locale_t) 0);
		}
	}
	return tl;
}

static inline const char *php_excel_thread_locale_name(const php_excel_thread_locale *tl, int category)
{
	return tl->names[category] ? tl->names[category] : php_excel_locale_c_name;
}

/* Same form glibc uses: one name when every category agrees, otherwise
 * "LC_CTYPE=...;LC_NUMERIC=...;..." in category order. */
static inline const char *php_excel_thread_locale_all_name(const php_excel_thread_locale *tl)
{
	const char *names[PHP_EXCEL_LC_COUNT];
	const char *first = NULL, *result;
	bool same = true;
	size_t len = 1, off = 0;
	char *buf;
	int c;

	for (c = 0; c < PHP_EXCEL_LC_COUNT; c++) {
		if (c == LC_ALL) {
			continue;
		}
		names[c] = php_excel_thread_locale_name(tl, c);
		if (!first) {
			first = names[c];
		} else if (strcmp(names[c], first) != 0) {
			same = false;
		}
		len += strlen(php_excel_lc_names[c]) + strlen(names[c]) + 2;
	}
	if (same) {
		return first;
	}
	buf = malloc(len);
	if (!buf) {
		return NULL;
	}
	for (c = 0; c < PHP_EXCEL_LC_COUNT; c++) {
		if (c == LC_ALL) {
			continue;
		}
		off += (size_t) snprintf(buf + off, len - off, "%s%s=%s", off ? ";" : "", php_excel_lc_names[c], names[c]);
	}
	result = php_excel_locale_intern(buf);
	free(buf);
	return result;
}

/* setlocale() as LibXL sees it, for the calling thread only. It changes
 * nothing the thread runs on outside LibXL's libc calls. A new locale is
 * built from "C" and the names already recorded, so neither LC_GLOBAL_LOCALE
 * nor setlocale() is consulted. */
static inline char *php_excel_thread_setlocale(int category, const char *value)
{
	php_excel_thread_locale *tl;
	const char *names[PHP_EXCEL_LC_COUNT];
	locale_t loc, next;
	bool any = false;
	int mask, c, err;

	if (category < 0 || category >= PHP_EXCEL_LC_COUNT) {
		errno = EINVAL;
		return NULL;
	}
	tl = php_excel_thread_locale_get();
	if (!tl) {
		errno = ENOMEM;
		return NULL;
	}
	if (!value) {
		return (char *) (category == LC_ALL ? php_excel_thread_locale_all_name(tl) : php_excel_thread_locale_name(tl, category));
	}

	mask = category == LC_ALL ? LC_ALL_MASK : 1 << category;
	loc = newlocale(LC_ALL_MASK, "C", (locale_t) 0);
	if (!loc) {
		return NULL;
	}
	for (c = 0; c < PHP_EXCEL_LC_COUNT; c++) {
		if (c == LC_ALL || !tl->names[c] || (mask & (1 << c))) {
			continue;
		}
		next = newlocale(1 << c, tl->names[c], loc);
		if (!next) {
			err = errno;
			freelocale(loc);
			errno = err;
			return NULL;
		}
		loc = next;
	}
	next = newlocale(mask, value, loc);
	if (!next) {
		err = errno;
		freelocale(loc);
		errno = err;
		return NULL;
	}
	loc = next;

	memcpy(names, tl->names, sizeof(names));
	for (c = 0; c < PHP_EXCEL_LC_COUNT; c++) {
		const char *name;

		if (c == LC_ALL || !(mask & (1 << c))) {
			continue;
		}
		name = nl_langinfo_l(_NL_LOCALE_NAME(c), loc);
		if (!name || strcmp(name, "C") == 0) {
			names[c] = NULL;
			continue;
		}
		names[c] = php_excel_locale_intern(name);
		if (!names[c]) {
			freelocale(loc);
			errno = ENOMEM;
			return NULL;
		}
	}
	for (c = 0; c < PHP_EXCEL_LC_COUNT; c++) {
		any = any || names[c];
	}
	if (!any) {
		freelocale(loc);
		loc = (locale_t) 0;
	}
	php_excel_thread_locale_set_obj(tl, loc);
	memcpy(tl->names, names, sizeof(names));

	return (char *) (category == LC_ALL ? php_excel_thread_locale_all_name(tl) : php_excel_thread_locale_name(tl, category));
}

static inline locale_t php_excel_libxl_locale(void)
{
	return php_excel_thread_locale_obj(php_excel_thread_locale_get());
}

/* Runs one libc call under the thread's LibXL locale, keeping its errno. */
#define PHP_EXCEL_LIBXL_CALL(type, call) do { \
		locale_t php_excel_prev_ = uselocale(php_excel_libxl_locale()); \
		type php_excel_ret_ = (call); \
		int php_excel_err_ = errno; \
		uselocale(php_excel_prev_); \
		errno = php_excel_err_; \
		return php_excel_ret_; \
	} while (0)

#define PHP_EXCEL_LIBXL_VCALL(type, last, vcall) do { \
		va_list ap; \
		locale_t php_excel_prev_; \
		type php_excel_ret_; \
		int php_excel_err_; \
		va_start(ap, last); \
		php_excel_prev_ = uselocale(php_excel_libxl_locale()); \
		php_excel_ret_ = (vcall); \
		php_excel_err_ = errno; \
		uselocale(php_excel_prev_); \
		va_end(ap); \
		errno = php_excel_err_; \
		return php_excel_ret_; \
	} while (0)

/* The plain names, not the ISO C99 ones the headers redirect to: LibXL
 * imports sscanf and swscanf, which keep the GNU %a extension. */
extern int php_excel_gnu_vsscanf(const char *s, const char *format, va_list ap) __asm__("vsscanf");
extern int php_excel_gnu_vswscanf(const wchar_t *s, const wchar_t *format, va_list ap) __asm__("vswscanf");

static char *php_excel_libxl_setlocale(int category, const char *value)
{
	return php_excel_thread_setlocale(category, value);
}

static inline char php_excel_langinfo_char(nl_item item, locale_t loc)
{
	return *nl_langinfo_l(item, loc);
}

static inline char *php_excel_langinfo_grouping(nl_item item, locale_t loc)
{
	char *grouping = nl_langinfo_l(item, loc);

	return *grouping == '\177' || *grouping == '\377' ? (char *) "" : grouping;
}

/* glibc's localeconv() fills one static struct for the whole process; this
 * fills per-thread storage from the thread's LibXL locale instead. */
static struct lconv *php_excel_libxl_localeconv(void)
{
	php_excel_thread_locale *tl = php_excel_thread_locale_get();
	struct lconv *conv;
	locale_t loc;

	if (!tl) {
		return localeconv();
	}
	loc = php_excel_thread_locale_obj(tl);
	if (!loc) {
		return localeconv();
	}
	conv = &tl->conv;
	conv->decimal_point = nl_langinfo_l(RADIXCHAR, loc);
	conv->thousands_sep = nl_langinfo_l(THOUSEP, loc);
	conv->grouping = php_excel_langinfo_grouping(__GROUPING, loc);
	conv->int_curr_symbol = nl_langinfo_l(__INT_CURR_SYMBOL, loc);
	conv->currency_symbol = nl_langinfo_l(__CURRENCY_SYMBOL, loc);
	conv->mon_decimal_point = nl_langinfo_l(__MON_DECIMAL_POINT, loc);
	conv->mon_thousands_sep = nl_langinfo_l(__MON_THOUSANDS_SEP, loc);
	conv->mon_grouping = php_excel_langinfo_grouping(__MON_GROUPING, loc);
	conv->positive_sign = nl_langinfo_l(__POSITIVE_SIGN, loc);
	conv->negative_sign = nl_langinfo_l(__NEGATIVE_SIGN, loc);
	conv->int_frac_digits = php_excel_langinfo_char(__INT_FRAC_DIGITS, loc);
	conv->frac_digits = php_excel_langinfo_char(__FRAC_DIGITS, loc);
	conv->p_cs_precedes = php_excel_langinfo_char(__P_CS_PRECEDES, loc);
	conv->p_sep_by_space = php_excel_langinfo_char(__P_SEP_BY_SPACE, loc);
	conv->n_cs_precedes = php_excel_langinfo_char(__N_CS_PRECEDES, loc);
	conv->n_sep_by_space = php_excel_langinfo_char(__N_SEP_BY_SPACE, loc);
	conv->p_sign_posn = php_excel_langinfo_char(__P_SIGN_POSN, loc);
	conv->n_sign_posn = php_excel_langinfo_char(__N_SIGN_POSN, loc);
	conv->int_p_cs_precedes = php_excel_langinfo_char(__INT_P_CS_PRECEDES, loc);
	conv->int_p_sep_by_space = php_excel_langinfo_char(__INT_P_SEP_BY_SPACE, loc);
	conv->int_n_cs_precedes = php_excel_langinfo_char(__INT_N_CS_PRECEDES, loc);
	conv->int_n_sep_by_space = php_excel_langinfo_char(__INT_N_SEP_BY_SPACE, loc);
	conv->int_p_sign_posn = php_excel_langinfo_char(__INT_P_SIGN_POSN, loc);
	conv->int_n_sign_posn = php_excel_langinfo_char(__INT_N_SIGN_POSN, loc);
	return conv;
}

/* The inline ctype macros LibXL was built with index this table, and the
 * compiler may keep the returned pointer across calls, so it points at
 * per-thread storage that follows the LibXL locale. */
static const unsigned short **php_excel_libxl_ctype_b_loc(void)
{
	php_excel_thread_locale *tl = php_excel_thread_locale_get();

	if (!tl || !tl->ctype_b) {
		return __ctype_b_loc();
	}
	return &tl->ctype_b;
}

static size_t php_excel_libxl_ctype_get_mb_cur_max(void)
{
	PHP_EXCEL_LIBXL_CALL(size_t, __ctype_get_mb_cur_max());
}

static size_t php_excel_libxl_mbstowcs(wchar_t *dst, const char *src, size_t n)
{
	PHP_EXCEL_LIBXL_CALL(size_t, mbstowcs(dst, src, n));
}

static size_t php_excel_libxl_wcstombs(char *dst, const wchar_t *src, size_t n)
{
	PHP_EXCEL_LIBXL_CALL(size_t, wcstombs(dst, src, n));
}

static size_t php_excel_libxl_wcrtomb(char *s, wchar_t wc, mbstate_t *ps)
{
	PHP_EXCEL_LIBXL_CALL(size_t, wcrtomb(s, wc, ps));
}

static int php_excel_libxl_isalnum(int c) { return isalnum_l(c, php_excel_libxl_locale()); }
static int php_excel_libxl_isalpha(int c) { return isalpha_l(c, php_excel_libxl_locale()); }
static int php_excel_libxl_isspace(int c) { return isspace_l(c, php_excel_libxl_locale()); }
static int php_excel_libxl_isxdigit(int c) { return isxdigit_l(c, php_excel_libxl_locale()); }
static int php_excel_libxl_tolower(int c) { return tolower_l(c, php_excel_libxl_locale()); }
static int php_excel_libxl_toupper(int c) { return toupper_l(c, php_excel_libxl_locale()); }
static int php_excel_libxl_iswalnum(wint_t c) { return iswalnum_l(c, php_excel_libxl_locale()); }
static int php_excel_libxl_iswalpha(wint_t c) { return iswalpha_l(c, php_excel_libxl_locale()); }
static int php_excel_libxl_iswdigit(wint_t c) { return iswdigit_l(c, php_excel_libxl_locale()); }
static wint_t php_excel_libxl_towlower(wint_t c) { return towlower_l(c, php_excel_libxl_locale()); }

static int php_excel_libxl_strcasecmp(const char *a, const char *b)
{
	return strcasecmp_l(a, b, php_excel_libxl_locale());
}

static int php_excel_libxl_strcoll(const char *a, const char *b)
{
	return strcoll_l(a, b, php_excel_libxl_locale());
}

static int php_excel_libxl_wcscasecmp(const wchar_t *a, const wchar_t *b)
{
	return wcscasecmp_l(a, b, php_excel_libxl_locale());
}

static double php_excel_libxl_strtod(const char *s, char **end)
{
	return strtod_l(s, end, php_excel_libxl_locale());
}

static long php_excel_libxl_strtol(const char *s, char **end, int base)
{
	return strtol_l(s, end, base, php_excel_libxl_locale());
}

static long long php_excel_libxl_strtoll(const char *s, char **end, int base)
{
	return strtoll_l(s, end, base, php_excel_libxl_locale());
}

static unsigned long php_excel_libxl_strtoul(const char *s, char **end, int base)
{
	return strtoul_l(s, end, base, php_excel_libxl_locale());
}

static int php_excel_libxl_sprintf(char *s, const char *format, ...)
{
	PHP_EXCEL_LIBXL_VCALL(int, format, vsprintf(s, format, ap));
}

static int php_excel_libxl_snprintf(char *s, size_t n, const char *format, ...)
{
	PHP_EXCEL_LIBXL_VCALL(int, format, vsnprintf(s, n, format, ap));
}

static int php_excel_libxl_vsnprintf(char *s, size_t n, const char *format, va_list ap)
{
	PHP_EXCEL_LIBXL_CALL(int, vsnprintf(s, n, format, ap));
}

static int php_excel_libxl_fprintf(FILE *stream, const char *format, ...)
{
	PHP_EXCEL_LIBXL_VCALL(int, format, vfprintf(stream, format, ap));
}

static int php_excel_libxl_swprintf(wchar_t *s, size_t n, const wchar_t *format, ...)
{
	PHP_EXCEL_LIBXL_VCALL(int, format, vswprintf(s, n, format, ap));
}

static int php_excel_libxl_sscanf(const char *s, const char *format, ...)
{
	PHP_EXCEL_LIBXL_VCALL(int, format, php_excel_gnu_vsscanf(s, format, ap));
}

static int php_excel_libxl_swscanf(const wchar_t *s, const wchar_t *format, ...)
{
	PHP_EXCEL_LIBXL_VCALL(int, format, php_excel_gnu_vswscanf(s, format, ap));
}

/* php_excel's own setlocale() calls and the checks that predict how LibXL
 * converts text use the same per-thread state as LibXL. */
static inline char *php_excel_setlocale(int category, const char *value)
{
	if (php_excel_locale_isolation.active) {
		return php_excel_thread_setlocale(category, value);
	}
	return setlocale(category, value);
}

static inline size_t php_excel_locale_mbstowcs(wchar_t *dst, const char *src, size_t n)
{
	if (php_excel_locale_isolation.active) {
		return php_excel_libxl_mbstowcs(dst, src, n);
	}
	return mbstowcs(dst, src, n);
}

static inline const char *php_excel_locale_codeset(void)
{
	if (php_excel_locale_isolation.active) {
		return nl_langinfo_l(CODESET, php_excel_libxl_locale());
	}
	return nl_langinfo(CODESET);
}

/* Drops the calling thread's LibXL locale, for request boundaries. It also
 * puts the thread back on LC_GLOBAL_LOCALE in case a bailout left a LibXL
 * libc call before it could switch back. */
static inline void php_excel_locale_thread_reset(void)
{
	php_excel_thread_locale *tl;

	if (!php_excel_locale_isolation.active) {
		return;
	}
	uselocale(LC_GLOBAL_LOCALE);
	tl = pthread_getspecific(php_excel_locale_isolation.key);
	if (tl) {
		php_excel_thread_locale_clear(tl);
		tl->books = 0;
	}
}

static inline void php_excel_locale_book_created(void)
{
	php_excel_thread_locale *tl;

	if (!php_excel_locale_isolation.active) {
		return;
	}
	tl = php_excel_thread_locale_get();
	if (tl) {
		tl->books++;
	}
}

/* Each release restores the LC_CTYPE its book saw at creation, so books
 * released out of creation order leave a LibXL locale behind. Once the
 * thread holds no book, LibXL has nothing left to restore and the thread's
 * LibXL locale goes back to "C". FrankenPHP workers skip RINIT between
 * requests, so this, not the request hooks, is what ends it there. */
static inline void php_excel_locale_book_released(void)
{
	php_excel_thread_locale *tl;

	if (!php_excel_locale_isolation.active) {
		return;
	}
	tl = pthread_getspecific(php_excel_locale_isolation.key);
	if (!tl) {
		return;
	}
	if (tl->books > 0) {
		tl->books--;
	}
	if (tl->books == 0) {
		php_excel_thread_locale_clear(tl);
	}
}

typedef struct {
	uintptr_t target;
	uintptr_t self;
	bool found;
	bool same_image;
	bool overflow;
	size_t nslots;
	void **slots[PHP_EXCEL_LOCALE_MAX_SLOTS];
	void *wrappers[PHP_EXCEL_LOCALE_MAX_SLOTS];
	int prot[PHP_EXCEL_LOCALE_MAX_SLOTS];
} php_excel_locale_scan;

/* setlocale() and localeconv(), plus every libc function in the LibXL 4.6 to
 * 5.2 import tables whose result depends on the locale. */
static const struct {
	const char *name;
	void *wrapper;
} php_excel_locale_wrappers[] = {
	{ "setlocale", (void *) php_excel_libxl_setlocale },
	{ "localeconv", (void *) php_excel_libxl_localeconv },
	{ "__ctype_b_loc", (void *) php_excel_libxl_ctype_b_loc },
	{ "__ctype_get_mb_cur_max", (void *) php_excel_libxl_ctype_get_mb_cur_max },
	{ "mbstowcs", (void *) php_excel_libxl_mbstowcs },
	{ "wcstombs", (void *) php_excel_libxl_wcstombs },
	{ "wcrtomb", (void *) php_excel_libxl_wcrtomb },
	{ "isalnum", (void *) php_excel_libxl_isalnum },
	{ "isalpha", (void *) php_excel_libxl_isalpha },
	{ "isspace", (void *) php_excel_libxl_isspace },
	{ "isxdigit", (void *) php_excel_libxl_isxdigit },
	{ "tolower", (void *) php_excel_libxl_tolower },
	{ "toupper", (void *) php_excel_libxl_toupper },
	{ "iswalnum", (void *) php_excel_libxl_iswalnum },
	{ "iswalpha", (void *) php_excel_libxl_iswalpha },
	{ "iswdigit", (void *) php_excel_libxl_iswdigit },
	{ "towlower", (void *) php_excel_libxl_towlower },
	{ "strcasecmp", (void *) php_excel_libxl_strcasecmp },
	{ "strcoll", (void *) php_excel_libxl_strcoll },
	{ "wcscasecmp", (void *) php_excel_libxl_wcscasecmp },
	{ "strtod", (void *) php_excel_libxl_strtod },
	{ "strtol", (void *) php_excel_libxl_strtol },
	{ "strtoll", (void *) php_excel_libxl_strtoll },
	{ "strtoul", (void *) php_excel_libxl_strtoul },
	{ "sprintf", (void *) php_excel_libxl_sprintf },
	{ "snprintf", (void *) php_excel_libxl_snprintf },
	{ "vsnprintf", (void *) php_excel_libxl_vsnprintf },
	{ "fprintf", (void *) php_excel_libxl_fprintf },
	{ "swprintf", (void *) php_excel_libxl_swprintf },
	{ "sscanf", (void *) php_excel_libxl_sscanf },
	{ "swscanf", (void *) php_excel_libxl_swscanf },
};

static inline void php_excel_locale_scan_relocs(php_excel_locale_scan *scan, const struct dl_phdr_info *info,
	const ElfW(Rela) *rela, size_t size, const ElfW(Sym) *symtab, const char *strtab, size_t strsz)
{
	size_t i, count = size / sizeof(ElfW(Rela));

	for (i = 0; i < count; i++) {
		unsigned long type = ELF64_R_TYPE(rela[i].r_info);
		const ElfW(Sym) *sym;
		size_t w;

		if (type != PHP_EXCEL_R_JUMP_SLOT && type != PHP_EXCEL_R_GLOB_DAT) {
			continue;
		}
		sym = &symtab[ELF64_R_SYM(rela[i].r_info)];
		if (sym->st_name >= strsz) {
			continue;
		}
		for (w = 0; w < sizeof(php_excel_locale_wrappers) / sizeof(php_excel_locale_wrappers[0]); w++) {
			if (strcmp(strtab + sym->st_name, php_excel_locale_wrappers[w].name) == 0) {
				break;
			}
		}
		if (w == sizeof(php_excel_locale_wrappers) / sizeof(php_excel_locale_wrappers[0])) {
			continue;
		}
		if (scan->nslots == PHP_EXCEL_LOCALE_MAX_SLOTS) {
			scan->overflow = true;
			return;
		}
		scan->wrappers[scan->nslots] = php_excel_locale_wrappers[w].wrapper;
		scan->slots[scan->nslots++] = (void **) (info->dlpi_addr + rela[i].r_offset);
	}
}

/* Protection the dynamic loader left on the page holding addr: that of its
 * PT_LOAD segment, or read-only inside the page-aligned PT_GNU_RELRO span. */
static inline int php_excel_locale_page_prot(const struct dl_phdr_info *info, uintptr_t addr)
{
	uintptr_t page = (uintptr_t) sysconf(_SC_PAGESIZE);
	int prot = -1;
	ElfW(Half) i;

	for (i = 0; i < info->dlpi_phnum; i++) {
		const ElfW(Phdr) *ph = &info->dlpi_phdr[i];
		uintptr_t start = info->dlpi_addr + ph->p_vaddr;

		if (ph->p_type == PT_LOAD && addr >= start && addr < start + ph->p_memsz) {
			prot = ((ph->p_flags & PF_R) ? PROT_READ : 0)
				| ((ph->p_flags & PF_W) ? PROT_WRITE : 0)
				| ((ph->p_flags & PF_X) ? PROT_EXEC : 0);
		}
	}
	for (i = 0; i < info->dlpi_phnum; i++) {
		const ElfW(Phdr) *ph = &info->dlpi_phdr[i];
		uintptr_t start = (info->dlpi_addr + ph->p_vaddr) & ~(page - 1);
		uintptr_t end = (info->dlpi_addr + ph->p_vaddr + ph->p_memsz) & ~(page - 1);

		if (ph->p_type == PT_GNU_RELRO && addr >= start && addr < end) {
			prot = PROT_READ;
		}
	}
	return prot;
}

static inline int php_excel_locale_scan_object(struct dl_phdr_info *info, size_t size, void *data)
{
	php_excel_locale_scan *scan = data;
	const ElfW(Dyn) *dyn = NULL;
	const ElfW(Rela) *jmprel = NULL, *rela = NULL;
	const ElfW(Sym) *symtab = NULL;
	const char *strtab = NULL;
	size_t pltrelsz = 0, relasz = 0, strsz = 0, i;
	ElfW(Sxword) pltrel = 0;
	bool contains = false;

	(void) size;
	for (i = 0; i < info->dlpi_phnum; i++) {
		const ElfW(Phdr) *ph = &info->dlpi_phdr[i];
		uintptr_t start = info->dlpi_addr + ph->p_vaddr;

		if (ph->p_type == PT_LOAD && scan->target >= start && scan->target < start + ph->p_memsz) {
			contains = true;
		} else if (ph->p_type == PT_DYNAMIC) {
			dyn = (const ElfW(Dyn) *) start;
		}
	}
	if (!contains) {
		return 0;
	}
	scan->found = true;
	for (i = 0; i < info->dlpi_phnum; i++) {
		const ElfW(Phdr) *ph = &info->dlpi_phdr[i];
		uintptr_t start = info->dlpi_addr + ph->p_vaddr;

		if (ph->p_type == PT_LOAD && scan->self >= start && scan->self < start + ph->p_memsz) {
			scan->same_image = true;
			return 1;
		}
	}
	if (!dyn) {
		return 1;
	}
	/* ld.so rewrites these entries to run-time addresses on glibc; an entry
	 * below the load base is still a link-time offset. */
#define PHP_EXCEL_DYN_PTR(type, v) \
	((const type *) ((uintptr_t) (v) < info->dlpi_addr ? info->dlpi_addr + (uintptr_t) (v) : (uintptr_t) (v)))
	for (; dyn->d_tag != DT_NULL; dyn++) {
		switch (dyn->d_tag) {
			case DT_SYMTAB: symtab = PHP_EXCEL_DYN_PTR(ElfW(Sym), dyn->d_un.d_ptr); break;
			case DT_STRTAB: strtab = PHP_EXCEL_DYN_PTR(char, dyn->d_un.d_ptr); break;
			case DT_STRSZ: strsz = dyn->d_un.d_val; break;
			case DT_JMPREL: jmprel = PHP_EXCEL_DYN_PTR(ElfW(Rela), dyn->d_un.d_ptr); break;
			case DT_PLTRELSZ: pltrelsz = dyn->d_un.d_val; break;
			case DT_PLTREL: pltrel = dyn->d_un.d_val; break;
			case DT_RELA: rela = PHP_EXCEL_DYN_PTR(ElfW(Rela), dyn->d_un.d_ptr); break;
			case DT_RELASZ: relasz = dyn->d_un.d_val; break;
			default: break;
		}
	}
#undef PHP_EXCEL_DYN_PTR
	if (!symtab || !strtab) {
		return 1;
	}
	if (jmprel && pltrel == DT_RELA) {
		php_excel_locale_scan_relocs(scan, info, jmprel, pltrelsz, symtab, strtab, strsz);
	}
	if (rela) {
		php_excel_locale_scan_relocs(scan, info, rela, relasz, symtab, strtab, strsz);
	}
	for (i = 0; i < scan->nslots; i++) {
		scan->prot[i] = php_excel_locale_page_prot(info, (uintptr_t) scan->slots[i]);
	}
	return 1;
}

/* 0 on success; -1 when nothing was written; -2 when the slot holds the new
 * value but its page could not be made read-only again. */
static inline int php_excel_locale_write_slot(void **slot, void *value, int prot)
{
	uintptr_t pagesize = (uintptr_t) sysconf(_SC_PAGESIZE);
	void *page = (void *) ((uintptr_t) slot & ~(pagesize - 1));

	if (!(prot & PROT_WRITE) && mprotect(page, pagesize, prot | PROT_WRITE) != 0) {
		return -1;
	}
	__atomic_store_n(slot, value, __ATOMIC_SEQ_CST);
	if (!(prot & PROT_WRITE) && mprotect(page, pagesize, prot) != 0) {
		return -2;
	}
	return 0;
}

/* libxl_symbol is the address of any function LibXL exports; it identifies
 * the loaded LibXL image whatever its file name. Call while no other thread
 * runs LibXL code (MINIT). */
static inline bool php_excel_locale_isolation_install(const void *libxl_symbol)
{
	php_excel_locale_scan scan;
	bool has_setlocale = false;
	size_t i;

	if (php_excel_locale_isolation.active) {
		return true;
	}
	memset(&scan, 0, sizeof(scan));
	scan.target = (uintptr_t) libxl_symbol;
	scan.self = (uintptr_t) php_excel_libxl_setlocale;
	dl_iterate_phdr(php_excel_locale_scan_object, &scan);
	if (!scan.found) {
		snprintf(php_excel_locale_isolation.status, sizeof(php_excel_locale_isolation.status),
			"disabled (LibXL image not found)");
		return false;
	}
	/* A static build links LibXL into the same image as this code; its
	 * relocations then belong to PHP as a whole. */
	if (scan.same_image) {
		snprintf(php_excel_locale_isolation.status, sizeof(php_excel_locale_isolation.status),
			"disabled (LibXL is not a separate shared library)");
		return false;
	}
	for (i = 0; i < scan.nslots; i++) {
		has_setlocale = has_setlocale || scan.wrappers[i] == (void *) php_excel_libxl_setlocale;
	}
	if (scan.overflow || !has_setlocale) {
		snprintf(php_excel_locale_isolation.status, sizeof(php_excel_locale_isolation.status),
			"disabled (%s setlocale relocations in LibXL)", scan.overflow ? "too many" : "no");
		return false;
	}
	for (i = 0; i < scan.nslots; i++) {
		if (scan.prot[i] < 0) {
			snprintf(php_excel_locale_isolation.status, sizeof(php_excel_locale_isolation.status),
				"disabled (locale relocation outside LibXL segments)");
			return false;
		}
	}
	php_excel_locale_isolation.c_locale = newlocale(LC_ALL_MASK, "C", (locale_t) 0);
	if (!php_excel_locale_isolation.c_locale) {
		snprintf(php_excel_locale_isolation.status, sizeof(php_excel_locale_isolation.status),
			"disabled (newlocale failed)");
		return false;
	}
	if (pthread_key_create(&php_excel_locale_isolation.key, php_excel_thread_locale_dtor) != 0) {
		freelocale(php_excel_locale_isolation.c_locale);
		php_excel_locale_isolation.c_locale = (locale_t) 0;
		snprintf(php_excel_locale_isolation.status, sizeof(php_excel_locale_isolation.status),
			"disabled (pthread_key_create failed)");
		return false;
	}
	php_excel_locale_isolation.active = true;
	for (i = 0; i < scan.nslots; i++) {
		int written;

		php_excel_locale_isolation.slots[i] = scan.slots[i];
		php_excel_locale_isolation.prot[i] = scan.prot[i];
		php_excel_locale_isolation.saved[i] = __atomic_load_n(scan.slots[i], __ATOMIC_SEQ_CST);
		written = php_excel_locale_write_slot(scan.slots[i], scan.wrappers[i], scan.prot[i]);
		if (written != 0) {
			int err = errno;

			/* A page left writable is not worth running with; put every
			 * slot back, this one included when it was written. */
			if (written == -2) {
				i++;
			}
			while (i-- > 0) {
				php_excel_locale_write_slot(scan.slots[i], php_excel_locale_isolation.saved[i], scan.prot[i]);
			}
			php_excel_locale_isolation.active = false;
			pthread_key_delete(php_excel_locale_isolation.key);
			freelocale(php_excel_locale_isolation.c_locale);
			php_excel_locale_isolation.c_locale = (locale_t) 0;
			snprintf(php_excel_locale_isolation.status, sizeof(php_excel_locale_isolation.status),
				"disabled (mprotect failed: %s)", strerror(err));
			return false;
		}
	}
	php_excel_locale_isolation.nslots = scan.nslots;
	for (i = 0; i < scan.nslots; i++) {
		php_excel_locale_isolation.localeconv = php_excel_locale_isolation.localeconv
			|| scan.wrappers[i] == (void *) php_excel_libxl_localeconv;
	}
	snprintf(php_excel_locale_isolation.status, sizeof(php_excel_locale_isolation.status), "enabled");
	return true;
}

/* Restores LibXL's relocations before this code can be unloaded. When a
 * slot cannot be written back, LibXL still calls into this file, so the
 * state it needs is kept. pthread_key_delete() runs no destructor, so the
 * per-thread state of threads other than the caller is leaked; nothing
 * refers to it once the relocations are restored. */
static inline void php_excel_locale_isolation_uninstall(void)
{
	php_excel_thread_locale *tl;
	php_excel_locale_name *entry;
	bool restored = true;
	size_t i;

	if (!php_excel_locale_isolation.active) {
		return;
	}
	for (i = 0; i < php_excel_locale_isolation.nslots; i++) {
		/* -2 leaves a writable page behind but the original binding is back. */
		if (php_excel_locale_write_slot(php_excel_locale_isolation.slots[i], php_excel_locale_isolation.saved[i],
				php_excel_locale_isolation.prot[i]) == -1) {
			restored = false;
		}
	}
	if (!restored) {
		snprintf(php_excel_locale_isolation.status, sizeof(php_excel_locale_isolation.status),
			"enabled (relocations could not be restored)");
		return;
	}
	php_excel_locale_isolation.nslots = 0;
	tl = pthread_getspecific(php_excel_locale_isolation.key);
	if (tl) {
		pthread_setspecific(php_excel_locale_isolation.key, NULL);
		php_excel_thread_locale_dtor(tl);
	}
	pthread_key_delete(php_excel_locale_isolation.key);
	php_excel_locale_isolation.active = false;
	php_excel_locale_isolation.localeconv = false;
	freelocale(php_excel_locale_isolation.c_locale);
	php_excel_locale_isolation.c_locale = (locale_t) 0;
	while ((entry = php_excel_locale_isolation.names)) {
		php_excel_locale_isolation.names = entry->next;
		free(entry);
	}
	snprintf(php_excel_locale_isolation.status, sizeof(php_excel_locale_isolation.status), "disabled (not installed)");
}

static inline const char *php_excel_locale_isolation_status(void)
{
	return php_excel_locale_isolation.status;
}

#else

#include <stdlib.h>

static inline bool php_excel_locale_isolation_install(const void *libxl_symbol)
{
	(void) libxl_symbol;
	return false;
}

static inline void php_excel_locale_isolation_uninstall(void)
{
}

static inline void php_excel_locale_thread_reset(void)
{
}

static inline void php_excel_locale_book_created(void)
{
}

static inline void php_excel_locale_book_released(void)
{
}

static inline char *php_excel_setlocale(int category, const char *value)
{
	return setlocale(category, value);
}

static inline size_t php_excel_locale_mbstowcs(wchar_t *dst, const char *src, size_t n)
{
	return mbstowcs(dst, src, n);
}

#ifdef HAVE_NL_LANGINFO
# include <langinfo.h>
static inline const char *php_excel_locale_codeset(void)
{
	return nl_langinfo(CODESET);
}
#endif

static inline const char *php_excel_locale_isolation_status(void)
{
	return PHP_EXCEL_LOCALE_ISOLATION_OFF;
}

#endif

#endif	/* PHP_EXCEL_LIBXL_LOCALE_H */
