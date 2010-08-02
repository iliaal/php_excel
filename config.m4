dnl $Id$
dnl config.m4 for extension excel

PHP_ARG_WITH(excel, for excel support,
[  --with-excel             Include excel support])

if test "$PHP_EXCEL" != "no"; then
  SEARCH_PATH="/usr/local /usr"     # you might want to change this
  SEARCH_FOR="/include/libxl.h"  # you most likely want to change this
  if test -r $PHP_EXCEL/$SEARCH_FOR; then # path given as parameter
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

  PHP_ADD_INCLUDE($EXCEL_DIR/include)

  AC_MSG_CHECKING(for LibXL version)
  if grep -c FILLPATTERN_HORSTRIPE $EXCEL_DIR/include/enum.h 2>&1 >/dev/null; then
	AC_DEFINE([HAVE_LIBXL_243_PLUS], [1], [Have LibXL 2.4.3+])
  fi

  AC_MSG_RESULT([$EXCEL_DIR/lib])
  PHP_ADD_LIBRARY_WITH_PATH(xl, $EXCEL_DIR/lib, XL_SHARED_LIBADD)
  PHP_ADD_INCLUDE($PHP_ZLIB_INCDIR)

  PHP_NEW_EXTENSION(excel, excel.c, $ext_shared)
fi
