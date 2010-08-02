dnl $Id$
dnl config.m4 for extension excel

PHP_ARG_WITH(excel, for excel support,
[  --with-excel[=DIR]             Include excel support])

if test "$PHP_EXCEL" != "no"; then
  SEARCH_PATH="/usr/local /usr"
  SEARCH_FOR="/include/libxl.h"

  if test -r $PHP_EXCEL/$SEARCH_FOR; then
  	EXCEL_DIR=$PHP_EXCEL
  else # search default path list
  	AC_MSG_CHECKING([for excel files in default path])
	for i in $SEARCH_PATH ; do
		if test -r $i/$SEARCH_FOR; then
			EXCEL_DIR=$i
			AC_MSG_RESULT(found in $i)
		fi
	done
  fi

  if test -z "$EXCEL_DIR"; then
	AC_MSG_RESULT([not found])
	AC_MSG_ERROR([Please reinstall the excel distribution])
  fi

  PHP_CHECK_LIBRARY(xl, xlCreateBookCA,
  [
    PHP_ADD_INCLUDE($EXCEL_DIR/include)
    PHP_ADD_LIBRARY_WITH_PATH(xl, $EXCEL_DIR/$PHP_LIBDIR, EXCEL_SHARED_LIBADD)
    AC_DEFINE(HAVE_EXCELLIB,1,[ ])
  ], [
    AC_MSG_ERROR(excel module requires libxl >= 2.4.3)
  ], [
    -L$EXCEL_DIR/$PHP_LIBDIR
  ])

  AC_MSG_CHECKING(for LibXL version)
  if grep -c FILLPATTERN_HORSTRIPE $EXCEL_DIR/include/enum.h 2>&1 >/dev/null; then
	AC_DEFINE([HAVE_LIBXL_243_PLUS], [1], [Have LibXL 2.4.3+])
  fi

  PHP_SUBST(EXCEL_SHARED_LIBADD)
  PHP_NEW_EXTENSION(excel, excel.c, $ext_shared)
fi
