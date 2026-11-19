/* Loads a stand-in for libxl.so and installs the isolation against it.
 * Exit 0 when the status matches the expected prefix: the install must be
 * refused when the library imports a locale-dependent function that has no
 * wrapper, and must succeed otherwise. */

#include "libxl_locale.h"
#include <dlfcn.h>
#include <stdio.h>
#include <string.h>

int main(int argc, char **argv)
{
	void *lib, *sym;
	char status[sizeof(php_excel_locale_isolation.status)];

	if (argc != 3) {
		fprintf(stderr, "usage: %s library expected-status-prefix\n", argv[0]);
		return 2;
	}
	lib = dlopen(argv[1], RTLD_NOW | RTLD_LOCAL);
	if (!lib) {
		fprintf(stderr, "%s\n", dlerror());
		return 2;
	}
	sym = dlsym(lib, "fake_libxl_setlocale");
	if (!sym) {
		fprintf(stderr, "%s\n", dlerror());
		return 2;
	}
	php_excel_locale_isolation_install(sym);
	snprintf(status, sizeof(status), "%s", php_excel_locale_isolation_status());
	php_excel_locale_isolation_uninstall();
	if (strncmp(status, argv[2], strlen(argv[2])) != 0) {
		printf("FAIL: %s: status \"%s\", expected \"%s...\"\n", argv[1], status, argv[2]);
		return 1;
	}
	printf("PASS: %s: %s\n", argv[1], status);
	return 0;
}
