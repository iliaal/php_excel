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

  dnl Check minimum PHP version (8.3.0 = 80300)
  PHP_VERSION_ID=$($PHP_CONFIG --vernum)
  if test "$PHP_VERSION_ID" -lt "80300"; then
    AC_MSG_ERROR([php_excel requires PHP 8.3.0 or later (found $PHP_VERSION_ID)])
  fi

  SEARCH_PATH="/usr/local /usr"
  SEARCH_FOR="libxl.h"

  AC_MSG_CHECKING([for excel includes])
  if test -r "$PHP_LIBXL_INCDIR/include/$SEARCH_FOR"; then
    EXCEL_INCDIR=$PHP_LIBXL_INCDIR/include
  elif test -r "$PHP_LIBXL_INCDIR/$SEARCH_FOR"; then
    EXCEL_INCDIR=$PHP_LIBXL_INCDIR
  else # search default path list
    for i in $SEARCH_PATH ; do
      if test -r "$i/include/$SEARCH_FOR"; then
        EXCEL_INCDIR=$i/include
      fi
    done
  fi

  if test -z "$EXCEL_INCDIR"; then
    AC_MSG_RESULT([not found])
    AC_MSG_ERROR([Please reinstall the excel distribution])
  else
    AC_MSG_RESULT(found in $EXCEL_INCDIR)
  fi

  SEARCH_FOR="libxl.${SHLIB_SUFFIX_NAME:-so}"

  AC_MSG_CHECKING([for excel libraries])
  if test -r "$PHP_LIBXL_LIBDIR/$PHP_LIBDIR/$SEARCH_FOR"; then
    EXCEL_LIBDIR=$PHP_LIBXL_LIBDIR/$PHP_LIBDIR
  elif test -r "$PHP_LIBXL_LIBDIR/$SEARCH_FOR"; then
    EXCEL_LIBDIR=$PHP_LIBXL_LIBDIR
  elif test -z "$EXCEL_LIBDIR"; then
    for i in $SEARCH_PATH ; do
      if test -r "$i/$PHP_LIBDIR/$SEARCH_FOR"; then
        EXCEL_LIBDIR=$i/$PHP_LIBDIR
      fi
    done
  fi

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

  dnl Developer build flags
  if test "$PHP_EXCEL_DEV" = "yes"; then
    EXCEL_DEV_CFLAGS="-Wall -Wextra -Wno-unused-parameter -Wimplicit-fallthrough -Werror"
    PHP_EXCEL_CFLAGS="$PHP_EXCEL_CFLAGS $EXCEL_DEV_CFLAGS"
    CFLAGS="$CFLAGS $EXCEL_DEV_CFLAGS"
  fi

  PHP_SUBST(EXCEL_SHARED_LIBADD)
  PHP_NEW_EXTENSION(excel, excel.c, $ext_shared,, $PHP_EXCEL_CFLAGS)
fi
