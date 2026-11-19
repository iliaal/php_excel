/* Stand-in for libxl.so in locale_unwrapped.c: it calls setlocale() like
 * LibXL, and strftime(), which has no wrapper, when built with
 * -DFAKE_UNWRAPPED. */

#include <locale.h>
#include <stddef.h>
#include <time.h>

char *fake_libxl_setlocale(int category, const char *value)
{
	return setlocale(category, value);
}

#ifdef FAKE_UNWRAPPED
size_t fake_libxl_strftime(char *buf, size_t len)
{
	time_t t = 0;

	return strftime(buf, len, "%c", gmtime(&t));
}
#endif
