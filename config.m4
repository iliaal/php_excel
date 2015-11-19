dnl $Id$
dnl config.m4 for extension excel

PHP_ARG_WITH(excel, whether to enable excel support,
[  --with-excel          Enable excel support])

PHP_ARG_WITH(libxl-incdir, C include dir for libxl,
[  --with-libxl-incdir[=DIR] Include path for the C headers of libxl])

PHP_ARG_WITH(libxl-libdir, lib dir for libxl,
[  --with-libxl-libdir[=DIR] Library path for libxl])

PHP_ARG_WITH(libxml-dir, libxml2 install dir,
[  --with-libxml-dir=DIR XML: libxml2 install prefix])

if test "$PHP_EXCEL" != "no"; then
  SEARCH_PATH="/usr/local /usr"
  SEARCH_FOR="libxl.h"

  AC_MSG_CHECKING([for excel includes])
  if test -r $PHP_LIBXL_INCDIR/include/$SEARCH_FOR; then
    EXCEL_INCDIR=$PHP_LIBXL_INCDIR/include
  elif test -r $PHP_LIBXL_INCDIR/$SEARCH_FOR; then
    EXCEL_INCDIR=$PHP_LIBXL_INCDIR
  else # search default path list
    for i in $SEARCH_PATH ; do
      if test -r $i/include/$SEARCH_FOR; then
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

  SEARCH_FOR="libxl.${SHLIB_SUFFIX_NAME}"

  AC_MSG_CHECKING([for excel libraries])
  if test -r $PHP_LIBXL_LIBDIR/$PHP_LIBDIR/$SEARCH_FOR; then
    EXCEL_LIBDIR=$PHP_LIBXL_LIBDIR/$PHP_LIBDIR
  elif test -r $PHP_LIBXL_LIBDIR/$SEARCH_FOR; then
    EXCEL_LIBDIR=$PHP_LIBXL_LIBDIR
  elif test -z "$EXCEL_LIBDIR"; then
    for i in $SEARCH_PATH ; do
      if test -r $i/$PHP_LIBDIR/$SEARCH_FOR; then
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
 
  AC_MSG_CHECKING([for libxml2])
  if test -r $PHP_LIBXML_DIR/libxml; then
    LIBXML_INCDIR=$PHP_LIBXML_DIR
  fi

  if test -z "$LIBXML_INCDIR"; then
    AC_MSG_RESULT([not found])
    if test $PHP_ENABLE_LIBXML != "no"; then
      AC_DEFINE(EXCEL_WITH_LIBXML, 1, [ ])
    fi
  else
    AC_DEFINE(EXCEL_WITH_LIBXML, 1, [ ])
    AC_MSG_RESULT(found in $LIBXML_INCDIR)
  fi

  PHP_CHECK_LIBRARY(xl, xlCreateBookCA,
  [
    PHP_ADD_INCLUDE($EXCEL_INCDIR)
    PHP_ADD_INCLUDE($LIBXML_INCDIR)
    PHP_ADD_LIBRARY_WITH_PATH(xl, $EXCEL_LIBDIR, EXCEL_SHARED_LIBADD)
    AC_DEFINE(HAVE_EXCELLIB, 1, [ ])
  ], [
    AC_MSG_ERROR([excel module requires libxl >= 2.4.3])
  ], [
    -L$EXCEL_LIBDIR
  ])

  PHP_CHECK_LIBRARY(xl,xlBookSetKeyA,
  [
    AC_DEFINE(HAVE_LIBXL_SETKEY,1,[ ])
  ],[],[])

  if test `grep -c FILLPATTERN_HORSTRIPE $EXCEL_INCDIR/enum.h` -eq 1; then
    AC_DEFINE(HAVE_LIBXL_243_PLUS,1,[ ])
  fi

  PHP_SUBST(EXCEL_SHARED_LIBADD)
  PHP_NEW_EXTENSION(excel, excel.c, $ext_shared)
fi
