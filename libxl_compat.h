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

/* Private libxl compatibility shims for the excel extension. Included by
 * excel.c only (after libxl.h); never installed or included elsewhere. */

#ifndef PHP_EXCEL_LIBXL_COMPAT_H
#define PHP_EXCEL_LIBXL_COMPAT_H 1

/* work-around for buggy/missing macros in libxl.h */
#if LIBXL_VERSION >= 0x05010000
#undef xlSheetRemoveConditionalFormatting
#define xlSheetRemoveConditionalFormatting xlSheetRemoveConditionalFormattingA
#undef xlSheetConditionalFormattingSize
#define xlSheetConditionalFormattingSize xlSheetConditionalFormattingSizeA
#endif
#ifndef xlSheetSetBorder
#define xlSheetSetBorder xlSheetSetBorderA
#endif

#endif	/* PHP_EXCEL_LIBXL_COMPAT_H */
