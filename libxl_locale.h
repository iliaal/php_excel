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
 * process. On glibc the LibXL relocations for setlocale() and localeconv()
 * are redirected to an emulation built on uselocale(), so those changes stay
 * on the calling thread. The file uses no Zend API so a standalone harness
 * can include it. */

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
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>

#define PHP_EXCEL_LC_COUNT (LC_IDENTIFICATION + 1)
#define PHP_EXCEL_LOCALE_MAX_SLOTS 8

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

/* names[c] is set only while category c differs from the global locale; loc
 * is the uselocale() object for this thread, or 0 while it follows the
 * global locale. */
typedef struct {
	locale_t loc;
	const char *names[PHP_EXCEL_LC_COUNT];
	struct lconv conv;
} php_excel_thread_locale;

static struct {
	bool active;
	pthread_key_t key;
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

/* LibXL keeps the pointer setlocale(cat, NULL) returned and passes it back
 * later, so every name handed out lives until the isolation is removed. The
 * set of distinct locale names a process meets is small. Interned names
 * compare by pointer. */
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

static inline void php_excel_thread_locale_clear(php_excel_thread_locale *tl)
{
	if (tl->loc) {
		uselocale(LC_GLOBAL_LOCALE);
		freelocale(tl->loc);
		tl->loc = (locale_t) 0;
	}
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
	}
	return tl;
}

static inline const char *php_excel_thread_locale_name(const php_excel_thread_locale *tl, int category)
{
	const char *global;

	if (tl->names[category]) {
		return tl->names[category];
	}
	global = setlocale(category, NULL);
	return global ? php_excel_locale_intern(global) : NULL;
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
		if (!names[c]) {
			return NULL;
		}
		if (!first) {
			first = names[c];
		} else if (names[c] != first) {
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

/* setlocale() for the calling thread only. Categories that match the global
 * locale are not pinned, so the thread keeps following the global locale for
 * them, and a thread with nothing pinned goes back to LC_GLOBAL_LOCALE. */
static inline char *php_excel_thread_setlocale(int category, const char *value)
{
	php_excel_thread_locale *tl;
	const char *names[PHP_EXCEL_LC_COUNT];
	locale_t loc, next;
	bool pinned = false;
	int mask, c, err;

	if (category < 0 || category >= PHP_EXCEL_LC_COUNT) {
		errno = EINVAL;
		return NULL;
	}
	tl = php_excel_thread_locale_get();
	if (!tl) {
		/* Without per-thread state the process-wide behavior is the only
		 * one left; LibXL cannot handle a NULL from a query. */
		return setlocale(category, value);
	}
	if (!value) {
		return (char *) (category == LC_ALL ? php_excel_thread_locale_all_name(tl) : php_excel_thread_locale_name(tl, category));
	}

	mask = category == LC_ALL ? LC_ALL_MASK : 1 << category;
	loc = duplocale(LC_GLOBAL_LOCALE);
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
		if (c == LC_ALL || !(mask & (1 << c))) {
			continue;
		}
		names[c] = php_excel_locale_intern(nl_langinfo_l(_NL_LOCALE_NAME(c), loc));
		if (!names[c]) {
			freelocale(loc);
			errno = ENOMEM;
			return NULL;
		}
	}
	for (c = 0; c < PHP_EXCEL_LC_COUNT; c++) {
		const char *global;

		if (!names[c]) {
			continue;
		}
		global = setlocale(c, NULL);
		if (global && php_excel_locale_intern(global) == names[c]) {
			names[c] = NULL;
		} else {
			pinned = true;
		}
	}

	if (pinned) {
		uselocale(loc);
	} else {
		uselocale(LC_GLOBAL_LOCALE);
		freelocale(loc);
		loc = (locale_t) 0;
	}
	if (tl->loc) {
		freelocale(tl->loc);
	}
	tl->loc = loc;
	memcpy(tl->names, names, sizeof(names));

	return (char *) (category == LC_ALL ? php_excel_thread_locale_all_name(tl) : php_excel_thread_locale_name(tl, category));
}

/* What LibXL's setlocale() relocation points at once installed. */
static char *php_excel_libxl_setlocale(int category, const char *value)
{
	return php_excel_thread_setlocale(category, value);
}

static inline char php_excel_langinfo_char(nl_item item)
{
	return *nl_langinfo(item);
}

static inline char *php_excel_langinfo_grouping(nl_item item)
{
	char *grouping = nl_langinfo(item);

	return *grouping == '\177' || *grouping == '\377' ? (char *) "" : grouping;
}

/* glibc's localeconv() fills one static struct for the whole process, so a
 * LibXL call running under LC_NUMERIC "C" would publish "." to every thread
 * that reads it. This builds the same fields from the calling thread's
 * locale into per-thread storage instead. */
static struct lconv *php_excel_libxl_localeconv(void)
{
	php_excel_thread_locale *tl = php_excel_thread_locale_get();
	struct lconv *conv;

	if (!tl) {
		return localeconv();
	}
	conv = &tl->conv;
	conv->decimal_point = nl_langinfo(RADIXCHAR);
	conv->thousands_sep = nl_langinfo(THOUSEP);
	conv->grouping = php_excel_langinfo_grouping(__GROUPING);
	conv->int_curr_symbol = nl_langinfo(__INT_CURR_SYMBOL);
	conv->currency_symbol = nl_langinfo(__CURRENCY_SYMBOL);
	conv->mon_decimal_point = nl_langinfo(__MON_DECIMAL_POINT);
	conv->mon_thousands_sep = nl_langinfo(__MON_THOUSANDS_SEP);
	conv->mon_grouping = php_excel_langinfo_grouping(__MON_GROUPING);
	conv->positive_sign = nl_langinfo(__POSITIVE_SIGN);
	conv->negative_sign = nl_langinfo(__NEGATIVE_SIGN);
	conv->int_frac_digits = php_excel_langinfo_char(__INT_FRAC_DIGITS);
	conv->frac_digits = php_excel_langinfo_char(__FRAC_DIGITS);
	conv->p_cs_precedes = php_excel_langinfo_char(__P_CS_PRECEDES);
	conv->p_sep_by_space = php_excel_langinfo_char(__P_SEP_BY_SPACE);
	conv->n_cs_precedes = php_excel_langinfo_char(__N_CS_PRECEDES);
	conv->n_sep_by_space = php_excel_langinfo_char(__N_SEP_BY_SPACE);
	conv->p_sign_posn = php_excel_langinfo_char(__P_SIGN_POSN);
	conv->n_sign_posn = php_excel_langinfo_char(__N_SIGN_POSN);
	conv->int_p_cs_precedes = php_excel_langinfo_char(__INT_P_CS_PRECEDES);
	conv->int_p_sep_by_space = php_excel_langinfo_char(__INT_P_SEP_BY_SPACE);
	conv->int_n_cs_precedes = php_excel_langinfo_char(__INT_N_CS_PRECEDES);
	conv->int_n_sep_by_space = php_excel_langinfo_char(__INT_N_SEP_BY_SPACE);
	conv->int_p_sign_posn = php_excel_langinfo_char(__INT_P_SIGN_POSN);
	conv->int_n_sign_posn = php_excel_langinfo_char(__INT_N_SIGN_POSN);
	return conv;
}

static inline char *php_excel_setlocale(int category, const char *value)
{
	if (php_excel_locale_isolation.active) {
		return php_excel_thread_setlocale(category, value);
	}
	return setlocale(category, value);
}

/* Drops whatever the calling thread pinned, for request boundaries: a book
 * released after its request has changed the global locale would otherwise
 * leave the worker thread pinned for every later request. */
static inline void php_excel_locale_thread_reset(void)
{
	php_excel_thread_locale *tl;

	if (!php_excel_locale_isolation.active) {
		return;
	}
	tl = pthread_getspecific(php_excel_locale_isolation.key);
	if (tl) {
		php_excel_thread_locale_clear(tl);
	}
}

typedef struct {
	uintptr_t target;
	bool found;
	bool overflow;
	size_t nslots;
	void **slots[PHP_EXCEL_LOCALE_MAX_SLOTS];
	void *wrappers[PHP_EXCEL_LOCALE_MAX_SLOTS];
	int prot[PHP_EXCEL_LOCALE_MAX_SLOTS];
} php_excel_locale_scan;

static const struct {
	const char *name;
	void *wrapper;
} php_excel_locale_wrappers[] = {
	{ "setlocale", (void *) php_excel_libxl_setlocale },
	{ "localeconv", (void *) php_excel_libxl_localeconv },
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

static inline bool php_excel_locale_write_slot(void **slot, void *value, int prot)
{
	uintptr_t pagesize = (uintptr_t) sysconf(_SC_PAGESIZE);
	void *page = (void *) ((uintptr_t) slot & ~(pagesize - 1));

	if (!(prot & PROT_WRITE) && mprotect(page, pagesize, prot | PROT_WRITE) != 0) {
		return false;
	}
	__atomic_store_n(slot, value, __ATOMIC_SEQ_CST);
	if (!(prot & PROT_WRITE)) {
		mprotect(page, pagesize, prot);
	}
	return true;
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
	dl_iterate_phdr(php_excel_locale_scan_object, &scan);
	if (!scan.found) {
		snprintf(php_excel_locale_isolation.status, sizeof(php_excel_locale_isolation.status),
			"disabled (LibXL image not found)");
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
				"disabled (setlocale relocation outside LibXL segments)");
			return false;
		}
	}
	if (pthread_key_create(&php_excel_locale_isolation.key, php_excel_thread_locale_dtor) != 0) {
		snprintf(php_excel_locale_isolation.status, sizeof(php_excel_locale_isolation.status),
			"disabled (pthread_key_create failed)");
		return false;
	}
	php_excel_locale_isolation.active = true;
	for (i = 0; i < scan.nslots; i++) {
		php_excel_locale_isolation.slots[i] = scan.slots[i];
		php_excel_locale_isolation.prot[i] = scan.prot[i];
		php_excel_locale_isolation.saved[i] = __atomic_load_n(scan.slots[i], __ATOMIC_SEQ_CST);
		if (!php_excel_locale_write_slot(scan.slots[i], scan.wrappers[i], scan.prot[i])) {
			int err = errno;

			while (i-- > 0) {
				php_excel_locale_write_slot(scan.slots[i], php_excel_locale_isolation.saved[i], scan.prot[i]);
			}
			pthread_key_delete(php_excel_locale_isolation.key);
			php_excel_locale_isolation.active = false;
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

/* Restores LibXL's relocations before this code can be unloaded. Threads
 * other than the caller that still pin a locale keep it until they exit. */
static inline void php_excel_locale_isolation_uninstall(void)
{
	php_excel_thread_locale *tl;
	php_excel_locale_name *entry;
	size_t i;

	if (!php_excel_locale_isolation.active) {
		return;
	}
	for (i = 0; i < php_excel_locale_isolation.nslots; i++) {
		php_excel_locale_write_slot(php_excel_locale_isolation.slots[i], php_excel_locale_isolation.saved[i],
			php_excel_locale_isolation.prot[i]);
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

static inline char *php_excel_setlocale(int category, const char *value)
{
	return setlocale(category, value);
}

static inline const char *php_excel_locale_isolation_status(void)
{
	return PHP_EXCEL_LOCALE_ISOLATION_OFF;
}

#endif

#endif	/* PHP_EXCEL_LIBXL_LOCALE_H */
