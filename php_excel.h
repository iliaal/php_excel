/*
  +----------------------------------------------------------------------+
  | PHP Version 5                                                        |
  +----------------------------------------------------------------------+
  | Copyright (c) 1997-2010 The PHP Group                                |
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
#define PHP_EXCEL_H

extern zend_module_entry excel_module_entry;
#define phpext_excel_ptr &excel_module_entry

ZEND_BEGIN_MODULE_GLOBALS(excel)
	char *ini_license_name;
	char *ini_license_key;
	int ini_skip_empty;
ZEND_END_MODULE_GLOBALS(excel)


#ifdef PHP_WIN32
#define PHP_EXCEL_API __declspec(dllexport)
#else
#define PHP_EXCEL_API
#endif

#ifdef ZTS
#include "TSRM.h"
#endif

#if ZEND_MODULE_API_NO >= 20100409
#ifndef ZEND_ENGINE_2_4
#define ZEND_ENGINE_2_4
#endif
#endif

#if PHP_VERSION_ID >= 50300
# define PHP_EXCEL_ERROR_HANDLING() \
	zend_error_handling error_handling; \
	zend_replace_error_handling(EH_THROW, NULL, &error_handling TSRMLS_CC);
# define PHP_EXCEL_RESTORE_ERRORS() zend_restore_error_handling(&error_handling TSRMLS_CC);
#else
# define PHP_EXCEL_ERROR_HANDLING() php_set_error_handling(EH_THROW, NULL TSRMLS_CC); 
# define PHP_EXCEL_RESTORE_ERRORS() php_std_error_handling();
#endif

#ifndef Z_SET_ISREF_P
# define Z_SET_ISREF_P(pz)				(pz)->is_ref = 1
# define Z_SET_ISREF_PP(ppz)			Z_SET_ISREF_P(*(ppz))
# define Z_SET_ISREF(z)				Z_SET_ISREF_P(&(z))
#endif

#ifndef Z_SET_REFCOUNT_P
# define Z_SET_REFCOUNT_P(pz, rc)      (pz)->refcount = rc
# define Z_SET_REFCOUNT_PP(ppz, rc)    Z_SET_REFCOUNT_P(*(ppz), rc)
#endif

#endif	/* PHP_EXCEL_H */

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
