dnl config.m4 for extension excel

PHP_ARG_WITH(excel, whether to enable excel support,
[  --with-excel          Enable excel support])

PHP_ARG_WITH(libxl-incdir, C include dir for libxl,
[  --with-libxl-incdir[=DIR] Include path for the C headers of libxl])

PHP_ARG_WITH(libxl-libdir, lib dir for libxl,
[  --with-libxl-libdir[=DIR] Library path for libxl])

PHP_ARG_ENABLE(excel-dev, whether to enable developer build flags,
[  --enable-excel-dev    Enable developer build flags (warnings, sanitizers)], no, no)

if test "$PHP_EXCEL" != "no"; then

  dnl Check minimum PHP version (8.1.0 = 80100)
  PHP_VERSION_ID=$($PHP_CONFIG --vernum)
  if test "$PHP_VERSION_ID" -lt "80100"; then
    AC_MSG_ERROR([php_excel requires PHP 8.1.0 or later (found $PHP_VERSION_ID)])
  fi

  SEARCH_PATH="/usr/local /usr"
  SEARCH_FOR="libxl.h"

  EXCEL_INCDIR=""
  EXCEL_LIBDIR=""

  AC_MSG_CHECKING([for excel includes])
  if test "$PHP_LIBXL_INCDIR" != "no" && test -r "$PHP_LIBXL_INCDIR/include/$SEARCH_FOR"; then
    EXCEL_INCDIR=$PHP_LIBXL_INCDIR/include
  elif test "$PHP_LIBXL_INCDIR" != "no" && test -r "$PHP_LIBXL_INCDIR/$SEARCH_FOR"; then
    EXCEL_INCDIR=$PHP_LIBXL_INCDIR
  fi

  SEARCH_FOR="libxl.${SHLIB_SUFFIX_NAME:-so}"
  if test "$PHP_LIBXL_LIBDIR" != "no" && test -r "$PHP_LIBXL_LIBDIR/$PHP_LIBDIR/$SEARCH_FOR"; then
    EXCEL_LIBDIR=$PHP_LIBXL_LIBDIR/$PHP_LIBDIR
  elif test "$PHP_LIBXL_LIBDIR" != "no" && test -r "$PHP_LIBXL_LIBDIR/$SEARCH_FOR"; then
    EXCEL_LIBDIR=$PHP_LIBXL_LIBDIR
  fi

  dnl When neither path is explicit, select headers and library from one prefix.
  dnl Feature-gated link probes below also reject mixed explicit versions.
  if test -z "$EXCEL_INCDIR" && test -z "$EXCEL_LIBDIR"; then
    for i in $SEARCH_PATH ; do
      candidate_incdir=""
      candidate_libdir=""
      if test -r "$i/include/libxl.h"; then
        candidate_incdir=$i/include
      elif test -r "$i/include_c/libxl.h"; then
        candidate_incdir=$i/include_c
      fi
      if test -r "$i/$PHP_LIBDIR/$SEARCH_FOR"; then
        candidate_libdir=$i/$PHP_LIBDIR
      elif test -r "$i/lib64/$SEARCH_FOR"; then
        candidate_libdir=$i/lib64
      fi
      if test -n "$candidate_incdir" && test -n "$candidate_libdir"; then
        EXCEL_INCDIR=$candidate_incdir
        EXCEL_LIBDIR=$candidate_libdir
        break
      fi
    done
  else
    if test -z "$EXCEL_INCDIR"; then
      for i in $SEARCH_PATH ; do
        if test -r "$i/include/libxl.h"; then
          EXCEL_INCDIR=$i/include
          break
        elif test -r "$i/include_c/libxl.h"; then
          EXCEL_INCDIR=$i/include_c
          break
        fi
      done
    fi
    if test -z "$EXCEL_LIBDIR"; then
      for i in $SEARCH_PATH ; do
        if test -r "$i/$PHP_LIBDIR/$SEARCH_FOR"; then
          EXCEL_LIBDIR=$i/$PHP_LIBDIR
          break
        elif test -r "$i/lib64/$SEARCH_FOR"; then
          EXCEL_LIBDIR=$i/lib64
          break
        fi
      done
    fi
  fi

  if test -z "$EXCEL_INCDIR"; then
    AC_MSG_RESULT([not found])
    AC_MSG_ERROR([Please reinstall the excel distribution])
  else
    AC_MSG_RESULT(found in $EXCEL_INCDIR)
  fi

  AC_MSG_CHECKING([for excel libraries])
  if test -z "$EXCEL_LIBDIR"; then
    AC_MSG_RESULT([not found])
    AC_MSG_ERROR([Please reinstall the excel distribution])
  else
    AC_MSG_RESULT(found in $EXCEL_LIBDIR)
  fi

  PHP_CHECK_LIBRARY(xl, xlCreateBookCA,
  [
    PHP_ADD_INCLUDE($EXCEL_INCDIR)
    PHP_ADD_LIBRARY_WITH_PATH(xl, $EXCEL_LIBDIR, EXCEL_SHARED_LIBADD)
    AC_DEFINE(HAVE_EXCELLIB, 1, [ ])
  ], [
    AC_MSG_ERROR([excel module requires libxl >= 4.6.0])
  ], [
    -L$EXCEL_LIBDIR
  ])

  dnl Feature probe only: the include path and -lxl link flag are already
  dnl added by the xlCreateBookCA check above. Re-adding them here would
  dnl duplicate -I/-L/-lxl/-rpath in EXCEL_SHARED_LIBADD.
  PHP_CHECK_LIBRARY(xl, xlBookSetKeyA,
  [
    AC_DEFINE(HAVE_LIBXL_SETKEY,1,[ ])
  ],[],[
    -L$EXCEL_LIBDIR
  ])

  save_CPPFLAGS="$CPPFLAGS"
  CPPFLAGS="$CPPFLAGS -I$EXCEL_INCDIR"
  AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[
#include <libxl.h>
#if LIBXL_VERSION < 0x04060000
# error libxl headers are too old
#endif
  ]], [[]])], [], [AC_MSG_ERROR([excel module requires libxl headers >= 4.6.0])])

  AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[
#include <libxl.h>
#if LIBXL_VERSION < 0x05000000
# error older header
#endif
  ]], [[]])], [libxl_header_ge_50000=yes], [libxl_header_ge_50000=no])
  AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[
#include <libxl.h>
#if LIBXL_VERSION < 0x05000100
# error older header
#endif
  ]], [[]])], [libxl_header_ge_50001=yes], [libxl_header_ge_50001=no])
  AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[
#include <libxl.h>
#if LIBXL_VERSION < 0x05010000
# error older header
#endif
  ]], [[]])], [libxl_header_ge_50100=yes], [libxl_header_ge_50100=no])
  AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[
#include <libxl.h>
#if LIBXL_VERSION < 0x05020000
# error older header
#endif
  ]], [[]])], [libxl_header_ge_50200=yes], [libxl_header_ge_50200=no])

  AC_MSG_CHECKING([whether xlAutoFilterGetSort takes a sort level])
  AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[
#include <libxl.h>
  ]], [[
    int column_index, descending;
    xlAutoFilterGetSort((AutoFilterHandle) 0, &column_index, &descending, 0);
  ]])], [
    AC_MSG_RESULT([yes])
    AC_DEFINE(HAVE_LIBXL_AUTOFILTER_GETSORT_LEVEL, 1, [Define if xlAutoFilterGetSort takes a sort level])
  ], [
    AC_MSG_RESULT([no])
  ])
  CPPFLAGS="$save_CPPFLAGS"

  if test "$libxl_header_ge_50000" = "yes"; then
    PHP_CHECK_LIBRARY(xl, xlBookLoadRawPartiallyA, [], [
      AC_MSG_ERROR([libxl headers require 5.0.0 features missing from $EXCEL_LIBDIR/libxl])
    ], [-L$EXCEL_LIBDIR])
  fi
  if test "$libxl_header_ge_50001" = "yes"; then
    PHP_CHECK_LIBRARY(xl, xlBookLoadInfoRawA, [], [
      AC_MSG_ERROR([libxl headers require 5.0.1 features missing from $EXCEL_LIBDIR/libxl])
    ], [-L$EXCEL_LIBDIR])
  fi
  if test "$libxl_header_ge_50100" = "yes"; then
    PHP_CHECK_LIBRARY(xl, xlBookConditionalFormatSizeA, [], [
      AC_MSG_ERROR([libxl headers require 5.1.0 features missing from $EXCEL_LIBDIR/libxl])
    ], [-L$EXCEL_LIBDIR])
  fi
  if test "$libxl_header_ge_50200" = "yes"; then
    PHP_CHECK_LIBRARY(xl, xlSheetDataValidationSizeA, [], [
      AC_MSG_ERROR([libxl headers require 5.2.0 features missing from $EXCEL_LIBDIR/libxl])
    ], [-L$EXCEL_LIBDIR])
  fi

  dnl Developer build flags
  if test "$PHP_EXCEL_DEV" = "yes"; then
    EXCEL_DEV_CFLAGS="-Wall -Wextra -Wno-unused-parameter -Wimplicit-fallthrough -Werror"
    PHP_EXCEL_CFLAGS="$PHP_EXCEL_CFLAGS $EXCEL_DEV_CFLAGS"
    CFLAGS="$CFLAGS $EXCEL_DEV_CFLAGS"
  fi

  PHP_SUBST(EXCEL_SHARED_LIBADD)
  PHP_NEW_EXTENSION(excel, excel.c, $ext_shared,, $PHP_EXCEL_CFLAGS)
fi
