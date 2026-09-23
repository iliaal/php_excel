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

#ifndef PHP_EXCEL_H
#define PHP_EXCEL_H 1

extern zend_module_entry excel_module_entry;
#define phpext_excel_ptr &excel_module_entry

ZEND_BEGIN_MODULE_GLOBALS(excel)
	char *ini_license_name;
	char *ini_license_key;
	zend_long ini_skip_empty;
ZEND_END_MODULE_GLOBALS(excel)

#define EXCEL_G(v) ZEND_MODULE_GLOBALS_ACCESSOR(excel, v)

#if defined(ZTS) && defined(COMPILE_DL_EXCEL)
ZEND_TSRMLS_CACHE_EXTERN()
#endif

#ifdef PHP_WIN32
#define PHP_EXCEL_API __declspec(dllexport)
#else
#define PHP_EXCEL_API
#endif

/* The zend_register_internal_class_with_flags() polyfill for PHP < 8.4 lives
 * in excel.c (static); this header must not emit definitions. */

#endif	/* PHP_EXCEL_H */
