/* Two threads create, save, load and release workbooks while a third reads
 * the locale. "unpatched" shows LibXL's setlocale() calls reaching the
 * observer; "patched" installs the isolation from libxl_locale.h first and
 * expects the observer to see none of them. Exit 0 means the mode behaved
 * as expected. Run it through tests/threads/locale_race.sh. */

#include <libxl.h>
#include "libxl_locale.h"
#include <langinfo.h>
#include <pthread.h>
#include <stdatomic.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define ITERATIONS 200

static atomic_int stop;
static atomic_long bad_value, saver_numeric_flips, saver_errors;
static atomic_long observed[5];
static atomic_long samples;
static const char *tmpdir;

static void *saver(void *arg)
{
	char path[4096];

	snprintf(path, sizeof path, "%s/php_excel_locale_race_%d_%d.xlsx", tmpdir, (int) getpid(), (int) (long) arg);
	for (int i = 0; i < ITERATIONS; i++) {
		BookHandle b = xlCreateXMLBook();
		SheetHandle s = xlBookAddSheetA(b, "S", 0);
		xlSheetWriteNumA(s, 2, 1, 1.5, 0);
		if (!xlBookSaveA(b, path)) {
			atomic_fetch_add(&saver_errors, 1);
		}
		xlBookRelease(b);
		BookHandle r = xlCreateXMLBook();
		if (!xlBookLoadA(r, path)) {
			atomic_fetch_add(&saver_errors, 1);
		} else if (xlSheetReadNumA(xlBookGetSheet(r, 0), 2, 1, 0) != 1.5) {
			atomic_fetch_add(&bad_value, 1);
		}
		xlBookRelease(r);
		if (localeconv()->decimal_point[0] != ',') {
			atomic_fetch_add(&saver_numeric_flips, 1);
		}
	}
	unlink(path);
	return NULL;
}

static void *observer(void *arg)
{
	long n = 0, numeric = 0, radix = 0, ctype_name = 0, ctype_codeset = 0, final_numeric = 0;

	(void) arg;
	while (!atomic_load(&stop)) {
		if (localeconv()->decimal_point[0] != ',') {
			numeric++;
		}
		if (nl_langinfo(RADIXCHAR)[0] != ',') {
			radix++;
		}
		const char *ct = setlocale(LC_CTYPE, NULL);
		if (!ct || strcmp(ct, "C") != 0) {
			ctype_name++;
		}
		if (strcmp(nl_langinfo(CODESET), "ANSI_X3.4-1968") != 0) {
			ctype_codeset++;
		}
		n++;
	}
	const char *num = setlocale(LC_NUMERIC, NULL);
	if (!num || strcmp(num, "de_DE.utf8") != 0) {
		final_numeric = 1;
	}
	atomic_store(&samples, n);
	atomic_store(&observed[0], numeric);
	atomic_store(&observed[1], radix);
	atomic_store(&observed[2], ctype_name);
	atomic_store(&observed[3], ctype_codeset);
	atomic_store(&observed[4], final_numeric);
	return NULL;
}

int main(int argc, char **argv)
{
	pthread_t s1, s2, o;
	int patched;
	long changed = 0;

	if (argc != 2 || (strcmp(argv[1], "patched") != 0 && strcmp(argv[1], "unpatched") != 0)) {
		fprintf(stderr, "usage: %s patched|unpatched\n", argv[0]);
		return 2;
	}
	patched = strcmp(argv[1], "patched") == 0;
	tmpdir = getenv("TMPDIR");
	if (!tmpdir || !*tmpdir) {
		tmpdir = "/tmp";
	}

	/* LibXL sets LC_CTYPE "" (the environment, C.UTF-8 under the wrapper
	 * script) and LC_NUMERIC "C"; both differ from what is set here. */
	setlocale(LC_ALL, "C");
	if (!setlocale(LC_NUMERIC, "de_DE.utf8")) {
		puts("skip: de_DE.utf8 locale unavailable");
		return 77;
	}
	if (patched) {
		php_excel_locale_isolation_install((const void *) xlCreateBookCA);
		if (strcmp(php_excel_locale_isolation_status(), "enabled") != 0) {
			printf("FAIL: isolation %s\n", php_excel_locale_isolation_status());
			return 1;
		}
	}

	pthread_create(&o, NULL, observer, NULL);
	pthread_create(&s1, NULL, saver, (void *) 1L);
	pthread_create(&s2, NULL, saver, (void *) 2L);
	pthread_join(s1, NULL);
	pthread_join(s2, NULL);
	atomic_store(&stop, 1);
	pthread_join(o, NULL);

	for (int i = 0; i < 5; i++) {
		changed += atomic_load(&observed[i]);
	}
	printf("%s: observer samples=%ld localeconv_changed=%ld radixchar_changed=%ld lc_ctype_name_changed=%ld lc_ctype_codeset_changed=%ld final_lc_numeric_changed=%ld\n",
		argv[1], atomic_load(&samples), atomic_load(&observed[0]), atomic_load(&observed[1]),
		atomic_load(&observed[2]), atomic_load(&observed[3]), atomic_load(&observed[4]));
	printf("%s: savers bad_value=%ld numeric_after_release_changed=%ld errors=%ld\n",
		argv[1], atomic_load(&bad_value), atomic_load(&saver_numeric_flips), atomic_load(&saver_errors));
	if (patched) {
		php_excel_locale_isolation_uninstall();
	}

	if (atomic_load(&saver_errors) || atomic_load(&bad_value)) {
		puts("FAIL: LibXL save or load failed");
		return 1;
	}
	if (patched) {
		if (changed || atomic_load(&saver_numeric_flips)) {
			puts("FAIL: LibXL locale changes reached other threads");
			return 1;
		}
		puts("PASS: no LibXL locale change seen outside the calling thread");
		return 0;
	}
	if (!changed) {
		puts("FAIL: unpatched run saw no locale change, so the harness does not discriminate");
		return 1;
	}
	puts("PASS: unpatched run reproduced process-wide LibXL locale changes");
	return 0;
}
