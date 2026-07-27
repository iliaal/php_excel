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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "libxl.h"
#include <limits.h>
#include <stdlib.h>

#include "php.h"
#include "php_ini.h"
#if PHP_VERSION_ID < 80200
#include "ext/standard/php_random.h"
#else
#include "ext/random/php_random.h"
#endif
#include "ext/standard/info.h"
#include "ext/date/php_date.h"

#include "php_excel.h"
#include "zend_exceptions.h"
#include "Zend/zend_smart_str.h"

#if !defined(LIBXL_VERSION) || LIBXL_VERSION < 0x04060000
#error "LibXL version 4.6.0+ required"
#endif

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

#define PHP_EXCEL_DATE 1
#define PHP_EXCEL_FORMULA 2
#define PHP_EXCEL_NUMERIC_STRING 3
#define PHP_EXCEL_TEXT 4

#define PHP_EXCEL_VERSION "2.5.0"

#ifdef COMPILE_DL_EXCEL
#ifdef ZTS
ZEND_TSRMLS_CACHE_DEFINE()
#endif
ZEND_GET_MODULE(excel)
#endif

ZEND_DECLARE_MODULE_GLOBALS(excel)

static PHP_GINIT_FUNCTION(excel);

PHP_INI_BEGIN()
#if defined(HAVE_LIBXL_SETKEY)
	/* Commercial license credentials: PHP_INI_SYSTEM so a per-request
	 * ini_set() or per-directory .user.ini/.htaccess cannot mutate the key
	 * in a shared pool. A runtime key can still be supplied per book via the
	 * ExcelBook constructor. */
	STD_PHP_INI_ENTRY("excel.license_name", NULL, PHP_INI_SYSTEM, OnUpdateString, ini_license_name, zend_excel_globals, excel_globals)
	STD_PHP_INI_ENTRY("excel.license_key", NULL, PHP_INI_SYSTEM, OnUpdateString, ini_license_key, zend_excel_globals, excel_globals)
#endif
	STD_PHP_INI_ENTRY("excel.skip_empty", "0", PHP_INI_ALL, OnUpdateLong, ini_skip_empty, zend_excel_globals, excel_globals)
PHP_INI_END()

/* {{{ OO init/structure stuff */
/* default_object_handlers is a class-entry field added in PHP 8.3; on
 * 8.1/8.2 each create_object handler sets intern->std.handlers directly,
 * which is the correct per-object mechanism there. */
#if PHP_VERSION_ID >= 80300
#define EXCEL_SET_DEFAULT_OBJECT_HANDLERS(c_name) \
	excel_ce_ ## c_name->default_object_handlers = &excel_object_handlers_ ## c_name
#else
#define EXCEL_SET_DEFAULT_OBJECT_HANDLERS(c_name) ((void)0)
#endif

/* Class registration uses the gen_stub-generated register_class_*()
 * functions from excel_arginfo.h; this macro layers on create_object,
 * NOT_SERIALIZABLE, and per-class object_handlers. */
#define REGISTER_EXCEL_CLASS(name, c_name, clone) \
	{ \
		excel_ce_ ## c_name = register_class_Excel ## name(); \
		excel_ce_ ## c_name->ce_flags |= ZEND_ACC_NOT_SERIALIZABLE; \
		excel_ce_ ## c_name->create_object = excel_object_new_ ## c_name; \
		memcpy(&excel_object_handlers_ ## c_name, zend_get_std_object_handlers(), sizeof(zend_object_handlers)); \
		EXCEL_SET_DEFAULT_OBJECT_HANDLERS(c_name); \
		excel_object_handlers_ ## c_name .offset = offsetof(excel_ ## c_name ## _object, std); \
		excel_object_handlers_ ## c_name .free_obj = excel_ ## c_name ## _object_free_storage; \
		excel_object_handlers_ ## c_name .clone_obj = clone; \
	}

zend_class_entry *excel_ce_book, *excel_ce_sheet, *excel_ce_format, *excel_ce_font, *excel_ce_filtercolumn, *excel_ce_autofilter;
zend_class_entry *excel_ce_richstring, *excel_ce_formcontrol, *excel_ce_conditionalformat, *excel_ce_conditionalformatting, *excel_ce_coreproperties, *excel_ce_table;

static zend_object_handlers excel_object_handlers_book;
static zend_object_handlers excel_object_handlers_sheet;
static zend_object_handlers excel_object_handlers_format;
static zend_object_handlers excel_object_handlers_font;
static zend_object_handlers excel_object_handlers_autofilter;
static zend_object_handlers excel_object_handlers_filtercolumn;
static zend_object_handlers excel_object_handlers_richstring;
static zend_object_handlers excel_object_handlers_formcontrol;
static zend_object_handlers excel_object_handlers_conditionalformat;
static zend_object_handlers excel_object_handlers_conditionalformatting;
static zend_object_handlers excel_object_handlers_coreproperties;
static zend_object_handlers excel_object_handlers_table;

typedef struct _excel_book_object {
	BookHandle book;
	/* Bumped whenever the underlying libxl book state is reset (load,
	 * loadFile, clear, loadInfo, loadInfoRaw, manual __construct reuse).
	 * Every child wrapper stamps this at creation and refuses to use a
	 * stale book after it diverges. */
	uint64_t generation;
	/* Bumped for sheet-topology mutations that leave workbook-scoped
	 * handles valid but can retarget sheet-derived wrappers. */
	uint64_t sheet_generation;
	/* Bumped when libxl removes child handles that existing PHP wrappers
	 * may still hold; scoped by handle family to keep owners usable. */
	uint64_t autofilter_generation;
	uint64_t conditional_formatting_generation;
	/* True for xlCreateXMLBook() (XLSX, 1048576x16384), false for
	 * xlCreateBook() (XLS, 65536x256). Used by EXCEL_VALIDATE_ROW_COL to
	 * reject impossible coordinates per book type. */
	bool is_xlsx;
	bool locale_is_utf8;
	/* Lazily-allocated default format used by IS_DATE writes when the
	 * caller doesn't supply an explicit format, so bulk date exports
	 * don't create one new format per cell and bloat the style table.
	 * Reset along with `book` on __construct reuse. */
	FormatHandle default_date_format;
	/* True while a LibXL-owned buffer (SaveRaw/getPicture) is being copied
	 * into a PHP string. Mutating APIs must not free/replace that buffer. */
	bool native_buffer_borrowed;
	zend_object std;
} excel_book_object;

static inline excel_book_object *php_excel_book_object_fetch_object(zend_object *obj) {
	return (excel_book_object *)((char *)(obj) - offsetof(excel_book_object, std));
}

#define Z_EXCEL_BOOK_OBJ_P(zv) php_excel_book_object_fetch_object(Z_OBJ_P(zv));

#define BOOK_FROM_OBJECT(book, object) \
	{ \
		excel_book_object *obj = Z_EXCEL_BOOK_OBJ_P(object); \
		book = obj->book; \
		if (!book) { \
			php_error_docref(NULL, E_WARNING, "The book wasn't initialized"); \
			RETURN_FALSE; \
		} \
	}

/* Post-stream-read variant: the fetch runs after user stream callbacks, so an
 * owned buffer is already live and must be released on the bail path. */
#define BOOK_FROM_OBJECT_RELEASE_STR(book, object, contents_zs) \
	{ \
		excel_book_object *obj = Z_EXCEL_BOOK_OBJ_P(object); \
		book = obj->book; \
		if (!book) { \
			zend_string_release(contents_zs); \
			php_error_docref(NULL, E_WARNING, "The book wasn't initialized"); \
			RETURN_FALSE; \
		} \
	}

typedef struct _excel_sheet_object {
	SheetHandle	sheet;
	BookHandle book;
	uint64_t book_generation;
	uint64_t sheet_generation;
	zval parent;
	zend_object std;
} excel_sheet_object;

static inline excel_sheet_object *php_excel_sheet_object_fetch_object(zend_object *obj) {
	return (excel_sheet_object *)((char *)(obj) - offsetof(excel_sheet_object, std));
}

#define Z_EXCEL_SHEET_OBJ_P(zv) php_excel_sheet_object_fetch_object(Z_OBJ_P(zv));

#define SHEET_FROM_OBJECT(sheet, object) \
	{ \
		excel_sheet_object *obj = Z_EXCEL_SHEET_OBJ_P(object); \
		sheet = obj->sheet; \
		if (!sheet) { \
			php_error_docref(NULL, E_WARNING, "The sheet wasn't initialized"); \
			RETURN_FALSE; \
		} \
		CHECK_BOOK_AND_SHEET_GENERATION(obj); \
	}

#define SHEET_AND_BOOK_FROM_OBJECT(sheet, book, object) \
	{ \
		excel_sheet_object *obj = Z_EXCEL_SHEET_OBJ_P(object); \
		sheet = obj->sheet; \
		book = obj->book; \
		if (!sheet) { \
			php_error_docref(NULL, E_WARNING, "The sheet wasn't initialized"); \
			RETURN_FALSE; \
		} \
		CHECK_BOOK_AND_SHEET_GENERATION(obj); \
	}

typedef struct _excel_font_object {
	FontHandle font;
	BookHandle book;
	uint64_t book_generation;
	zval parent;
	zend_object std;
} excel_font_object;

static inline excel_font_object *php_excel_font_object_fetch_object(zend_object *obj) {
	return (excel_font_object *)((char *)(obj) - offsetof(excel_font_object, std));
}
#define Z_EXCEL_FONT_OBJ_P(zv) php_excel_font_object_fetch_object(Z_OBJ_P(zv));

#define FONT_FROM_OBJECT(font, object) \
	{ \
		excel_font_object *obj = Z_EXCEL_FONT_OBJ_P(object); \
		font = obj->font; \
		if (!font) { \
			php_error_docref(NULL, E_WARNING, "The font wasn't initialized"); \
			RETURN_FALSE; \
		} \
		CHECK_BOOK_GENERATION(obj); \
	}

#define FORMAT_FROM_OBJECT(format, object) \
	{ \
		excel_format_object *obj = Z_EXCEL_FORMAT_OBJ_P(object); \
		format = obj->format; \
		if (!format) { \
			php_error_docref(NULL, E_WARNING, "The format wasn't initialized"); \
			RETURN_FALSE; \
		} \
		CHECK_BOOK_GENERATION(obj); \
	}

#define AUTOFILTER_FROM_OBJECT(autofilter, object) \
	{ \
		excel_autofilter_object *obj = Z_EXCEL_AUTOFILTER_OBJ_P(object); \
		autofilter = obj->autofilter; \
		if (!autofilter) { \
			php_error_docref(NULL, E_WARNING, "The autofilter wasn't initialized"); \
			RETURN_FALSE; \
		} \
		CHECK_BOOK_SHEET_AND_AUTOFILTER_GENERATION(obj); \
	}

#define FILTERCOLUMN_FROM_OBJECT(filtercolumn, object) \
	{ \
		excel_filtercolumn_object *obj = Z_EXCEL_FILTERCOLUMN_OBJ_P(object); \
		filtercolumn = obj->filtercolumn; \
		if (!filtercolumn) { \
			php_error_docref(NULL, E_WARNING, "The filtercolumn wasn't initialized"); \
			RETURN_FALSE; \
		} \
		CHECK_BOOK_SHEET_AND_AUTOFILTER_GENERATION(obj); \
	}

typedef struct _excel_format_object {
	FormatHandle format;
	BookHandle book;
	uint64_t book_generation;
	zval parent;
	zend_object std;
} excel_format_object;

static inline excel_format_object *php_excel_format_object_fetch_object(zend_object *obj) {
	return (excel_format_object *)((char *)(obj) - offsetof(excel_format_object, std));
}
#define Z_EXCEL_FORMAT_OBJ_P(zv) php_excel_format_object_fetch_object(Z_OBJ_P(zv));

typedef struct _excel_autofilter_object {
	AutoFilterHandle autofilter;
	SheetHandle sheet;
	uint64_t book_generation;
	uint64_t sheet_generation;
	uint64_t autofilter_generation;
	zval parent;
	zend_object std;
} excel_autofilter_object;

static inline excel_autofilter_object *php_excel_autofilter_object_fetch_object(zend_object *obj) {
	return (excel_autofilter_object *)((char *)(obj) - offsetof(excel_autofilter_object, std));
}
#define Z_EXCEL_AUTOFILTER_OBJ_P(zv) php_excel_autofilter_object_fetch_object(Z_OBJ_P(zv));

typedef struct _excel_filtercolumn_object {
	FilterColumnHandle filtercolumn;
	AutoFilterHandle autofilter;
	uint64_t book_generation;
	uint64_t sheet_generation;
	uint64_t autofilter_generation;
	zval parent;
	zend_object std;
} excel_filtercolumn_object;

static inline excel_filtercolumn_object *php_excel_filtercolumn_object_fetch_object(zend_object *obj) {
	return (excel_filtercolumn_object *)((char *)(obj) - offsetof(excel_filtercolumn_object, std));
}
#define Z_EXCEL_FILTERCOLUMN_OBJ_P(zv) php_excel_filtercolumn_object_fetch_object(Z_OBJ_P(zv));

typedef struct _excel_richstring_object {
	RichStringHandle richstring;
	BookHandle book;
	uint64_t book_generation;
	zval parent;
	zend_object std;
} excel_richstring_object;

static inline excel_richstring_object *php_excel_richstring_object_fetch_object(zend_object *obj) {
	return (excel_richstring_object *)((char *)(obj) - offsetof(excel_richstring_object, std));
}
#define Z_EXCEL_RICHSTRING_OBJ_P(zv) php_excel_richstring_object_fetch_object(Z_OBJ_P(zv));

#define RICHSTRING_FROM_OBJECT(rs_var, object) \
	{ \
		excel_richstring_object *obj = Z_EXCEL_RICHSTRING_OBJ_P(object); \
		rs_var = obj->richstring; \
		if (!rs_var) { \
			php_error_docref(NULL, E_WARNING, "The richstring wasn't initialized"); \
			RETURN_FALSE; \
		} \
		CHECK_BOOK_GENERATION(obj); \
	}

typedef struct _excel_formcontrol_object {
	FormControlHandle formcontrol;
	SheetHandle sheet;
	uint64_t book_generation;
	uint64_t sheet_generation;
	zval parent;
	zend_object std;
} excel_formcontrol_object;

static inline excel_formcontrol_object *php_excel_formcontrol_object_fetch_object(zend_object *obj) {
	return (excel_formcontrol_object *)((char *)(obj) - offsetof(excel_formcontrol_object, std));
}
#define Z_EXCEL_FORMCONTROL_OBJ_P(zv) php_excel_formcontrol_object_fetch_object(Z_OBJ_P(zv));

#define FORMCONTROL_FROM_OBJECT(fc_var, object) \
	{ \
		excel_formcontrol_object *obj = Z_EXCEL_FORMCONTROL_OBJ_P(object); \
		fc_var = obj->formcontrol; \
		if (!fc_var) { \
			php_error_docref(NULL, E_WARNING, "The formcontrol wasn't initialized"); \
			RETURN_FALSE; \
		} \
		CHECK_BOOK_AND_SHEET_GENERATION(obj); \
	}

typedef struct _excel_conditionalformat_object {
	ConditionalFormatHandle conditionalformat;
	BookHandle book;
	uint64_t book_generation;
	zval parent;
	zend_object std;
} excel_conditionalformat_object;

static inline excel_conditionalformat_object *php_excel_conditionalformat_object_fetch_object(zend_object *obj) {
	return (excel_conditionalformat_object *)((char *)(obj) - offsetof(excel_conditionalformat_object, std));
}
#define Z_EXCEL_CONDITIONALFORMAT_OBJ_P(zv) php_excel_conditionalformat_object_fetch_object(Z_OBJ_P(zv));

#define CONDITIONALFORMAT_FROM_OBJECT(cf_var, object) \
	{ \
		excel_conditionalformat_object *obj = Z_EXCEL_CONDITIONALFORMAT_OBJ_P(object); \
		cf_var = obj->conditionalformat; \
		if (!cf_var) { \
			php_error_docref(NULL, E_WARNING, "The conditionalformat wasn't initialized"); \
			RETURN_FALSE; \
		} \
		CHECK_BOOK_GENERATION(obj); \
	}

typedef struct _excel_conditionalformatting_object {
	ConditionalFormattingHandle conditionalformatting;
	SheetHandle sheet;
	uint64_t book_generation;
	uint64_t sheet_generation;
	uint64_t conditional_formatting_generation;
	zval parent;
	zend_object std;
} excel_conditionalformatting_object;

static inline excel_conditionalformatting_object *php_excel_conditionalformatting_object_fetch_object(zend_object *obj) {
	return (excel_conditionalformatting_object *)((char *)(obj) - offsetof(excel_conditionalformatting_object, std));
}
#define Z_EXCEL_CONDITIONALFORMATTING_OBJ_P(zv) php_excel_conditionalformatting_object_fetch_object(Z_OBJ_P(zv));

#define CONDITIONALFORMATTING_FROM_OBJECT(cfing_var, object) \
	{ \
		excel_conditionalformatting_object *obj = Z_EXCEL_CONDITIONALFORMATTING_OBJ_P(object); \
		cfing_var = obj->conditionalformatting; \
		if (!cfing_var) { \
			php_error_docref(NULL, E_WARNING, "The conditionalformatting wasn't initialized"); \
			RETURN_FALSE; \
		} \
		CHECK_BOOK_SHEET_AND_CONDITIONALFORMATTING_GENERATION(obj); \
	}

typedef struct _excel_coreproperties_object {
	CorePropertiesHandle coreproperties;
	BookHandle book;
	uint64_t book_generation;
	zval parent;
	zend_object std;
} excel_coreproperties_object;

static inline excel_coreproperties_object *php_excel_coreproperties_object_fetch_object(zend_object *obj) {
	return (excel_coreproperties_object *)((char *)(obj) - offsetof(excel_coreproperties_object, std));
}
#define Z_EXCEL_COREPROPERTIES_OBJ_P(zv) php_excel_coreproperties_object_fetch_object(Z_OBJ_P(zv));

#define COREPROPERTIES_FROM_OBJECT(cp_var, object) \
	{ \
		excel_coreproperties_object *obj = Z_EXCEL_COREPROPERTIES_OBJ_P(object); \
		cp_var = obj->coreproperties; \
		if (!cp_var) { \
			php_error_docref(NULL, E_WARNING, "The coreproperties wasn't initialized"); \
			RETURN_FALSE; \
		} \
		CHECK_BOOK_GENERATION(obj); \
	}

typedef struct _excel_table_object {
	TableHandle table;
	SheetHandle sheet;
	uint64_t book_generation;
	uint64_t sheet_generation;
	zval parent;
	zend_object std;
} excel_table_object;

static inline excel_table_object *php_excel_table_object_fetch_object(zend_object *obj) {
	return (excel_table_object *)((char *)(obj) - offsetof(excel_table_object, std));
}
#define Z_EXCEL_TABLE_OBJ_P(zv) php_excel_table_object_fetch_object(Z_OBJ_P(zv));

#define TABLE_FROM_OBJECT(tbl_var, object) \
	{ \
		excel_table_object *obj = Z_EXCEL_TABLE_OBJ_P(object); \
		tbl_var = obj->table; \
		if (!tbl_var) { \
			php_error_docref(NULL, E_WARNING, "The table wasn't initialized"); \
			RETURN_FALSE; \
		} \
		CHECK_BOOK_AND_SHEET_GENERATION(obj); \
	}

/* Walk the parent zval chain up to the owning ExcelBook so that any descendant
 * (sheet, format, autofilter, filtercolumn, table, ...) can compare its
 * book_generation against the book's current generation counter. */
static inline excel_book_object *php_excel_resolve_book_obj(zval *parent_zv) {
	int hops = 0;
	while (parent_zv && Z_TYPE_P(parent_zv) == IS_OBJECT && hops++ < 8) {
		zend_class_entry *ce = Z_OBJCE_P(parent_zv);
		if (instanceof_function(ce, excel_ce_book)) {
			return php_excel_book_object_fetch_object(Z_OBJ_P(parent_zv));
		} else if (instanceof_function(ce, excel_ce_sheet)) {
			parent_zv = &php_excel_sheet_object_fetch_object(Z_OBJ_P(parent_zv))->parent;
		} else if (instanceof_function(ce, excel_ce_format)) {
			parent_zv = &php_excel_format_object_fetch_object(Z_OBJ_P(parent_zv))->parent;
		} else if (instanceof_function(ce, excel_ce_font)) {
			parent_zv = &php_excel_font_object_fetch_object(Z_OBJ_P(parent_zv))->parent;
		} else if (instanceof_function(ce, excel_ce_autofilter)) {
			parent_zv = &php_excel_autofilter_object_fetch_object(Z_OBJ_P(parent_zv))->parent;
		} else if (instanceof_function(ce, excel_ce_filtercolumn)) {
			parent_zv = &php_excel_filtercolumn_object_fetch_object(Z_OBJ_P(parent_zv))->parent;
		} else if (instanceof_function(ce, excel_ce_richstring)) {
			parent_zv = &php_excel_richstring_object_fetch_object(Z_OBJ_P(parent_zv))->parent;
		} else if (instanceof_function(ce, excel_ce_formcontrol)) {
			parent_zv = &php_excel_formcontrol_object_fetch_object(Z_OBJ_P(parent_zv))->parent;
		} else if (instanceof_function(ce, excel_ce_conditionalformat)) {
			parent_zv = &php_excel_conditionalformat_object_fetch_object(Z_OBJ_P(parent_zv))->parent;
		} else if (instanceof_function(ce, excel_ce_conditionalformatting)) {
			parent_zv = &php_excel_conditionalformatting_object_fetch_object(Z_OBJ_P(parent_zv))->parent;
		} else if (instanceof_function(ce, excel_ce_coreproperties)) {
			parent_zv = &php_excel_coreproperties_object_fetch_object(Z_OBJ_P(parent_zv))->parent;
		} else if (instanceof_function(ce, excel_ce_table)) {
			parent_zv = &php_excel_table_object_fetch_object(Z_OBJ_P(parent_zv))->parent;
		} else {
			return NULL;
		}
	}
	return NULL;
}

/* libxl Format/Font/RichString/AutoFilter/ConditionalFormat handles are scoped
 * to the workbook that created them — they index into that book's internal
 * tables. Applying one to a different workbook's sheet/cell/rule silently
 * produces wrong output and dangles once the source book is freed. Reject a
 * child-wrapper argument whose owning book differs from the target's. Returns 1
 * when both resolve to the same ExcelBook, 0 otherwise. Template-copy methods
 * such as Book::addFormat/addFont legitimately accept handles from another
 * book and must NOT use this guard. */
static zend_always_inline int php_excel_same_book(zval *arg_zv, zval *target_zv) {
	excel_book_object *ba = php_excel_resolve_book_obj(arg_zv);
	excel_book_object *bb = php_excel_resolve_book_obj(target_zv);
	return ba && bb && ba == bb;
}

/* Warn-and-reject form of php_excel_same_book for method bodies. Returns true
 * when the argument is usable; false after warning when books differ. */
static zend_always_inline bool php_excel_require_same_book(zval *arg_zv, zval *target_zv)
{
	if (php_excel_same_book(arg_zv, target_zv)) {
		return true;
	}
	php_error_docref(NULL, E_WARNING, "Argument belongs to a different ExcelBook");
	return false;
}

#define EXCEL_REQUIRE_SAME_BOOK(arg_zv, target_zv) \
	if (!php_excel_require_same_book((arg_zv), (target_zv))) { \
		RETURN_FALSE; \
	}

/* Generation predicates and warn-path checks. Throw variants (below) share the
 * same matchers so stale messages stay in one place per failure mode. */

static zend_always_inline bool php_excel_book_generation_matches(excel_book_object *b, uint64_t stamped)
{
	return b && b->book && b->generation == stamped;
}

static zend_always_inline bool php_excel_book_generation_ok(excel_book_object *b, uint64_t stamped)
{
	if (php_excel_book_generation_matches(b, stamped)) {
		return true;
	}
	php_error_docref(NULL, E_WARNING,
		"Underlying ExcelBook handle is stale (parent was reloaded, cleared, or reinitialized)");
	return false;
}

static zend_always_inline bool php_excel_sheet_generation_ok(excel_book_object *b, uint64_t stamped)
{
	if (b->sheet_generation == stamped) {
		return true;
	}
	php_error_docref(NULL, E_WARNING,
		"Underlying ExcelBook sheet topology changed; wrapper must be re-fetched");
	return false;
}

static zend_always_inline bool php_excel_autofilter_generation_ok(excel_book_object *b, uint64_t stamped)
{
	if (b->autofilter_generation == stamped) {
		return true;
	}
	php_error_docref(NULL, E_WARNING,
		"Underlying ExcelBook autofilter state changed; wrapper must be re-fetched");
	return false;
}

static zend_always_inline bool php_excel_conditional_formatting_generation_ok(excel_book_object *b, uint64_t stamped)
{
	if (b->conditional_formatting_generation == stamped) {
		return true;
	}
	php_error_docref(NULL, E_WARNING,
		"Underlying ExcelBook conditional formatting state changed; wrapper must be re-fetched");
	return false;
}

/* Preresolved: caller already has the owning book (hot paths that also need
 * coordinate limits can resolve once and feed both checks). */
static zend_always_inline bool php_excel_check_book_generation_pr(excel_book_object *vb, uint64_t book_stamped)
{
	return php_excel_book_generation_ok(vb, book_stamped);
}

static zend_always_inline bool php_excel_check_book_and_sheet_generation_pr(
	excel_book_object *vb, uint64_t book_stamped, uint64_t sheet_stamped)
{
	if (!php_excel_book_generation_ok(vb, book_stamped)) {
		return false;
	}
	return php_excel_sheet_generation_ok(vb, sheet_stamped);
}

static zend_always_inline bool php_excel_check_book_sheet_and_autofilter_generation_pr(
	excel_book_object *vb, uint64_t book_stamped, uint64_t sheet_stamped, uint64_t autofilter_stamped)
{
	if (!php_excel_check_book_and_sheet_generation_pr(vb, book_stamped, sheet_stamped)) {
		return false;
	}
	return php_excel_autofilter_generation_ok(vb, autofilter_stamped);
}

static zend_always_inline bool php_excel_check_book_sheet_and_conditional_formatting_generation_pr(
	excel_book_object *vb, uint64_t book_stamped, uint64_t sheet_stamped, uint64_t cf_stamped)
{
	if (!php_excel_check_book_and_sheet_generation_pr(vb, book_stamped, sheet_stamped)) {
		return false;
	}
	return php_excel_conditional_formatting_generation_ok(vb, cf_stamped);
}

static zend_always_inline bool php_excel_check_book_generation(zval *parent_zv, uint64_t book_stamped)
{
	return php_excel_check_book_generation_pr(php_excel_resolve_book_obj(parent_zv), book_stamped);
}

static zend_always_inline bool php_excel_check_book_and_sheet_generation(
	zval *parent_zv, uint64_t book_stamped, uint64_t sheet_stamped)
{
	return php_excel_check_book_and_sheet_generation_pr(
		php_excel_resolve_book_obj(parent_zv), book_stamped, sheet_stamped);
}

static zend_always_inline bool php_excel_check_book_sheet_and_autofilter_generation(
	zval *parent_zv, uint64_t book_stamped, uint64_t sheet_stamped, uint64_t autofilter_stamped)
{
	return php_excel_check_book_sheet_and_autofilter_generation_pr(
		php_excel_resolve_book_obj(parent_zv), book_stamped, sheet_stamped, autofilter_stamped);
}

static zend_always_inline bool php_excel_check_book_sheet_and_conditional_formatting_generation(
	zval *parent_zv, uint64_t book_stamped, uint64_t sheet_stamped, uint64_t cf_stamped)
{
	return php_excel_check_book_sheet_and_conditional_formatting_generation_pr(
		php_excel_resolve_book_obj(parent_zv), book_stamped, sheet_stamped, cf_stamped);
}

#define CHECK_BOOK_GENERATION(child_obj) \
	do { \
		if (!php_excel_check_book_generation(&(child_obj)->parent, (child_obj)->book_generation)) { \
			RETURN_FALSE; \
		} \
	} while (0)

#define CHECK_BOOK_AND_SHEET_GENERATION(child_obj) \
	do { \
		if (!php_excel_check_book_and_sheet_generation(&(child_obj)->parent, \
				(child_obj)->book_generation, (child_obj)->sheet_generation)) { \
			RETURN_FALSE; \
		} \
	} while (0)

#define CHECK_BOOK_SHEET_AND_AUTOFILTER_GENERATION(child_obj) \
	do { \
		if (!php_excel_check_book_sheet_and_autofilter_generation(&(child_obj)->parent, \
				(child_obj)->book_generation, (child_obj)->sheet_generation, \
				(child_obj)->autofilter_generation)) { \
			RETURN_FALSE; \
		} \
	} while (0)

#define CHECK_BOOK_SHEET_AND_CONDITIONALFORMATTING_GENERATION(child_obj) \
	do { \
		if (!php_excel_check_book_sheet_and_conditional_formatting_generation(&(child_obj)->parent, \
				(child_obj)->book_generation, (child_obj)->sheet_generation, \
				(child_obj)->conditional_formatting_generation)) { \
			RETURN_FALSE; \
		} \
	} while (0)

/* Preresolved variants: resolve_book_obj once for stale check + coord limits. */
#define CHECK_BOOK_GENERATION_PR(child_obj, vb) \
	do { \
		if (!php_excel_check_book_generation_pr((vb), (child_obj)->book_generation)) { \
			RETURN_FALSE; \
		} \
	} while (0)

#define CHECK_BOOK_AND_SHEET_GENERATION_PR(child_obj, vb) \
	do { \
		if (!php_excel_check_book_and_sheet_generation_pr((vb), \
				(child_obj)->book_generation, (child_obj)->sheet_generation)) { \
			RETURN_FALSE; \
		} \
	} while (0)

/* Stamp child wrapper with current book generation and copy parent zval.
 * Use at every child creation site instead of a bare ZVAL_COPY(&x->parent, p).
 * Re-invoking a child __construct() reaches this with parent already holding a
 * refcounted zval from the first construct; drop it before the overwrite or
 * that reference leaks. First construct is safe: zend_object_alloc zeroed
 * parent to IS_UNDEF. */
static zend_always_inline void php_excel_replace_parent_zval(zval *parent_field, zval *parent_zv)
{
	if (Z_TYPE_P(parent_field) != IS_UNDEF) {
		zval_ptr_dtor(parent_field);
	}
	ZVAL_COPY(parent_field, parent_zv);
}

static zend_always_inline void php_excel_init_parent(
	uint64_t *book_generation, zval *parent_field, zval *parent_zv)
{
	excel_book_object *bg = php_excel_resolve_book_obj(parent_zv);
	*book_generation = bg ? bg->generation : 0;
	php_excel_replace_parent_zval(parent_field, parent_zv);
}

static zend_always_inline void php_excel_init_sheet_parent(
	uint64_t *book_generation, uint64_t *sheet_generation, zval *parent_field, zval *parent_zv)
{
	excel_book_object *bg = php_excel_resolve_book_obj(parent_zv);
	*book_generation = bg ? bg->generation : 0;
	*sheet_generation = bg ? bg->sheet_generation : 0;
	php_excel_replace_parent_zval(parent_field, parent_zv);
}

static zend_always_inline void php_excel_init_autofilter_parent(
	uint64_t *book_generation, uint64_t *sheet_generation, uint64_t *autofilter_generation,
	zval *parent_field, zval *parent_zv)
{
	excel_book_object *bg = php_excel_resolve_book_obj(parent_zv);
	*book_generation = bg ? bg->generation : 0;
	*sheet_generation = bg ? bg->sheet_generation : 0;
	*autofilter_generation = bg ? bg->autofilter_generation : 0;
	php_excel_replace_parent_zval(parent_field, parent_zv);
}

static zend_always_inline void php_excel_init_conditional_formatting_parent(
	uint64_t *book_generation, uint64_t *sheet_generation, uint64_t *cf_generation,
	zval *parent_field, zval *parent_zv)
{
	excel_book_object *bg = php_excel_resolve_book_obj(parent_zv);
	*book_generation = bg ? bg->generation : 0;
	*sheet_generation = bg ? bg->sheet_generation : 0;
	*cf_generation = bg ? bg->conditional_formatting_generation : 0;
	php_excel_replace_parent_zval(parent_field, parent_zv);
}

#define EXCEL_INIT_PARENT(child_obj, parent_zv) \
	php_excel_init_parent(&(child_obj)->book_generation, &(child_obj)->parent, (parent_zv))

#define EXCEL_INIT_SHEET_PARENT(child_obj, parent_zv) \
	php_excel_init_sheet_parent(&(child_obj)->book_generation, &(child_obj)->sheet_generation, \
		&(child_obj)->parent, (parent_zv))

#define EXCEL_INIT_AUTOFILTER_PARENT(child_obj, parent_zv) \
	php_excel_init_autofilter_parent(&(child_obj)->book_generation, &(child_obj)->sheet_generation, \
		&(child_obj)->autofilter_generation, &(child_obj)->parent, (parent_zv))

#define EXCEL_INIT_CONDITIONALFORMATTING_PARENT(child_obj, parent_zv) \
	php_excel_init_conditional_formatting_parent(&(child_obj)->book_generation, \
		&(child_obj)->sheet_generation, &(child_obj)->conditional_formatting_generation, \
		&(child_obj)->parent, (parent_zv))

#define EXCEL_REJECT_RECONSTRUCTION(child_obj, handle_field) \
	do { \
		if ((child_obj)->handle_field) { \
			zend_throw_exception(NULL, "Cannot call constructor twice", 0); \
			RETURN_THROWS(); \
		} \
	} while (0)

/* Bump the sheet-topology counter so sheet-derived wrappers fail their
 * stale-check. Workbook-scoped handles remain valid. */
static inline void php_excel_book_bump_sheet_generation(zval *book_zv) {
	excel_book_object *bobj = php_excel_book_object_fetch_object(Z_OBJ_P(book_zv));
	bobj->sheet_generation++;
}

static inline void php_excel_book_bump_autofilter_generation(zval *parent_zv) {
	excel_book_object *bobj = php_excel_resolve_book_obj(parent_zv);
	if (bobj) {
		bobj->autofilter_generation++;
	}
}

static inline void php_excel_book_bump_conditional_formatting_generation(zval *parent_zv) {
	excel_book_object *bobj = php_excel_resolve_book_obj(parent_zv);
	if (bobj) {
		bobj->conditional_formatting_generation++;
	}
}

/* Full state reset: bump every wrapper generation domain and drop the
 * cached default date format. Use after libxl operations that free the
 * internal format table (load, loadFile, loadInfo, loadInfoRaw, clear,
 * __construct reuse). Reusing the stale FormatHandle from
 * default_date_format after one of these would silently write cells with
 * no format, corrupting AS_DATE-typed output. */
static inline int php_excel_book_reset_state(zval *book_zv) {
	excel_book_object *bobj = php_excel_book_object_fetch_object(Z_OBJ_P(book_zv));
	if (bobj->native_buffer_borrowed) {
		php_error_docref(NULL, E_WARNING,
			"Cannot reinitialize workbook while a native I/O buffer is in use");
		return 0;
	}
	bobj->generation++;
	bobj->sheet_generation++;
	bobj->autofilter_generation++;
	bobj->conditional_formatting_generation++;
	bobj->default_date_format = NULL;
	return 1;
}

/* Throw-on-stale variant for code paths that cannot use RETURN_FALSE — most
 * importantly clone handlers, which must always produce an object and signal
 * failure via exception. Returns 1 on valid, 0 (and throws) on stale. Shares
 * matchers with the warn-path helpers above. */
static inline int php_excel_check_book_generation_throw(zval *parent_zv, uint64_t stamped) {
	excel_book_object *b = php_excel_resolve_book_obj(parent_zv);
	if (php_excel_book_generation_matches(b, stamped)) {
		return 1;
	}
	zend_throw_exception(NULL,
		"Underlying ExcelBook handle is stale (parent was reloaded, cleared, or reinitialized)",
		0);
	return 0;
}

static inline int php_excel_check_book_and_sheet_generation_throw(zval *parent_zv, uint64_t book_stamped, uint64_t sheet_stamped) {
	excel_book_object *b = php_excel_resolve_book_obj(parent_zv);
	if (!php_excel_book_generation_matches(b, book_stamped)) {
		zend_throw_exception(NULL,
			"Underlying ExcelBook handle is stale (parent was reloaded, cleared, or reinitialized)",
			0);
		return 0;
	}
	if (b->sheet_generation == sheet_stamped) {
		return 1;
	}
	zend_throw_exception(NULL,
		"Underlying ExcelBook sheet topology changed; wrapper must be re-fetched",
		0);
	return 0;
}

static inline int php_excel_check_book_sheet_and_autofilter_generation_throw(zval *parent_zv, uint64_t book_stamped, uint64_t sheet_stamped, uint64_t autofilter_stamped) {
	if (!php_excel_check_book_and_sheet_generation_throw(parent_zv, book_stamped, sheet_stamped)) {
		return 0;
	}
	{
		excel_book_object *b = php_excel_resolve_book_obj(parent_zv);
		if (b->autofilter_generation == autofilter_stamped) {
			return 1;
		}
	}
	zend_throw_exception(NULL,
		"Underlying ExcelBook autofilter state changed; wrapper must be re-fetched",
		0);
	return 0;
}

/* Throw-on-error variants of the FROM_OBJECT macros. Used in constructors,
 * where PHP ignores the return value of RETURN_FALSE and the caller would
 * otherwise receive an uninitialized wrapper.
 *
 * Each macro fetches the handle, throws (and RETURN_THROWS) if the wrapper
 * is uninitialized, and runs the same generation check that the normal
 * accessors do. */
#define SHEET_FROM_OBJECT_THROW(sheet, object) \
	do { \
		excel_sheet_object *_obj = Z_EXCEL_SHEET_OBJ_P(object); \
		sheet = _obj->sheet; \
		if (!sheet) { \
			zend_throw_exception(NULL, "The sheet wasn't initialized", 0); \
			RETURN_THROWS(); \
		} \
		if (!php_excel_check_book_and_sheet_generation_throw(&_obj->parent, _obj->book_generation, _obj->sheet_generation)) { \
			RETURN_THROWS(); \
		} \
	} while (0)

#define AUTOFILTER_FROM_OBJECT_THROW(autofilter, object) \
	do { \
		excel_autofilter_object *_obj = Z_EXCEL_AUTOFILTER_OBJ_P(object); \
		autofilter = _obj->autofilter; \
		if (!autofilter) { \
			zend_throw_exception(NULL, "The autofilter wasn't initialized", 0); \
			RETURN_THROWS(); \
		} \
		if (!php_excel_check_book_sheet_and_autofilter_generation_throw(&_obj->parent, _obj->book_generation, _obj->sheet_generation, _obj->autofilter_generation)) { \
			RETURN_THROWS(); \
		} \
	} while (0)

#define BOOK_FROM_OBJECT_THROW(book, object) \
	do { \
		excel_book_object *_bobj = Z_EXCEL_BOOK_OBJ_P(object); \
		book = _bobj->book; \
		if (!book) { \
			zend_throw_exception(NULL, "The book wasn't initialized", 0); \
			RETURN_THROWS(); \
		} \
	} while (0)

static void excel_book_object_free_storage(zend_object *object)
{
	excel_book_object *intern = php_excel_book_object_fetch_object(object);
	zend_object_std_dtor(&intern->std);

	if (intern->book) {
		xlBookRelease(intern->book);
		intern->book = NULL;
	}
}

static zend_object *excel_object_new_book(zend_class_entry *class_type)
{
	excel_book_object *intern;

	intern = zend_object_alloc(sizeof(excel_book_object), class_type);

	zend_object_std_init(&intern->std, class_type);
	object_properties_init(&intern->std, class_type);

	intern->book = NULL;
	intern->generation = 0;
	intern->sheet_generation = 0;
	intern->autofilter_generation = 0;
	intern->conditional_formatting_generation = 0;
	intern->is_xlsx = false;
	intern->locale_is_utf8 = false;
	intern->default_date_format = NULL;
	intern->std.handlers = &excel_object_handlers_book;

	return &intern->std;
}

static void php_excel_child_object_std_dtor(zval *parent, zend_object *std)
{
	zval saved_parent;

	ZVAL_COPY_VALUE(&saved_parent, parent);
	ZVAL_UNDEF(parent);
	zend_object_std_dtor(std);
	zval_ptr_dtor(&saved_parent);
}

/* Child wrappers (except book, and font/format with clone helpers) share
 * identical free_storage / object_new bodies. Keep book and *_ex clone
 * constructors hand-written. */
#define EXCEL_CHILD_FREE_STORAGE(c_name) \
static void excel_ ## c_name ## _object_free_storage(zend_object *object) \
{ \
	excel_ ## c_name ## _object *intern = php_excel_ ## c_name ## _object_fetch_object(object); \
	php_excel_child_object_std_dtor(&intern->parent, &intern->std); \
}

#define EXCEL_CHILD_OBJECT_NEW(c_name) \
static zend_object *excel_object_new_ ## c_name(zend_class_entry *class_type) \
{ \
	excel_ ## c_name ## _object *intern; \
	intern = zend_object_alloc(sizeof(excel_ ## c_name ## _object), class_type); \
	zend_object_std_init(&intern->std, class_type); \
	object_properties_init(&intern->std, class_type); \
	intern->std.handlers = &excel_object_handlers_ ## c_name; \
	return &intern->std; \
}

EXCEL_CHILD_FREE_STORAGE(sheet)
EXCEL_CHILD_OBJECT_NEW(sheet)

EXCEL_CHILD_FREE_STORAGE(font)

#define REGISTER_EXCEL_CLASS_CONST_LONG(class_name, const_name, value) \
	zend_declare_class_constant_long(excel_ce_ ## class_name, const_name, sizeof(const_name)-1, (zend_long)value);

#define REGISTER_EXCEL_CLASS_CONST_STRING(class_name, const_name, value) \
	zend_declare_class_constant_string(excel_ce_ ## class_name, const_name, sizeof(const_name)-1, (char *)value);

static zend_object *excel_object_new_font_ex(zend_class_entry *class_type, excel_font_object **ptr)
{
	excel_font_object *intern;

	intern = zend_object_alloc(sizeof(excel_font_object), class_type);

	if (ptr) {
		*ptr = intern;
	}

	zend_object_std_init(&intern->std, class_type);
	object_properties_init(&intern->std, class_type);

	intern->std.handlers = &excel_object_handlers_font;

	return &intern->std;
}

static zend_object *excel_object_new_font(zend_class_entry *class_type)
{
	return excel_object_new_font_ex(class_type, NULL);
}

static zend_object *excel_font_object_clone(zend_object *object)
{
	zend_object *new_ov;
	FontHandle font;

	excel_font_object *new_obj = NULL;
	excel_font_object *old_obj = php_excel_font_object_fetch_object(object);
	new_ov = excel_object_new_font_ex(old_obj->std.ce, &new_obj);

	if (!old_obj->book || !old_obj->font) {
		zend_throw_exception(NULL, "Cannot clone: parent ExcelBook is no longer initialized", 0);
		return new_ov;
	}

	if (!php_excel_check_book_generation_throw(&old_obj->parent, old_obj->book_generation)) {
		return new_ov;
	}

	font = xlBookAddFont(old_obj->book, old_obj->font);
	if (!font) {
		zend_throw_exception(NULL, "Failed to copy font", 0);
	} else {
		new_obj->book = old_obj->book;
		new_obj->font = font;
		EXCEL_INIT_PARENT(new_obj, &old_obj->parent);
	}

	zend_objects_clone_members(&new_obj->std, &old_obj->std);

	return new_ov;
}

EXCEL_CHILD_FREE_STORAGE(format)

static zend_object *excel_object_new_format_ex(zend_class_entry *class_type, excel_format_object **ptr)
{
	excel_format_object *intern;

	intern = zend_object_alloc(sizeof(excel_format_object), class_type);

	if (ptr) {
		*ptr = intern;
	}

	zend_object_std_init(&intern->std, class_type);
	object_properties_init(&intern->std, class_type);

	intern->std.handlers = &excel_object_handlers_format;

	return &intern->std;
}

static zend_object *excel_object_new_format(zend_class_entry *class_type)
{
	return excel_object_new_format_ex(class_type, NULL);
}

static zend_object *excel_format_object_clone(zend_object *object)
{
	zend_object *new_ov;
	FormatHandle format;

	excel_format_object *new_obj = NULL;
	excel_format_object *old_obj = php_excel_format_object_fetch_object(object);
	new_ov = excel_object_new_format_ex(old_obj->std.ce, &new_obj);

	if (!old_obj->book || !old_obj->format) {
		zend_throw_exception(NULL, "Cannot clone: parent ExcelBook is no longer initialized", 0);
		return new_ov;
	}

	if (!php_excel_check_book_generation_throw(&old_obj->parent, old_obj->book_generation)) {
		return new_ov;
	}

	format = xlBookAddFormat(old_obj->book, old_obj->format);
	if (!format) {
		zend_throw_exception(NULL, "Failed to copy format", 0);
	} else {
		new_obj->book = old_obj->book;
		new_obj->format = format;
		EXCEL_INIT_PARENT(new_obj, &old_obj->parent);
	}

	zend_objects_clone_members(&new_obj->std, &old_obj->std);

	return new_ov;
}

EXCEL_CHILD_FREE_STORAGE(autofilter)
EXCEL_CHILD_OBJECT_NEW(autofilter)

EXCEL_CHILD_FREE_STORAGE(filtercolumn)
EXCEL_CHILD_OBJECT_NEW(filtercolumn)

EXCEL_CHILD_FREE_STORAGE(richstring)
EXCEL_CHILD_OBJECT_NEW(richstring)

EXCEL_CHILD_FREE_STORAGE(formcontrol)
EXCEL_CHILD_OBJECT_NEW(formcontrol)

EXCEL_CHILD_FREE_STORAGE(conditionalformat)
EXCEL_CHILD_OBJECT_NEW(conditionalformat)

EXCEL_CHILD_FREE_STORAGE(conditionalformatting)
EXCEL_CHILD_OBJECT_NEW(conditionalformatting)

EXCEL_CHILD_FREE_STORAGE(coreproperties)
EXCEL_CHILD_OBJECT_NEW(coreproperties)

EXCEL_CHILD_FREE_STORAGE(table)
EXCEL_CHILD_OBJECT_NEW(table)

/* Child wrappers hold a strong zval reference to their parent book/sheet
 * (excel_<c_name>_object.parent) that the std object handlers do not expose.
 * Without a get_gc that reports it, the cycle collector cannot see a
 * user-formed cycle through that hidden edge (e.g. $book->x = $sheet, where
 * $sheet->parent points back at $book), so gc_collect_cycles() leaks it.
 * These handlers surface the parent zval alongside the object's own
 * properties. */
#define EXCEL_GET_GC_FN(c_name) \
	static HashTable *excel_ ## c_name ## _get_gc(zend_object *object, zval **table, int *n) \
	{ \
		excel_ ## c_name ## _object *intern = php_excel_ ## c_name ## _object_fetch_object(object); \
		zend_get_gc_buffer *gc_buffer = zend_get_gc_buffer_create(); \
		zend_get_gc_buffer_add_zval(gc_buffer, &intern->parent); \
		zend_get_gc_buffer_use(gc_buffer, table, n); \
		return zend_std_get_properties(object); \
	}

EXCEL_GET_GC_FN(sheet)
EXCEL_GET_GC_FN(font)
EXCEL_GET_GC_FN(format)
EXCEL_GET_GC_FN(autofilter)
EXCEL_GET_GC_FN(filtercolumn)
EXCEL_GET_GC_FN(richstring)
EXCEL_GET_GC_FN(formcontrol)
EXCEL_GET_GC_FN(conditionalformat)
EXCEL_GET_GC_FN(conditionalformatting)
EXCEL_GET_GC_FN(coreproperties)
EXCEL_GET_GC_FN(table)

/* Attach the parent-aware get_gc after class registration (the shared
 * REGISTER_EXCEL_CLASS memcpy'd the std handlers, which is correct for Book
 * since it has no parent). */
#define EXCEL_SET_GC(c_name) \
	excel_object_handlers_ ## c_name .get_gc = excel_ ## c_name ## _get_gc

#define EXCEL_METHOD(class_name, function_name) \
	PHP_METHOD(Excel ## class_name, function_name)

/* String / size validators return true when the value is acceptable. Callers
 * (or the EXCEL_* shims below) map false onto RETURN_FALSE. */

static zend_always_inline bool php_excel_non_empty_string(zend_string *s)
{
	return s && ZSTR_LEN(s) >= 1;
}

/* PHP zend_string is binary-safe; libxl C ABI is NUL-terminated.
 * Reject embedded NUL bytes so libxl never sees a silently-truncated
 * value while the application-side validator saw the full string. */
static zend_always_inline bool php_excel_nul_safe_string(zend_string *s)
{
	if (s && ZSTR_LEN(s) > 0 && memchr(ZSTR_VAL(s), 0, ZSTR_LEN(s))) {
		php_error_docref(NULL, E_WARNING, "String must not contain NUL bytes");
		return false;
	}
	return true;
}

/* libxl raw load/picture APIs take an `unsigned` size; zend_string length is
 * size_t. Reject buffers over UINT_MAX before the narrowing conversion so a
 * >4 GiB payload cannot silently truncate. Matches the loadPartially guard. */
static zend_always_inline bool php_excel_validate_uint_size(zend_string *s)
{
	if (ZSTR_LEN(s) > UINT_MAX) {
		php_error_docref(NULL, E_WARNING, "Data string too large");
		return false;
	}
	return true;
}

#define EXCEL_NON_EMPTY_STRING(string_zval) \
	if (!php_excel_non_empty_string(string_zval)) { \
		RETURN_FALSE; \
	}

#define EXCEL_NUL_SAFE_STRING(string_zval) \
	if (!php_excel_nul_safe_string(string_zval)) { \
		RETURN_FALSE; \
	}

#define EXCEL_VALIDATE_UINT_SIZE(string_zval) \
	if (!php_excel_validate_uint_size(string_zval)) { \
		RETURN_FALSE; \
	}

typedef enum {
	PHP_EXCEL_STREAM_READ_OK,
	PHP_EXCEL_STREAM_READ_ERROR,
	PHP_EXCEL_STREAM_READ_TOO_LARGE
} php_excel_stream_read_status;

static zend_object *php_excel_suspend_exception(void)
{
	zend_object *exception = EG(exception);

	EG(exception) = NULL;
	return exception;
}

static void php_excel_restore_exception(zend_object *exception)
{
	if (!exception) {
		return;
	}
	if (EG(exception)) {
		zend_object *cleanup_exception = EG(exception);
		EG(exception) = NULL;
		zend_exception_set_previous(exception, cleanup_exception);
	}
	EG(exception) = exception;
}

static int php_excel_stream_close_preserving_exception(php_stream *stream)
{
	zend_object *exception = php_excel_suspend_exception();
	int result = php_stream_close(stream);

	php_excel_restore_exception(exception);
	return result;
}

/* Seekable streams are rejected from stat before allocating. Wrappers without
 * stat support are capped during the read. A zero-byte read is EOF only when
 * the wrapper reports EOF; otherwise it is an I/O failure. */
static zend_string *php_excel_stream_read_all(php_stream *stream, php_excel_stream_read_status *status)
{
	char buffer[65536];
	smart_str contents = {0};
	size_t total = 0;
	ssize_t read_len;
	php_stream_statbuf ssb;
	int close_result;

	*status = PHP_EXCEL_STREAM_READ_OK;
	if (php_stream_stat(stream, &ssb) == 0 && ssb.sb.st_size > 0
	    && (zend_ulong) ssb.sb.st_size >= UINT_MAX) {
		*status = PHP_EXCEL_STREAM_READ_TOO_LARGE;
		goto close_stream;
	}
	if (EG(exception)) {
		*status = PHP_EXCEL_STREAM_READ_ERROR;
		goto close_stream;
	}

	for (;;) {
		read_len = php_stream_read(stream, buffer, sizeof(buffer));
		if (EG(exception) || read_len < 0) {
			*status = PHP_EXCEL_STREAM_READ_ERROR;
			break;
		}
		if (read_len == 0) {
			if (!php_stream_eof(stream) || EG(exception)) {
				*status = PHP_EXCEL_STREAM_READ_ERROR;
			}
			break;
		}
		if ((size_t) read_len > ((size_t) UINT_MAX - 1) - total) {
			*status = PHP_EXCEL_STREAM_READ_TOO_LARGE;
			break;
		}
		smart_str_appendl(&contents, buffer, (size_t) read_len);
		total += (size_t) read_len;
	}

close_stream:
	close_result = php_excel_stream_close_preserving_exception(stream);
	if (close_result != 0 || EG(exception)) {
		*status = PHP_EXCEL_STREAM_READ_ERROR;
	}
	if (*status != PHP_EXCEL_STREAM_READ_OK) {
		smart_str_free(&contents);
		return NULL;
	}
	smart_str_0(&contents);
	return contents.s;
}

static bool php_excel_wrapper_supports_atomic_save(zend_string *url)
{
	php_stream_wrapper *wrapper = php_stream_locate_url_wrapper(ZSTR_VAL(url), NULL, 0);

	return wrapper && wrapper->wops && wrapper->wops->rename && wrapper->wops->unlink;
}

/* Claim an unused sibling URL of `target` by creating it exclusively, so a
 * pre-existing file or a planted symlink cannot capture the staged write. On
 * success the caller owns both the returned name, which it must unlink unless
 * the rename succeeds, and the open stream. Returns NULL after 8 attempts. */
static zend_string *php_excel_reserve_staging_url(zend_string *target, php_stream **stream)
{
	int attempt;

	*stream = NULL;

	for (attempt = 0; attempt < 8; attempt++) {
		unsigned char random_bytes[8];
		char random_suffix[17];
		zend_string *tmp_name;
		int i;

		if (php_random_bytes_silent(random_bytes, sizeof(random_bytes)) == FAILURE) {
			return NULL;
		}
		for (i = 0; i < (int) sizeof(random_bytes); i++) {
			snprintf(random_suffix + (i * 2), 3, "%02x", random_bytes[i]);
		}
		tmp_name = zend_strpprintf(0, "%s.%s.tmp", ZSTR_VAL(target), random_suffix);

		*stream = php_stream_open_wrapper(ZSTR_VAL(tmp_name), "xb", 0, NULL);
		if (*stream) {
			return tmp_name;
		}
		zend_string_release(tmp_name);
	}
	return NULL;
}

/* Carry the destination's permission bits onto the staged file so an atomic
 * replace does not widen a deliberately restricted target. A missing
 * destination keeps the umask-derived mode the reservation created. */
static void php_excel_copy_destination_mode(zend_string *destination, zend_string *staged)
{
	zend_stat_t sb;

	if (VCWD_STAT(ZSTR_VAL(destination), &sb) != 0) {
		return;
	}
	VCWD_CHMOD(ZSTR_VAL(staged), sb.st_mode & 07777);
}

static bool php_excel_wrapper_rename(zend_string *from, zend_string *to)
{
	php_stream_wrapper *wrapper = php_stream_locate_url_wrapper(ZSTR_VAL(from), NULL, 0);

	if (!wrapper || !wrapper->wops || !wrapper->wops->rename) {
		return false;
	}
	return wrapper->wops->rename(wrapper, ZSTR_VAL(from), ZSTR_VAL(to), REPORT_ERRORS, NULL);
}

static bool php_excel_wrapper_unlink_preserving_exception(zend_string *url)
{
	zend_object *exception = php_excel_suspend_exception();
	php_stream_wrapper *wrapper = php_stream_locate_url_wrapper(ZSTR_VAL(url), NULL, 0);
	bool result = false;

	if (wrapper && wrapper->wops && wrapper->wops->unlink) {
		result = wrapper->wops->unlink(wrapper, ZSTR_VAL(url), REPORT_ERRORS, NULL);
	}
	php_excel_restore_exception(exception);
	return result;
}

/* libxl APIs take int for row/col/dimension args; zend_long is 64-bit.
 * Reject out-of-int-range values before the implicit narrowing cast. */
static zend_always_inline bool php_excel_validate_int_range(zend_long arg)
{
	if (arg < 0 || arg > INT_MAX) {
		php_error_docref(NULL, E_WARNING, "Argument out of int range");
		return false;
	}
	return true;
}

/* RGB component setters take an int per channel; only 0-255 is meaningful.
 * Out-of-range values silently corrupt the colour, so reject them. */
static zend_always_inline bool php_excel_validate_rgb(zend_long arg)
{
	if (arg < 0 || arg > 255) {
		php_error_docref(NULL, E_WARNING, "RGB component out of range (0-255)");
		return false;
	}
	return true;
}

static zend_always_inline bool php_excel_validate_finite(double arg)
{
	if (!zend_finite(arg)) {
		php_error_docref(NULL, E_WARNING, "Floating-point argument must be finite");
		return false;
	}
	return true;
}

/* Named-range scope crosses into a libxl int. SCOPE_UNDEFINED (-2) and
 * SCOPE_WORKBOOK (-1) are valid sentinels; a real sheet scope is 0..INT_MAX.
 * Reject anything outside [-2, INT_MAX] so a 64-bit value can't alias a
 * sentinel or a different sheet after narrowing. */
static zend_always_inline bool php_excel_validate_scope(zend_long arg)
{
	if (arg < SCOPE_UNDEFINED || arg > INT_MAX) {
		php_error_docref(NULL, E_WARNING, "Scope id out of range");
		return false;
	}
	return true;
}

#define EXCEL_VALIDATE_INT_RANGE(arg) \
	if (!php_excel_validate_int_range((arg))) { \
		RETURN_FALSE; \
	}

#define EXCEL_VALIDATE_RGB(arg) \
	if (!php_excel_validate_rgb((arg))) { \
		RETURN_FALSE; \
	}

#define EXCEL_VALIDATE_FINITE(arg) \
	if (!php_excel_validate_finite((arg))) { \
		RETURN_FALSE; \
	}

#define EXCEL_VALIDATE_SCOPE(arg) \
	if (!php_excel_validate_scope((arg))) { \
		RETURN_FALSE; \
	}

/* Coordinate validation for sheet read/write paths. Limits depend on book
 * type: XLSX is 1048576 rows x 16384 cols; XLS is 65536 rows x 256 cols.
 * libxl write paths reject out-of-range writes themselves, but read paths
 * silently return empty cells, so the check must run before either. */
#define EXCEL_MAX_ROW_XLSX 1048575
#define EXCEL_MAX_COL_XLSX 16383
#define EXCEL_MAX_ROW_XLS  65535
#define EXCEL_MAX_COL_XLS  255
#define PHP_EXCEL_MAX_RANGE_CELLS 1048576

/* Unresolvable parent falls back to XLS limits (same as the prior macros). */
static zend_always_inline void php_excel_book_coord_limits(
	excel_book_object *vb, zend_long *maxr, zend_long *maxc)
{
	if (vb && vb->is_xlsx) {
		*maxr = EXCEL_MAX_ROW_XLSX;
		*maxc = EXCEL_MAX_COL_XLSX;
	} else {
		*maxr = EXCEL_MAX_ROW_XLS;
		*maxc = EXCEL_MAX_COL_XLS;
	}
}

static zend_always_inline bool php_excel_validate_row_col_pr(
	zend_long r, zend_long c, excel_book_object *vb)
{
	zend_long maxr, maxc;

	php_excel_book_coord_limits(vb, &maxr, &maxc);
	if (r < 0 || r > maxr || c < 0 || c > maxc) {
		php_error_docref(NULL, E_WARNING,
			"Invalid coordinates: row=" ZEND_LONG_FMT ", column=" ZEND_LONG_FMT,
			r, c);
		return false;
	}
	return true;
}

static zend_always_inline bool php_excel_validate_row_col(
	zend_long r, zend_long c, zval *parent_zv)
{
	return php_excel_validate_row_col_pr(r, c, php_excel_resolve_book_obj(parent_zv));
}

/* Row-range validation for insertRow/removeRow (both args are rows). */
static zend_always_inline bool php_excel_validate_row_range(
	zend_long rfirst, zend_long rlast, zval *parent_zv)
{
	excel_book_object *vb = php_excel_resolve_book_obj(parent_zv);
	zend_long maxr, maxc;

	php_excel_book_coord_limits(vb, &maxr, &maxc);
	(void)maxc;
	if (rfirst < 0 || rfirst > maxr || rlast < 0 || rlast > maxr) {
		php_error_docref(NULL, E_WARNING,
			"Invalid row range: first=" ZEND_LONG_FMT ", last=" ZEND_LONG_FMT,
			rfirst, rlast);
		return false;
	}
	if (rfirst > rlast) {
		php_error_docref(NULL, E_WARNING,
			"The range row start cannot be greater than row end.");
		return false;
	}
	return true;
}

/* Column-range validation for insertCol/removeCol. */
static zend_always_inline bool php_excel_validate_col_range(
	zend_long cfirst, zend_long clast, zval *parent_zv)
{
	excel_book_object *vb = php_excel_resolve_book_obj(parent_zv);
	zend_long maxr, maxc;

	php_excel_book_coord_limits(vb, &maxr, &maxc);
	(void)maxr;
	if (cfirst < 0 || cfirst > maxc || clast < 0 || clast > maxc) {
		php_error_docref(NULL, E_WARNING,
			"Invalid column range: first=" ZEND_LONG_FMT ", last=" ZEND_LONG_FMT,
			cfirst, clast);
		return false;
	}
	if (cfirst > clast) {
		php_error_docref(NULL, E_WARNING,
			"The range column start cannot be greater than column end.");
		return false;
	}
	return true;
}

/* Single-axis validators (colWidth/rowHeight/setRowHidden/...). */
static zend_always_inline bool php_excel_validate_row(zend_long r, zval *parent_zv)
{
	excel_book_object *vb = php_excel_resolve_book_obj(parent_zv);
	zend_long maxr, maxc;

	php_excel_book_coord_limits(vb, &maxr, &maxc);
	(void)maxc;
	if (r < 0 || r > maxr) {
		php_error_docref(NULL, E_WARNING, "Invalid row: " ZEND_LONG_FMT, r);
		return false;
	}
	return true;
}

static zend_always_inline bool php_excel_validate_col(zend_long c, zval *parent_zv)
{
	excel_book_object *vb = php_excel_resolve_book_obj(parent_zv);
	zend_long maxr, maxc;

	php_excel_book_coord_limits(vb, &maxr, &maxc);
	(void)maxr;
	if (c < 0 || c > maxc) {
		php_error_docref(NULL, E_WARNING, "Invalid column: " ZEND_LONG_FMT, c);
		return false;
	}
	return true;
}

#define EXCEL_VALIDATE_ROW_COL(r, c, parent_zv) \
	do { \
		if (!php_excel_validate_row_col((r), (c), (parent_zv))) { \
			RETURN_FALSE; \
		} \
	} while (0)

#define EXCEL_VALIDATE_ROW_COL_PR(r, c, vb) \
	do { \
		if (!php_excel_validate_row_col_pr((r), (c), (vb))) { \
			RETURN_FALSE; \
		} \
	} while (0)

#define EXCEL_VALIDATE_ROW_RANGE(rfirst, rlast, parent_zv) \
	do { \
		if (!php_excel_validate_row_range((rfirst), (rlast), (parent_zv))) { \
			RETURN_FALSE; \
		} \
	} while (0)

#define EXCEL_VALIDATE_COL_RANGE(cfirst, clast, parent_zv) \
	do { \
		if (!php_excel_validate_col_range((cfirst), (clast), (parent_zv))) { \
			RETURN_FALSE; \
		} \
	} while (0)

#define EXCEL_VALIDATE_ROW(r, parent_zv) \
	do { \
		if (!php_excel_validate_row((r), (parent_zv))) { \
			RETURN_FALSE; \
		} \
	} while (0)

#define EXCEL_VALIDATE_COL(c, parent_zv) \
	do { \
		if (!php_excel_validate_col((c), (parent_zv))) { \
			RETURN_FALSE; \
		} \
	} while (0)

#if LIBXL_VERSION >= 0x05000000
static inline int php_excel_validate_partial_load_args(zend_long sheet_index, zend_long row_first, zend_long row_last)
{
	if (sheet_index < 0 || sheet_index > INT_MAX) {
		php_error_docref(NULL, E_WARNING, "Sheet index out of range");
		return 0;
	}
	if (row_first < 0 || row_first > INT_MAX || row_last < 0 || row_last > INT_MAX) {
		php_error_docref(NULL, E_WARNING, "Row range out of int range");
		return 0;
	}
	if (row_last < row_first) {
		php_error_docref(NULL, E_WARNING, "Starting row is greater than ending row");
		return 0;
	}
	return 1;
}
#endif

#define PE_RETURN_IS_LONG RETURN_LONG
#define PE_RETURN_IS_BOOL RETURN_BOOL
#define PE_RETURN_IS_DOUBLE RETURN_DOUBLE
#define PE_RETURN_IS_STRING(data) if (data) { RETURN_STRING((char *)data); } else { RETURN_NULL(); }


/* No-arg Book method: fetch handle, call xlBook{func_name}, return. */
#define PHP_EXCEL_BOOK_INFO(func_name, type) \
{ \
	BookHandle book; \
	zval *object = ZEND_THIS; \
	ZEND_PARSE_PARAMETERS_NONE(); \
	BOOK_FROM_OBJECT(book, object); \
	PE_RETURN_ ## type (xlBook ## func_name (book)); \
}

/* {{{ proto bool ExcelBook::requiresKey()
	true if license key is required. */
EXCEL_METHOD(Book, requiresKey)
{
	ZEND_PARSE_PARAMETERS_NONE();
#if defined(HAVE_LIBXL_SETKEY)
	RETURN_BOOL(1);
#else
	RETURN_BOOL(0);
#endif
}
/* }}} */

/* {{{ proto bool ExcelBook::load(string data)
	Load Excel data string. */
EXCEL_METHOD(Book, load)
{
	BookHandle book;
	zval *object = ZEND_THIS;
	zend_string *data_zs = NULL;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "S", &data_zs) == FAILURE) {
		RETURN_FALSE;
	}

	EXCEL_NON_EMPTY_STRING(data_zs)
	EXCEL_VALIDATE_UINT_SIZE(data_zs)

	BOOK_FROM_OBJECT(book, object);

	if (!php_excel_book_reset_state(object)) {
		RETURN_FALSE;
	}
	if (!xlBookLoadRaw(book, ZSTR_VAL(data_zs), ZSTR_LEN(data_zs))) {
		php_error_docref(NULL, E_WARNING, "Failed to load workbook: %s", xlBookErrorMessage(book));
		RETURN_FALSE;
	}
	RETURN_TRUE;
}
/* }}} */

/* {{{ proto bool ExcelBook::loadFile(string filename)
	Load Excel from file. */
EXCEL_METHOD(Book, loadFile)
{
	BookHandle book;
	zval *object = ZEND_THIS;
	zend_string *filename_zs = NULL;
	php_stream *stream;
	zend_string *contents;
	php_excel_stream_read_status stream_status;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "S", &filename_zs) == FAILURE) {
		RETURN_FALSE;
	}

	EXCEL_NON_EMPTY_STRING(filename_zs)
	EXCEL_NUL_SAFE_STRING(filename_zs)

	/* With open_basedir active, PHP must perform the actual open. A separate
	 * policy check followed by a libxl path open has a rename/symlink race. */
	if (!strstr(ZSTR_VAL(filename_zs), "://")
	    && (!PG(open_basedir) || !*PG(open_basedir))) {
		BOOK_FROM_OBJECT(book, object);
		if (!php_excel_book_reset_state(object)) {
			RETURN_FALSE;
		}
		if (!xlBookLoad(book, ZSTR_VAL(filename_zs))) {
			php_error_docref(NULL, E_WARNING, "Failed to load workbook: %s", xlBookErrorMessage(book));
			RETURN_FALSE;
		}
		RETURN_TRUE;
	}
	if (!strstr(ZSTR_VAL(filename_zs), "://")
	    && PG(open_basedir) && *PG(open_basedir)
	    && php_check_open_basedir(ZSTR_VAL(filename_zs))) {
		RETURN_FALSE;
	}

	stream = php_stream_open_wrapper(ZSTR_VAL(filename_zs), "rb", REPORT_ERRORS, NULL);

	if (!stream) {
		RETURN_FALSE;
	}

	contents = php_excel_stream_read_all(stream, &stream_status);
	if (EG(exception)) {
		RETURN_THROWS();
	}
	if (stream_status == PHP_EXCEL_STREAM_READ_TOO_LARGE) {
		php_error_docref(NULL, E_WARNING, "Source file too large");
		RETURN_FALSE;
	}
	if (stream_status == PHP_EXCEL_STREAM_READ_ERROR) {
		php_error_docref(NULL, E_WARNING, "Failed to read source stream");
		RETURN_FALSE;
	}

	if (!contents) {
		php_error_docref(NULL, E_WARNING, "Source file is empty");
		RETURN_FALSE;
	}

	if (ZSTR_LEN(contents) < 1) {
		php_error_docref(NULL, E_WARNING, "Source file is empty");
		zend_string_release(contents);
		RETURN_FALSE;
	}

	if (ZSTR_LEN(contents) >= UINT_MAX) {
		php_error_docref(NULL, E_WARNING, "Data string too large");
		zend_string_release(contents);
		RETURN_FALSE;
	}

	/* Stream callbacks may reconstruct the receiver while the file is read.
	 * Fetch the current handle only after every open/read/close callback. */
	BOOK_FROM_OBJECT_RELEASE_STR(book, object, contents);
	if (!php_excel_book_reset_state(object)) {
		zend_string_release(contents);
		RETURN_FALSE;
	}
	if (!xlBookLoadRaw(book, ZSTR_VAL(contents), ZSTR_LEN(contents))) {
		php_error_docref(NULL, E_WARNING, "Failed to load workbook: %s", xlBookErrorMessage(book));
		zend_string_release(contents);
		RETURN_FALSE;
	}
	zend_string_release(contents);
	RETURN_TRUE;
}
/* }}} */

#if LIBXL_VERSION >= 0x05000000
/* {{{ proto bool ExcelBook::loadPartially(string data, int sheet_index, int row_first, int row_last [, bool keep_all_sheets])
	Load a row slice from Excel data string. */
EXCEL_METHOD(Book, loadPartially)
{
	BookHandle book;
	zval *object = ZEND_THIS;
	zend_string *data_zs = NULL;
	zend_long sheet_index, row_first, row_last;
	bool keep_all_sheets = 0;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "Slll|b", &data_zs, &sheet_index, &row_first, &row_last, &keep_all_sheets) == FAILURE) {
		RETURN_FALSE;
	}

	EXCEL_NON_EMPTY_STRING(data_zs)

	if (!php_excel_validate_partial_load_args(sheet_index, row_first, row_last)) {
		RETURN_FALSE;
	}
	if (ZSTR_LEN(data_zs) > UINT_MAX) {
		php_error_docref(NULL, E_WARNING, "Data string too large");
		RETURN_FALSE;
	}

	BOOK_FROM_OBJECT(book, object);

	if (!php_excel_book_reset_state(object)) {
		RETURN_FALSE;
	}
	if (!xlBookLoadRawPartially(book, ZSTR_VAL(data_zs), (unsigned) ZSTR_LEN(data_zs), (int) sheet_index, (int) row_first, (int) row_last, keep_all_sheets)) {
		php_error_docref(NULL, E_WARNING, "Failed to load workbook: %s", xlBookErrorMessage(book));
		RETURN_FALSE;
	}
	RETURN_TRUE;
}
/* }}} */

/* {{{ proto bool ExcelBook::loadFilePartially(string filename, int sheet_index, int row_first, int row_last [, bool keep_all_sheets])
	Load a row slice from Excel file. */
EXCEL_METHOD(Book, loadFilePartially)
{
	BookHandle book;
	zval *object = ZEND_THIS;
	zend_string *filename_zs = NULL;
	zend_long sheet_index, row_first, row_last;
	bool keep_all_sheets = 0;
	php_stream *stream;
	zend_string *contents;
	php_excel_stream_read_status stream_status;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "Slll|b", &filename_zs, &sheet_index, &row_first, &row_last, &keep_all_sheets) == FAILURE) {
		RETURN_FALSE;
	}

	EXCEL_NON_EMPTY_STRING(filename_zs)
	EXCEL_NUL_SAFE_STRING(filename_zs)

	if (!php_excel_validate_partial_load_args(sheet_index, row_first, row_last)) {
		RETURN_FALSE;
	}

	if (!strstr(ZSTR_VAL(filename_zs), "://")
	    && (!PG(open_basedir) || !*PG(open_basedir))) {
		BOOK_FROM_OBJECT(book, object);
		if (!php_excel_book_reset_state(object)) {
			RETURN_FALSE;
		}
		if (!xlBookLoadPartially(book, ZSTR_VAL(filename_zs), (int) sheet_index, (int) row_first, (int) row_last, keep_all_sheets)) {
			php_error_docref(NULL, E_WARNING, "Failed to load workbook: %s", xlBookErrorMessage(book));
			RETURN_FALSE;
		}
		RETURN_TRUE;
	}
	if (!strstr(ZSTR_VAL(filename_zs), "://")
	    && PG(open_basedir) && *PG(open_basedir)
	    && php_check_open_basedir(ZSTR_VAL(filename_zs))) {
		RETURN_FALSE;
	}

	stream = php_stream_open_wrapper(ZSTR_VAL(filename_zs), "rb", REPORT_ERRORS, NULL);
	if (!stream) {
		RETURN_FALSE;
	}

	contents = php_excel_stream_read_all(stream, &stream_status);
	if (EG(exception)) {
		RETURN_THROWS();
	}
	if (stream_status == PHP_EXCEL_STREAM_READ_TOO_LARGE) {
		php_error_docref(NULL, E_WARNING, "Source file too large");
		RETURN_FALSE;
	}
	if (stream_status == PHP_EXCEL_STREAM_READ_ERROR) {
		php_error_docref(NULL, E_WARNING, "Failed to read source stream");
		RETURN_FALSE;
	}
	if (!contents) {
		php_error_docref(NULL, E_WARNING, "Source file is empty");
		RETURN_FALSE;
	}
	if (ZSTR_LEN(contents) < 1) {
		php_error_docref(NULL, E_WARNING, "Source file is empty");
		zend_string_release(contents);
		RETURN_FALSE;
	}
	if (ZSTR_LEN(contents) >= UINT_MAX) {
		php_error_docref(NULL, E_WARNING, "Source file too large");
		zend_string_release(contents);
		RETURN_FALSE;
	}

	BOOK_FROM_OBJECT_RELEASE_STR(book, object, contents);
	if (!php_excel_book_reset_state(object)) {
		zend_string_release(contents);
		RETURN_FALSE;
	}
	if (!xlBookLoadRawPartially(book, ZSTR_VAL(contents), (unsigned) ZSTR_LEN(contents), (int) sheet_index, (int) row_first, (int) row_last, keep_all_sheets)) {
		zend_string_release(contents);
		php_error_docref(NULL, E_WARNING, "Failed to load workbook: %s", xlBookErrorMessage(book));
		RETURN_FALSE;
	}
	zend_string_release(contents);
	RETURN_TRUE;
}
/* }}} */

/* {{{ proto bool ExcelBook::loadFileWithoutEmptyCells(string filename)
	Load Excel from file without empty cells. */
EXCEL_METHOD(Book, loadFileWithoutEmptyCells)
{
	BookHandle book;
	zval *object = ZEND_THIS;
	zend_string *filename_zs = NULL;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "S", &filename_zs) == FAILURE) {
		RETURN_FALSE;
	}

	EXCEL_NON_EMPTY_STRING(filename_zs)
	EXCEL_NUL_SAFE_STRING(filename_zs)

	if (strstr(ZSTR_VAL(filename_zs), "://")) {
		php_error_docref(NULL, E_WARNING, "Stream wrappers are not supported by loadFileWithoutEmptyCells");
		RETURN_FALSE;
	}
	if (php_check_open_basedir(ZSTR_VAL(filename_zs))) {
		RETURN_FALSE;
	}
	if (PG(open_basedir) && *PG(open_basedir)) {
		php_error_docref(NULL, E_WARNING, "loadFileWithoutEmptyCells is not available while open_basedir is active");
		RETURN_FALSE;
	}

	BOOK_FROM_OBJECT(book, object);

	if (!php_excel_book_reset_state(object)) {
		RETURN_FALSE;
	}
	if (!xlBookLoadWithoutEmptyCells(book, ZSTR_VAL(filename_zs))) {
		php_error_docref(NULL, E_WARNING, "Failed to load workbook: %s", xlBookErrorMessage(book));
		RETURN_FALSE;
	}
	RETURN_TRUE;
}
/* }}} */
#endif

/* Stage a plain local save through libxl's own pathname writer and rename it
 * into place. xlBookSave() streams the archive to disk, where xlBookSaveRaw()
 * would first materialize all of it in memory, so this keeps the save atomic
 * without a peak-memory cost that scales with the workbook. Only reachable
 * with open_basedir inactive; otherwise PHP has to perform the open itself. */
static void php_excel_save_local_atomic(INTERNAL_FUNCTION_PARAMETERS, BookHandle book, zend_string *filename_zs)
{
	php_stream *stream;
	zend_string *tmp_name = php_excel_reserve_staging_url(filename_zs, &stream);

	if (!tmp_name) {
		php_error_docref(NULL, E_WARNING, "Failed to save workbook: could not create an exclusive temporary file for atomic save");
		RETURN_FALSE;
	}
	/* libxl reopens the path by name, so hand the reservation over to it. */
	php_stream_close(stream);

	if (!xlBookSave(book, ZSTR_VAL(tmp_name))) {
		php_error_docref(NULL, E_WARNING, "Failed to save workbook: %s", xlBookErrorMessage(book));
		php_excel_wrapper_unlink_preserving_exception(tmp_name);
		zend_string_release(tmp_name);
		RETURN_FALSE;
	}

	php_excel_copy_destination_mode(filename_zs, tmp_name);

	if (!php_excel_wrapper_rename(tmp_name, filename_zs)) {
		php_excel_wrapper_unlink_preserving_exception(tmp_name);
		zend_string_release(tmp_name);
		php_error_docref(NULL, E_WARNING, "Could not replace destination with completed temporary file; destination left unchanged");
		RETURN_FALSE;
	}

	zend_string_release(tmp_name);
	RETURN_TRUE;
}

/* {{{ proto mixed ExcelBook::save([string filename])
	Save Excel file. */
EXCEL_METHOD(Book, save)
{
	BookHandle book;
	zval *object = ZEND_THIS;
	zend_string *filename_zs = NULL;
	unsigned int len = 0;
	char *contents = NULL;
	bool has_path, is_local_path, basedir_active;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "|S", &filename_zs) == FAILURE) {
		RETURN_FALSE;
	}

	has_path = filename_zs && ZSTR_LEN(filename_zs) > 0;
	if (has_path) {
		EXCEL_NUL_SAFE_STRING(filename_zs)
	}
	is_local_path = has_path && !strstr(ZSTR_VAL(filename_zs), "://");
	basedir_active = PG(open_basedir) && *PG(open_basedir);

	BOOK_FROM_OBJECT(book, object);

	if (is_local_path && !basedir_active) {
		php_excel_save_local_atomic(INTERNAL_FUNCTION_PARAM_PASSTHRU, book, filename_zs);
		return;
	}

	/* Remaining path-based saves go through SaveRaw + PHP streams so staging
	 * and rename can preserve the destination on a short write. open_basedir
	 * is enforced by the stream open itself. */
	if (is_local_path && basedir_active && php_check_open_basedir(ZSTR_VAL(filename_zs))) {
		RETURN_FALSE;
	}

	if (!xlBookSaveRaw(book, (const char **) &contents, &len)) {
		php_error_docref(NULL, E_WARNING, "Failed to save workbook: %s", xlBookErrorMessage(book));
		RETURN_FALSE;
	}

	if (has_path) {
		ssize_t numbytes;
		php_stream *stream;
		excel_book_object *book_obj = Z_EXCEL_BOOK_OBJ_P(object);
		zend_string *owned_contents;

		/* Borrow the LibXL buffer only for the duration of the copy into a
		 * PHP string so a GC-driven load/clear cannot free it mid-memcpy. */
		book_obj->native_buffer_borrowed = true;
		owned_contents = zend_string_init(contents, len, 0);
		book_obj->native_buffer_borrowed = false;

		/* If the destination wrapper exposes rename+unlink, stage the full
		 * buffer to a sibling temp URL and rename it into place. A short
		 * write then fails against the temp, leaving the caller's existing
		 * file untouched, instead of truncating the destination up front. */
		if (php_excel_wrapper_supports_atomic_save(filename_zs)) {
			zend_string *tmp_name = php_excel_reserve_staging_url(filename_zs, &stream);
			int flush_result = 0;
			int close_result;

			if (!tmp_name) {
				zend_string_release(owned_contents);
				php_error_docref(NULL, E_WARNING, "Failed to save workbook: could not create an exclusive temporary file for atomic save");
				RETURN_FALSE;
			}

			numbytes = php_stream_write(stream, ZSTR_VAL(owned_contents), ZSTR_LEN(owned_contents));
			if (!EG(exception) && numbytes == (ssize_t) ZSTR_LEN(owned_contents)) {
				flush_result = php_stream_flush(stream);
			}
			close_result = php_excel_stream_close_preserving_exception(stream);

			if (numbytes != (ssize_t) ZSTR_LEN(owned_contents)
			    || flush_result != 0 || close_result != 0 || EG(exception)) {
				php_excel_wrapper_unlink_preserving_exception(tmp_name);
				zend_string_release(tmp_name);
				zend_string_release(owned_contents);
				if (EG(exception)) {
					RETURN_THROWS();
				}
				if (flush_result != 0 || close_result != 0) {
					php_error_docref(NULL, E_WARNING, "Failed to flush or close completed temporary file; destination left unchanged");
					RETURN_FALSE;
				}
				php_error_docref(NULL, E_WARNING, "Only %zd of %u bytes written, possibly out of free disk space; destination left unchanged", numbytes, len);
				RETURN_FALSE;
			}

			/* No-op for wrapper URLs, which do not stat; this only matters for
			 * the local paths that reach here because open_basedir is active. */
			php_excel_copy_destination_mode(filename_zs, tmp_name);

			if (php_excel_wrapper_rename(tmp_name, filename_zs)) {
				zend_string_release(tmp_name);
				zend_string_release(owned_contents);
				RETURN_TRUE;
			}

			php_excel_wrapper_unlink_preserving_exception(tmp_name);
			zend_string_release(tmp_name);
			if (EG(exception)) {
				zend_string_release(owned_contents);
				RETURN_THROWS();
			}

			/* The wrapper took the staged write but cannot rename it into
			 * place. A user-defined wrapper that omits rename() lands here:
			 * PHP installs a dispatcher in wops for every user wrapper whether
			 * or not the class implements the method, so the capability probe
			 * above cannot tell them apart. The staged file is already gone and
			 * owned_contents still holds the whole workbook, so fall through to
			 * the direct write instead of failing a save such a wrapper used to
			 * complete. */
			php_error_docref(NULL, E_WARNING, "Could not replace destination with the completed temporary file; falling back to a non-atomic direct write");
		}

		/* Wrapper cannot stage/rename: write directly. A short write here is
		 * inherently non-atomic and may leave the target truncated. */
		stream = php_stream_open_wrapper(ZSTR_VAL(filename_zs), "wb", REPORT_ERRORS, NULL);

		if (!stream) {
			zend_string_release(owned_contents);
			php_error_docref(NULL, E_WARNING, "Failed to save workbook: could not open destination for writing");
			RETURN_FALSE;
		}

		{
			int flush_result = 0;
			int close_result;

			numbytes = php_stream_write(stream, ZSTR_VAL(owned_contents), ZSTR_LEN(owned_contents));
			if (!EG(exception) && numbytes == (ssize_t) ZSTR_LEN(owned_contents)) {
				flush_result = php_stream_flush(stream);
			}
			close_result = php_excel_stream_close_preserving_exception(stream);
			if (numbytes != (ssize_t) ZSTR_LEN(owned_contents)
			    || flush_result != 0 || close_result != 0 || EG(exception)) {
				zend_string_release(owned_contents);
				if (EG(exception)) {
					RETURN_THROWS();
				}
				if (flush_result != 0 || close_result != 0) {
					php_error_docref(NULL, E_WARNING, "Failed to flush or close destination stream");
					RETURN_FALSE;
				}
				php_error_docref(NULL, E_WARNING, "Only %zd of %u bytes written, possibly out of free disk space", numbytes, len);
				RETURN_FALSE;
			}
		}
		zend_string_release(owned_contents);
		RETURN_TRUE;
	} else {
		excel_book_object *book_obj = Z_EXCEL_BOOK_OBJ_P(object);
		zend_string *owned;

		book_obj->native_buffer_borrowed = true;
		owned = zend_string_init(contents, len, 0);
		book_obj->native_buffer_borrowed = false;
		RETURN_STR(owned);
	}

}
/* }}} */

/* {{{ proto ExcelSheet ExcelBook::getSheet([int sheet])
	Get an excel sheet. */
EXCEL_METHOD(Book, getSheet)
{
	BookHandle book;
	zval *object = ZEND_THIS;
	zend_long sheet = 0;
	SheetHandle sh;
	excel_sheet_object *fo;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "|l", &sheet) == FAILURE) {
		RETURN_FALSE;
	}

	EXCEL_VALIDATE_INT_RANGE(sheet)

	BOOK_FROM_OBJECT(book, object);

	if (!(sh = xlBookGetSheet(book, sheet))) {
		RETURN_FALSE;
	}

	ZVAL_OBJ(return_value, excel_object_new_sheet(excel_ce_sheet));
	fo = Z_EXCEL_SHEET_OBJ_P(return_value);
	fo->sheet = sh;
	fo->book = book;
	EXCEL_INIT_SHEET_PARENT(fo, object);
}
/* }}} */

/* {{{ proto ExcelSheet ExcelBook::getSheetByName(string name [, bool case_insensitive])
	Get an excel sheet by name. */
EXCEL_METHOD(Book, getSheetByName)
{
	BookHandle book;
	zval *object = ZEND_THIS;
	zend_string *sheet_name_zs = NULL;
	zend_long sheet;
	excel_sheet_object *fo;
	zend_long sheet_count;
	bool case_s = 0;
	const char *s;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "S|b", &sheet_name_zs, &case_s) == FAILURE) {
		RETURN_FALSE;
	}

	EXCEL_NON_EMPTY_STRING(sheet_name_zs)
	EXCEL_NUL_SAFE_STRING(sheet_name_zs)

	BOOK_FROM_OBJECT(book, object);

	sheet_count = xlBookSheetCount(book);
	for(sheet = 0; sheet < sheet_count; sheet++) {
		SheetHandle sh = xlBookGetSheet(book, sheet);
		if (sh) {
			size_t needle_len = ZSTR_LEN(sheet_name_zs);
			s = xlSheetName(sh);
			if (s) {
				size_t hay_len = strlen(s);
				if (hay_len == needle_len &&
				    ((case_s && !zend_binary_strcasecmp(s, hay_len, ZSTR_VAL(sheet_name_zs), needle_len)) ||
				     (!case_s && !memcmp(s, ZSTR_VAL(sheet_name_zs), needle_len)))) {
					ZVAL_OBJ(return_value, excel_object_new_sheet(excel_ce_sheet));
					fo = Z_EXCEL_SHEET_OBJ_P(return_value);
					fo->sheet = sh;
					fo->book = book;
					EXCEL_INIT_SHEET_PARENT(fo, object);
					return;
				}
			}
		}
	}

	RETURN_FALSE;
}
/* }}} */

/* {{{ proto bool ExcelBook::deleteSheet(int sheet)
	Delete an excel sheet. */
EXCEL_METHOD(Book, deleteSheet)
{
	BookHandle book;
	zval *object = ZEND_THIS;
	zend_long sheet;
	int ret;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "l", &sheet) == FAILURE) {
		RETURN_FALSE;
	}

	EXCEL_VALIDATE_INT_RANGE(sheet)

	BOOK_FROM_OBJECT(book, object);

	ret = xlBookDelSheet(book, sheet);
	if (ret) {
		/* The deleted sheet's libxl handle is freed; an existing PHP
		 * wrapper for it now points at freed memory and would crash with
		 * "pure virtual method called" on next use. Bump the sheet topology
		 * generation so sheet-derived wrappers fail the stale-check instead.
		 * Sibling sheets must be re-fetched via getSheet/getSheetByName. */
		php_excel_book_bump_sheet_generation(object);
	}
	RETURN_BOOL(ret);
}
/* }}} */

/* {{{ proto int ExcelBook::activeSheet([int sheet])
	Get or set an active excel sheet. */
EXCEL_METHOD(Book, activeSheet)
{
	BookHandle book;
	zval *object = ZEND_THIS;
	zend_long sheet = -1;
	zend_long res;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "|l", &sheet) == FAILURE) {
		RETURN_FALSE;
	}

	if (sheet < -1 || sheet > INT_MAX) {
		php_error_docref(NULL, E_WARNING, "Argument out of int range");
		RETURN_FALSE;
	}

	BOOK_FROM_OBJECT(book, object);

	if (sheet > -1) {
		xlBookSetActiveSheet(book, sheet);
	}

	res = xlBookActiveSheet(book);
	if (sheet == -1 || res == sheet) {
		RETURN_LONG(res);
	} else {
		RETURN_FALSE;
	}
}
/* }}} */

/* {{{ proto ExcelSheet ExcelBook::addSheet(string name)
	Add an excel sheet. */
EXCEL_METHOD(Book, addSheet)
{
	BookHandle book;
	zval *object = ZEND_THIS;
	SheetHandle sh;
	excel_sheet_object *fo;
	zend_string *name_zs = NULL;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "S", &name_zs) == FAILURE) {
		RETURN_FALSE;
	}

	EXCEL_NON_EMPTY_STRING(name_zs)
	EXCEL_NUL_SAFE_STRING(name_zs)

	BOOK_FROM_OBJECT(book, object);

	sh = xlBookAddSheet(book, ZSTR_VAL(name_zs), 0);
	if (!sh) {
		RETURN_FALSE;
	}

	ZVAL_OBJ(return_value, excel_object_new_sheet(excel_ce_sheet));
	fo = Z_EXCEL_SHEET_OBJ_P(return_value);
	fo->sheet = sh;
	fo->book = book;
	EXCEL_INIT_SHEET_PARENT(fo, object);
}
/* }}} */

/* {{{ proto ExcelSheet ExcelBook::copySheet(string name, int sheet_number)
	Copy an excel sheet. */
EXCEL_METHOD(Book, copySheet)
{
	BookHandle book;
	zval *object = ZEND_THIS;
	SheetHandle sh;
	excel_sheet_object *fo;
	zend_string *name_zs = NULL;
	zend_long num;
	SheetHandle osh;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "Sl", &name_zs, &num) == FAILURE) {
		RETURN_FALSE;
	}

	EXCEL_VALIDATE_INT_RANGE(num)

	EXCEL_NON_EMPTY_STRING(name_zs)
	EXCEL_NUL_SAFE_STRING(name_zs)

	BOOK_FROM_OBJECT(book, object);

	if (!(osh = xlBookGetSheet(book, num))) {
		RETURN_FALSE;
	}
	sh = xlBookAddSheet(book, ZSTR_VAL(name_zs), osh);

	if (!sh) {
		RETURN_FALSE;
	}

	ZVAL_OBJ(return_value, excel_object_new_sheet(excel_ce_sheet));
	fo = Z_EXCEL_SHEET_OBJ_P(return_value);
	fo->sheet = sh;
	fo->book = book;
	EXCEL_INIT_SHEET_PARENT(fo, object);
}
/* }}} */

/* {{{ proto int ExcelBook::sheetCount()
	Get the number of sheets inside a file. */
EXCEL_METHOD(Book, sheetCount)
{
	PHP_EXCEL_BOOK_INFO(SheetCount, IS_LONG)
}
/* }}} */

/* {{{ proto string ExcelBook::getError()
	Get Excel error string. */
EXCEL_METHOD(Book, getError)
{
	BookHandle book;
	zval *object = ZEND_THIS;
	char *err;

	ZEND_PARSE_PARAMETERS_NONE();

	BOOK_FROM_OBJECT(book, object);

	err = (char *)xlBookErrorMessage(book);
	if (err) {
		if (!strcmp(err, "ok")) {
			RETURN_FALSE;
		} else {
			RETURN_STRING(err);
		}
	} else {
		RETURN_STRING("Unknown Error");
	}
}
/* }}} */

/* {{{ proto ExcelFont ExcelBook::addFont([ExcelFont font])
	Add or Copy ExcelFont object. */
EXCEL_METHOD(Book, addFont)
{
	BookHandle book;
	zval *object = ZEND_THIS;
	FontHandle nfont;
	FontHandle font = NULL;
	excel_font_object *fo;
	zval *fob = NULL;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "|O!", &fob, excel_ce_font) == FAILURE) {
		RETURN_FALSE;
	}

	BOOK_FROM_OBJECT(book, object);
	if (fob) {
		FONT_FROM_OBJECT(font, fob);
	}

	nfont = xlBookAddFont(book, font);
	if (!nfont) {
		RETURN_FALSE;
	}

	ZVAL_OBJ(return_value, excel_object_new_font(excel_ce_font));
	fo = Z_EXCEL_FONT_OBJ_P(return_value);
	fo->font = nfont;
	fo->book = book;
	EXCEL_INIT_PARENT(fo, object);
}
/* }}} */

/* {{{ proto ExcelFormat ExcelBook::addFormat([ExcelFormat format])
	Add or Copy ExcelFormat object. */
EXCEL_METHOD(Book, addFormat)
{
	BookHandle book;
	zval *object = ZEND_THIS;
	FormatHandle nformat;
	FormatHandle format = NULL;
	excel_format_object *fo;
	zval *fob = NULL;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "|O!", &fob, excel_ce_format) == FAILURE) {
		RETURN_FALSE;
	}

	BOOK_FROM_OBJECT(book, object);
	if (fob) {
		FORMAT_FROM_OBJECT(format, fob);
	}

	nformat = xlBookAddFormat(book, format);
	if (!nformat) {
		RETURN_FALSE;
	}

	ZVAL_OBJ(return_value, excel_object_new_format(excel_ce_format));
	fo = Z_EXCEL_FORMAT_OBJ_P(return_value);
	fo->format = nformat;
	fo->book = book;
	EXCEL_INIT_PARENT(fo, object);
}
/* }}} */

/* {{{ proto array ExcelBook::getAllFormats()
	Get an array of all ExcelFormat objects used inside a document. */
EXCEL_METHOD(Book, getAllFormats)
{
	BookHandle book;
	zval *object = ZEND_THIS;
	unsigned short fc;
	unsigned short c;

	ZEND_PARSE_PARAMETERS_NONE();

	BOOK_FROM_OBJECT(book, object);

	fc = xlBookFormatSize(book);
	if (!fc) {
		array_init(return_value);
		return;
	}

	array_init_size(return_value, fc);

	for (c = 0; c < fc; c++) {
		FormatHandle format;

		if ((format = xlBookFormat(book, c))) {
			excel_format_object *fo;
			zval value;

			ZVAL_OBJ(&value, excel_object_new_format(excel_ce_format));
			fo = Z_EXCEL_FORMAT_OBJ_P(&value);
			fo->format = format;
			fo->book = book;
			EXCEL_INIT_PARENT(fo, object);

			add_next_index_zval(return_value, &value);
		}
	}
}
/* }}} */

/* {{{ proto int ExcelBook::addCustomFormat(string format)
	Create a custom cell format */
EXCEL_METHOD(Book, addCustomFormat)
{
	BookHandle book;
	zval *object = ZEND_THIS;
	zend_string *format_zs = NULL;
	int id;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "S", &format_zs) == FAILURE) {
		RETURN_FALSE;
	}

	EXCEL_NON_EMPTY_STRING(format_zs)
	EXCEL_NUL_SAFE_STRING(format_zs)

	BOOK_FROM_OBJECT(book, object);

	if (!(id = xlBookAddCustomNumFormat(book, ZSTR_VAL(format_zs)))) {
		RETURN_FALSE;
	}
	RETURN_LONG(id);
}
/* }}} */

/* {{{ proto string ExcelBook::getCustomFormat(int id)
	Get a custom cell format */
EXCEL_METHOD(Book, getCustomFormat)
{
	BookHandle book;
	zval *object = ZEND_THIS;
	zend_long id;
	char *data;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "l", &id) == FAILURE) {
		RETURN_FALSE;
	}

	if (id < 1 || id > INT_MAX) {
		RETURN_FALSE;
	}

	BOOK_FROM_OBJECT(book, object);

	if (!(data = (char *)xlBookCustomNumFormat(book, id))) {
		RETURN_FALSE;
	}
	RETURN_STRING(data);
}
/* }}} */

static double _php_excel_date_pack(BookHandle book, zend_long ts)
{
	struct tm tm;

	if (!php_localtime_r(&ts, &tm)) {
		return -1;
	}

	tm.tm_year += 1900;
	tm.tm_mon += 1;

	return xlBookDatePack(book, tm.tm_year, tm.tm_mon, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec, 0);
}

/* {{{ proto float ExcelBook::packDate(int timestamp)
	Pack a unix timestamp into an Excel Double */
EXCEL_METHOD(Book, packDate)
{
	BookHandle book;
	zval *object = ZEND_THIS;
	zend_long ts;
	double dt;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "l", &ts) == FAILURE) {
		RETURN_FALSE;
	}

	if (ts < 0) {
		RETURN_FALSE;
	}

	BOOK_FROM_OBJECT(book, object);

	if ((dt = _php_excel_date_pack(book, ts)) == -1) {
		RETURN_FALSE;
	}
	RETURN_DOUBLE(dt);
}
/* }}} */


/* {{{ proto float ExcelBook::packDateValues(int year, int month, int day, int hour, int minute, int second)
	Pack a date by single values into an Excel Double */
EXCEL_METHOD(Book, packDateValues)
{
	BookHandle book;
	zval *object = ZEND_THIS;
	zend_long year, month, day, hour, min, sec;
	double dt;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "llllll", &year, &month, &day, &hour, &min, &sec) == FAILURE) {
		RETURN_FALSE;
	}

	// if it is a date or just a time - hout, min & sec must be checked

	if (hour < 0 || hour > 23) {
		php_error_docref(NULL, E_WARNING, "Invalid '" ZEND_LONG_FMT "' value for hour", hour);
		RETURN_FALSE;
	}
	if (min < 0 || min > 59) {
		php_error_docref(NULL, E_WARNING, "Invalid '" ZEND_LONG_FMT "' value for minute", min);
		RETURN_FALSE;
	}
	if (sec < 0 || sec > 59) {
		php_error_docref(NULL, E_WARNING, "Invalid '" ZEND_LONG_FMT "' value for second", sec);
		RETURN_FALSE;
	}

	// check date only if there are values
	// is every value=0 - it's okay for generating a time
	if (year != 0 || month != 0 || day != 0) {
		if (year < 1 || year > INT_MAX) {
			php_error_docref(NULL, E_WARNING, "Invalid '" ZEND_LONG_FMT "' value for year", year);
			RETURN_FALSE;
		}
		if (month < 1 || month > 12) {
			php_error_docref(NULL, E_WARNING, "Invalid '" ZEND_LONG_FMT "' value for month", month);
			RETURN_FALSE;
		}
		if (day < 1 || day > 31) {
			php_error_docref(NULL, E_WARNING, "Invalid '" ZEND_LONG_FMT "' value for day", day);
			RETURN_FALSE;
		}
	}

	BOOK_FROM_OBJECT(book, object);

	if ((dt = xlBookDatePack(book, year, month, day, hour, min, sec, 0)) == -1) {
		RETURN_FALSE;
	}
	RETURN_DOUBLE(dt);
}
/* }}} */

static zend_long _php_excel_date_unpack(BookHandle book, double dt)
{
	struct tm tm = {0};
	int msec;

	if (!xlBookDateUnpack(book, dt, (int *) &(tm.tm_year), (int *) &(tm.tm_mon), (int *) &(tm.tm_mday), (int *) &(tm.tm_hour), (int *) &(tm.tm_min), (int *) &(tm.tm_sec), &msec)) {
		return -1;
	}

	tm.tm_year -= 1900;
	tm.tm_mon -= 1;
	tm.tm_isdst = -1;

	return mktime(&tm);
}

/* {{{ proto int ExcelBook::unpackDate(double date)
	Unpack a unix timestamp from an Excel Double */
EXCEL_METHOD(Book, unpackDate)
{
	BookHandle book;
	zval *object = ZEND_THIS;
	double dt;
	time_t t;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "d", &dt) == FAILURE) {
		RETURN_FALSE;
	}

	EXCEL_VALIDATE_FINITE(dt)
	/* Excel serial day 0 is the epoch baseline; fractional values in [0, 1)
	 * are valid time-of-day-only dates (see packDateValues year=month=day=0). */
	if (dt < 0) {
		RETURN_FALSE;
	}

	BOOK_FROM_OBJECT(book, object);

	if ((t = _php_excel_date_unpack(book, dt)) == -1) {
		RETURN_FALSE;
	}
	RETURN_LONG(t);
}
/* }}} */

/* {{{ proto bool ExcelBook::isDate1904()
	Returns whether the 1904 date system is active: true - 1904 date system, false - 1900 date system */
EXCEL_METHOD(Book, isDate1904)
{
	PHP_EXCEL_BOOK_INFO(IsDate1904, IS_BOOL)
}
/* }}} */

/* {{{ proto bool ExcelBook::setDate1904(bool date_type)
	Sets the date system mode: true - 1904 date system, false - 1900 date system (default) */
EXCEL_METHOD(Book, setDate1904)
{
	BookHandle book;
	zval *object = ZEND_THIS;
	bool date_type;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "b", &date_type) == FAILURE) {
		RETURN_FALSE;
	}

	BOOK_FROM_OBJECT(book, object);

	xlBookSetDate1904(book, (int)date_type);

	RETURN_TRUE;
}
/* }}} */

/* {{{ proto int ExcelBook::getActiveSheet()
	Get the active sheet inside a file. */
EXCEL_METHOD(Book, getActiveSheet)
{
	PHP_EXCEL_BOOK_INFO(ActiveSheet, IS_LONG)
}
/* }}} */

/* {{{ proto array ExcelBook::getDefaultFont()
	Get the default font. */
EXCEL_METHOD(Book, getDefaultFont)
{
	BookHandle book;
	zval *object = ZEND_THIS;
	const char *font;
	int font_size;

	ZEND_PARSE_PARAMETERS_NONE();

	BOOK_FROM_OBJECT(book, object);

	if (!(font = xlBookDefaultFont(book, &font_size))) {
		RETURN_FALSE;
	}

	array_init(return_value);
	add_assoc_string(return_value, "font", (char *)font);
	add_assoc_long(return_value, "font_size", font_size);
}
/* }}} */

/* {{{ proto void ExcelBook::setDefaultFont(string font, int font_size)
	Set the default font, and size. */
EXCEL_METHOD(Book, setDefaultFont)
{
	BookHandle book;
	zval *object = ZEND_THIS;
	zend_long font_size;
	zend_string *font_zs = NULL;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "Sl", &font_zs, &font_size) == FAILURE || font_size < 1 || font_size > INT_MAX) {
		RETURN_FALSE;
	}

	EXCEL_NON_EMPTY_STRING(font_zs)
	EXCEL_NUL_SAFE_STRING(font_zs)

	BOOK_FROM_OBJECT(book, object);

	xlBookSetDefaultFont(book, ZSTR_VAL(font_zs), (int)font_size);
}
/* }}} */

static bool php_excel_locale_is_utf8(zend_string *locale)
{
	const unsigned char *value = (const unsigned char *) ZSTR_VAL(locale);
	size_t length = ZSTR_LEN(locale);
	size_t i;

	for (i = 0; i < length; i++) {
		if (i + 5 <= length
		    && (value[i] == 'U' || value[i] == 'u')
		    && (value[i + 1] == 'T' || value[i + 1] == 't')
		    && (value[i + 2] == 'F' || value[i + 2] == 'f')
		    && value[i + 3] == '-'
		    && value[i + 4] == '8') {
			return true;
		}
		if (i + 4 <= length
		    && (value[i] == 'U' || value[i] == 'u')
		    && (value[i + 1] == 'T' || value[i + 1] == 't')
		    && (value[i + 2] == 'F' || value[i + 2] == 'f')
		    && value[i + 3] == '8') {
			return true;
		}
	}
	return false;
}

/* {{{ proto void ExcelBook::setLocale(string locale)
	Set the locale. */
EXCEL_METHOD(Book, setLocale)
{
	BookHandle book;
	zval *object = ZEND_THIS;
	zend_string *locale_zs = NULL;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "S", &locale_zs) == FAILURE) {
		RETURN_FALSE;
	}

	EXCEL_NON_EMPTY_STRING(locale_zs)
	EXCEL_NUL_SAFE_STRING(locale_zs)

	BOOK_FROM_OBJECT(book, object);

	if (xlBookSetLocale(book, ZSTR_VAL(locale_zs))) {
		excel_book_object *book_obj = Z_EXCEL_BOOK_OBJ_P(object);
		book_obj->locale_is_utf8 = php_excel_locale_is_utf8(locale_zs);
	}
}
/* }}} */

/* {{{ proto ExcelBook ExcelBook::__construct([string license_name, string license_key [, bool excel_2007 = false]])
	Book Constructor. */
EXCEL_METHOD(Book, __construct)
{
	BookHandle book;
	zval *object = ZEND_THIS;
	char *name = NULL, *key = NULL;
	size_t name_len = 0, key_len = 0;
	bool new_excel = 0;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "|s!s!b", &name, &name_len, &key, &key_len, &new_excel) == FAILURE) {
		RETURN_THROWS();
	}

	/* Reject NUL-bearing license arguments before creating the libxl book.
	 * PHP ignores constructor return values, so we throw — otherwise the
	 * caller would get a usable workbook back from rejected input. */
	if ((name && name_len != strlen(name)) || (key && key_len != strlen(key))) {
		zend_throw_exception(NULL, "License name/key must not contain NUL bytes", 0);
		RETURN_THROWS();
	}

	{
		excel_book_object *obj = Z_EXCEL_BOOK_OBJ_P(object);
		if (new_excel) {
			book = xlCreateXMLBook();
		} else {
			book = xlCreateBook();
		}
		if (!book) {
			zend_throw_exception(NULL, "Failed to create book", 0);
			RETURN_THROWS();
		}
		if (obj->book) {
			xlBookRelease(obj->book);
			/* If the book object is being reconstructed, every existing child
			 * wrapper now references freed libxl handles. Bump generation so
			 * any subsequent use of those wrappers fails the stale-check
			 * instead of dereferencing freed memory. */
			obj->generation++;
			obj->sheet_generation++;
			obj->autofilter_generation++;
			obj->conditional_formatting_generation++;
		}
		obj->book = book;
		obj->is_xlsx = new_excel;
		obj->locale_is_utf8 = false;
		obj->default_date_format = NULL;
	}

#if defined(HAVE_LIBXL_SETKEY)

	if (name_len == 0 && EXCEL_G(ini_license_name) && EXCEL_G(ini_license_key)) {
		name = EXCEL_G(ini_license_name);
		name_len = strlen(name);
		key = EXCEL_G(ini_license_key);
		key_len = strlen(key);
	}

	/* Apply a license key only when both a name and key are available
	 * (passed explicitly, or sourced from the INI settings above). A
	 * missing license is not a construction error: the workbook has
	 * already been created and is usable, and PHP ignores constructor
	 * return values anyway -- the previous RETURN_FALSE here was dead,
	 * misleading code. Explicitly-passed NUL-bearing arguments are
	 * already rejected at the top of the constructor; INI-sourced values
	 * are NUL-free by construction (their length came from strlen()). */
	if (name && name_len >= 1 && key && key_len >= 1) {
		xlBookSetKey(book, name, key);
	}

#endif
}
/* }}} */

/* {{{ proto bool ExcelBook::setActiveSheet(int sheet)
	Set the sheet active. */
EXCEL_METHOD(Book, setActiveSheet)
{
	BookHandle book;
	zval *object = ZEND_THIS;
	zend_long id;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "l", &id) == FAILURE) {
		RETURN_FALSE;
	}

	EXCEL_VALIDATE_INT_RANGE(id)

	BOOK_FROM_OBJECT(book, object);

	xlBookSetActiveSheet(book, id);
	RETURN_BOOL(id == xlBookActiveSheet(book));
}
/* }}} */

static void php_excel_add_picture(INTERNAL_FUNCTION_PARAMETERS, int mode) /* {{{ */
{
	zend_string *data_zs = NULL;
	BookHandle book;
	zval *object = ZEND_THIS;
	int ret;
	php_stream *stream;
	zend_string *contents;
	php_excel_stream_read_status stream_status;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "S", &data_zs) == FAILURE) {
		RETURN_FALSE;
	}

	EXCEL_NON_EMPTY_STRING(data_zs)

	BOOK_FROM_OBJECT(book, object);

	if (mode == 1) {
		EXCEL_VALIDATE_UINT_SIZE(data_zs)
		ret = xlBookAddPicture2(book, ZSTR_VAL(data_zs), ZSTR_LEN(data_zs));
	} else {
		/* path-mode: data_zs is a filesystem path. NUL truncation here
		 * silently opens a different file than the caller validated. */
		EXCEL_NUL_SAFE_STRING(data_zs)
		/* With open_basedir active, PHP must perform the actual open. */
		if (!strstr(ZSTR_VAL(data_zs), "://")
		    && (!PG(open_basedir) || !*PG(open_basedir))) {
			ret = xlBookAddPicture(book, ZSTR_VAL(data_zs));
			goto picture_done;
		}
		if (!strstr(ZSTR_VAL(data_zs), "://")
		    && PG(open_basedir) && *PG(open_basedir)
		    && php_check_open_basedir(ZSTR_VAL(data_zs))) {
			RETURN_FALSE;
		}
		stream = php_stream_open_wrapper(ZSTR_VAL(data_zs), "rb", REPORT_ERRORS, NULL);

		if (!stream) {
			RETURN_FALSE;
		}

		contents = php_excel_stream_read_all(stream, &stream_status);
		if (EG(exception)) {
			RETURN_THROWS();
		}
		if (stream_status == PHP_EXCEL_STREAM_READ_TOO_LARGE) {
			php_error_docref(NULL, E_WARNING, "Source file too large");
			RETURN_FALSE;
		}
		if (stream_status == PHP_EXCEL_STREAM_READ_ERROR) {
			php_error_docref(NULL, E_WARNING, "Failed to read source stream");
			RETURN_FALSE;
		}

		if (!contents || ZSTR_LEN(contents) < 1) {
			if (contents) {
				zend_string_release(contents);
			}
			RETURN_FALSE;
		}
		if (ZSTR_LEN(contents) >= UINT_MAX) {
			php_error_docref(NULL, E_WARNING, "Data string too large");
			zend_string_release(contents);
			RETURN_FALSE;
		}
		/* The wrapper can execute userland and reconstruct the book. */
		BOOK_FROM_OBJECT_RELEASE_STR(book, object, contents);
		ret = xlBookAddPicture2(book, ZSTR_VAL(contents), ZSTR_LEN(contents));
		zend_string_release(contents);
	}

picture_done:
	if (ret == -1) {
		RETURN_FALSE;
	} else {
		RETURN_LONG(ret);
	}
}

/* {{{ proto int ExcelBook::addPictureFromFile(string filename)
	Add picture from file. */
EXCEL_METHOD(Book, addPictureFromFile)
{
	php_excel_add_picture(INTERNAL_FUNCTION_PARAM_PASSTHRU, 0);
}
/* }}} */

/* {{{ proto int ExcelBook::addPictureFromString(string data)
	Add picture from string. */
EXCEL_METHOD(Book, addPictureFromString)
{
	php_excel_add_picture(INTERNAL_FUNCTION_PARAM_PASSTHRU, 1);
}
/* }}} */

/* {{{ proto bool ExcelBook::rgbMode()
	Returns whether the RGB mode is active. */
EXCEL_METHOD(Book, rgbMode)
{
	PHP_EXCEL_BOOK_INFO(RgbMode, IS_BOOL)
}
/* }}} */

/* {{{ proto void ExcelBook::setRGBMode(bool mode)
	Sets a RGB mode on or off. */
EXCEL_METHOD(Book, setRGBMode)
{
	BookHandle book;
	zval *object = ZEND_THIS;
	bool val;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "b", &val) == FAILURE) {
		RETURN_FALSE;
	}

	BOOK_FROM_OBJECT(book, object);

	xlBookSetRgbMode(book, val);
	RETURN_TRUE;
}
/* }}} */

/* {{{ proto int ExcelBook::colorPack(int r, int g, int b)
	Packs red, green and blue components in color value. Used for xlsx format only. */
EXCEL_METHOD(Book, colorPack)
{
	BookHandle book;
	zval *object = ZEND_THIS;
	zend_long r, g, b;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "lll", &r, &g, &b) == FAILURE) {
		RETURN_FALSE;
	}

	if (r < 0 || r > 255) {
		php_error_docref(NULL, E_WARNING, "Invalid '" ZEND_LONG_FMT "' value for color red", r);
		RETURN_FALSE;
	} else if (g < 0 || g > 255) {
		php_error_docref(NULL, E_WARNING, "Invalid '" ZEND_LONG_FMT "' value for color green", g);
		RETURN_FALSE;
	} else if (b < 0 || b > 255) {
		php_error_docref(NULL, E_WARNING, "Invalid '" ZEND_LONG_FMT "' value for color blue", b);
		RETURN_FALSE;
	}

	BOOK_FROM_OBJECT(book, object);

	RETURN_LONG(xlBookColorPack(book, (unsigned short)r, (unsigned short)g, (unsigned short)b));
}
/* }}} */

/* {{{ proto array ExcelBook::colorUnpack(int color)
	Unpacks color value to red, green and blue components. Used for xlsx format only. */
EXCEL_METHOD(Book, colorUnpack)
{
	BookHandle book;
	zval *object = ZEND_THIS;
	int r, g, b;
	zend_long color;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "l", &color) == FAILURE) {
		RETURN_FALSE;
	}

	if (color < 0 || color > INT_MAX) {
		php_error_docref(NULL, E_WARNING, "Invalid '" ZEND_LONG_FMT "' value for color code", color);
		RETURN_FALSE;
	}

	BOOK_FROM_OBJECT(book, object);

	xlBookColorUnpack(book, (int)color, &r, &g, &b);

	array_init(return_value);
	add_assoc_long(return_value, "red", r);
	add_assoc_long(return_value, "green", g);
	add_assoc_long(return_value, "blue", b);
}
/* }}} */

/* {{{ proto string ExcelBook::getLibXlVersion()
	Returns the version of libXL library */
EXCEL_METHOD(Book, getLibXlVersion)
{
	char libxl_api[25];

	ZEND_PARSE_PARAMETERS_NONE();

	snprintf(libxl_api, sizeof(libxl_api), "%x", LIBXL_VERSION);
	RETURN_STRING(libxl_api);
}
/* }}} */

/* {{{ proto string ExcelBook::getPhpExcelVersion()
	Returns the version of PHP Excel extension */
EXCEL_METHOD(Book, getPhpExcelVersion)
{
	ZEND_PARSE_PARAMETERS_NONE();

	RETURN_STRING(PHP_EXCEL_VERSION);
}
/* }}} */

/* {{{ proto bool ExcelBook::loadInfo(string filename)
	Loads only information about sheets. Afterwards you can call Book::sheetCount()
	and Book::getSheetName() methods. Returns false if error occurs. Get error
	info with Book::errorMessage(). */
EXCEL_METHOD(Book, loadInfo)
{
	BookHandle book;
	zval *object = ZEND_THIS;
	zend_string *filename_zs = NULL;
#if LIBXL_VERSION >= 0x05000100
	php_stream *stream;
	zend_string *contents;
	php_excel_stream_read_status stream_status;
#endif

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "S", &filename_zs) == FAILURE) {
		RETURN_FALSE;
	}

	EXCEL_NON_EMPTY_STRING(filename_zs)
	EXCEL_NUL_SAFE_STRING(filename_zs)

#if LIBXL_VERSION >= 0x05000100
	if (strstr(ZSTR_VAL(filename_zs), "://") || (PG(open_basedir) && *PG(open_basedir))) {
		if (!strstr(ZSTR_VAL(filename_zs), "://")
		    && php_check_open_basedir(ZSTR_VAL(filename_zs))) {
			RETURN_FALSE;
		}
		stream = php_stream_open_wrapper(ZSTR_VAL(filename_zs), "rb", REPORT_ERRORS, NULL);
		if (!stream) {
			RETURN_FALSE;
		}
		contents = php_excel_stream_read_all(stream, &stream_status);
		if (EG(exception)) {
			RETURN_THROWS();
		}
		if (stream_status == PHP_EXCEL_STREAM_READ_TOO_LARGE) {
			php_error_docref(NULL, E_WARNING, "Source file too large");
			RETURN_FALSE;
		}
		if (stream_status == PHP_EXCEL_STREAM_READ_ERROR) {
			php_error_docref(NULL, E_WARNING, "Failed to read source stream");
			RETURN_FALSE;
		}
		if (!contents || ZSTR_LEN(contents) < 1) {
			if (contents) {
				zend_string_release(contents);
			}
			php_error_docref(NULL, E_WARNING, "Source file is empty");
			RETURN_FALSE;
		}
		if (ZSTR_LEN(contents) >= UINT_MAX) {
			zend_string_release(contents);
			php_error_docref(NULL, E_WARNING, "Source file too large");
			RETURN_FALSE;
		}
		BOOK_FROM_OBJECT_RELEASE_STR(book, object, contents);
		if (!php_excel_book_reset_state(object)) {
			zend_string_release(contents);
			RETURN_FALSE;
		}
		if (!xlBookLoadInfoRaw(book, ZSTR_VAL(contents), (unsigned) ZSTR_LEN(contents))) {
			php_error_docref(NULL, E_WARNING, "Failed to load workbook: %s", xlBookErrorMessage(book));
			zend_string_release(contents);
			RETURN_FALSE;
		}
		zend_string_release(contents);
		RETURN_TRUE;
	}
#else
	if (PG(open_basedir) && *PG(open_basedir)) {
		if (php_check_open_basedir(ZSTR_VAL(filename_zs))) {
			RETURN_FALSE;
		}
		php_error_docref(NULL, E_WARNING, "loadInfo is not available while open_basedir is active with LibXL older than 5.0.1");
		RETURN_FALSE;
	}
#endif

	BOOK_FROM_OBJECT(book, object);
	if (!php_excel_book_reset_state(object)) {
		RETURN_FALSE;
	}
	if (!xlBookLoadInfo(book, ZSTR_VAL(filename_zs))) {
		php_error_docref(NULL, E_WARNING, "Failed to load workbook: %s", xlBookErrorMessage(book));
		RETURN_FALSE;
	}
	RETURN_TRUE;
}
/* }}} */

/* {{{ proto string ExcelBook::getSheetName(int index)
	Returns a sheet name with specified index. Returns
	NULL if error occurs. Get error info with xlBookErrorMessage(). */
EXCEL_METHOD(Book, getSheetName)
{
	BookHandle book;
	zval *object = ZEND_THIS;
	zend_long index;
	char *data;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "l", &index) == FAILURE) {
		RETURN_FALSE;
	}

	EXCEL_VALIDATE_INT_RANGE(index)

	BOOK_FROM_OBJECT(book, object);

	if (!(data = (char *)xlBookGetSheetName(book, index))) {
		RETURN_FALSE;
	}
	RETURN_STRING(data);
}
/* }}} */

EXCEL_METHOD(Book, addRichString)
{
	BookHandle book;
	zval *object = ZEND_THIS;
	RichStringHandle rs;
	excel_richstring_object *ro;

	ZEND_PARSE_PARAMETERS_NONE();

	BOOK_FROM_OBJECT(book, object);

	rs = xlBookAddRichString(book);
	if (!rs) {
		RETURN_FALSE;
	}

	ZVAL_OBJ(return_value, excel_object_new_richstring(excel_ce_richstring));
	ro = Z_EXCEL_RICHSTRING_OBJ_P(return_value);
	ro->richstring = rs;
	ro->book = book;
	EXCEL_INIT_PARENT(ro, object);
}

EXCEL_METHOD(Book, calcMode)
{
	PHP_EXCEL_BOOK_INFO(CalcMode, IS_LONG)
}

EXCEL_METHOD(Book, setCalcMode)
{
	BookHandle book;
	zval *object = ZEND_THIS;
	zend_long mode;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "l", &mode) == FAILURE) {
		RETURN_FALSE;
	}

	EXCEL_VALIDATE_INT_RANGE(mode)

	BOOK_FROM_OBJECT(book, object);

	xlBookSetCalcMode(book, mode);
	RETURN_TRUE;
}

EXCEL_METHOD(Book, addConditionalFormat)
{
	BookHandle book;
	zval *object = ZEND_THIS;
	ConditionalFormatHandle cf;
	excel_conditionalformat_object *cfo;

	ZEND_PARSE_PARAMETERS_NONE();

	BOOK_FROM_OBJECT(book, object);

	cf = xlBookAddConditionalFormat(book);
	if (!cf) {
		RETURN_FALSE;
	}

	ZVAL_OBJ(return_value, excel_object_new_conditionalformat(excel_ce_conditionalformat));
	cfo = Z_EXCEL_CONDITIONALFORMAT_OBJ_P(return_value);
	cfo->conditionalformat = cf;
	cfo->book = book;
	EXCEL_INIT_PARENT(cfo, object);
}

EXCEL_METHOD(Book, addFormatFromStyle)
{
	BookHandle book;
	zval *object = ZEND_THIS;
	zend_long style;
	FormatHandle format;
	excel_format_object *fo;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "l", &style) == FAILURE) {
		RETURN_FALSE;
	}

	EXCEL_VALIDATE_INT_RANGE(style)

	BOOK_FROM_OBJECT(book, object);

	format = xlBookAddFormatFromStyle(book, style);
	if (!format) {
		RETURN_FALSE;
	}

	ZVAL_OBJ(return_value, excel_object_new_format(excel_ce_format));
	fo = Z_EXCEL_FORMAT_OBJ_P(return_value);
	fo->format = format;
	fo->book = book;
	EXCEL_INIT_PARENT(fo, object);
}

EXCEL_METHOD(Book, removeVBA)
{
	PHP_EXCEL_BOOK_INFO(RemoveVBA, IS_BOOL)
}

EXCEL_METHOD(Book, removePrinterSettings)
{
	PHP_EXCEL_BOOK_INFO(RemovePrinterSettings, IS_BOOL)
}

#if LIBXL_VERSION >= 0x05000000
EXCEL_METHOD(Book, setPassword)
{
	BookHandle book;
	zval *object = ZEND_THIS;
	zend_string *password;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "S", &password) == FAILURE) {
		RETURN_FALSE;
	}

	EXCEL_NUL_SAFE_STRING(password)

	BOOK_FROM_OBJECT(book, object);

	xlBookSetPassword(book, ZSTR_VAL(password));
	RETURN_TRUE;
}

EXCEL_METHOD(Book, dpiAwareness)
{
	PHP_EXCEL_BOOK_INFO(DpiAwareness, IS_LONG)
}

EXCEL_METHOD(Book, setDpiAwareness)
{
	BookHandle book;
	zval *object = ZEND_THIS;
	zend_long val;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "l", &val) == FAILURE) {
		RETURN_FALSE;
	}
	EXCEL_VALIDATE_INT_RANGE(val)

	BOOK_FROM_OBJECT(book, object);

	xlBookSetDpiAwareness(book, val);
	RETURN_TRUE;
}
#endif

#if LIBXL_VERSION >= 0x05000100
EXCEL_METHOD(Book, loadInfoRaw)
{
	BookHandle book;
	zval *object = ZEND_THIS;
	zend_string *data;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "S", &data) == FAILURE) {
		RETURN_FALSE;
	}

	EXCEL_VALIDATE_UINT_SIZE(data)

	BOOK_FROM_OBJECT(book, object);

	if (!php_excel_book_reset_state(object)) {
		RETURN_FALSE;
	}
	if (!xlBookLoadInfoRaw(book, ZSTR_VAL(data), ZSTR_LEN(data))) {
		php_error_docref(NULL, E_WARNING, "Failed to load workbook: %s", xlBookErrorMessage(book));
		RETURN_FALSE;
	}
	RETURN_TRUE;
}
#endif

#if LIBXL_VERSION >= 0x05010000
EXCEL_METHOD(Book, errorCode)
{
	PHP_EXCEL_BOOK_INFO(ErrorCode, IS_LONG)
}

EXCEL_METHOD(Book, conditionalFormat)
{
	BookHandle book;
	zval *object = ZEND_THIS;
	zend_long index;
	excel_conditionalformat_object *cfo;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "l", &index) == FAILURE) {
		RETURN_FALSE;
	}

	EXCEL_VALIDATE_INT_RANGE(index)

	BOOK_FROM_OBJECT(book, object);

	ConditionalFormatHandle cfh = xlBookConditionalFormat(book, index);
	if (!cfh) {
		RETURN_FALSE;
	}

	ZVAL_OBJ(return_value, excel_object_new_conditionalformat(excel_ce_conditionalformat));
	cfo = Z_EXCEL_CONDITIONALFORMAT_OBJ_P(return_value);
	cfo->conditionalformat = cfh;
	cfo->book = book;
	EXCEL_INIT_PARENT(cfo, object);
}

EXCEL_METHOD(Book, conditionalFormatSize)
{
	PHP_EXCEL_BOOK_INFO(ConditionalFormatSize, IS_LONG)
}

EXCEL_METHOD(Book, clear)
{
	BookHandle book;
	zval *object = ZEND_THIS;

	ZEND_PARSE_PARAMETERS_NONE();

	BOOK_FROM_OBJECT(book, object);

	if (!php_excel_book_reset_state(object)) {
		RETURN_FALSE;
	}
	xlBookClear(book);
	RETURN_TRUE;
}
#endif

EXCEL_METHOD(Book, coreProperties)
{
	BookHandle book;
	zval *object = ZEND_THIS;
	CorePropertiesHandle cp;
	excel_coreproperties_object *cpo;

	ZEND_PARSE_PARAMETERS_NONE();

	BOOK_FROM_OBJECT(book, object);

	cp = xlBookCoreProperties(book);
	if (!cp) {
		RETURN_FALSE;
	}

	ZVAL_OBJ(return_value, excel_object_new_coreproperties(excel_ce_coreproperties));
	cpo = Z_EXCEL_COREPROPERTIES_OBJ_P(return_value);
	cpo->coreproperties = cp;
	cpo->book = book;
	EXCEL_INIT_PARENT(cpo, object);
}

EXCEL_METHOD(Book, removeAllPhonetics)
{
	BookHandle book;
	zval *object = ZEND_THIS;

	ZEND_PARSE_PARAMETERS_NONE();

	BOOK_FROM_OBJECT(book, object);

	xlBookRemoveAllPhonetics(book);
	RETURN_TRUE;
}

/* {{{ proto int ExcelFont::size([int size])
	Get or set the font size */
EXCEL_METHOD(Font, size)
{
	zval *object = ZEND_THIS;
	FontHandle font;
	zend_long size = 0;
	bool size_is_null = 1;

	ZEND_PARSE_PARAMETERS_START(0, 1)
		Z_PARAM_OPTIONAL
		Z_PARAM_LONG_OR_NULL(size, size_is_null)
	ZEND_PARSE_PARAMETERS_END_EX(RETURN_FALSE);

	FONT_FROM_OBJECT(font, object);

	/* size() with no argument (or null) is getter mode. An explicit size must
	 * be a positive int in libxl range; reject 0/negative/overflow rather than
	 * silently no-op'ing and returning the current size like a getter. */
	if (!size_is_null) {
		if (size <= 0 || size > INT_MAX) {
			php_error_docref(NULL, E_WARNING, "Font size must be a positive integer");
			RETURN_FALSE;
		}
		xlFontSetSize(font, (int)size);
	}

	RETURN_LONG(xlFontSize(font));
}
/* }}} */

/* Font long/bool option helpers — same pattern as Format options.
 * size() and name() stay hand-written (size rejects <=0; name is string). */
#define PHP_EXCEL_LONG_FONT_OPTION(method_name, api_name) \
	{ \
		FontHandle font; \
		zval *object = ZEND_THIS; \
		zend_long data = 0; \
		bool data_is_null = 1; \
		ZEND_PARSE_PARAMETERS_START(0, 1) \
			Z_PARAM_OPTIONAL \
			Z_PARAM_LONG_OR_NULL(data, data_is_null) \
		ZEND_PARSE_PARAMETERS_END_EX(RETURN_FALSE); \
		FONT_FROM_OBJECT(font, object); \
		if (!data_is_null) { \
			if (data < 0 || data > INT_MAX) { \
				php_error_docref(NULL, E_WARNING, "Argument out of int range"); \
				RETURN_FALSE; \
			} \
			xlFontSet ## api_name (font, (int)data); \
		} \
		RETURN_LONG(xlFont ## api_name (font)); \
	}

#define PHP_EXCEL_BOOL_FONT_OPTION(method_name, api_name) \
	{ \
		FontHandle font; \
		zval *object = ZEND_THIS; \
		bool data = 0; \
		bool data_is_null = 1; \
		ZEND_PARSE_PARAMETERS_START(0, 1) \
			Z_PARAM_OPTIONAL \
			Z_PARAM_BOOL_OR_NULL(data, data_is_null) \
		ZEND_PARSE_PARAMETERS_END_EX(RETURN_FALSE); \
		FONT_FROM_OBJECT(font, object); \
		if (!data_is_null) { \
			xlFontSet ## api_name (font, data); \
		} \
		RETURN_BOOL(xlFont ## api_name (font)); \
	}

/* {{{ proto bool ExcelFont::italics([bool italics])
	Get or set the if italics are enabled */
EXCEL_METHOD(Font, italics)
{
	PHP_EXCEL_BOOL_FONT_OPTION(italics, Italic);
}
/* }}} */

/* {{{ proto bool ExcelFont::strike([bool strike])
	Get or set the font strike-through */
EXCEL_METHOD(Font, strike)
{
	PHP_EXCEL_BOOL_FONT_OPTION(strike, StrikeOut);
}
/* }}} */

/* {{{ proto bool ExcelFont::bold([bool bold])
	Get or set the font bold */
EXCEL_METHOD(Font, bold)
{
	PHP_EXCEL_BOOL_FONT_OPTION(bold, Bold);
}
/* }}} */

/* {{{ proto int ExcelFont::color([int color])
	Get or set the font color */
EXCEL_METHOD(Font, color)
{
	PHP_EXCEL_LONG_FONT_OPTION(color, Color);
}
/* }}} */

/* {{{ proto int ExcelFont::mode([int mode])
	Get or set the font mode */
EXCEL_METHOD(Font, mode)
{
	/* LibXL names this "Script" (super/subscript); PHP API is mode(). */
	PHP_EXCEL_LONG_FONT_OPTION(mode, Script);
}
/* }}} */

/* {{{ proto int ExcelFont::underline([int underline_style])
	Get or set the font underline style */
EXCEL_METHOD(Font, underline)
{
	PHP_EXCEL_LONG_FONT_OPTION(underline, Underline);
}
/* }}} */

/* {{{ proto string ExcelFont::name([string name])
	Get or set the font name */
EXCEL_METHOD(Font, name)
{
	zval *object = ZEND_THIS;
	FontHandle font;
	zend_string *name_zs = NULL;

	/* null = getter; previously `|S` weak-coerced null to "" and reset
	 * the font name to empty (e.g. name(null) reset "Arial" -> ""). */
	ZEND_PARSE_PARAMETERS_START(0, 1)
		Z_PARAM_OPTIONAL
		Z_PARAM_STR_OR_NULL(name_zs)
	ZEND_PARSE_PARAMETERS_END_EX(RETURN_FALSE);

	EXCEL_NUL_SAFE_STRING(name_zs)

	FONT_FROM_OBJECT(font, object);

	if (name_zs) {
		xlFontSetName(font, ZSTR_VAL(name_zs));
	}

	{
		const char *fn = xlFontName(font);
		if (!fn) {
			RETURN_FALSE;
		}
		RETURN_STRING((char *)fn);
	}
}
/* }}} */

/* {{{ proto ExcelFormat ExcelFormat::__construct(ExcelBook book)
	Format Constructor. */
EXCEL_METHOD(Format, __construct)
{
	BookHandle book;
	FormatHandle format;
	zval *object = ZEND_THIS;
	excel_format_object *obj;
	zval *zbook;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "O", &zbook, excel_ce_book) == FAILURE) {
		RETURN_THROWS();
	}

	BOOK_FROM_OBJECT_THROW(book, zbook);

	obj = Z_EXCEL_FORMAT_OBJ_P(object);
	EXCEL_REJECT_RECONSTRUCTION(obj, format);

	format = xlBookAddFormat(book, NULL);
	if (!format) {
		zend_throw_exception(NULL, "Failed to add format", 0);
		RETURN_THROWS();
	}

	obj->format = format;
	obj->book = book;
	EXCEL_INIT_PARENT(obj, zbook);
}
/* }}} */

/* {{{ proto ExcelFont ExcelFont::__construct(ExcelBook book)
	Font Constructor. */
EXCEL_METHOD(Font, __construct)
{
	BookHandle book;
	FontHandle font;
	zval *object = ZEND_THIS;
	excel_font_object *obj;
	zval *zbook;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "O", &zbook, excel_ce_book) == FAILURE) {
		RETURN_THROWS();
	}

	BOOK_FROM_OBJECT_THROW(book, zbook);

	obj = Z_EXCEL_FONT_OBJ_P(object);
	EXCEL_REJECT_RECONSTRUCTION(obj, font);

	font = xlBookAddFont(book, NULL);
	if (!font) {
		zend_throw_exception(NULL, "Failed to add font", 0);
		RETURN_THROWS();
	}

	obj->font = font;
	obj->book = book;
	EXCEL_INIT_PARENT(obj, zbook);
}
/* }}} */

/* {{{ proto bool ExcelFormat::setFont(ExcelFont font)
	Set the font for a format. */
EXCEL_METHOD(Format, setFont)
{
	FormatHandle format;
	zval *object = ZEND_THIS;
	FontHandle font;
	zval *zfont;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "O", &zfont, excel_ce_font) == FAILURE) {
		RETURN_FALSE;
	}

	FORMAT_FROM_OBJECT(format, object);
	FONT_FROM_OBJECT(font, zfont);
	EXCEL_REQUIRE_SAME_BOOK(zfont, object);

	if (!xlFormatSetFont(format, font)) {
		RETURN_FALSE;
	}
	RETURN_TRUE;
}
/* }}} */

/* {{{ proto ExcelFont ExcelFormat::getFont()
	Get the font for this format. */
EXCEL_METHOD(Format, getFont)
{
	FormatHandle format;
	zval *object = ZEND_THIS;
	FontHandle font;
	excel_font_object *fo;

	ZEND_PARSE_PARAMETERS_NONE();

	FORMAT_FROM_OBJECT(format, object);

	font = xlFormatFont(format);
	if (!font) {
		RETURN_FALSE;
	}

	{
		excel_format_object *fobj = Z_EXCEL_FORMAT_OBJ_P(object);
		ZVAL_OBJ(return_value, excel_object_new_font(excel_ce_font));
		fo = Z_EXCEL_FONT_OBJ_P(return_value);
		fo->font = font;
		fo->book = fobj->book;
		EXCEL_INIT_PARENT(fo, object);
	}
}
/* }}} */

/* Optional-setter macro: null or omitted => getter. Any int => setter,
 * range-checked against the libxl int boundary. Without LONG_OR_NULL the
 * stub-declared `mixed = null` got coerced to 0 and silently mutated the
 * underlying format slot to 0 (e.g. numberFormat(null) reset format 7 -> 0).
 */
#define PHP_EXCEL_LONG_FORMAT_OPTION(func_name) \
	{ \
		FormatHandle format; \
		zval *object = ZEND_THIS; \
		zend_long data = 0; \
		bool data_is_null = 1; \
		ZEND_PARSE_PARAMETERS_START(0, 1) \
			Z_PARAM_OPTIONAL \
			Z_PARAM_LONG_OR_NULL(data, data_is_null) \
		ZEND_PARSE_PARAMETERS_END_EX(RETURN_FALSE); \
		FORMAT_FROM_OBJECT(format, object); \
		if (!data_is_null) { \
			if (data < 0 || data > INT_MAX) { \
				php_error_docref(NULL, E_WARNING, "Argument out of int range"); \
				RETURN_FALSE; \
			} \
			xlFormatSet ## func_name (format, (int)data); \
		} \
		RETURN_LONG(xlFormat ## func_name (format)); \
	}

/* Write-only variant for setters whose libxl getter does not exist
 * (xlFormatSetBorder / xlFormatSetBorderColor have no xlFormatBorder /
 * xlFormatBorderColor counterpart). Returns true on success instead of
 * reading the value back, so no fake getter shim is needed. */
#define PHP_EXCEL_LONG_FORMAT_OPTION_WRITEONLY(func_name) \
	{ \
		FormatHandle format; \
		zval *object = ZEND_THIS; \
		zend_long data = 0; \
		bool data_is_null = 1; \
		ZEND_PARSE_PARAMETERS_START(0, 1) \
			Z_PARAM_OPTIONAL \
			Z_PARAM_LONG_OR_NULL(data, data_is_null) \
		ZEND_PARSE_PARAMETERS_END_EX(RETURN_FALSE); \
		FORMAT_FROM_OBJECT(format, object); \
		if (!data_is_null) { \
			if (data < 0 || data > INT_MAX) { \
				php_error_docref(NULL, E_WARNING, "Argument out of int range"); \
				RETURN_FALSE; \
			} \
			xlFormatSet ## func_name (format, (int)data); \
		} \
		RETURN_TRUE; \
	}

#define PHP_EXCEL_BOOL_FORMAT_OPTION(func_name) \
	{ \
		FormatHandle format; \
		zval *object = ZEND_THIS; \
		bool data = 0; \
		bool data_is_null = 1; \
		ZEND_PARSE_PARAMETERS_START(0, 1) \
			Z_PARAM_OPTIONAL \
			Z_PARAM_BOOL_OR_NULL(data, data_is_null) \
		ZEND_PARSE_PARAMETERS_END_EX(RETURN_FALSE); \
		FORMAT_FROM_OBJECT(format, object); \
		if (!data_is_null) { \
			xlFormatSet ## func_name (format, data); \
		} \
		RETURN_BOOL(xlFormat ## func_name (format)); \
	}

/* {{{ proto int ExcelFormat::numberFormat([int format])
	Get or set the cell number format */
EXCEL_METHOD(Format, numberFormat)
{
	PHP_EXCEL_LONG_FORMAT_OPTION(NumFormat);
}
/* }}} */

/* {{{ proto int ExcelFormat::horizontalAlign([int align_mode])
	Get or set the cell horizontal alignment */
EXCEL_METHOD(Format, horizontalAlign)
{
	PHP_EXCEL_LONG_FORMAT_OPTION(AlignH);
}
/* }}} */

/* {{{ proto int ExcelFormat::verticalAlign([int align_mode])
	Get or set the cell vertical alignment */
EXCEL_METHOD(Format, verticalAlign)
{
	PHP_EXCEL_LONG_FORMAT_OPTION(AlignV);
}
/* }}} */

/* {{{ proto bool ExcelFormat::wrap([bool wrap])
	Get or set the cell wrapping */
EXCEL_METHOD(Format, wrap)
{
	PHP_EXCEL_BOOL_FORMAT_OPTION(Wrap);
}
/* }}} */

/* {{{ proto int ExcelFormat::rotate([int angle])
	Get or set the cell data rotation */
EXCEL_METHOD(Format, rotate)
{
	FormatHandle format;
	zval *object = ZEND_THIS;
	zend_long angle = 0;
	bool angle_is_null = 1;

	ZEND_PARSE_PARAMETERS_START(0, 1)
		Z_PARAM_OPTIONAL
		Z_PARAM_LONG_OR_NULL(angle, angle_is_null)
	ZEND_PARSE_PARAMETERS_END_EX(RETURN_FALSE);

	FORMAT_FROM_OBJECT(format, object);

	if (!angle_is_null) {
		if (angle < 0 || (angle > 180 && angle != 255)) {
			php_error_docref(NULL, E_WARNING, "Rotation can be a number between 0 and 180 or 255");
			RETURN_FALSE;
		}
		xlFormatSetRotation(format, (int)angle);
	}

	RETURN_LONG(xlFormatRotation(format));
}
/* }}} */

/* {{{ proto int ExcelFormat::indent([int indent])
	Get or set the cell text indentation level */
EXCEL_METHOD(Format, indent)
{
	FormatHandle format;
	zval *object = ZEND_THIS;
	zend_long indent = 0;
	bool indent_is_null = 1;

	ZEND_PARSE_PARAMETERS_START(0, 1)
		Z_PARAM_OPTIONAL
		Z_PARAM_LONG_OR_NULL(indent, indent_is_null)
	ZEND_PARSE_PARAMETERS_END_EX(RETURN_FALSE);

	FORMAT_FROM_OBJECT(format, object);

	if (!indent_is_null) {
		if (indent < 0 || indent > 15) {
			php_error_docref(NULL, E_WARNING, "Text indentation level must be less than or equal to 15");
			RETURN_FALSE;
		}
		xlFormatSetIndent(format, (int)indent);
	}

	RETURN_LONG(xlFormatIndent(format));
}
/* }}} */

/* {{{ proto bool ExcelFormat::shrinkToFit([bool shrink])
	Get or set whether the cell is shrink-to-fit */
EXCEL_METHOD(Format, shrinkToFit)
{
	PHP_EXCEL_BOOL_FORMAT_OPTION(ShrinkToFit);
}
/* }}} */

/* {{{ proto int ExcelFormat::borderStyle([int style])
	Get or set the cell border */
EXCEL_METHOD(Format, borderStyle)
{
	PHP_EXCEL_LONG_FORMAT_OPTION_WRITEONLY(Border);
}
/* }}} */

/* {{{ proto int ExcelFormat::borderColor([int color])
	Get or set the cell color */
EXCEL_METHOD(Format, borderColor)
{
	PHP_EXCEL_LONG_FORMAT_OPTION_WRITEONLY(BorderColor);
}
/* }}} */

/* {{{ proto int ExcelFormat::borderLeftStyle([int style])
	Get or set the cell left border */
EXCEL_METHOD(Format, borderLeftStyle)
{
	PHP_EXCEL_LONG_FORMAT_OPTION(BorderLeft);
}
/* }}} */

/* {{{ proto int ExcelFormat::borderLeftColor([int color])
	Get or set the cell left color */
EXCEL_METHOD(Format, borderLeftColor)
{
	PHP_EXCEL_LONG_FORMAT_OPTION(BorderLeftColor);
}
/* }}} */

/* {{{ proto int ExcelFormat::borderRightStyle([int style])
	Get or set the cell right border */
EXCEL_METHOD(Format, borderRightStyle)
{
	PHP_EXCEL_LONG_FORMAT_OPTION(BorderRight);
}
/* }}} */

/* {{{ proto int ExcelFormat::borderRightColor([int color])
	Get or set the cell right color */
EXCEL_METHOD(Format, borderRightColor)
{
	PHP_EXCEL_LONG_FORMAT_OPTION(BorderRightColor);
}
/* }}} */

/* {{{ proto int ExcelFormat::borderTopStyle([int style])
	Get or set the cell top border */
EXCEL_METHOD(Format, borderTopStyle)
{
	PHP_EXCEL_LONG_FORMAT_OPTION(BorderTop);
}
/* }}} */

/* {{{ proto int ExcelFormat::borderTopColor([int color])
	Get or set the cell top color */
EXCEL_METHOD(Format, borderTopColor)
{
	PHP_EXCEL_LONG_FORMAT_OPTION(BorderTopColor);
}
/* }}} */

/* {{{ proto int ExcelFormat::borderBottomStyle([int style])
	Get or set the cell bottom border */
EXCEL_METHOD(Format, borderBottomStyle)
{
	PHP_EXCEL_LONG_FORMAT_OPTION(BorderBottom);
}
/* }}} */

/* {{{ proto int ExcelFormat::borderBottomColor([int color])
	Get or set the cell bottom color */
EXCEL_METHOD(Format, borderBottomColor)
{
	PHP_EXCEL_LONG_FORMAT_OPTION(BorderBottomColor);
}
/* }}} */

/* {{{ proto int ExcelFormat::borderDiagonalStyle([int style])
	Get or set the cell diagonal border */
EXCEL_METHOD(Format, borderDiagonalStyle)
{
	PHP_EXCEL_LONG_FORMAT_OPTION(BorderDiagonal);
}
/* }}} */

/* {{{ proto int ExcelFormat::borderDiagonalColor([int color])
	Get or set the cell diagonal color */
EXCEL_METHOD(Format, borderDiagonalColor)
{
	PHP_EXCEL_LONG_FORMAT_OPTION(BorderDiagonalColor);
}
/* }}} */

/* {{{ proto int ExcelFormat::fillPattern([int patern])
	Get or set the cell fill pattern */
EXCEL_METHOD(Format, fillPattern)
{
	PHP_EXCEL_LONG_FORMAT_OPTION(FillPattern);
}
/* }}} */

/* {{{ proto int ExcelFormat::patternForegroundColor([int color])
	Get or set the cell pattern foreground color */
EXCEL_METHOD(Format, patternForegroundColor)
{
	PHP_EXCEL_LONG_FORMAT_OPTION(PatternForegroundColor);
}
/* }}} */

/* {{{ proto int ExcelFormat::patternBackgroundColor([int color])
	Get or set the cell pattern background color */
EXCEL_METHOD(Format, patternBackgroundColor)
{
	PHP_EXCEL_LONG_FORMAT_OPTION(PatternBackgroundColor);
}
/* }}} */

/* {{{ proto bool ExcelFormat::locked([bool locked])
	Get or set whether the cell is locked */
EXCEL_METHOD(Format, locked)
{
	PHP_EXCEL_BOOL_FORMAT_OPTION(Locked);
}
/* }}} */

/* {{{ proto bool ExcelFormat::hidden([bool hidden])
	Get or set whether the cell is hidden */
EXCEL_METHOD(Format, hidden)
{
	PHP_EXCEL_BOOL_FORMAT_OPTION(Hidden);
}
/* }}} */

/* {{{ proto ExcelSheet ExcelSheet::__construct(ExcelBook book, string name)
	Sheet Constructor. */
EXCEL_METHOD(Sheet, __construct)
{
	BookHandle book;
	SheetHandle sh;
	zval *object = ZEND_THIS;
	excel_sheet_object *obj;
	zval *zbook = NULL;
	zend_string *name_zs = NULL;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "OS", &zbook, excel_ce_book, &name_zs) == FAILURE) {
		RETURN_THROWS();
	}

	if (!zbook) {
		zend_throw_exception(NULL, "ExcelBook is required", 0);
		RETURN_THROWS();
	}
	if (!name_zs || ZSTR_LEN(name_zs) < 1) {
		zend_throw_exception(NULL, "Sheet name cannot be empty", 0);
		RETURN_THROWS();
	}
	if (ZSTR_LEN(name_zs) != strlen(ZSTR_VAL(name_zs))) {
		zend_throw_exception(NULL, "Sheet name must not contain NUL bytes", 0);
		RETURN_THROWS();
	}

	BOOK_FROM_OBJECT_THROW(book, zbook);

	obj = Z_EXCEL_SHEET_OBJ_P(object);
	EXCEL_REJECT_RECONSTRUCTION(obj, sheet);

	sh = xlBookAddSheet(book, ZSTR_VAL(name_zs), 0);

	if (!sh) {
		zend_throw_exception(NULL, "Failed to add sheet", 0);
		RETURN_THROWS();
	}

	obj->sheet = sh;
	obj->book = book;
	EXCEL_INIT_SHEET_PARENT(obj, zbook);
}
/* }}} */

/* {{{ proto int ExcelSheet::cellType(int row, int column)
	Get cell type */
EXCEL_METHOD(Sheet, cellType)
{
	zval *object = ZEND_THIS;
	SheetHandle sheet;
	zend_long row, col;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "ll", &row, &col) == FAILURE) {
		RETURN_FALSE;
	}

	EXCEL_VALIDATE_ROW_COL(row, col, object);
	SHEET_FROM_OBJECT(sheet, object);

	RETURN_LONG(xlSheetCellType(sheet, row, col));
}
/* }}} */

/* {{{ proto ExcelFormat ExcelSheet::cellFormat(int row, int column)
	Get cell format */
EXCEL_METHOD(Sheet, cellFormat)
{
	zval *object = ZEND_THIS;
	SheetHandle sheet;
	BookHandle book;
	FormatHandle format;
	zend_long row, col;
	excel_format_object *fo;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "ll", &row, &col) == FAILURE) {
		RETURN_FALSE;
	}

	EXCEL_VALIDATE_ROW_COL(row, col, object);
	SHEET_AND_BOOK_FROM_OBJECT(sheet, book, object);

	format = xlSheetCellFormat(sheet, row, col);
	if (!format) {
		RETURN_FALSE;
	}

	ZVAL_OBJ(return_value, excel_object_new_format(excel_ce_format));
	fo = Z_EXCEL_FORMAT_OBJ_P(return_value);
	fo->format = format;
	fo->book = book;
	EXCEL_INIT_PARENT(fo, object);
}
/* }}} */

/* {{{ proto void ExcelFormat ExcelSheet::setCellFormat(int row, int column, ExcelFormat format)
	Set cell format */
EXCEL_METHOD(Sheet, setCellFormat)
{
	zval *object = ZEND_THIS;
	SheetHandle sheet;
	FormatHandle format;
	zval *oformat;
	zend_long row, col;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "llO", &row, &col, &oformat, excel_ce_format) == FAILURE) {
		RETURN_FALSE;
	}

	EXCEL_VALIDATE_ROW_COL(row, col, object);
	SHEET_FROM_OBJECT(sheet, object);
	FORMAT_FROM_OBJECT(format, oformat);
	EXCEL_REQUIRE_SAME_BOOK(oformat, object);

	xlSheetSetCellFormat(sheet, row, col, format);
}
/* }}} */

/* Returns 1 on success, 0 on failure (caller may use xlBookErrorMessage),
 * -1 when date→timestamp conversion failed (caller should use a dedicated
 * message — the book error is unrelated). */
static int php_excel_read_cell(int row, int col, zval *val, SheetHandle sheet, BookHandle book, FormatHandle *format, bool read_formula)
{
	const char *s;
	if (read_formula && xlSheetIsFormula(sheet, row, col)) {
		s = xlSheetReadFormula(sheet, row, col, format);
		if (!s) {
			return 0;
		}
		ZVAL_STRING(val, (char *)s);
		return 1;
	}

	switch (xlSheetCellType(sheet, row, col)) {
		case CELLTYPE_EMPTY:
			*format = xlSheetCellFormat(sheet, row, col);
			ZVAL_EMPTY_STRING(val);
			return 1;

		case CELLTYPE_BLANK:
			if (!xlSheetReadBlank(sheet, row, col, format)) {
				return 0;
			}
			ZVAL_NULL(val);
			return 1;

		case CELLTYPE_NUMBER:
#ifdef CELLTYPE_STRICTDATE
		case CELLTYPE_STRICTDATE:
#endif
		{
			double d = xlSheetReadNum(sheet, row, col, format);
			if (xlSheetIsDate(sheet, row, col)) {
				zend_long dt = _php_excel_date_unpack(book, d);
				if (dt == -1) {
					return -1;
				}
				ZVAL_LONG(val, dt);
				return 1;
			}
			ZVAL_DOUBLE(val, d);
			return 1;
		}

		case CELLTYPE_STRING: {
			s = xlSheetReadStr(sheet, row, col, format);
			if (!s) {
				return 0;
			}
			ZVAL_STRING(val, (char *)s);
			return 1;
		}

		case CELLTYPE_BOOLEAN:
			ZVAL_BOOL(val, xlSheetReadBool(sheet, row, col, format));
			return 1;

		case CELLTYPE_ERROR:
			*format = xlSheetCellFormat(sheet, row, col);
			ZVAL_LONG(val, xlSheetReadError(sheet, row, col));
			return 1;
	}

	return 0;
}

static inline int php_excel_validate_read_row_bounds(SheetHandle sheet, zend_long row_start, zend_long *row_end, bool allow_default_end)
{
	int lr = xlSheetLastRow(sheet);

	if (row_start < 0 || row_start > lr) {
		php_error_docref(NULL, E_WARNING, "Invalid starting row number '" ZEND_LONG_FMT "'", row_start);
		return 0;
	}
	if (allow_default_end && *row_end == -1) {
		if (lr == 0) {
			/* Empty used range: leave end < start so callers build empty arrays. */
			*row_end = row_start - 1;
			return 1;
		}
		*row_end = lr - 1;
	}
	if (*row_end < row_start || *row_end > lr) {
		php_error_docref(NULL, E_WARNING, "Invalid ending row number '" ZEND_LONG_FMT "'", *row_end);
		return 0;
	}
	return 1;
}

static inline int php_excel_validate_read_col_bounds(SheetHandle sheet, zend_long col_start, zend_long *col_end, bool allow_default_end)
{
	int lc = xlSheetLastCol(sheet);

	if (col_start < 0 || col_start > lc) {
		php_error_docref(NULL, E_WARNING, "Invalid starting column number '" ZEND_LONG_FMT "'", col_start);
		return 0;
	}
	if (allow_default_end && *col_end == -1) {
		if (lc == 0) {
			*col_end = col_start - 1;
			return 1;
		}
		*col_end = lc - 1;
	}
	if (*col_end < col_start || *col_end > lc) {
		php_error_docref(NULL, E_WARNING, "Invalid ending column number '" ZEND_LONG_FMT "'", *col_end);
		return 0;
	}
	return 1;
}

/* {{{ proto array ExcelSheet::readRow(int row [, int start_col [, int end_column [, bool read_formula]]])
	Read an entire row worth of data */
EXCEL_METHOD(Sheet, readRow)
{
	zval *object = ZEND_THIS;
	zend_long row;
	zend_long col_start = 0;
	zend_long col_end = -1;
	int lc;
	SheetHandle sheet;
	BookHandle book;
	bool read_formula = 1;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "l|llb", &row, &col_start, &col_end, &read_formula) == FAILURE) {
		RETURN_FALSE;
	}

	SHEET_AND_BOOK_FROM_OBJECT(sheet, book, object);

	if (row < 0 || row > xlSheetLastRow(sheet)) {
		php_error_docref(NULL, E_WARNING, "Invalid row number '" ZEND_LONG_FMT "'", row);
		RETURN_FALSE;
	}

	lc = xlSheetLastCol(sheet);
	if (col_start < 0 || col_start > lc) {
		php_error_docref(NULL, E_WARNING, "Invalid starting column number '" ZEND_LONG_FMT "'", col_start);
		RETURN_FALSE;
	}

	/* xlSheetLastCol() returns the last used column index plus one
	 * (exclusive), so the unspecified-end default (-1) maps to the last
	 * used column. The bounds checks deliberately allow col_start/col_end
	 * up to lastCol() inclusive (one past the last used column); such reads
	 * just return empty cells, so the leniency is harmless. */
	if (col_end == -1) {
		if (lc == 0) {
			array_init(return_value);
			return;
		}
		col_end = lc - 1;
	}

	if (col_end < col_start || col_end > lc) {
		php_error_docref(NULL, E_WARNING, "Invalid ending column number '" ZEND_LONG_FMT "'", col_end);
		RETURN_FALSE;
	}

	/* Excel column max is 16384 (XFD); cap before array_init_size to avoid
	 * INT_MAX-sized bucket pre-allocation if libxl ever surfaces an extreme value. */
	if ((col_end - col_start) > 16384) {
		php_error_docref(NULL, E_WARNING, "Column range too large");
		RETURN_FALSE;
	}

	lc = col_start;

	array_init_size(return_value, col_end - col_start + 1);
	while (lc < (col_end + 1)) {
		zval value;
		ZVAL_UNDEF(&value);
		FormatHandle format = NULL;

		{
			int read_rc = php_excel_read_cell(row, lc, &value, sheet, book, &format, read_formula);
			if (read_rc != 1) {
				zval_ptr_dtor(&value);
				zval_ptr_dtor(return_value);
				if (read_rc < 0) {
					php_error_docref(NULL, E_WARNING, "Failed to convert Excel date to timestamp");
				} else {
					php_error_docref(NULL, E_WARNING, "Failed to read cell in row " ZEND_LONG_FMT ", column %d with error '%s'", row, lc, xlBookErrorMessage(book));
				}
				RETURN_FALSE;
			}
		}
		add_next_index_zval(return_value, &value);

		lc++;
	}
}
/* }}} */

/* {{{ proto array ExcelSheet::readCol(int column [, int start_row [, int end_row [, bool read_formula]]])
	Read an entire column worth of data */
EXCEL_METHOD(Sheet, readCol)
{
	zval *object = ZEND_THIS;
	zend_long col;
	zend_long row_start = 0;
	zend_long row_end = -1;
	int lc;
	SheetHandle sheet;
	BookHandle book;
	bool read_formula = 1;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "l|llb", &col, &row_start, &row_end, &read_formula) == FAILURE) {
		RETURN_FALSE;
	}

	SHEET_AND_BOOK_FROM_OBJECT(sheet, book, object);

	if (col < 0 || col > xlSheetLastCol(sheet)) {
		php_error_docref(NULL, E_WARNING, "Invalid column number '" ZEND_LONG_FMT "'", col);
		RETURN_FALSE;
	}

	lc = xlSheetLastRow(sheet);
	if (row_start < 0 || row_start > lc) {
		php_error_docref(NULL, E_WARNING, "Invalid starting row number '" ZEND_LONG_FMT "'", row_start);
		RETURN_FALSE;
	}

	/* xlSheetLastRow() returns the last used row index plus one (exclusive),
	 * so the unspecified-end default (-1) maps to the last used row. The
	 * bounds checks deliberately allow row_start/row_end up to lastRow()
	 * inclusive (one past the last used row); such reads just return empty
	 * cells, so the leniency is harmless. */
	if (row_end == -1) {
		if (lc == 0) {
			array_init(return_value);
			return;
		}
		row_end = lc - 1;
	}

	if (row_end < row_start || row_end > lc) {
		php_error_docref(NULL, E_WARNING, "Invalid ending row number '" ZEND_LONG_FMT "'", row_end);
		RETURN_FALSE;
	}

	/* Excel row max is 1048576 (XFD); cap before array_init_size. */
	if ((row_end - row_start) > 1048576) {
		php_error_docref(NULL, E_WARNING, "Row range too large");
		RETURN_FALSE;
	}

	lc = row_start;

	array_init_size(return_value, row_end - row_start + 1);
	while (lc < (row_end + 1)) {
		zval value;
		ZVAL_UNDEF(&value);
		FormatHandle format = NULL;

		{
			int read_rc = php_excel_read_cell(lc, col, &value, sheet, book, &format, read_formula);
			if (read_rc != 1) {
				zval_ptr_dtor(&value);
				zval_ptr_dtor(return_value);
				if (read_rc < 0) {
					php_error_docref(NULL, E_WARNING, "Failed to convert Excel date to timestamp");
				} else {
					php_error_docref(NULL, E_WARNING, "Failed to read cell in row %d, column " ZEND_LONG_FMT " with error '%s'", lc, col, xlBookErrorMessage(book));
				}
				RETURN_FALSE;
			}
		}
		add_next_index_zval(return_value, &value);

		lc++;
	}
}
/* }}} */

/* {{{ proto array ExcelSheet::readRange(int row_start, int row_end, int col_start, int col_end [, bool read_formula])
	Read a rectangular range of cells */
EXCEL_METHOD(Sheet, readRange)
{
	zval *object = ZEND_THIS;
	zend_long row_start, row_end, col_start, col_end;
	zend_long r, c;
	zend_ulong row_count, col_count;
	SheetHandle sheet;
	BookHandle book;
	bool read_formula = 1;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "llll|b", &row_start, &row_end, &col_start, &col_end, &read_formula) == FAILURE) {
		RETURN_FALSE;
	}

	SHEET_AND_BOOK_FROM_OBJECT(sheet, book, object);

	if (!php_excel_validate_read_row_bounds(sheet, row_start, &row_end, 0) ||
	    !php_excel_validate_read_col_bounds(sheet, col_start, &col_end, 0)) {
		RETURN_FALSE;
	}

	row_count = (zend_ulong)(row_end - row_start + 1);
	col_count = (zend_ulong)(col_end - col_start + 1);
	if (row_count != 0 && col_count > (PHP_EXCEL_MAX_RANGE_CELLS / row_count)) {
		php_error_docref(NULL, E_WARNING, "Cell range too large");
		RETURN_FALSE;
	}

	array_init_size(return_value, row_count);
	for (r = row_start; r <= row_end; r++) {
		zval row_value;
		array_init_size(&row_value, col_count);
		for (c = col_start; c <= col_end; c++) {
			zval value;
			FormatHandle format = NULL;

			ZVAL_UNDEF(&value);
			{
				int read_rc = php_excel_read_cell((int) r, (int) c, &value, sheet, book, &format, read_formula);
				if (read_rc != 1) {
					zval_ptr_dtor(&value);
					zval_ptr_dtor(&row_value);
					zval_ptr_dtor(return_value);
					if (read_rc < 0) {
						php_error_docref(NULL, E_WARNING, "Failed to convert Excel date to timestamp");
					} else {
						php_error_docref(NULL, E_WARNING, "Failed to read cell in row " ZEND_LONG_FMT ", column " ZEND_LONG_FMT " with error '%s'", r, c, xlBookErrorMessage(book));
					}
					RETURN_FALSE;
				}
			}
			add_next_index_zval(&row_value, &value);
		}
		add_next_index_zval(return_value, &row_value);
	}
}
/* }}} */

/* {{{ proto array ExcelSheet::readSparseRow(int row [, int start_col [, int end_column [, bool read_formula]]])
	Read non-empty cells from a row, keyed by original column index */
EXCEL_METHOD(Sheet, readSparseRow)
{
	zval *object = ZEND_THIS;
	zend_long row;
	zend_long col_start = 0;
	zend_long col_end = -1;
	zend_long c;
	int lr;
	SheetHandle sheet;
	BookHandle book;
	bool read_formula = 1;
	bool default_end;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "l|llb", &row, &col_start, &col_end, &read_formula) == FAILURE) {
		RETURN_FALSE;
	}
	default_end = (col_end == -1);

	SHEET_AND_BOOK_FROM_OBJECT(sheet, book, object);

	lr = xlSheetLastRow(sheet);
	if (row < 0 || row > lr) {
		php_error_docref(NULL, E_WARNING, "Invalid row number '" ZEND_LONG_FMT "'", row);
		RETURN_FALSE;
	}
	if (!php_excel_validate_read_col_bounds(sheet, col_start, &col_end, 1)) {
		RETURN_FALSE;
	}

	array_init(return_value);
	if (default_end) {
		int first_col = xlSheetFirstFilledCol(sheet);
		if (first_col > col_end) {
			return;
		}
		if (first_col > col_start) {
			col_start = first_col;
		}
	}
	for (c = col_start; c <= col_end; c++) {
		zval value;
		FormatHandle format = NULL;

		if (xlSheetCellType(sheet, (int) row, (int) c) == CELLTYPE_EMPTY) {
			continue;
		}
		ZVAL_UNDEF(&value);
		{
			int read_rc = php_excel_read_cell((int) row, (int) c, &value, sheet, book, &format, read_formula);
			if (read_rc != 1) {
				zval_ptr_dtor(&value);
				zval_ptr_dtor(return_value);
				if (read_rc < 0) {
					php_error_docref(NULL, E_WARNING, "Failed to convert Excel date to timestamp");
				} else {
					php_error_docref(NULL, E_WARNING, "Failed to read cell in row " ZEND_LONG_FMT ", column " ZEND_LONG_FMT " with error '%s'", row, c, xlBookErrorMessage(book));
				}
				RETURN_FALSE;
			}
		}
		add_index_zval(return_value, (zend_ulong)c, &value);
	}
}
/* }}} */

/* {{{ proto array ExcelSheet::readSparseCol(int column [, int start_row [, int end_row [, bool read_formula]]])
	Read non-empty cells from a column, keyed by original row index */
EXCEL_METHOD(Sheet, readSparseCol)
{
	zval *object = ZEND_THIS;
	zend_long col;
	zend_long row_start = 0;
	zend_long row_end = -1;
	zend_long r;
	int lc;
	SheetHandle sheet;
	BookHandle book;
	bool read_formula = 1;
	bool default_end;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "l|llb", &col, &row_start, &row_end, &read_formula) == FAILURE) {
		RETURN_FALSE;
	}
	default_end = (row_end == -1);

	SHEET_AND_BOOK_FROM_OBJECT(sheet, book, object);

	lc = xlSheetLastCol(sheet);
	if (col < 0 || col > lc) {
		php_error_docref(NULL, E_WARNING, "Invalid column number '" ZEND_LONG_FMT "'", col);
		RETURN_FALSE;
	}
	if (!php_excel_validate_read_row_bounds(sheet, row_start, &row_end, 1)) {
		RETURN_FALSE;
	}

	array_init(return_value);
	if (default_end) {
		int first_row = xlSheetFirstFilledRow(sheet);
		if (first_row > row_end) {
			return;
		}
		if (first_row > row_start) {
			row_start = first_row;
		}
	}
	for (r = row_start; r <= row_end; r++) {
		zval value;
		FormatHandle format = NULL;

		if (xlSheetCellType(sheet, (int) r, (int) col) == CELLTYPE_EMPTY) {
			continue;
		}
		ZVAL_UNDEF(&value);
		{
			int read_rc = php_excel_read_cell((int) r, (int) col, &value, sheet, book, &format, read_formula);
			if (read_rc != 1) {
				zval_ptr_dtor(&value);
				zval_ptr_dtor(return_value);
				if (read_rc < 0) {
					php_error_docref(NULL, E_WARNING, "Failed to convert Excel date to timestamp");
				} else {
					php_error_docref(NULL, E_WARNING, "Failed to read cell in row " ZEND_LONG_FMT ", column " ZEND_LONG_FMT " with error '%s'", r, col, xlBookErrorMessage(book));
				}
				RETURN_FALSE;
			}
		}
		add_index_zval(return_value, (zend_ulong)r, &value);
	}
}
/* }}} */

/* {{{ proto mixed ExcelSheet::read(int row, int column [, mixed &format [, bool read_formula]])
	Read data stored inside a cell */
EXCEL_METHOD(Sheet, read)
{
	zval *object = ZEND_THIS;
	excel_sheet_object *sheet_obj;
	excel_book_object *book_obj;
	SheetHandle sheet;
	BookHandle book;
	zend_long row, col;
	zval *oformat = NULL;
	FormatHandle format = NULL;
	zval format_result;
	bool read_formula = 1;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "ll|zb", &row, &col, &oformat, &read_formula) == FAILURE) {
		RETURN_FALSE;
	}

	/* Resolve the owning book once and reuse it for the coordinate limits and
	 * the stale-generation check (previously two parent-chain walks). */
	book_obj = php_excel_resolve_book_obj(object);
	EXCEL_VALIDATE_ROW_COL_PR(row, col, book_obj);

	sheet_obj = Z_EXCEL_SHEET_OBJ_P(object);
	sheet = sheet_obj->sheet;
	book = sheet_obj->book;
	if (!sheet) {
		php_error_docref(NULL, E_WARNING, "The sheet wasn't initialized");
		RETURN_FALSE;
	}
	CHECK_BOOK_AND_SHEET_GENERATION_PR(sheet_obj, book_obj);

	{
		int read_rc = php_excel_read_cell(row, col, return_value, sheet, book, &format, read_formula);
		if (read_rc != 1) {
			if (read_rc < 0) {
				php_error_docref(NULL, E_WARNING, "Failed to convert Excel date to timestamp");
			} else {
				php_error_docref(NULL, E_WARNING, "Failed to read cell in row " ZEND_LONG_FMT ", column " ZEND_LONG_FMT " with error '%s'", row, col, xlBookErrorMessage(book));
			}
			/* Clear the out-param last: it can run a userland destructor, so
			 * no native work (including xlBookErrorMessage above) may follow. */
			if (oformat) {
				ZVAL_DEREF(oformat);
				zval_ptr_dtor(oformat);
				ZVAL_NULL(oformat);
			}
			RETURN_FALSE;
		}
	}

	if (oformat) {
		excel_format_object *fo;

		/* Build the wrapper while the native handle and generation are current.
		 * Replacing the caller's old zval can invoke arbitrary userland. No native
		 * work may follow that destructor callback. */
		ZVAL_OBJ(&format_result, excel_object_new_format(excel_ce_format));
		fo = Z_EXCEL_FORMAT_OBJ_P(&format_result);
		fo->format = format;
		fo->book = book;
		EXCEL_INIT_PARENT(fo, object);

		ZVAL_DEREF(oformat);
		zval_ptr_dtor(oformat);
		ZVAL_COPY_VALUE(oformat, &format_result);
	}
}
/* }}} */

/* A finite double casts to zend_long without float-cast-overflow UB only
 * within [-2^63, 2^63). 2^63 (9223372036854775808.0) is exactly representable;
 * zend_long max is 2^63-1, so a value >= 2^63 (or < -2^63) is out of range.
 * The AS_DATE path casts the timestamp to zend_long, so guard it there. */
static zend_always_inline int php_excel_double_in_long_range(double d)
{
	return d >= -9223372036854775808.0 && d < 9223372036854775808.0;
}

/* Write an already-packed Excel date serial. When no explicit format was
 * supplied, apply the book's lazily-created shared date format. If that
 * format cannot be allocated (e.g. the style table is exhausted), fail
 * closed rather than writing a bare number that renders as a plain value
 * and misreports success. */
static int php_excel_write_date_serial(SheetHandle sheet, BookHandle book, excel_book_object *book_obj, int row, int col, double dt, FormatHandle format)
{
	if (!format) {
		if (!book_obj->default_date_format) {
			book_obj->default_date_format = xlBookAddFormat(book, NULL);
			if (book_obj->default_date_format) {
				xlFormatSetNumFormat(book_obj->default_date_format, NUMFORMAT_DATE);
			}
		}
		format = book_obj->default_date_format;
		if (!format) {
			return 0;
		}
	}
	return xlSheetWriteNum(sheet, row, col, dt, format);
}

/* Explicit dtype must match a zval type that write_cell can honor; otherwise
 * write would silently store a different kind (e.g. string + AS_DATE → text). */
static zend_always_inline int php_excel_dtype_matches_zval(zend_long dtype, zval *data)
{
	if (dtype == -1) {
		return 1;
	}
	/* A null writes a blank cell (or is skipped under excel.skip_empty)
	 * whatever the dtype says, so it is never a mismatch. Rejecting it would
	 * only break bulk writes of a typed column that has gaps. */
	if (Z_TYPE_P(data) == IS_NULL) {
		return 1;
	}
	switch (dtype) {
		case PHP_EXCEL_DATE:
			return Z_TYPE_P(data) == IS_LONG || Z_TYPE_P(data) == IS_DOUBLE;
		case PHP_EXCEL_FORMULA:
		case PHP_EXCEL_NUMERIC_STRING:
		case PHP_EXCEL_TEXT:
			return Z_TYPE_P(data) == IS_STRING;
		default:
			return 1;
	}
}

/* Pre-scan predicate for writeRow/writeCol: returns 0 when php_excel_write_cell
 * would reject this value for a PHP-side reason knowable without touching the
 * sheet (unsupported type, embedded NUL, unpackable AS_DATE timestamp). Lets
 * the bulk writers reject a bad row up front instead of committing the cells
 * before it and returning false mid-loop. libxl-side failures (style-table
 * exhaustion, disk) remain non-atomic and are out of scope. */
static const char *php_excel_cell_value_rejection(BookHandle book, zval *data, zend_long dtype)
{
try_again:
	if (Z_TYPE_P(data) == IS_REFERENCE) {
		data = Z_REFVAL_P(data);
		goto try_again;
	}
	if (!php_excel_dtype_matches_zval(dtype, data)) {
		return "value type does not match the requested data type";
	}
	switch (Z_TYPE_P(data)) {
		case IS_NULL:
		case IS_TRUE:
		case IS_FALSE:
			return NULL;
		case IS_LONG:
			if (dtype == PHP_EXCEL_DATE && _php_excel_date_pack(book, Z_LVAL_P(data)) == -1) {
				return "timestamp is outside the range Excel can store as a date";
			}
			return NULL;
		case IS_DOUBLE:
			if (!zend_finite(Z_DVAL_P(data))) {
				return "number is not finite (NAN/INF)";
			}
			if (dtype == PHP_EXCEL_DATE) {
				if (!php_excel_double_in_long_range(Z_DVAL_P(data))
				    || _php_excel_date_pack(book, (zend_long) Z_DVAL_P(data)) == -1) {
					return "timestamp is outside the range Excel can store as a date";
				}
			}
			return NULL;
		case IS_STRING:
			if (ZSTR_LEN(Z_STR_P(data)) > 0 && memchr(ZSTR_VAL(Z_STR_P(data)), 0, ZSTR_LEN(Z_STR_P(data)))) {
				return "string must not contain NUL bytes";
			}
			if (dtype == PHP_EXCEL_NUMERIC_STRING) {
				zend_long lval;
				double dval;
				if (is_numeric_string(Z_STRVAL_P(data), Z_STRLEN_P(data), &lval, &dval, 0) == IS_DOUBLE
				    && !zend_finite(dval)) {
					return "numeric string does not represent a finite number";
				}
			}
			return NULL;
		case IS_REFERENCE:
			data = Z_REFVAL_P(data);
			goto try_again;
		default:
			return "value type is not supported for cell writes";
	}
}

bool php_excel_write_cell(SheetHandle sheet, excel_book_object *book_obj, int row, int col, zval *data, FormatHandle format, zend_long dtype)
{
	zend_string *data_zs;
	BookHandle book = book_obj->book;

	try_again:
	if (Z_TYPE_P(data) == IS_REFERENCE) {
		ZVAL_DEREF(data);
		goto try_again;
	}
	if (!php_excel_dtype_matches_zval(dtype, data)) {
		return 0;
	}
	switch (Z_TYPE_P(data)) {
		case IS_NULL:
			if (EXCEL_G(ini_skip_empty) > 0) {
				return 1;
			}
			return xlSheetWriteBlank(sheet, row, col, format);

		case IS_LONG:
			if (dtype == PHP_EXCEL_DATE) {
				double dt;
				if ((dt = _php_excel_date_pack(book, Z_LVAL_P(data))) == -1) {
					return 0;
				}
				return php_excel_write_date_serial(sheet, book, book_obj, row, col, dt, format);
			} else {
				return xlSheetWriteNum(sheet, row, col, (double) Z_LVAL_P(data), format);
			}

		case IS_DOUBLE:
			/* NAN/INF serialize to a corrupt cell that reads back as garbage,
			 * and the AS_DATE cast to zend_long below is undefined for a
			 * non-finite double. Reject before either. */
			if (!zend_finite(Z_DVAL_P(data))) {
				return 0;
			}
			if (dtype == PHP_EXCEL_DATE) {
				/* AS_DATE for a float timestamp: pack the whole-second part
				 * as an Excel date serial, mirroring the IS_LONG path.
				 * Without this a float unix time is written as a bare number
				 * that renders as a plain value, not a date. A finite value
				 * beyond zend_long range cannot be cast without UB, so reject
				 * it rather than truncating to a garbage timestamp. */
				double dt;
				if (!php_excel_double_in_long_range(Z_DVAL_P(data))) {
					return 0;
				}
				if ((dt = _php_excel_date_pack(book, (zend_long) Z_DVAL_P(data))) == -1) {
					return 0;
				}
				return php_excel_write_date_serial(sheet, book, book_obj, row, col, dt, format);
			}
			return xlSheetWriteNum(sheet, row, col, Z_DVAL_P(data), format);

		case IS_STRING:
			data_zs = Z_STR_P(data);
			/* libxl writes use NUL-terminated C strings; reject embedded
			 * NULs so the caller's value isn't silently truncated. */
			if (ZSTR_LEN(data_zs) > 0 && memchr(ZSTR_VAL(data_zs), 0, ZSTR_LEN(data_zs))) {
				return 0;
			}
			/* AS_TEXT writes the value verbatim: no quote-prefix stripping, no
			 * formula promotion, no numeric coercion, no skip_empty. This is
			 * the safe dtype for untrusted input (formula injection). */
			if (dtype == PHP_EXCEL_TEXT) {
				return xlSheetWriteStr(sheet, row, col, (const char*) ZSTR_VAL(data_zs), format);
			}
			if (Z_STRLEN_P(data) > 0 && '\'' == Z_STRVAL_P(data)[0]) {
				return xlSheetWriteStr(sheet, row, col, (const char*) ZSTR_VAL(data_zs) + 1, format);
			}
			/* Implicit '=' formula promotion applies only when no dtype was
			 * passed; an explicit dtype is honored as given. */
			if (dtype == -1 && Z_STRLEN_P(data) > 0 && '=' == Z_STRVAL_P(data)[0]) {
				dtype = PHP_EXCEL_FORMULA;
			}
			if (dtype == PHP_EXCEL_FORMULA) {
				return xlSheetWriteFormula(sheet, row, col, Z_STRVAL_P(data), format);
			} else {
				if (dtype == PHP_EXCEL_NUMERIC_STRING) {
					zend_long lval;
					double dval;

					switch (is_numeric_string(Z_STRVAL_P(data), Z_STRLEN_P(data), &lval, &dval, 0)) {
						case IS_LONG:
							return xlSheetWriteNum(sheet, row, col, (double) lval, format);

						case IS_DOUBLE:
							if (!zend_finite(dval)) {
								php_error_docref(NULL, E_WARNING, "Cannot write a non-finite number (NAN/INF) to a cell");
								return 0;
							}
							return xlSheetWriteNum(sheet, row, col, dval, format);
					}
				}
				if (Z_STRLEN_P(data) == 0 && EXCEL_G(ini_skip_empty) == 2) {
					return 1;
				}
				return xlSheetWriteStr(sheet, row, col, (const char*) ZSTR_VAL(data_zs), format);
			}

		case IS_TRUE:
			return xlSheetWriteBool(sheet, row, col, 1, format);

		case IS_FALSE:
			return xlSheetWriteBool(sheet, row, col, 0, format);

		case IS_REFERENCE:
			ZVAL_DEREF(data);
			goto try_again;

		default:
			return 0;
	}
}

/* {{{ proto bool ExcelSheet::write(int row, int column, mixed data [, ExcelFormat format [, int datatype]])
	Write data into a cell */
EXCEL_METHOD(Sheet, write)
{
	zval *object = ZEND_THIS;
	excel_sheet_object *sheet_obj;
	excel_book_object *book_obj;
	SheetHandle sheet;
	BookHandle book;
	FormatHandle format = NULL;
	zend_long row, col;
	zval *oformat = NULL;
	zend_long dtype = -1;
	zval *data;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "llz|O!l", &row, &col, &data, &oformat, excel_ce_format, &dtype) == FAILURE) {
		RETURN_FALSE;
	}

	/* Resolve the owning book once and reuse it for the coordinate limits, the
	 * stale-generation check, and the write (previously three separate
	 * parent-chain walks per single-cell write). */
	book_obj = php_excel_resolve_book_obj(object);
	EXCEL_VALIDATE_ROW_COL_PR(row, col, book_obj);

	sheet_obj = Z_EXCEL_SHEET_OBJ_P(object);
	sheet = sheet_obj->sheet;
	book = sheet_obj->book;
	if (!sheet) {
		php_error_docref(NULL, E_WARNING, "The sheet wasn't initialized");
		RETURN_FALSE;
	}
	CHECK_BOOK_AND_SHEET_GENERATION_PR(sheet_obj, book_obj);

	if (oformat) {
		FORMAT_FROM_OBJECT(format, oformat);
		EXCEL_REQUIRE_SAME_BOOK(oformat, object);
	}
	{
		const char *rejection = php_excel_cell_value_rejection(book, data, dtype);
		if (rejection) {
			php_error_docref(NULL, E_WARNING, "Cell value cannot be written: %s", rejection);
			RETURN_FALSE;
		}
	}

	if (!php_excel_write_cell(sheet, book_obj, row, col, data, oformat ? format : 0, dtype)) {
		php_error_docref(NULL, E_WARNING, "Failed to write cell in row " ZEND_LONG_FMT ", column " ZEND_LONG_FMT " with error '%s'", row, col, xlBookErrorMessage(book));
		RETURN_FALSE;
	}

	RETURN_TRUE;
}
/* }}} */

/* {{{ proto bool ExcelSheet::writeRow(int row, array data [, int start_column [, ExcelFormat format [, int datatype]]])
	Write an array of values into a row */
EXCEL_METHOD(Sheet, writeRow)
{
	zval *object = ZEND_THIS;
	SheetHandle sheet;
	BookHandle book;
	FormatHandle format = NULL;
	zend_long row, col = 0;
	zval *oformat = NULL;
	zval *data;
	zval *element;
	zend_long i;
	zend_long dtype = -1;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "la|lO!l", &row, &data, &col, &oformat, excel_ce_format, &dtype) == FAILURE) {
		RETURN_FALSE;
	}

	/* One-shot book resolve for coordinate limits, generation check, and write. */
	{
		excel_book_object *book_obj;
		excel_sheet_object *sheet_obj;
		zend_ulong count;

		book_obj = php_excel_resolve_book_obj(object);
		EXCEL_VALIDATE_ROW_COL_PR(row, col, book_obj);

		count = zend_array_count(Z_ARRVAL_P(data));
		{
			zend_long _maxc = (book_obj && book_obj->is_xlsx) ? EXCEL_MAX_COL_XLSX : EXCEL_MAX_COL_XLS;
			if (count > (zend_ulong)(_maxc - col + 1)) {
				php_error_docref(NULL, E_WARNING,
					"writeRow would overflow column range: start=" ZEND_LONG_FMT
					", count=" ZEND_ULONG_FMT, col, count);
				RETURN_FALSE;
			}
		}

		sheet_obj = Z_EXCEL_SHEET_OBJ_P(object);
		sheet = sheet_obj->sheet;
		book = sheet_obj->book;
		if (!sheet) {
			php_error_docref(NULL, E_WARNING, "The sheet wasn't initialized");
			RETURN_FALSE;
		}
		CHECK_BOOK_AND_SHEET_GENERATION_PR(sheet_obj, book_obj);

		if (oformat) {
			FORMAT_FROM_OBJECT(format, oformat);
			EXCEL_REQUIRE_SAME_BOOK(oformat, object);
		}

		i = col;

		{
			zend_long scan_col = col;
			ZEND_HASH_FOREACH_VAL(Z_ARRVAL_P(data), element) {
				const char *rejection = php_excel_cell_value_rejection(book, element, dtype);
				if (rejection) {
					php_error_docref(NULL, E_WARNING, "writeRow: value for column " ZEND_LONG_FMT " cannot be written: %s; no cells were modified", scan_col, rejection);
					RETURN_FALSE;
				}
				scan_col++;
			} ZEND_HASH_FOREACH_END();
		}

		ZEND_HASH_FOREACH_VAL(Z_ARRVAL_P(data), element) {
			if (!php_excel_write_cell(sheet, book_obj, row, i++, element, (oformat ? format : 0), dtype)) {
				php_error_docref(NULL, E_WARNING, "Failed to write cell in row " ZEND_LONG_FMT ", column " ZEND_LONG_FMT " with error '%s'", row, i-1, xlBookErrorMessage(book));
				RETURN_FALSE;
			}
		} ZEND_HASH_FOREACH_END();
	}

	RETURN_TRUE;
}
/* }}} */

/* {{{ proto bool ExcelSheet::writeCol(int column, array data [, int start_row [, ExcelFormat format [, int datatype]]])
	Write an array of values into a column */
EXCEL_METHOD(Sheet, writeCol)
{
	zval *object = ZEND_THIS;
	SheetHandle sheet;
	BookHandle book;
	FormatHandle format = NULL;
	zend_long row = 0, col;
	zval *oformat = NULL;
	zval *data;
	zval *element;
	zend_long i;
	zend_long dtype = -1;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "la|lO!l", &col, &data, &row, &oformat, excel_ce_format, &dtype) == FAILURE) {
		RETURN_FALSE;
	}

	{
		excel_book_object *book_obj;
		excel_sheet_object *sheet_obj;
		zend_ulong count;

		book_obj = php_excel_resolve_book_obj(object);
		EXCEL_VALIDATE_ROW_COL_PR(row, col, book_obj);

		count = zend_array_count(Z_ARRVAL_P(data));
		{
			zend_long _maxr = (book_obj && book_obj->is_xlsx) ? EXCEL_MAX_ROW_XLSX : EXCEL_MAX_ROW_XLS;
			if (count > (zend_ulong)(_maxr - row + 1)) {
				php_error_docref(NULL, E_WARNING,
					"writeCol would overflow row range: start=" ZEND_LONG_FMT
					", count=" ZEND_ULONG_FMT, row, count);
				RETURN_FALSE;
			}
		}

		sheet_obj = Z_EXCEL_SHEET_OBJ_P(object);
		sheet = sheet_obj->sheet;
		book = sheet_obj->book;
		if (!sheet) {
			php_error_docref(NULL, E_WARNING, "The sheet wasn't initialized");
			RETURN_FALSE;
		}
		CHECK_BOOK_AND_SHEET_GENERATION_PR(sheet_obj, book_obj);

		if (oformat) {
			FORMAT_FROM_OBJECT(format, oformat);
			EXCEL_REQUIRE_SAME_BOOK(oformat, object);
		}

		i = row;

		{
			zend_long scan_row = row;
			ZEND_HASH_FOREACH_VAL(Z_ARRVAL_P(data), element) {
				const char *rejection = php_excel_cell_value_rejection(book, element, dtype);
				if (rejection) {
					php_error_docref(NULL, E_WARNING, "writeCol: value for row " ZEND_LONG_FMT " cannot be written: %s; no cells were modified", scan_row, rejection);
					RETURN_FALSE;
				}
				scan_row++;
			} ZEND_HASH_FOREACH_END();
		}

		ZEND_HASH_FOREACH_VAL(Z_ARRVAL_P(data), element) {
			if (!php_excel_write_cell(sheet, book_obj, i++, col, element, oformat ? format : 0, dtype)) {
				php_error_docref(NULL, E_WARNING, "Failed to write cell in row " ZEND_LONG_FMT ", column " ZEND_LONG_FMT " with error '%s'", i-1, col, xlBookErrorMessage(book));
				RETURN_FALSE;
			}
		} ZEND_HASH_FOREACH_END();
	}

	RETURN_TRUE;
}
/* }}} */

/* Cell-coordinate variant: (row, column). */
#define PHP_EXCEL_SHEET_GET_BOOL_STATE(func_name) \
	{ \
		SheetHandle sheet; \
		zval *object = ZEND_THIS; \
		zend_long r, c; \
		if (zend_parse_parameters(ZEND_NUM_ARGS(), "ll", &r, &c) == FAILURE) { \
			RETURN_FALSE; \
		} \
		EXCEL_VALIDATE_ROW_COL(r, c, object); \
		SHEET_FROM_OBJECT(sheet, object); \
		RETURN_BOOL(xlSheet ## func_name (sheet, r, c)); \
	}

/* Row-pair variant: (row_first, row_last). */
#define PHP_EXCEL_SHEET_ROW_RANGE_OP(func_name) \
	{ \
		SheetHandle sheet; \
		zval *object = ZEND_THIS; \
		zend_long rfirst, rlast; \
		if (zend_parse_parameters(ZEND_NUM_ARGS(), "ll", &rfirst, &rlast) == FAILURE) { \
			RETURN_FALSE; \
		} \
		EXCEL_VALIDATE_ROW_RANGE(rfirst, rlast, object); \
		SHEET_FROM_OBJECT(sheet, object); \
		RETURN_BOOL(xlSheet ## func_name (sheet, rfirst, rlast)); \
	}

/* Column-pair variant: (col_first, col_last). */
#define PHP_EXCEL_SHEET_COL_RANGE_OP(func_name) \
	{ \
		SheetHandle sheet; \
		zval *object = ZEND_THIS; \
		zend_long cfirst, clast; \
		if (zend_parse_parameters(ZEND_NUM_ARGS(), "ll", &cfirst, &clast) == FAILURE) { \
			RETURN_FALSE; \
		} \
		EXCEL_VALIDATE_COL_RANGE(cfirst, clast, object); \
		SHEET_FROM_OBJECT(sheet, object); \
		RETURN_BOOL(xlSheet ## func_name (sheet, cfirst, clast)); \
	}

/* {{{ proto bool ExcelSheet::isFormula(int row, int column)
	Determine if the cell contains a formula */
EXCEL_METHOD(Sheet, isFormula)
{
	PHP_EXCEL_SHEET_GET_BOOL_STATE(IsFormula)
}
/* }}} */

/* {{{ proto bool ExcelSheet::isDate(int row, int column)
	Determine if the cell contains a date */
EXCEL_METHOD(Sheet, isDate)
{
	zval *object = ZEND_THIS;
	zend_long r, c;
	SheetHandle sheet;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "ll", &r, &c) == FAILURE) {
		RETURN_FALSE;
	}

	EXCEL_VALIDATE_ROW_COL(r, c, object);
	SHEET_FROM_OBJECT(sheet, object);

	{
		int ctype = xlSheetCellType(sheet, r, c);
		if (ctype != CELLTYPE_NUMBER
#ifdef CELLTYPE_STRICTDATE
		    && ctype != CELLTYPE_STRICTDATE
#endif
		) {
			RETURN_FALSE;
		}
	}

	RETURN_BOOL(xlSheetIsDate(sheet, r, c));
}
/* }}} */

/* {{{ proto bool ExcelSheet::insertRow(int row_first, int row_last)
	Inserts rows from rowFirst to rowLast */
EXCEL_METHOD(Sheet, insertRow)
{
	PHP_EXCEL_SHEET_ROW_RANGE_OP(InsertRow)
}
/* }}} */

/* {{{ proto bool ExcelSheet::insertCol(int col_first, int col_last)
	Inserts columns from colFirst to colLast */
EXCEL_METHOD(Sheet, insertCol)
{
	PHP_EXCEL_SHEET_COL_RANGE_OP(InsertCol)
}
/* }}} */

/* {{{ proto bool ExcelSheet::removeRow(int row_first, int row_last)
	Removes rows from rowFirst to rowLast */
EXCEL_METHOD(Sheet, removeRow)
{
	PHP_EXCEL_SHEET_ROW_RANGE_OP(RemoveRow)
}
/* }}} */

/* {{{ proto bool ExcelSheet::removeCol(int col_first, int col_last)
	Removes columns from colFirst to colLast */
EXCEL_METHOD(Sheet, removeCol)
{
	PHP_EXCEL_SHEET_COL_RANGE_OP(RemoveCol)
}
/* }}} */

#define PHP_EXCEL_SHEET_GET_DOUBLE_BY_COL(func_name) \
	{ \
		SheetHandle sheet; \
		zval *object = ZEND_THIS; \
		zend_long val; \
		if (zend_parse_parameters(ZEND_NUM_ARGS(), "l", &val) == FAILURE) { \
			RETURN_FALSE; \
		} \
		EXCEL_VALIDATE_COL(val, object); \
		SHEET_FROM_OBJECT(sheet, object); \
		RETURN_DOUBLE(xlSheet ## func_name (sheet, val)); \
	}

#define PHP_EXCEL_SHEET_GET_DOUBLE_BY_ROW(func_name) \
	{ \
		SheetHandle sheet; \
		zval *object = ZEND_THIS; \
		zend_long val; \
		if (zend_parse_parameters(ZEND_NUM_ARGS(), "l", &val) == FAILURE) { \
			RETURN_FALSE; \
		} \
		EXCEL_VALIDATE_ROW(val, object); \
		SHEET_FROM_OBJECT(sheet, object); \
		RETURN_DOUBLE(xlSheet ## func_name (sheet, val)); \
	}

/* {{{ proto double ExcelSheet::colWidth(int column)
	Returns the cell width */
EXCEL_METHOD(Sheet, colWidth)
{
	PHP_EXCEL_SHEET_GET_DOUBLE_BY_COL(ColWidth)
}
/* }}} */

/* {{{ proto double ExcelSheet::rowHeight(int row)
	Returns the cell height */
EXCEL_METHOD(Sheet, rowHeight)
{
	PHP_EXCEL_SHEET_GET_DOUBLE_BY_ROW(RowHeight)
}
/* }}} */

/* {{{ proto string ExcelSheet::readComment(int row, int column)
	Read comment from a cell */
EXCEL_METHOD(Sheet, readComment)
{
		SheetHandle sheet;
		zval *object = ZEND_THIS;
		const char *s;
		zend_long r, c;

		if (zend_parse_parameters(ZEND_NUM_ARGS(), "ll", &r, &c) == FAILURE) {
			RETURN_FALSE;
		}

		EXCEL_VALIDATE_ROW_COL(r, c, object);
		SHEET_FROM_OBJECT(sheet, object);

		s = xlSheetReadComment(sheet, r, c);
		if (!s) {
			RETURN_FALSE;
		}
		RETURN_STRING((char *)s);
}
/* }}} */

/* {{{ proto void ExcelSheet::writeComment(int row, int column, string value, string author, int width, int height)
	Write comment to a cell */
EXCEL_METHOD(Sheet, writeComment)
{
		SheetHandle sheet;
		zval *object = ZEND_THIS;
		zend_string *val_zs = NULL, *auth_zs = NULL;
		zend_long r, c, w, h;

		if (zend_parse_parameters(ZEND_NUM_ARGS(), "llSSll", &r, &c, &val_zs, &auth_zs, &w, &h) == FAILURE) {
			RETURN_FALSE;
		}

		EXCEL_NON_EMPTY_STRING(auth_zs)
		EXCEL_NON_EMPTY_STRING(val_zs)
		EXCEL_NUL_SAFE_STRING(val_zs)
		EXCEL_NUL_SAFE_STRING(auth_zs)
		EXCEL_VALIDATE_ROW_COL(r, c, object);
		EXCEL_VALIDATE_INT_RANGE(w)
		EXCEL_VALIDATE_INT_RANGE(h)

		SHEET_FROM_OBJECT(sheet, object);

		xlSheetWriteComment(sheet, r, c, ZSTR_VAL(val_zs), ZSTR_VAL(auth_zs), w, h);
}
/* }}} */

/* {{{ proto void ExcelSheet::setColWidth(int column_start, int column_end, double width [, bool hidden [, ExcelFormat format]])
	Set width of cells within column(s); Value -1 is used for autofit column widths in LibXL 3.6+ */
EXCEL_METHOD(Sheet, setColWidth)
{
		SheetHandle sheet;
		FormatHandle format = NULL;
		zval *object = ZEND_THIS;
		zend_long s, e;
		double width;
		zval *f = NULL;
		bool h = 0;

		if (zend_parse_parameters(ZEND_NUM_ARGS(), "lld|bO!", &s, &e, &width, &h, &f, excel_ce_format) == FAILURE) {
			RETURN_FALSE;
		}

		EXCEL_VALIDATE_FINITE(width)
		EXCEL_VALIDATE_COL_RANGE(s, e, object);
		SHEET_FROM_OBJECT(sheet, object);

		if (f) {
			FORMAT_FROM_OBJECT(format, f);
			EXCEL_REQUIRE_SAME_BOOK(f, object);
		}

		if (width < -1) {
			php_error_docref(NULL, E_WARNING, "Width cannot be less then -1");
			RETURN_FALSE;
		}

		RETURN_BOOL(xlSheetSetCol(sheet, s, e, width, f ? format : 0, h));
}
/* }}} */

/* {{{ proto bool ExcelSheet::setRowHeight(int row, double height [, ExcelFormat format [, bool hidden]])
	Set row height */
EXCEL_METHOD(Sheet, setRowHeight)
{
		SheetHandle sheet;
		FormatHandle format = NULL;
		zval *object = ZEND_THIS;
		zend_long row;
		double height;
		zval *f = NULL;
		bool h = 0;

		if (zend_parse_parameters(ZEND_NUM_ARGS(), "ld|O!b", &row, &height, &f, excel_ce_format, &h) == FAILURE) {
			RETURN_FALSE;
		}

		EXCEL_VALIDATE_FINITE(height)
		{
			excel_book_object *_vb = php_excel_resolve_book_obj(object);
			zend_long _maxr = (_vb && _vb->is_xlsx) ? EXCEL_MAX_ROW_XLSX : EXCEL_MAX_ROW_XLS;
			if (row < 0 || row > _maxr) {
				php_error_docref(NULL, E_WARNING, "Invalid row: " ZEND_LONG_FMT, row);
				RETURN_FALSE;
			}
		}
		SHEET_FROM_OBJECT(sheet, object);

		if (f) {
			FORMAT_FROM_OBJECT(format, f);
			EXCEL_REQUIRE_SAME_BOOK(f, object);
		}

		if (height < 0) {
			php_error_docref(NULL, E_WARNING, "Height cannot be less then 0");
			RETURN_FALSE;
		}

		RETURN_BOOL(xlSheetSetRow(sheet, row, height, f ? format : 0, h));
}
/* }}} */

/* {{{ proto array ExcelSheet::getMerge(int row, int column)
	Get cell merge range */
EXCEL_METHOD(Sheet, getMerge)
{
		SheetHandle sheet;
		zval *object = ZEND_THIS;
		zend_long row, col;
		int rowFirst, rowLast, colFirst, colLast;

		if (zend_parse_parameters(ZEND_NUM_ARGS(), "ll", &row, &col) == FAILURE) {
			RETURN_FALSE;
		}

		EXCEL_VALIDATE_ROW_COL(row, col, object);
		SHEET_FROM_OBJECT(sheet, object);

		if (!xlSheetGetMerge(sheet, row, col, &rowFirst, &rowLast, &colFirst, &colLast)) {
			RETURN_FALSE;
		}

		array_init(return_value);
		add_assoc_long(return_value, "row_first", rowFirst);
		add_assoc_long(return_value, "row_last", rowLast);
		add_assoc_long(return_value, "col_first", colFirst);
		add_assoc_long(return_value, "col_last", colLast);
}
/* }}} */

/* {{{ proto bool ExcelSheet::setMerge(int row_start, int row_end, int col_start, int col_end)
	Set cell merge range */
EXCEL_METHOD(Sheet, setMerge)
{
	SheetHandle sheet;
	zval *object = ZEND_THIS;
	zend_long row_s, col_s, row_e, col_e;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "llll", &row_s, &row_e, &col_s, &col_e) == FAILURE) {
		RETURN_FALSE;
	}

	EXCEL_VALIDATE_ROW_RANGE(row_s, row_e, object);
	EXCEL_VALIDATE_COL_RANGE(col_s, col_e, object);
	SHEET_FROM_OBJECT(sheet, object);

	RETURN_BOOL(xlSheetSetMerge(sheet, row_s, row_e, col_s, col_e));
}
/* }}} */

/* {{{ proto bool ExcelSheet::deleteMerge(int row, int column)
	Delete cell merge */
EXCEL_METHOD(Sheet, deleteMerge)
{
	SheetHandle sheet;
	zval *object = ZEND_THIS;
	zend_long row, col;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "ll", &row, &col) == FAILURE) {
		RETURN_FALSE;
	}

	EXCEL_VALIDATE_ROW_COL(row, col, object);
	SHEET_FROM_OBJECT(sheet, object);

	RETURN_BOOL(xlSheetDelMerge(sheet, row, col));
}
/* }}} */

/* {{{ proto void ExcelSheet::addPictureScaled(int row, int column, int pic_id, double scale [, int x_offset [, int y_offset]])
	Insert picture into a cell with a set scale */
EXCEL_METHOD(Sheet, addPictureScaled)
{
	SheetHandle sheet;
	zval *object = ZEND_THIS;
	zend_long row, col, pic_id;
	zend_long x_offset = 0, y_offset = 0, pos = 0;
	double scale;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "llld|lll", &row, &col, &pic_id, &scale, &x_offset, &y_offset, &pos) == FAILURE) {
		RETURN_FALSE;
	}

	EXCEL_VALIDATE_FINITE(scale)
	EXCEL_VALIDATE_ROW_COL(row, col, object);
	EXCEL_VALIDATE_INT_RANGE(pic_id)
	EXCEL_VALIDATE_INT_RANGE(x_offset)
	EXCEL_VALIDATE_INT_RANGE(y_offset)
	EXCEL_VALIDATE_INT_RANGE(pos)
	SHEET_FROM_OBJECT(sheet, object);

	xlSheetSetPicture(sheet, row, col, pic_id, scale, x_offset, y_offset
, pos
	);
}
/* }}} */

/* {{{ proto void ExcelSheet::addPictureDim(int row, int column, int pic_id, int width, int height [, int x_offset [, int y_offset]])
	Insert picture into a cell with a given dimensions */
EXCEL_METHOD(Sheet, addPictureDim)
{
	SheetHandle sheet;
	zval *object = ZEND_THIS;
	zend_long row, col, pic_id, w, h;
	zend_long x_offset = 0, y_offset = 0, pos = 0;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "lllll|lll", &row, &col, &pic_id, &w, &h, &x_offset, &y_offset, &pos) == FAILURE) {
		RETURN_FALSE;
	}

	EXCEL_VALIDATE_ROW_COL(row, col, object);
	EXCEL_VALIDATE_INT_RANGE(pic_id)
	EXCEL_VALIDATE_INT_RANGE(w)
	EXCEL_VALIDATE_INT_RANGE(h)
	EXCEL_VALIDATE_INT_RANGE(x_offset)
	EXCEL_VALIDATE_INT_RANGE(y_offset)
	EXCEL_VALIDATE_INT_RANGE(pos)
	SHEET_FROM_OBJECT(sheet, object);

	xlSheetSetPicture2(sheet, row, col, pic_id, w, h, x_offset, y_offset
, pos
	);
}
/* }}} */

#define PHP_EXCEL_SHEET_SET_ROW_BREAK(func_name) \
	{ \
		SheetHandle sheet; \
		zval *object = ZEND_THIS; \
		zend_long val; \
		bool brk; \
		if (zend_parse_parameters(ZEND_NUM_ARGS(), "lb", &val, &brk) == FAILURE) { \
			RETURN_FALSE; \
		} \
		EXCEL_VALIDATE_ROW(val, object); \
		SHEET_FROM_OBJECT(sheet, object); \
		RETURN_BOOL(xlSheet ## func_name (sheet, val, brk)); \
	}

#define PHP_EXCEL_SHEET_SET_COL_BREAK(func_name) \
	{ \
		SheetHandle sheet; \
		zval *object = ZEND_THIS; \
		zend_long val; \
		bool brk; \
		if (zend_parse_parameters(ZEND_NUM_ARGS(), "lb", &val, &brk) == FAILURE) { \
			RETURN_FALSE; \
		} \
		EXCEL_VALIDATE_COL(val, object); \
		SHEET_FROM_OBJECT(sheet, object); \
		RETURN_BOOL(xlSheet ## func_name (sheet, val, brk)); \
	}

/* {{{ proto bool ExcelSheet::horPageBreak(int row, bool break)
	Set/Remove horizontal page break */
EXCEL_METHOD(Sheet, horPageBreak)
{
	PHP_EXCEL_SHEET_SET_ROW_BREAK(SetHorPageBreak)
}
/* }}} */

/* {{{ proto bool ExcelSheet::verPageBreak(int col, bool break)
	Set/Remove vertical page break */
EXCEL_METHOD(Sheet, verPageBreak)
{
	PHP_EXCEL_SHEET_SET_COL_BREAK(SetVerPageBreak)
}
/* }}} */

/* {{{ proto void ExcelSheet::splitSheet(int row, int column)
	Split sheet at indicated position */
EXCEL_METHOD(Sheet, splitSheet)
{
	SheetHandle sheet;
	zval *object = ZEND_THIS;
	zend_long row, col;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "ll", &row, &col) == FAILURE) {
		RETURN_FALSE;
	}

	EXCEL_VALIDATE_ROW_COL(row, col, object);
	SHEET_FROM_OBJECT(sheet, object);

	xlSheetSplit(sheet, row, col);
}
/* }}} */

#define PHP_EXCEL_SHEET_GROUP_ROWS(func_name) \
	{ \
		SheetHandle sheet; \
		zval *object = ZEND_THIS; \
		zend_long s, e; \
		bool brk = 0; \
		if (zend_parse_parameters(ZEND_NUM_ARGS(), "ll|b", &s, &e, &brk) == FAILURE) { \
			RETURN_FALSE; \
		} \
		EXCEL_VALIDATE_ROW_RANGE(s, e, object); \
		SHEET_FROM_OBJECT(sheet, object); \
		RETURN_BOOL(xlSheet ## func_name (sheet, s, e, brk)); \
	}

#define PHP_EXCEL_SHEET_GROUP_COLS(func_name) \
	{ \
		SheetHandle sheet; \
		zval *object = ZEND_THIS; \
		zend_long s, e; \
		bool brk = 0; \
		if (zend_parse_parameters(ZEND_NUM_ARGS(), "ll|b", &s, &e, &brk) == FAILURE) { \
			RETURN_FALSE; \
		} \
		EXCEL_VALIDATE_COL_RANGE(s, e, object); \
		SHEET_FROM_OBJECT(sheet, object); \
		RETURN_BOOL(xlSheet ## func_name (sheet, s, e, brk)); \
	}

/* {{{ proto bool ExcelSheet::groupRows(int start_row, int end_row [, bool collapse])
	Group rows from rowFirst to rowLast */
EXCEL_METHOD(Sheet, groupRows)
{
	PHP_EXCEL_SHEET_GROUP_ROWS(GroupRows)
}
/* }}} */

/* {{{ proto bool ExcelSheet::groupCols(int start_column, int end_column [, bool collapse])
	Group columns from colFirst to colLast */
EXCEL_METHOD(Sheet, groupCols)
{
	PHP_EXCEL_SHEET_GROUP_COLS(GroupCols)
}
/* }}} */

/* {{{ proto void ExcelSheet::clear(int row_s, int row_e, int col_s, int col_e)
	Clear cells in specified area. */
EXCEL_METHOD(Sheet, clear)
{
	SheetHandle sheet;
	zval *object = ZEND_THIS;
	zend_long row_s, col_s, col_e, row_e;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "llll", &row_s, &row_e, &col_s, &col_e) == FAILURE) {
		RETURN_FALSE;
	}

	EXCEL_VALIDATE_ROW_RANGE(row_s, row_e, object);
	EXCEL_VALIDATE_COL_RANGE(col_s, col_e, object);
	SHEET_FROM_OBJECT(sheet, object);

	xlSheetClear(sheet, row_s, row_e, col_s, col_e);
}
/* }}} */

/* {{{ proto void ExcelSheet::copy(int row, int col, int to_row, int to_col)
	Copy a cell */
EXCEL_METHOD(Sheet, copy)
{
	SheetHandle sheet;
	zval *object = ZEND_THIS;
	zend_long row, col, to_row, to_col;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "llll", &row, &col, &to_row, &to_col) == FAILURE) {
		RETURN_FALSE;
	}

	EXCEL_VALIDATE_ROW_COL(row, col, object);
	EXCEL_VALIDATE_ROW_COL(to_row, to_col, object);
	SHEET_FROM_OBJECT(sheet, object);

	RETURN_BOOL(xlSheetCopyCell(sheet, row, col, to_row, to_col));
}
/* }}} */


/* No-arg Sheet method that calls a void libxl op and returns true. */
#define PHP_EXCEL_SHEET_VOID(func_name) \
{ \
	SheetHandle sheet; \
	zval *object = ZEND_THIS; \
	ZEND_PARSE_PARAMETERS_NONE(); \
	SHEET_FROM_OBJECT(sheet, object); \
	xlSheet ## func_name (sheet); \
	RETURN_TRUE; \
}

#define PHP_EXCEL_INFO(func_name, type) \
{ \
	SheetHandle sheet; \
	zval *object = ZEND_THIS; \
	ZEND_PARSE_PARAMETERS_NONE(); \
	SHEET_FROM_OBJECT(sheet, object); \
	PE_RETURN_ ## type (xlSheet ## func_name (sheet)); \
}

#define PHP_EXCEL_SET_BOOL_VAL(func_name) \
	{ \
		SheetHandle sheet; \
		zval *object = ZEND_THIS; \
		bool val; \
		if (zend_parse_parameters(ZEND_NUM_ARGS(), "b", &val) == FAILURE) { \
			RETURN_FALSE; \
		} \
		SHEET_FROM_OBJECT(sheet, object); \
		xlSheet ## func_name (sheet, val); \
	}

#define PHP_EXCEL_SET_LONG_VAL(func_name) \
	{ \
		SheetHandle sheet; \
		zval *object = ZEND_THIS; \
		zend_long val; \
		if (zend_parse_parameters(ZEND_NUM_ARGS(), "l", &val) == FAILURE) { \
			RETURN_FALSE; \
		} \
		EXCEL_VALIDATE_INT_RANGE(val) \
		SHEET_FROM_OBJECT(sheet, object); \
		xlSheet ## func_name (sheet, val); \
	}

#define PHP_EXCEL_SET_DOUBLE_VAL(func_name) \
	{ \
		SheetHandle sheet; \
		zval *object = ZEND_THIS; \
		double val; \
		if (zend_parse_parameters(ZEND_NUM_ARGS(), "d", &val) == FAILURE) { \
			RETURN_FALSE; \
		} \
		EXCEL_VALIDATE_FINITE(val) \
		SHEET_FROM_OBJECT(sheet, object); \
		xlSheet ## func_name (sheet, val); \
	}

/* {{{ proto int ExcelSheet::firstRow()
	Returns the first row in the sheet that contains a used cell. */
EXCEL_METHOD(Sheet, firstRow)
{
	PHP_EXCEL_INFO(FirstRow, IS_LONG)
}
/* }}} */

/* {{{ proto int ExcelSheet::lastRow()
	Returns the zero-based index of the row after the last row in the sheet that contains a used cell. */
EXCEL_METHOD(Sheet, lastRow)
{
	PHP_EXCEL_INFO(LastRow, IS_LONG)
}
/* }}} */

/* {{{ proto int ExcelSheet::firstCol()
	Returns the first column in the sheet that contains a used cell. */
EXCEL_METHOD(Sheet, firstCol)
{
	PHP_EXCEL_INFO(FirstCol, IS_LONG)
}
/* }}} */

/* {{{ proto int ExcelSheet::lastCol()
	Returns the zero-based index of the column after the last column in the sheet that contains a used cell. */
EXCEL_METHOD(Sheet, lastCol)
{
	PHP_EXCEL_INFO(LastCol, IS_LONG)
}
/* }}} */

/* {{{ proto bool ExcelSheet::displayGridlines()
	Returns whether the gridlines are displayed */
EXCEL_METHOD(Sheet, displayGridlines)
{
	PHP_EXCEL_INFO(DisplayGridlines, IS_BOOL)
}
/* }}} */

/* {{{ proto bool ExcelSheet::printGridlines()
	Returns whether the gridlines are printed */
EXCEL_METHOD(Sheet, printGridlines)
{
	PHP_EXCEL_INFO(PrintGridlines, IS_BOOL)
}
/* }}} */

/* {{{ proto void ExcelSheet::setDisplayGridlines(bool value)
	Sets gridlines for displaying */
EXCEL_METHOD(Sheet, setDisplayGridlines)
{
	PHP_EXCEL_SET_BOOL_VAL(SetDisplayGridlines)
}
/* }}} */

/* {{{ proto bool ExcelSheet::setHidden(bool value)
	Hides/unhides the sheet. */
EXCEL_METHOD(Sheet, setHidden)
{
	SheetHandle sheet;
	zval *object = ZEND_THIS;
	bool val;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "b", &val) == FAILURE) {
		RETURN_FALSE;
	}

	SHEET_FROM_OBJECT(sheet, object);

	RETURN_BOOL(xlSheetSetHidden(sheet, val));
}
/* }}} */

/* {{{ proto bool ExcelSheet::isHidden()
	Returns whether sheet is hidden. */
EXCEL_METHOD(Sheet, isHidden)
{
	PHP_EXCEL_INFO(Hidden, IS_BOOL)
}
/* }}} */

/* {{{ proto array ExcelSheet::getTopLeftView()
	Extracts the first visible row and the leftmost visible column of the sheet. */
EXCEL_METHOD(Sheet, getTopLeftView)
{
	SheetHandle sheet;
	zval *object = ZEND_THIS;
	int r = 0, c = 0;

	ZEND_PARSE_PARAMETERS_NONE();

	SHEET_FROM_OBJECT(sheet, object);

	xlSheetGetTopLeftView(sheet, &r, &c);

	array_init(return_value);
	add_assoc_long(return_value, "row", r);
	add_assoc_long(return_value, "column", c);
}
/* }}} */

/* {{{ proto bool ExcelSheet::setTopLeftView(int row, int column)
	Sets the first visible row and the leftmost visible column of the sheet. */
EXCEL_METHOD(Sheet, setTopLeftView)
{
	SheetHandle sheet;
	zval *object = ZEND_THIS;
	zend_long r,c;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "ll", &r, &c) == FAILURE) {
		RETURN_FALSE;
	}

	EXCEL_VALIDATE_ROW_COL(r, c, object);

	SHEET_FROM_OBJECT(sheet, object);

	xlSheetSetTopLeftView(sheet, r, c);
	RETURN_TRUE;
}
/* }}} */

/* {{{ proto string ExcelSheet::rowColToAddr(int row, int col, boolean row_relative, boolean col_relative)
	Converts row and column to a cell reference. */
EXCEL_METHOD(Sheet, rowColToAddr)
{
	SheetHandle sheet;
	zval *object = ZEND_THIS;
	bool row_relative = 1, col_relative = 1;
	zend_long row, col;
	const char *cel_ref;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "ll|bb", &row, &col, &row_relative, &col_relative) == FAILURE) {
		RETURN_FALSE;
	}

	EXCEL_VALIDATE_ROW_COL(row, col, object);
	SHEET_FROM_OBJECT(sheet, object);

	cel_ref = xlSheetRowColToAddr(sheet, row, col, row_relative, col_relative);
	if (!cel_ref) {
		RETURN_FALSE;
	}
	RETURN_STRING(cel_ref);
}
/* }}} */

static bool php_excel_parse_cell_reference(
	zend_string *reference,
	int max_row,
	int max_col,
	int *row,
	int *col,
	int *row_relative,
	int *col_relative
)
{
	const unsigned char *p = (const unsigned char *) ZSTR_VAL(reference);
	const unsigned char *end = p + ZSTR_LEN(reference);
	unsigned int parsed_col = 0;
	unsigned int parsed_row = 0;
	bool has_col = false;
	bool has_row = false;

	*col_relative = 1;
	*row_relative = 1;
	if (p < end && *p == '$') {
		*col_relative = 0;
		p++;
	}
	while (p < end && ((*p >= 'A' && *p <= 'Z') || (*p >= 'a' && *p <= 'z'))) {
		unsigned int digit = (*p >= 'a' ? *p - 'a' : *p - 'A') + 1;
		if (parsed_col > ((unsigned int) max_col + 1 - digit) / 26) {
			return false;
		}
		parsed_col = parsed_col * 26 + digit;
		has_col = true;
		p++;
	}
	if (!has_col) {
		return false;
	}
	if (p < end && *p == '$') {
		*row_relative = 0;
		p++;
	}
	while (p < end && *p >= '0' && *p <= '9') {
		unsigned int digit = *p - '0';
		if (!has_row && digit == 0) {
			return false;
		}
		if (parsed_row > ((unsigned int) max_row + 1 - digit) / 10) {
			return false;
		}
		parsed_row = parsed_row * 10 + digit;
		has_row = true;
		p++;
	}
	if (!has_row || p != end || parsed_col == 0 || parsed_row == 0
	    || parsed_col > (unsigned int) max_col + 1
	    || parsed_row > (unsigned int) max_row + 1) {
		return false;
	}

	*col = (int) parsed_col - 1;
	*row = (int) parsed_row - 1;
	return true;
}

/* {{{ proto array ExcelSheet::addrToRowCol(string cell_reference)
	Converts a cell reference to row and column. */
EXCEL_METHOD(Sheet, addrToRowCol)
{
	SheetHandle sheet;
	zval *object = ZEND_THIS;
	zend_string *cell_reference_zs = NULL;
	int row = 0, col = 0, rowRelative = 0, colRelative = 0;
	excel_book_object *book_obj;
	int max_row, max_col;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "S", &cell_reference_zs) == FAILURE) {
		RETURN_FALSE;
	}

	if (!cell_reference_zs || ZSTR_LEN(cell_reference_zs) < 1) {
		php_error_docref(NULL, E_WARNING, "Cell reference cannot be empty");
		RETURN_FALSE;
	}
	EXCEL_NUL_SAFE_STRING(cell_reference_zs)

	SHEET_FROM_OBJECT(sheet, object);
	(void) sheet;
	book_obj = php_excel_resolve_book_obj(object);
	max_row = (book_obj && book_obj->is_xlsx) ? EXCEL_MAX_ROW_XLSX : EXCEL_MAX_ROW_XLS;
	max_col = (book_obj && book_obj->is_xlsx) ? EXCEL_MAX_COL_XLSX : EXCEL_MAX_COL_XLS;
	if (!php_excel_parse_cell_reference(cell_reference_zs, max_row, max_col,
	    &row, &col, &rowRelative, &colRelative)) {
		php_error_docref(NULL, E_WARNING, "Invalid cell reference");
		RETURN_FALSE;
	}

	array_init(return_value);
	add_assoc_long(return_value, "row", row);
	add_assoc_long(return_value, "column", col);
	add_assoc_bool(return_value, "col_relative", colRelative);
	add_assoc_bool(return_value, "row_relative", rowRelative);
}
/* }}} */

/* {{{ proto void ExcelSheet::setPrintGridlines(bool value)
	Sets gridlines for printing */
EXCEL_METHOD(Sheet, setPrintGridlines)
{
	PHP_EXCEL_SET_BOOL_VAL(SetPrintGridlines)
}
/* }}} */

/* {{{ proto int ExcelSheet::zoom()
	Returns the zoom level of the current view as a percentage. */
EXCEL_METHOD(Sheet, zoom)
{
	PHP_EXCEL_INFO(Zoom, IS_LONG)
}
/* }}} */

/* {{{ proto int ExcelSheet::zoomPrint()
	Returns the scaling factor for printing as a percentage. */
EXCEL_METHOD(Sheet, zoomPrint)
{
	PHP_EXCEL_INFO(PrintZoom, IS_LONG)
}
/* }}} */

/* {{{ proto void ExcelSheet::setZoom(long value)
	Sets the zoom level of the current view. 100 is a usual view. */
EXCEL_METHOD(Sheet, setZoom)
{
	PHP_EXCEL_SET_LONG_VAL(SetZoom)
}
/* }}} */

/* {{{ proto void ExcelSheet::setZoomPrint(long value)
	Sets the scaling factor for printing as a percentage. */
EXCEL_METHOD(Sheet, setZoomPrint)
{
	PHP_EXCEL_SET_LONG_VAL(SetPrintZoom)
}
/* }}} */

/* {{{ proto void ExcelSheet::setLandscape(bool value)
	Sets landscape or portrait mode for printing, 1 - pages are printed using landscape mode, 0 - pages are printed using portrait mode. */
EXCEL_METHOD(Sheet, setLandscape)
{
	PHP_EXCEL_SET_BOOL_VAL(SetLandscape)
}
/* }}} */

/* {{{ proto bool ExcelSheet::landscape()
	Returns a page orientation mode, 1 - landscape mode, 0 - portrait mode. */
EXCEL_METHOD(Sheet, landscape)
{
	PHP_EXCEL_INFO(Landscape, IS_BOOL)
}
/* }}} */

/* {{{ proto int ExcelSheet::paper()
	Returns the paper size. */
EXCEL_METHOD(Sheet, paper)
{
	PHP_EXCEL_INFO(Paper, IS_LONG)
}
/* }}} */

/* {{{ proto void ExcelSheet::setPaper(long value)
	Sets the paper size. */
EXCEL_METHOD(Sheet, setPaper)
{
	PHP_EXCEL_SET_LONG_VAL(SetPaper)
}
/* }}} */

/* {{{ proto string ExcelSheet::header()
	Returns the header text of the sheet when printed. */
EXCEL_METHOD(Sheet, header)
{
	PHP_EXCEL_INFO(Header, IS_STRING)
}
/* }}} */

/* {{{ proto string ExcelSheet::footer()
	Returns the footer text of the sheet when printed. */
EXCEL_METHOD(Sheet, footer)
{
	PHP_EXCEL_INFO(Footer, IS_STRING)
}
/* }}} */

static bool php_excel_utf8_length_within(zend_string *value, size_t limit)
{
	const unsigned char *p = (const unsigned char *) ZSTR_VAL(value);
	const unsigned char *end = p + ZSTR_LEN(value);
	size_t count = 0;

	while (p < end) {
		size_t width;

		if (*p <= 0x7f) {
			width = 1;
		} else if (*p >= 0xc2 && *p <= 0xdf) {
			width = 2;
		} else if (*p >= 0xe0 && *p <= 0xef) {
			width = 3;
		} else if (*p >= 0xf0 && *p <= 0xf4) {
			width = 4;
		} else {
			return false;
		}
		if ((size_t) (end - p) < width) {
			return false;
		}
		if (width >= 2 && (p[1] < 0x80 || p[1] > 0xbf)) {
			return false;
		}
		if (width >= 3 && (p[2] < 0x80 || p[2] > 0xbf)) {
			return false;
		}
		if (width == 4 && (p[3] < 0x80 || p[3] > 0xbf)) {
			return false;
		}
		if ((p[0] == 0xe0 && p[1] < 0xa0)
		    || (p[0] == 0xed && p[1] > 0x9f)
		    || (p[0] == 0xf0 && p[1] < 0x90)
		    || (p[0] == 0xf4 && p[1] > 0x8f)) {
			return false;
		}
		count++;
		if (count > limit) {
			return false;
		}
		p += width;
	}
	return true;
}

#define PHP_EXCEL_SET_HF(func_name) \
	{ \
		SheetHandle sheet; \
		excel_book_object *book_obj; \
		zval *object = ZEND_THIS; \
		zend_string *val_zs = NULL; \
		double margin; \
		if (zend_parse_parameters(ZEND_NUM_ARGS(), "Sd", &val_zs, &margin) == FAILURE) { \
			RETURN_FALSE; \
		} \
		EXCEL_VALIDATE_FINITE(margin) \
		if (!val_zs) { \
			RETURN_FALSE; \
		} \
		EXCEL_NUL_SAFE_STRING(val_zs); \
		SHEET_FROM_OBJECT(sheet, object); \
		book_obj = php_excel_resolve_book_obj(object); \
		if ((book_obj && book_obj->locale_is_utf8 \
		     && !php_excel_utf8_length_within(val_zs, 255)) \
		    || ((!book_obj || !book_obj->locale_is_utf8) && ZSTR_LEN(val_zs) > 255)) { \
			RETURN_FALSE; \
		} \
		RETURN_BOOL(xlSheet ## func_name (sheet, ZSTR_VAL(val_zs), margin)); \
	}

/* {{{ proto bool ExcelSheet::setHeader(string header, double margin)
	Sets the header text of the sheet when printed. */
EXCEL_METHOD(Sheet, setHeader)
{
	PHP_EXCEL_SET_HF(SetHeader)
}
/* }}} */

/* {{{ proto bool ExcelSheet::setFooter(string footer, double margin)
	Sets the footer text of the sheet when printed. */
EXCEL_METHOD(Sheet, setFooter)
{
	PHP_EXCEL_SET_HF(SetFooter)
}
/* }}} */

/* {{{ proto double ExcelSheet::headerMargin()
	Returns the header margin in inches. */
EXCEL_METHOD(Sheet, headerMargin)
{
	PHP_EXCEL_INFO(HeaderMargin, IS_DOUBLE)
}
/* }}} */

/* {{{ proto double ExcelSheet::footerMargin()
	Returns the footer margin in inches. */
EXCEL_METHOD(Sheet, footerMargin)
{
	PHP_EXCEL_INFO(FooterMargin, IS_DOUBLE)
}
/* }}} */

/* {{{ proto bool ExcelSheet::hcenter()
	Returns whether the sheet is centered horizontally when printed: 1 - yes, 0 - no. */
EXCEL_METHOD(Sheet, hcenter)
{
	PHP_EXCEL_INFO(HCenter, IS_BOOL)
}
/* }}} */

/* {{{ proto bool ExcelSheet::vcenter()
	Returns whether the sheet is centered vertically when printed: 1 - yes, 0 - no. */
EXCEL_METHOD(Sheet, vcenter)
{
	PHP_EXCEL_INFO(VCenter, IS_BOOL)
}
/* }}} */

/* {{{ proto bool ExcelSheet::setHCenter(bool value)
	Sets a flag that the sheet is centered horizontally when printed: 1 - yes, 0 - no. */
EXCEL_METHOD(Sheet, setHCenter)
{
	PHP_EXCEL_SET_BOOL_VAL(SetHCenter)
}
/* }}} */

/* {{{ proto bool ExcelSheet::setVCenter(bool value)
	Sets a flag that the sheet is centered vertically when printed: 1 - yes, 0 - no. */
EXCEL_METHOD(Sheet, setVCenter)
{
	PHP_EXCEL_SET_BOOL_VAL(SetVCenter)
}
/* }}} */

/* {{{ proto double ExcelSheet::marginLeft()
	Returns the left margin of the sheet in inches. */
EXCEL_METHOD(Sheet, marginLeft)
{
	PHP_EXCEL_INFO(MarginLeft, IS_DOUBLE)
}
/* }}} */

/* {{{ proto double ExcelSheet::marginRight()
	Returns the right margin of the sheet in inches. */
EXCEL_METHOD(Sheet, marginRight)
{
	PHP_EXCEL_INFO(MarginRight, IS_DOUBLE)
}
/* }}} */

/* {{{ proto double ExcelSheet::marginTop()
	Returns the top margin of the sheet in inches. */
EXCEL_METHOD(Sheet, marginTop)
{
	PHP_EXCEL_INFO(MarginTop, IS_DOUBLE)
}
/* }}} */

/* {{{ proto double ExcelSheet::marginBottom()
	Returns the bottom margin of the sheet in inches. */
EXCEL_METHOD(Sheet, marginBottom)
{
	PHP_EXCEL_INFO(MarginBottom, IS_DOUBLE)
}
/* }}} */

/* {{{ proto void ExcelSheet::setMarginLeft(double value)
	Sets the left margin of the sheet in inches. */
EXCEL_METHOD(Sheet, setMarginLeft)
{
	PHP_EXCEL_SET_DOUBLE_VAL(SetMarginLeft)
}
/* }}} */

/* {{{ proto void ExcelSheet::setMarginRight(double value)
	Sets the right margin of the sheet in inches. */
EXCEL_METHOD(Sheet, setMarginRight)
{
	PHP_EXCEL_SET_DOUBLE_VAL(SetMarginRight)
}
/* }}} */

/* {{{ proto void ExcelSheet::setMarginTop(double value)
	Sets the top margin of the sheet in inches. */
EXCEL_METHOD(Sheet, setMarginTop)
{
	PHP_EXCEL_SET_DOUBLE_VAL(SetMarginTop)
}
/* }}} */

/* {{{ proto void ExcelSheet::setMarginBottom(double value)
	Sets the bottom margin of the sheet in inches. */
EXCEL_METHOD(Sheet, setMarginBottom)
{
	PHP_EXCEL_SET_DOUBLE_VAL(SetMarginBottom)
}
/* }}} */

/* {{{ proto bool ExcelSheet::printHeaders()
	Returns whether the row and column headers are printed: 1 - yes, 0 - no. */
EXCEL_METHOD(Sheet, printHeaders)
{
	PHP_EXCEL_INFO(PrintRowCol, IS_BOOL)
}
/* }}} */

/* {{{ proto bool ExcelSheet::setPrintHeaders(bool value)
	Sets a flag that the row and column headers are printed: 1 - yes, 0 - no. */
EXCEL_METHOD(Sheet, setPrintHeaders)
{
	PHP_EXCEL_SET_BOOL_VAL(SetPrintRowCol)
}
/* }}} */

/* {{{ proto string ExcelSheet::name()
	Returns the name of the sheet. */
EXCEL_METHOD(Sheet, name)
{
	PHP_EXCEL_INFO(Name, IS_STRING)
}
/* }}} */

/* {{{ proto void ExcelSheet::setName(string name)
	Sets the name of the sheet. */
EXCEL_METHOD(Sheet, setName)
{
	SheetHandle sheet;
	zval *object = ZEND_THIS;
	zend_string *val_zs = NULL;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "S", &val_zs) == FAILURE) {
		RETURN_FALSE;
	}

	EXCEL_NON_EMPTY_STRING(val_zs)
	EXCEL_NUL_SAFE_STRING(val_zs)

	SHEET_FROM_OBJECT(sheet, object);

	xlSheetSetName(sheet, ZSTR_VAL(val_zs));
}
/* }}} */

/* {{{ proto bool ExcelSheet::setNamedRange(string name, int row_first, int row_last, int col_first, int col_last [, int scope_id])
	Create a named range */
EXCEL_METHOD(Sheet, setNamedRange)
{
	SheetHandle sheet;
	zval *object = ZEND_THIS;
	zend_long row, to_row, col, to_col;
	zend_string *name_zs = NULL;
	zend_long scope_id = SCOPE_WORKBOOK;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "Sllll|l", &name_zs, &row, &to_row, &col, &to_col, &scope_id) == FAILURE) {
		RETURN_FALSE;
	}

	if (!name_zs || ZSTR_LEN(name_zs) < 1) {
		php_error_docref(NULL, E_WARNING, "The range name cannot be empty.");
		RETURN_FALSE;
	}
	EXCEL_NUL_SAFE_STRING(name_zs)

	if (row > to_row) {
		php_error_docref(NULL, E_WARNING, "The range row start cannot be greater than row end.");
		RETURN_FALSE;
	} else if (col > to_col) {
		php_error_docref(NULL, E_WARNING, "The range column start cannot be greater than column end.");
		RETURN_FALSE;
	}
	EXCEL_VALIDATE_ROW_RANGE(row, to_row, object);
	EXCEL_VALIDATE_COL_RANGE(col, to_col, object);
	EXCEL_VALIDATE_SCOPE(scope_id)

	SHEET_FROM_OBJECT(sheet, object);

	RETURN_BOOL(xlSheetSetNamedRange(sheet, ZSTR_VAL(name_zs), row, to_row, col, to_col, scope_id));
}
/* }}} */

/* {{{ proto bool ExcelSheet::delNamedRange(string name)
	Delete a named range. */
EXCEL_METHOD(Sheet, delNamedRange)
{
	SheetHandle sheet;
	zval *object = ZEND_THIS;
	zend_string *val_zs = NULL;
	zend_long scope_id = SCOPE_WORKBOOK;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "S|l", &val_zs, &scope_id) == FAILURE) {
		RETURN_FALSE;
	}

	if (!val_zs || ZSTR_LEN(val_zs) < 1) {
		php_error_docref(NULL, E_WARNING, "The range name cannot be empty.");
		RETURN_FALSE;
	}
	EXCEL_NUL_SAFE_STRING(val_zs)
	EXCEL_VALIDATE_SCOPE(scope_id)

	SHEET_FROM_OBJECT(sheet, object);

	RETURN_BOOL(xlSheetDelNamedRange(sheet, ZSTR_VAL(val_zs), scope_id));
}
/* }}} */

#define PHP_EXCEL_SHEET_PRINT_REPEAT_ROWS(func_name) \
	{ \
		SheetHandle sheet; \
		zval *object = ZEND_THIS; \
		zend_long s, e; \
		if (zend_parse_parameters(ZEND_NUM_ARGS(), "ll", &s, &e) == FAILURE) { \
			RETURN_FALSE; \
		} \
		EXCEL_VALIDATE_ROW_RANGE(s, e, object); \
		if (s > e) { \
			php_error_docref(NULL, E_WARNING, "The range start is greater than the end."); \
			RETURN_FALSE; \
		} \
		SHEET_FROM_OBJECT(sheet, object); \
		xlSheet ## func_name (sheet, s, e); \
		RETURN_TRUE; \
	}

#define PHP_EXCEL_SHEET_PRINT_REPEAT_COLS(func_name) \
	{ \
		SheetHandle sheet; \
		zval *object = ZEND_THIS; \
		zend_long s, e; \
		if (zend_parse_parameters(ZEND_NUM_ARGS(), "ll", &s, &e) == FAILURE) { \
			RETURN_FALSE; \
		} \
		EXCEL_VALIDATE_COL_RANGE(s, e, object); \
		if (s > e) { \
			php_error_docref(NULL, E_WARNING, "The range start is greater than the end."); \
			RETURN_FALSE; \
		} \
		SHEET_FROM_OBJECT(sheet, object); \
		xlSheet ## func_name (sheet, s, e); \
		RETURN_TRUE; \
	}

/* {{{ proto bool ExcelSheet::setPrintRepeatRows(int rowFirst, int rowLast)
	Sets repeated rows on each page from rowFirst to rowLast. */
EXCEL_METHOD(Sheet, setPrintRepeatRows)
{
	PHP_EXCEL_SHEET_PRINT_REPEAT_ROWS(SetPrintRepeatRows)
}
/* }}} */

/* {{{ proto bool ExcelSheet::setPrintRepeatCols(int colFirst, int colLast)
	Sets repeated columns on each page from colFirst to colLast. */
EXCEL_METHOD(Sheet, setPrintRepeatCols)
{
	PHP_EXCEL_SHEET_PRINT_REPEAT_COLS(SetPrintRepeatCols)
}
/* }}} */

/* {{{ proto bool ExcelSheet::getGroupSummaryBelow()
	Returns whether grouping rows summary is below. Returns 1 if summary is below and 0 if isn't. */
EXCEL_METHOD(Sheet, getGroupSummaryBelow)
{
	PHP_EXCEL_INFO(GroupSummaryBelow, IS_BOOL)
}
/* }}} */

/* {{{ proto bool ExcelSheet::setGroupSummaryBelow(bool direction)
	Sets a flag of grouping rows summary: 1 - below, 0 - above. */
EXCEL_METHOD(Sheet, setGroupSummaryBelow)
{
	SheetHandle sheet;
	zval *object = ZEND_THIS;
	bool val;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "b", &val) == FAILURE) {
		RETURN_FALSE;
	}

	SHEET_FROM_OBJECT(sheet, object);
	xlSheetSetGroupSummaryBelow(sheet, val);

	RETURN_TRUE;
}
/* }}} */

/* {{{ proto bool ExcelSheet::getGroupSummaryRight()
	Returns whether grouping columns summary is right. Returns 1 if summary is right and 0 if isn't. */
EXCEL_METHOD(Sheet, getGroupSummaryRight)
{
	PHP_EXCEL_INFO(GroupSummaryRight, IS_BOOL)
}
/* }}} */

/* {{{ proto bool ExcelSheet::setGroupSummaryRight(bool direction)
	Sets a flag of grouping columns summary: 1 - right, 0 - left. */
EXCEL_METHOD(Sheet, setGroupSummaryRight)
{
	SheetHandle sheet;
	zval *object = ZEND_THIS;
	bool val;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "b", &val) == FAILURE) {
		RETURN_FALSE;
	}

	SHEET_FROM_OBJECT(sheet, object);
	xlSheetSetGroupSummaryRight(sheet, val);

	RETURN_TRUE;
}
/* }}} */

/* {{{ proto bool ExcelSheet::setPrintFit(int wPages, int hPages)
	Fits sheet width and sheet height to wPages and hPages respectively. */
EXCEL_METHOD(Sheet, setPrintFit)
{
	SheetHandle sheet;
	zval *object = ZEND_THIS;
	zend_long wPages, hPages;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "ll", &wPages, &hPages) == FAILURE) {
		RETURN_FALSE;
	}

	EXCEL_VALIDATE_INT_RANGE(wPages)
	EXCEL_VALIDATE_INT_RANGE(hPages)

	SHEET_FROM_OBJECT(sheet, object);
	xlSheetSetPrintFit(sheet, wPages, hPages);

	RETURN_TRUE;
}
/* }}} */

/* {{{ proto mixed ExcelSheet::getPrintFit()
	Returns whether fit to page option is enabled, and if so to what width & height */
EXCEL_METHOD(Sheet, getPrintFit)
{
	SheetHandle sheet;
	zval *object = ZEND_THIS;
	int wPages, hPages;

	ZEND_PARSE_PARAMETERS_NONE();

	SHEET_FROM_OBJECT(sheet, object);
	if (xlSheetGetPrintFit(sheet, &wPages, &hPages)) {
		array_init(return_value);
		add_assoc_long(return_value, "width", wPages);
		add_assoc_long(return_value, "height", hPages);
	} else {
		RETURN_FALSE;
	}
}
/* }}} */

/* {{{ proto array ExcelSheet::getNamedRange(string name [, int scope_id])
	Gets the named range coordinates by name, returns false if range is not found. */
EXCEL_METHOD(Sheet, getNamedRange)
{
	SheetHandle sheet;
	zval *object = ZEND_THIS;
	zend_string *name_zs = NULL;
	int rf, rl, cf, cl;
	zend_long scope_id = SCOPE_WORKBOOK;
	int hidden = 0;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "S|l", &name_zs, &scope_id) == FAILURE) {
		RETURN_FALSE;
	}

	EXCEL_NON_EMPTY_STRING(name_zs)
	EXCEL_NUL_SAFE_STRING(name_zs)
	EXCEL_VALIDATE_SCOPE(scope_id)

	SHEET_FROM_OBJECT(sheet, object);

	if (xlSheetGetNamedRange(sheet, ZSTR_VAL(name_zs), &rf, &rl, &cf, &cl, scope_id, &hidden)) {
		array_init(return_value);
		add_assoc_long(return_value, "row_first", rf);
		add_assoc_long(return_value, "row_last", rl);
		add_assoc_long(return_value, "col_first", cf);
		add_assoc_long(return_value, "col_last", cl);
		add_assoc_bool(return_value, "hidden", hidden);
	} else {
		RETURN_FALSE;
	}
}

/* {{{ proto array ExcelSheet::getIndexRange(int index [, int scope_id])
	Gets the named range coordinates by index, returns false if range is not found. */
EXCEL_METHOD(Sheet, getIndexRange)
{
	SheetHandle sheet;
	zval *object = ZEND_THIS;
	zend_long index;
	int rf, rl, cf, cl;
	int hidden = 0;
	int scope_out = SCOPE_WORKBOOK;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "l", &index) == FAILURE) {
		RETURN_FALSE;
	}

	EXCEL_VALIDATE_INT_RANGE(index)

	SHEET_FROM_OBJECT(sheet, object);
	/* xlSheetNamedRange returns the range name (const char*, owned by libxl),
	 * or NULL if no range exists at this index. */
	const char *name = xlSheetNamedRange(sheet, (int)index, &rf, &rl, &cf, &cl, &scope_out, &hidden);
	if (name) {
		array_init(return_value);
		add_assoc_string(return_value, "name", (char *)name);
		add_assoc_long(return_value, "row_first", rf);
		add_assoc_long(return_value, "row_last", rl);
		add_assoc_long(return_value, "col_first", cf);
		add_assoc_long(return_value, "col_last", cl);
		add_assoc_bool(return_value, "hidden", hidden);
		add_assoc_long(return_value, "scope", scope_out);
	} else {
		RETURN_FALSE;
	}
}
/* }}} */

/* {{{ proto long ExcelSheet::namedRangeSize()
	Returns the number of named ranges in the sheet. */
EXCEL_METHOD(Sheet, namedRangeSize)
{
	PHP_EXCEL_INFO(NamedRangeSize, IS_LONG)
}
/* }}} */

/* {{{ proto long ExcelSheet::getVerPageBreak(int index)
	Returns column with vertical page break at position index. */
EXCEL_METHOD(Sheet, getVerPageBreak)
{
	SheetHandle sheet;
	zval *object = ZEND_THIS;
	zend_long index;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "l", &index) == FAILURE) {
		RETURN_FALSE;
	}

	EXCEL_VALIDATE_INT_RANGE(index)

	SHEET_FROM_OBJECT(sheet, object);
	RETURN_LONG(xlSheetGetVerPageBreak(sheet, index));
}
/* }}} */

/* {{{ proto long ExcelSheet::getVerPageBreakSize()
	Returns a number of vertical page breaks in the sheet. */
EXCEL_METHOD(Sheet, getVerPageBreakSize)
{
	PHP_EXCEL_INFO(GetVerPageBreakSize, IS_LONG)
}
/* }}} */

/* {{{ proto long ExcelSheet::getHorPageBreak(int index)
	Returns column with horizontal page break at position index. */
EXCEL_METHOD(Sheet, getHorPageBreak)
{
	SheetHandle sheet;
	zval *object = ZEND_THIS;
	zend_long index;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "l", &index) == FAILURE) {
		RETURN_FALSE;
	}

	EXCEL_VALIDATE_INT_RANGE(index)

	SHEET_FROM_OBJECT(sheet, object);
	RETURN_LONG(xlSheetGetHorPageBreak(sheet, index));
}
/* }}} */

/* {{{ proto long ExcelSheet::getHorPageBreakSize()
	Returns a number of horizontal page breaks in the sheet. */
EXCEL_METHOD(Sheet, getHorPageBreakSize)
{
	PHP_EXCEL_INFO(GetHorPageBreakSize, IS_LONG)
}
/* }}} */

/* {{{ proto array ExcelSheet::getPictureInfo(int index)
	Returns a information about a workbook picture at position index in worksheet. */
EXCEL_METHOD(Sheet, getPictureInfo)
{
	SheetHandle sheet;
	zval *object = ZEND_THIS;
	zend_long index;
	int rowTop, colLeft, rowBottom, colRight, width, height, offset_x, offset_y;
	int pic_index;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "l", &index) == FAILURE) {
		RETURN_FALSE;
	}

	EXCEL_VALIDATE_INT_RANGE(index)

	SHEET_FROM_OBJECT(sheet, object);

	if ((pic_index = xlSheetGetPicture(sheet, (int)index, &rowTop, &colLeft, &rowBottom, &colRight, &width, &height, &offset_x, &offset_y)) == -1) {
		RETURN_FALSE;
	}

	array_init(return_value);
	add_assoc_long(return_value, "picture_index", pic_index);
	add_assoc_long(return_value, "row_top", rowTop);
	add_assoc_long(return_value, "col_left", colLeft);
	add_assoc_long(return_value, "row_bottom", rowBottom);
	add_assoc_long(return_value, "col_right", colRight);
	add_assoc_long(return_value, "width", width);
	add_assoc_long(return_value, "height", height);
	add_assoc_long(return_value, "offset_x", offset_x);
	add_assoc_long(return_value, "offset_y", offset_y);
}
/* }}} */

/* {{{ proto long ExcelSheet::getNumPictures()
	Returns a number of pictures in this worksheet. */
EXCEL_METHOD(Sheet, getNumPictures)
{
	PHP_EXCEL_INFO(PictureSize, IS_LONG)
}
/* }}} */

/* {{{ proto long ExcelBook::biffVersion()
	Returns BIFF version of binary file. Used for xls format only. */
EXCEL_METHOD(Book, biffVersion)
{
	BookHandle book;
	zval *object = ZEND_THIS;
	int version;

	ZEND_PARSE_PARAMETERS_NONE();

	BOOK_FROM_OBJECT(book, object);
	if ((version = xlBookBiffVersion(book))) {
		RETURN_LONG(version);
	} else {
		RETURN_FALSE;
	}
}
/* }}} */

/* {{{ proto bool ExcelBook::getRefR1C1()
	Returns whether the R1C1 reference mode is active. */
EXCEL_METHOD(Book, getRefR1C1)
{
	PHP_EXCEL_BOOK_INFO(RefR1C1A, IS_BOOL)
}
/* }}} */

/* {{{ proto void ExcelBook::setRefR1C1(bool active)
	Sets the R1C1 reference mode. */
EXCEL_METHOD(Book, setRefR1C1)
{
	BookHandle book;
	zval *object = ZEND_THIS;
	bool active;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "b", &active) == FAILURE) {
		RETURN_FALSE;
	}

	BOOK_FROM_OBJECT(book, object);
	xlBookSetRefR1C1A(book, (int)active);
	RETURN_TRUE;
}
/* }}} */

/* {{{ proto array ExcelBook::getPicture(int picture_index)
	Returns a picture at position index. */
EXCEL_METHOD(Book, getPicture)
{
	BookHandle book;
	zval *object = ZEND_THIS;
	zend_long index;
	int type;
	const char *buf;
	unsigned int buf_len;
	enum PictureType {PICTURETYPE_PNG, PICTURETYPE_JPEG, PICTURETYPE_WMF, PICTURETYPE_DIB, PICTURETYPE_EMF, PICTURETYPE_PICT, PICTURETYPE_TIFF, PICTURETYPE_ERROR = 0xFF};

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "l", &index) == FAILURE) {
		RETURN_FALSE;
	}

	EXCEL_VALIDATE_INT_RANGE(index)

	BOOK_FROM_OBJECT(book, object);

	if ((type = xlBookGetPicture(book, (int)index, &buf, &buf_len)) == PICTURETYPE_ERROR) {
		RETURN_FALSE;
	}

	{
		excel_book_object *book_obj = Z_EXCEL_BOOK_OBJ_P(object);
		zend_string *owned;

		book_obj->native_buffer_borrowed = true;
		owned = zend_string_init(buf, buf_len, 0);
		book_obj->native_buffer_borrowed = false;

		array_init(return_value);
		add_assoc_str(return_value, "data", owned);
		add_assoc_long(return_value, "type", type);
	}
}
/* }}} */

/* {{{ proto long ExcelBook::getNumPictures()
	Returns a number of pictures in this workbook. */
EXCEL_METHOD(Book, getNumPictures)
{
	PHP_EXCEL_BOOK_INFO(PictureSize, IS_LONG)
}
/* }}} */

/* {{{ proto ExcelSheet ExcelBook::insertSheet(int index, string name [, ExcelSheet sh])
	Inserts a new sheet to this book at position index, returns the sheet handle. Set initSheet to 0 if you wish to add a new empty sheet or use existing sheet's handle for copying. */
EXCEL_METHOD(Book, insertSheet)
{
	BookHandle book;
	zval *object = ZEND_THIS;
	zval *shz = NULL;
	SheetHandle sh, sheet;
	excel_sheet_object *fo;
	zend_string *name_zs = NULL;
	zend_long index;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "lS|O!", &index, &name_zs, &shz, excel_ce_sheet) == FAILURE) {
		RETURN_FALSE;
	}

	EXCEL_NON_EMPTY_STRING(name_zs)
	EXCEL_NUL_SAFE_STRING(name_zs)
	EXCEL_VALIDATE_INT_RANGE(index)

	BOOK_FROM_OBJECT(book, object);
	if (shz) {
		SHEET_FROM_OBJECT(sheet, shz);
		EXCEL_REQUIRE_SAME_BOOK(shz, object);
		if (!(sh = xlBookInsertSheet(book, index, ZSTR_VAL(name_zs), sheet))) {
			RETURN_FALSE;
		}
	} else {
		if (!(sh = xlBookInsertSheet(book, index, ZSTR_VAL(name_zs), 0))) {
			RETURN_FALSE;
		}
	}

	ZVAL_OBJ(return_value, excel_object_new_sheet(excel_ce_sheet));
	fo = Z_EXCEL_SHEET_OBJ_P(return_value);
	fo->sheet = sh;
	fo->book = book;
	EXCEL_INIT_SHEET_PARENT(fo, object);
}
/* }}} */

/* {{{ proto bool ExcelBook::isTemplate()
	Returns whether the workbook is template. */
EXCEL_METHOD(Book, isTemplate)
{
	PHP_EXCEL_BOOK_INFO(IsTemplate, IS_BOOL)
}
/* }}} */

/* {{{ proto void ExcelBook::setTemplate(bool mode)
	Sets the template flag, if the workbook is template. */
EXCEL_METHOD(Book, setTemplate)
{
	BookHandle book;
	zval *object = ZEND_THIS;
	bool mode;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "b", &mode) == FAILURE) {
		RETURN_FALSE;
	}

	BOOK_FROM_OBJECT(book, object);
	xlBookSetTemplate(book, (int)mode);
	RETURN_TRUE;
}
/* }}} */

/* {{{ proto long ExcelSheet::getRightToLeft()
	Returns whether the text is displayed in right-to-left mode: 1 - yes, 0 - no. */
EXCEL_METHOD(Sheet, getRightToLeft)
{
	PHP_EXCEL_INFO(RightToLeft, IS_LONG)
}
/* }}} */

/* {{{ proto void ExcelBook::setRightToLeft(bool mode)
	Sets the right-to-left mode: 1 - the text is displayed in right-to-left mode, 0 - the text is displayed in left-to-right mode. */
EXCEL_METHOD(Sheet, setRightToLeft)
{
	SheetHandle sheet;
	zval *object = ZEND_THIS;
	zend_long mode;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "l", &mode) == FAILURE) {
		RETURN_FALSE;
	}

	EXCEL_VALIDATE_INT_RANGE(mode)

	SHEET_FROM_OBJECT(sheet, object);
	xlSheetSetRightToLeft(sheet, (int)mode);
}
/* }}} */

/* {{{ proto bool ExcelSheet::setPrintArea()
	Sets the print area. */
EXCEL_METHOD(Sheet, setPrintArea)
{
	zval *object = ZEND_THIS;
	SheetHandle sheet;
	zend_long row, col, to_row, to_col;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "llll", &row, &to_row, &col, &to_col) == FAILURE) {
		RETURN_FALSE;
	}

	EXCEL_VALIDATE_ROW_RANGE(row, to_row, object);
	EXCEL_VALIDATE_COL_RANGE(col, to_col, object);

	if (row > to_row) {
		php_error_docref(NULL, E_WARNING, "The range row start cannot be greater than row end.");
		RETURN_FALSE;
	} else if (col > to_col) {
		php_error_docref(NULL, E_WARNING, "The range column start cannot be greater than column end.");
		RETURN_FALSE;
	}

	SHEET_FROM_OBJECT(sheet, object);
	xlSheetSetPrintArea(sheet, row, to_row, col, to_col);
	RETURN_TRUE;
}
/* }}} */

/* {{{ proto bool ExcelSheet::clearPrintRepeats()
	Clears repeated rows and columns on each page. */
EXCEL_METHOD(Sheet, clearPrintRepeats)
{
	PHP_EXCEL_SHEET_VOID(ClearPrintRepeats)
}
/* }}} */

/* {{{ proto bool ExcelSheet::clearPrintArea()
	Clears the print area. */
EXCEL_METHOD(Sheet, clearPrintArea)
{
	PHP_EXCEL_SHEET_VOID(ClearPrintArea)
}
/* }}} */

/* {{{ proto bool ExcelSheet::protect()
	Returns whether sheet is protected: 1 - yes, 0 - no. */
EXCEL_METHOD(Sheet, protect)
{
	PHP_EXCEL_INFO(Protect, IS_BOOL)
}
/* }}} */

/* {{{ proto long ExcelSheet::hyperlinkSize()
	Returns the number of hyperlinks in the sheet. */
EXCEL_METHOD(Sheet, hyperlinkSize)
{
	PHP_EXCEL_INFO(HyperlinkSize, IS_LONG)
}
/* }}} */

/* {{{ proto array ExcelSheet::hyperlink(int index)
	Gets the hyperlink and its coordinates by index. */
EXCEL_METHOD(Sheet, hyperlink)
{
	SheetHandle sheet;
	zval *object = ZEND_THIS;
	zend_long index;
	int rowFirst, rowLast, colFirst, colLast;
	const char *s;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "l", &index) == FAILURE) {
		RETURN_FALSE;
	}

	EXCEL_VALIDATE_INT_RANGE(index)

	SHEET_FROM_OBJECT(sheet, object);

	s = xlSheetHyperlink(sheet, index, &rowFirst, &rowLast, &colFirst, &colLast);

	if (!s) {
		RETURN_FALSE;
	}

	array_init(return_value);
	add_assoc_string(return_value, "hyperlink", (char *)s);
	add_assoc_long(return_value, "row_first", rowFirst);
	add_assoc_long(return_value, "row_last", rowLast);
	add_assoc_long(return_value, "col_first", colFirst);
	add_assoc_long(return_value, "col_last", colLast);
}
/* }}} */

/* {{{ proto bool ExcelSheet::delHyperlink(int index)
	Removes hyperlink by index. */
EXCEL_METHOD(Sheet, delHyperlink)
{
	zval *object = ZEND_THIS;
	SheetHandle sheet;
	zend_long index;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "l", &index) == FAILURE) {
		RETURN_FALSE;
	}

	EXCEL_VALIDATE_INT_RANGE(index)

	SHEET_FROM_OBJECT(sheet, object);
	RETURN_BOOL(xlSheetDelHyperlink(sheet, index));
}
/* }}} */

/* {{{ proto void ExcelSheet::addHyperlink(string hyperlink, int row_first, int row_last, int col_first, int col_last)
	Adds the new hyperlink. */
EXCEL_METHOD(Sheet, addHyperlink)
{
	SheetHandle sheet;
	zval *object = ZEND_THIS;
	zend_string *val_zs = NULL;
	zend_long row_first, row_last, col_first, col_last;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "Sllll", &val_zs, &row_first, &row_last, &col_first, &col_last) == FAILURE) {
		RETURN_FALSE;
	}

	EXCEL_NON_EMPTY_STRING(val_zs)
	EXCEL_NUL_SAFE_STRING(val_zs)
	EXCEL_VALIDATE_ROW_RANGE(row_first, row_last, object);
	EXCEL_VALIDATE_COL_RANGE(col_first, col_last, object);

	SHEET_FROM_OBJECT(sheet, object);

	xlSheetAddHyperlink(sheet, ZSTR_VAL(val_zs), row_first, row_last, col_first, col_last);

	RETURN_TRUE;
}
/* }}} */

/* {{{ proto long ExcelSheet::mergeSize()
	Returns a number of merged cells in this worksheet. */
EXCEL_METHOD(Sheet, mergeSize)
{
	PHP_EXCEL_INFO(MergeSize, IS_LONG)
}
/* }}} */

/* {{{ proto array ExcelSheet::merge(int index)
	Gets the merged cells by index. */
EXCEL_METHOD(Sheet, merge)
{
	SheetHandle sheet;
	zval *object = ZEND_THIS;
	zend_long index;
	int rowFirst, rowLast, colFirst, colLast;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "l", &index) == FAILURE) {
		RETURN_FALSE;
	}

	EXCEL_VALIDATE_INT_RANGE(index)

	SHEET_FROM_OBJECT(sheet, object);

	if (!xlSheetMerge(sheet, index, &rowFirst, &rowLast, &colFirst, &colLast)) {
		RETURN_FALSE;
	}

	array_init(return_value);
	add_assoc_long(return_value, "row_first", rowFirst);
	add_assoc_long(return_value, "row_last", rowLast);
	add_assoc_long(return_value, "col_first", colFirst);
	add_assoc_long(return_value, "col_last", colLast);
}
/* }}} */

/* {{{ proto bool ExcelSheet::delMergeByIndex(int index)
	Removes merged cells by index. */
EXCEL_METHOD(Sheet, delMergeByIndex)
{
	zval *object = ZEND_THIS;
	SheetHandle sheet;
	zend_long index;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "l", &index) == FAILURE) {
		RETURN_FALSE;
	}

	EXCEL_VALIDATE_INT_RANGE(index)

	SHEET_FROM_OBJECT(sheet, object);
	RETURN_BOOL(xlSheetDelMergeByIndex(sheet, index));
}
/* }}} */

/* {{{ proto bool ExcelSheet::splitInfo()
	Gets the split information (position of frozen pane) in the sheet: row - vertical position of the split; col - horizontal position of the split. */
EXCEL_METHOD(Sheet, splitInfo)
{
	SheetHandle sheet;
	zval *object = ZEND_THIS;
	int row, col;

	ZEND_PARSE_PARAMETERS_NONE();

	SHEET_FROM_OBJECT(sheet, object);

	if (!xlSheetSplitInfo(sheet, &row, &col)) {
		RETURN_FALSE;
	}

	array_init(return_value);
	add_assoc_long(return_value, "row", row);
	add_assoc_long(return_value, "col", col);
}
/* }}} */

/* {{{ proto bool ExcelSheet::rowHidden(int row)
	Returns whether row is hidden. */
EXCEL_METHOD(Sheet, rowHidden)
{
	zval *object = ZEND_THIS;
	SheetHandle sheet;
	zend_long row;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "l", &row) == FAILURE) {
		RETURN_FALSE;
	}

	EXCEL_VALIDATE_ROW(row, object);
	SHEET_FROM_OBJECT(sheet, object);
	RETURN_BOOL(xlSheetRowHidden(sheet, row));
}
/* }}} */

/* {{{ proto bool ExcelSheet::setRowHidden(int row, bool hidden)
	Hides row. */
EXCEL_METHOD(Sheet, setRowHidden)
{
	zval *object = ZEND_THIS;
	SheetHandle sheet;
	zend_long row;
	bool hidden;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "lb", &row, &hidden) == FAILURE) {
		RETURN_FALSE;
	}

	EXCEL_VALIDATE_ROW(row, object);
	SHEET_FROM_OBJECT(sheet, object);
	RETURN_BOOL(xlSheetSetRowHidden(sheet, row, hidden));
}
/* }}} */

/* {{{ proto bool ExcelSheet::colHidden(int col)
	Returns whether column is hidden. */
EXCEL_METHOD(Sheet, colHidden)
{
	zval *object = ZEND_THIS;
	SheetHandle sheet;
	zend_long col;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "l", &col) == FAILURE) {
		RETURN_FALSE;
	}

	EXCEL_VALIDATE_COL(col, object);
	SHEET_FROM_OBJECT(sheet, object);
	RETURN_BOOL(xlSheetColHidden(sheet, col));
}
/* }}} */

/* {{{ proto bool ExcelSheet::setColHidden(int col, bool hidden)
	Hides column. */
EXCEL_METHOD(Sheet, setColHidden)
{
	zval *object = ZEND_THIS;
	SheetHandle sheet;
	zend_long col;
	bool hidden;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "lb", &col, &hidden) == FAILURE) {
		RETURN_FALSE;
	}

	EXCEL_VALIDATE_COL(col, object);
	SHEET_FROM_OBJECT(sheet, object);
	RETURN_BOOL(xlSheetSetColHidden(sheet, col, hidden));
}
/* }}} */

/* {{{ proto long ExcelBook::sheetType(int sheet)
	Returns type of sheet with specified index. */
EXCEL_METHOD(Book, sheetType)
{
	zval *object = ZEND_THIS;
	BookHandle book;
	zend_long index;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "l", &index) == FAILURE) {
		RETURN_FALSE;
	}

	EXCEL_VALIDATE_INT_RANGE(index)

	BOOK_FROM_OBJECT(book, object);
	RETURN_LONG(xlBookSheetType(book, index));
}
/* }}} */

/* {{{ proto bool ExcelSheet::isLicensed()
	Get license status */
EXCEL_METHOD(Sheet, isLicensed)
{
	char *err;
	zval *object = ZEND_THIS;
	SheetHandle sheet;
	BookHandle book;

	ZEND_PARSE_PARAMETERS_NONE();

	SHEET_AND_BOOK_FROM_OBJECT(sheet, book, object);

	xlSheetCellFormat(sheet, 0, 0);
	err = (char *)xlBookErrorMessage(book);
	if (err) {
		// on Linux
		if (!strcmp(err, "can't get access to format in row 0 in trial version")) {
			RETURN_FALSE;
		}
		// on Win
		if (!strcmp(err, "can't access row 0 in trial version")) {
			RETURN_FALSE;
		}
	}

	RETURN_TRUE;
}
/* }}} */

/* {{{ proto void ExcelSheet::setAutoFitArea(int rowFirst, int colFirst, int rowLast, int colLast)
	Sets the borders for autofit column widths feature.
	The function xlSheetSetCol() with -1 width value will
	affect only to the specified limited area. */
EXCEL_METHOD(Sheet, setAutoFitArea)
{
	zval *object = ZEND_THIS;
	SheetHandle sheet;
	zend_long rowFirst=0, colFirst=0, rowLast=-1, colLast=-1;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "|llll", &rowFirst, &rowLast, &colFirst, &colLast) == FAILURE) {
		RETURN_FALSE;
	}

	{
		excel_book_object *_vb = php_excel_resolve_book_obj(object);
		zend_long _maxr = (_vb && _vb->is_xlsx) ? EXCEL_MAX_ROW_XLSX : EXCEL_MAX_ROW_XLS;
		zend_long _maxc = (_vb && _vb->is_xlsx) ? EXCEL_MAX_COL_XLSX : EXCEL_MAX_COL_XLS;
		/* libxl uses -1 for "no limit" on the *Last params, so allow that
		 * sentinel; the *First params still must be 0..max. */
		if (rowFirst < 0 || rowFirst > _maxr || colFirst < 0 || colFirst > _maxc
		    || rowLast < -1 || rowLast > _maxr
		    || colLast < -1 || colLast > _maxc
		    || (rowLast != -1 && rowFirst > rowLast)
		    || (colLast != -1 && colFirst > colLast)) {
			php_error_docref(NULL, E_WARNING,
				"Invalid autofit area: rowFirst=" ZEND_LONG_FMT
				", rowLast=" ZEND_LONG_FMT ", colFirst=" ZEND_LONG_FMT
				", colLast=" ZEND_LONG_FMT,
				rowFirst, rowLast, colFirst, colLast);
			RETURN_FALSE;
		}
	}

	SHEET_FROM_OBJECT(sheet, object);

	xlSheetSetAutoFitArea(sheet, rowFirst, colFirst, rowLast, colLast);
	RETURN_TRUE;
}
/* }}} */

/* {{{ proto long ExcelSheet::printRepeatRows()
	Gets repeated rows on each page from rowFirst to rowLast.
	Returns 0 if repeated rows aren't found. */
EXCEL_METHOD(Sheet, printRepeatRows)
{
	zval *object = ZEND_THIS;
	SheetHandle sheet;
	int rowFirst, rowLast;

	ZEND_PARSE_PARAMETERS_NONE();

	SHEET_FROM_OBJECT(sheet, object);

	if (!xlSheetPrintRepeatRows(sheet, &rowFirst, &rowLast)) {
		RETURN_FALSE;
	}

	array_init(return_value);
	add_assoc_long(return_value, "row_start", rowFirst);
	add_assoc_long(return_value, "row_end", rowLast);
}
/* }}} */

/* {{{ proto long ExcelSheet::printRepeatCols()
	Gets repeated columns on each page from colFirst to colLast.
	Returns 0 if repeated columns aren't found. */
EXCEL_METHOD(Sheet, printRepeatCols)
{
	zval *object = ZEND_THIS;
	SheetHandle sheet;
	int colFirst, colLast;

	ZEND_PARSE_PARAMETERS_NONE();

	SHEET_FROM_OBJECT(sheet, object);

	if (!xlSheetPrintRepeatCols(sheet, &colFirst, &colLast)) {
		RETURN_FALSE;
	}

	array_init(return_value);
	add_assoc_long(return_value, "col_start", colFirst);
	add_assoc_long(return_value, "col_end", colLast);
}
/* }}} */

/* {{{ proto long ExcelSheet::printArea()
	Gets the print area. Returns 0 if print area isn't found. */
EXCEL_METHOD(Sheet, printArea)
{
	zval *object = ZEND_THIS;
	SheetHandle sheet;
	int rowFirst, colFirst, rowLast, colLast;

	ZEND_PARSE_PARAMETERS_NONE();

	SHEET_FROM_OBJECT(sheet, object);

	if (!xlSheetPrintArea(sheet, &rowFirst, &rowLast, &colFirst, &colLast)) {
		RETURN_FALSE;
	}

	array_init(return_value);
	add_assoc_long(return_value, "row_start", rowFirst);
	add_assoc_long(return_value, "col_start", colFirst);
	add_assoc_long(return_value, "row_end", rowLast);
	add_assoc_long(return_value, "col_end", colLast);
}
/* }}} */


/* {{{ proto void ExcelSheet::setProtect(bool protect, string password, int enhancedProtection)
	Protects the sheet with password and enchanced parameters below. It is possible to combine a few EnhancedProtection values with operator |. */
EXCEL_METHOD(Sheet, setProtect)
{
	zval *object = ZEND_THIS;
	SheetHandle sheet;
	bool protect;
	zend_string *password_zs = NULL;
	zend_long enhancedProtection = PROT_DEFAULT;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "b|Sl", &protect, &password_zs, &enhancedProtection) == FAILURE) {
		RETURN_FALSE;
	}

	EXCEL_NUL_SAFE_STRING(password_zs)
	/* PROT_DEFAULT == -1 is the "use libxl default protection" sentinel;
	 * otherwise enhancedProtection is a bitmask that must fit in int. */
	if (enhancedProtection < -1 || enhancedProtection > INT_MAX) {
		php_error_docref(NULL, E_WARNING, "Argument out of int range");
		RETURN_FALSE;
	}

	SHEET_FROM_OBJECT(sheet, object);

	xlSheetSetProtectEx(sheet, protect, password_zs ? ZSTR_VAL(password_zs) : "", enhancedProtection);

	RETURN_TRUE;
}
/* }}} */

/* {{{ proto long ExcelSheet::table()
	Gets the table parameters by index. */
EXCEL_METHOD(Sheet, table)
{
	zval *object = ZEND_THIS;
	SheetHandle sheet;
	zend_long index = 0;
	int rowFirst, rowLast, colFirst, colLast, headerRowCount, totalsRowCount;
	const char *name;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "l", &index) == FAILURE) {
		RETURN_FALSE;
	}

	EXCEL_VALIDATE_INT_RANGE(index)

	SHEET_FROM_OBJECT(sheet, object);

	if (!(name = xlSheetTable(sheet, index, &rowFirst, &rowLast, &colFirst, &colLast, &headerRowCount, &totalsRowCount))) {
		RETURN_FALSE;
	}

	array_init(return_value);
	add_assoc_string(return_value, "name", (char *)name);
	add_assoc_long(return_value, "row_first", rowFirst);
	add_assoc_long(return_value, "col_first", colFirst);
	add_assoc_long(return_value, "row_last", rowLast);
	add_assoc_long(return_value, "col_last", colLast);
	add_assoc_long(return_value, "header_row_count", headerRowCount);
	add_assoc_long(return_value, "totals_row_count", totalsRowCount);
}
/* }}} */

/* {{{ proto long ExcelSheet::setTabColor([int color])
	Sets the color for the sheet's tab. */
EXCEL_METHOD(Sheet, setTabColor)
{
	zval *object = ZEND_THIS;
	SheetHandle sheet;
	zend_long color = 0;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "|l", &color) == FAILURE) {
		RETURN_FALSE;
	}

	EXCEL_VALIDATE_INT_RANGE(color)

	SHEET_FROM_OBJECT(sheet, object);

	// @todo check for XLSX format
	xlSheetSetTabColor(sheet, color);
	RETURN_TRUE;
}
/* }}} */

/* {{{ proto long ExcelSheet::autoFilter()
	Returns the AutoFilter. Creates it if it doesn't exist. */
EXCEL_METHOD(Sheet, autoFilter)
{
	zval *object = ZEND_THIS;
	excel_autofilter_object *obj;
	SheetHandle sheet;

	ZEND_PARSE_PARAMETERS_NONE();

	SHEET_FROM_OBJECT(sheet, object);

	// @todo check for XLSX format
	AutoFilterHandle ah = xlSheetAutoFilter(sheet);
	if (!ah) {
		RETURN_FALSE;
	}

	ZVAL_OBJ(return_value, excel_object_new_autofilter(excel_ce_autofilter));
	obj = Z_EXCEL_AUTOFILTER_OBJ_P(return_value);
	obj->autofilter = ah;
	obj->sheet = sheet;
	EXCEL_INIT_AUTOFILTER_PARENT(obj, object);
}
/* }}} */

/* {{{ proto long ExcelSheet::applyFilter()
	Applies the AutoFilter to the sheet. */
EXCEL_METHOD(Sheet, applyFilter)
{
	zval *object = ZEND_THIS;
	SheetHandle sheet;

	ZEND_PARSE_PARAMETERS_NONE();

	SHEET_FROM_OBJECT(sheet, object);

	// @todo check for XLSX format
	xlSheetApplyFilter(sheet);

	RETURN_TRUE;
}
/* }}} */

/* {{{ proto long ExcelSheet::removeFilter()
	Removes the AutoFilter from the sheet. */
EXCEL_METHOD(Sheet, removeFilter)
{
	zval *object = ZEND_THIS;
	SheetHandle sheet;

	ZEND_PARSE_PARAMETERS_NONE();

	SHEET_FROM_OBJECT(sheet, object);

	// @todo check for XLSX format
	xlSheetRemoveFilter(sheet);
	php_excel_book_bump_autofilter_generation(object);

	RETURN_TRUE;
}
/* }}} */

/* {{{ proto long ExcelSheet::addIgnoredError()
	Adds the ignored error for specified range. It allows to hide green triangles on left sides of cells. */
EXCEL_METHOD(Sheet, addIgnoredError)
{
	zval *object = ZEND_THIS;
	SheetHandle sheet;
	zend_long iError, rowFirst=0, colFirst=0, rowLast=0, colLast=0;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "l|llll", &iError, &rowFirst, &colFirst, &rowLast, &colLast) == FAILURE) {
		RETURN_FALSE;
	}

	EXCEL_VALIDATE_ROW_RANGE(rowFirst, rowLast, object);
	EXCEL_VALIDATE_COL_RANGE(colFirst, colLast, object);
	EXCEL_VALIDATE_INT_RANGE(iError)
	SHEET_FROM_OBJECT(sheet, object);

	if (!xlSheetAddIgnoredError(sheet, rowFirst, colFirst, rowLast, colLast, iError)) {
		RETURN_FALSE;
	}

	RETURN_TRUE;
}
/* }}} */

/* {{{ proto long ExcelSheet::writeError()
	Writes error into the cell with specified format. If format equals 0 then format is ignored. */
EXCEL_METHOD(Sheet, writeError)
{
	zval *object = ZEND_THIS;
	SheetHandle sheet;
	zend_long iError=0, row=0, col=0;
	zval *oformat = NULL;
	FormatHandle format = NULL;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "lll|O!", &row, &col, &iError, &oformat, excel_ce_format) == FAILURE) {
		RETURN_FALSE;
	}

	EXCEL_VALIDATE_ROW_COL(row, col, object);
	EXCEL_VALIDATE_INT_RANGE(iError)

	SHEET_FROM_OBJECT(sheet, object);

	if (oformat) {
		FORMAT_FROM_OBJECT(format, oformat);
		EXCEL_REQUIRE_SAME_BOOK(oformat, object);
	}

	xlSheetWriteError(sheet, row, col, iError, format);
}
/* }}} */

/* {{{ proto long ExcelSheet::removeComment()
	Removes a comment from the cell (only for xls format). */
EXCEL_METHOD(Sheet, removeComment)
{
	zval *object = ZEND_THIS;
	SheetHandle sheet;
	zend_long row=0, col=0;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "ll", &row, &col) == FAILURE) {
		RETURN_FALSE;
	}

	EXCEL_VALIDATE_ROW_COL(row, col, object);
	SHEET_FROM_OBJECT(sheet, object);

	xlSheetRemoveComment(sheet, row, col);
}
/* }}} */

/* {{{ proto ExcelAutoFilter ExcelAutoFilter::__construct(ExcelSheet sheet)
	Sheet Constructor. */
EXCEL_METHOD(AutoFilter, __construct)
{
	AutoFilterHandle afh;
	SheetHandle sheet;
	zval *object = ZEND_THIS;
	excel_autofilter_object *obj;
	zval *zsheet = NULL;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "O", &zsheet, excel_ce_sheet) == FAILURE) {
		RETURN_THROWS();
	}

	SHEET_FROM_OBJECT_THROW(sheet, zsheet);

	obj = Z_EXCEL_AUTOFILTER_OBJ_P(object);
	EXCEL_REJECT_RECONSTRUCTION(obj, autofilter);

	afh = xlSheetAutoFilter(sheet);

	if (!afh) {
		zend_throw_exception(NULL, "Failed to create autofilter", 0);
		RETURN_THROWS();
	}

	obj->sheet = sheet;
	obj->autofilter = afh;
	EXCEL_INIT_AUTOFILTER_PARENT(obj, zsheet);
}
/* }}} */

/* {{{ proto long AutoFilter::getRef()
	Gets the cell range of AutoFilter with header. Returns 0 if error. */
EXCEL_METHOD(AutoFilter, getRef)
{
	zval *object = ZEND_THIS;
	AutoFilterHandle autofilter;
	int rowFirst=0, colFirst=0, rowLast=0, colLast=0;

	ZEND_PARSE_PARAMETERS_NONE();

	AUTOFILTER_FROM_OBJECT(autofilter, object);

	if (!xlAutoFilterGetRef(autofilter, &rowFirst, &rowLast, &colFirst, &colLast)) {
		RETURN_FALSE;
	}

	array_init(return_value);
	add_assoc_long(return_value, "row_first", rowFirst);
	add_assoc_long(return_value, "col_first", colFirst);
	add_assoc_long(return_value, "row_last", rowLast);
	add_assoc_long(return_value, "col_last", colLast);
}
/* }}} */

/* {{{ proto long AutoFilter::setRef()
	Sets the cell range of AutoFilter with header. */
EXCEL_METHOD(AutoFilter, setRef)
{
	zval *object = ZEND_THIS;
	AutoFilterHandle autofilter;
	zend_long rowFirst=0, rowLast=0, colFirst=0, colLast=0;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "llll", &rowFirst, &rowLast, &colFirst, &colLast) == FAILURE) {
		RETURN_FALSE;
	}

	EXCEL_VALIDATE_ROW_RANGE(rowFirst, rowLast, object);
	EXCEL_VALIDATE_COL_RANGE(colFirst, colLast, object);
	AUTOFILTER_FROM_OBJECT(autofilter, object);

	xlAutoFilterSetRef(autofilter, rowFirst, rowLast, colFirst, colLast);
}
/* }}} */

/* {{{ proto long AutoFilter::column()
	Returns the AutoFilter column by zero-based index. Creates it if it doesn't exist. */
EXCEL_METHOD(AutoFilter, column)
{
	zval *object = ZEND_THIS;
	AutoFilterHandle autofilter;
	zend_long colId;
	excel_filtercolumn_object *obj;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "l", &colId) == FAILURE) {
		RETURN_FALSE;
	}

	EXCEL_VALIDATE_INT_RANGE(colId)

	AUTOFILTER_FROM_OBJECT(autofilter, object);

	FilterColumnHandle fch = xlAutoFilterColumn(autofilter, colId);
	if (!fch) {
		RETURN_FALSE;
	}

	ZVAL_OBJ(return_value, excel_object_new_filtercolumn(excel_ce_filtercolumn));
	obj = Z_EXCEL_FILTERCOLUMN_OBJ_P(return_value);
	obj->autofilter = autofilter;
	obj->filtercolumn = fch;
	EXCEL_INIT_AUTOFILTER_PARENT(obj, object);
}
/* }}} */

/* {{{ proto long AutoFilter::columnSize()
	Returns the number of specified AutoFilter columns which have a filter information. */
EXCEL_METHOD(AutoFilter, columnSize)
{
	zval *object = ZEND_THIS;
	AutoFilterHandle autofilter;

	ZEND_PARSE_PARAMETERS_NONE();

	AUTOFILTER_FROM_OBJECT(autofilter, object);

	RETURN_LONG(xlAutoFilterColumnSize(autofilter));
}
/* }}} */

/* {{{ proto long AutoFilter::columnByIndex()
	Returns the specified AutoFilter column which have a filter information by index. */
EXCEL_METHOD(AutoFilter, columnByIndex)
{
	zval *object = ZEND_THIS;
	AutoFilterHandle autofilter;
	excel_filtercolumn_object *obj;
	zend_long index;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "l", &index) == FAILURE) {
		RETURN_FALSE;
	}

	EXCEL_VALIDATE_INT_RANGE(index)

	AUTOFILTER_FROM_OBJECT(autofilter, object);

	FilterColumnHandle fch = xlAutoFilterColumnByIndex(autofilter, index);
	if (!fch) {
		RETURN_FALSE;
	}

	ZVAL_OBJ(return_value, excel_object_new_filtercolumn(excel_ce_filtercolumn));
	obj = Z_EXCEL_FILTERCOLUMN_OBJ_P(return_value);
	obj->autofilter = autofilter;
	obj->filtercolumn = fch;
	EXCEL_INIT_AUTOFILTER_PARENT(obj, object);
}
/* }}} */

/* {{{ proto long AutoFilter::getSortRange()
	Gets the whole range of data to sort. Returns 0 if error. */
EXCEL_METHOD(AutoFilter, getSortRange)
{
	zval *object = ZEND_THIS;
	AutoFilterHandle autofilter;
	int rowFirst=0, rowLast=0, colFirst=0, colLast=0;

	ZEND_PARSE_PARAMETERS_NONE();

	AUTOFILTER_FROM_OBJECT(autofilter, object);

	if (!xlAutoFilterGetSortRange(autofilter, &rowFirst, &rowLast, &colFirst, &colLast)) {
		RETURN_FALSE;
	}

	array_init(return_value);
	add_assoc_long(return_value, "row_first", rowFirst);
	add_assoc_long(return_value, "row_last", rowLast);
	add_assoc_long(return_value, "col_first", colFirst);
	add_assoc_long(return_value, "col_last", colLast);
}
/* }}} */

/* {{{ proto long AutoFilter::getSort()
	Gets the zero-based index of sorted column in AutoFilter and its sort order. Returns 0 if error. */
EXCEL_METHOD(AutoFilter, getSort)
{
	zval *object = ZEND_THIS;
	AutoFilterHandle autofilter;
	int columnIndex, descending;

	ZEND_PARSE_PARAMETERS_NONE();

	AUTOFILTER_FROM_OBJECT(autofilter, object);

#ifdef HAVE_LIBXL_AUTOFILTER_GETSORT_LEVEL
	if (!xlAutoFilterGetSort(autofilter, &columnIndex, &descending, 0)) {
#else
	if (!xlAutoFilterGetSort(autofilter, &columnIndex, &descending)) {
#endif
		RETURN_FALSE;
	}

	array_init(return_value);
	add_assoc_long(return_value, "column_index", columnIndex);
	add_assoc_long(return_value, "descending", descending);
}
/* }}} */

/* {{{ proto long AutoFilter::setSort()
	Sets the sorted column in AutoFilter by zero-based index and its sort order. Returns 0 if error. */
EXCEL_METHOD(AutoFilter, setSort)
{
	zval *object = ZEND_THIS;
	AutoFilterHandle autofilter;
	zend_long columnIndex;
	bool descending;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "lb", &columnIndex, &descending) == FAILURE) {
		RETURN_FALSE;
	}

	EXCEL_VALIDATE_INT_RANGE(columnIndex)

	AUTOFILTER_FROM_OBJECT(autofilter, object);

	if (!xlAutoFilterSetSort(autofilter, columnIndex, descending)) {
		RETURN_FALSE;
	}

	RETURN_TRUE;
}
/* }}} */

EXCEL_METHOD(AutoFilter, addSort)
{
	zval *object = ZEND_THIS;
	AutoFilterHandle autofilter;
	zend_long columnIndex;
	bool descending;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "lb", &columnIndex, &descending) == FAILURE) {
		RETURN_FALSE;
	}

	EXCEL_VALIDATE_INT_RANGE(columnIndex)

	AUTOFILTER_FROM_OBJECT(autofilter, object);

	RETURN_BOOL(xlAutoFilterAddSort(autofilter, columnIndex, descending));
}

/* {{{ proto ExcelFilterColumn ExcelFilterColumn::__construct(ExcelAutoFilter autofilter)
	Sheet Constructor. */
EXCEL_METHOD(FilterColumn, __construct)
{
	FilterColumnHandle fch;
	AutoFilterHandle autofilter;
	zval *object = ZEND_THIS;
	excel_filtercolumn_object *obj;
	zval *zautofilter = NULL;
	zend_long colId;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "Ol", &zautofilter, excel_ce_autofilter, &colId) == FAILURE) {
		RETURN_THROWS();
	}

	if (colId < 0 || colId > INT_MAX) {
		zend_throw_exception(NULL, "Argument out of int range", 0);
		RETURN_THROWS();
	}

	AUTOFILTER_FROM_OBJECT_THROW(autofilter, zautofilter);

	obj = Z_EXCEL_FILTERCOLUMN_OBJ_P(object);
	EXCEL_REJECT_RECONSTRUCTION(obj, filtercolumn);

	fch = xlAutoFilterColumn(autofilter, colId);

	if (!fch) {
		zend_throw_exception(NULL, "Failed to get filter column", 0);
		RETURN_THROWS();
	}

	obj->filtercolumn = fch;
	obj->autofilter = autofilter;
	EXCEL_INIT_AUTOFILTER_PARENT(obj, zautofilter);
}
/* }}} */


#define PHP_EXCEL_FILTERCOLUMN_INFO(func_name, type) \
{ \
	zval *object = ZEND_THIS; \
	FilterColumnHandle filtercolumn; \
	ZEND_PARSE_PARAMETERS_NONE(); \
	FILTERCOLUMN_FROM_OBJECT(filtercolumn, object); \
	PE_RETURN_ ## type (xlFilterColumn ## func_name (filtercolumn)); \
}

/* {{{ proto long FilterColumn::index()
	Returns the zero-based index of this AutoFilter column. */
EXCEL_METHOD(FilterColumn, index)
{
	PHP_EXCEL_FILTERCOLUMN_INFO(Index, IS_LONG)
}
/* }}} */

/* {{{ proto long FilterColumn::filterType()
	Returns the filter type of this AutoFilter column. */
EXCEL_METHOD(FilterColumn, filterType)
{
	PHP_EXCEL_FILTERCOLUMN_INFO(FilterType, IS_LONG)
}
/* }}} */

/* {{{ proto long FilterColumn::filterSize()
	Returns the number of filter values. */
EXCEL_METHOD(FilterColumn, filterSize)
{
	PHP_EXCEL_FILTERCOLUMN_INFO(FilterSize, IS_LONG)
}
/* }}} */

/* {{{ proto long FilterColumn::filter()
	Returns the filter value by index. */
EXCEL_METHOD(FilterColumn, filter)
{
	zval *object = ZEND_THIS;
	FilterColumnHandle filtercolumn;
	zend_long filterIndex;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "l", &filterIndex) == FAILURE) {
		RETURN_FALSE;
	}

	EXCEL_VALIDATE_INT_RANGE(filterIndex)

	FILTERCOLUMN_FROM_OBJECT(filtercolumn, object);

	const char *val = xlFilterColumnFilter(filtercolumn, filterIndex);
	if (!val) {
		RETURN_FALSE;
	}
	RETURN_STRING((char *)val);
}
/* }}} */

/* {{{ proto long FilterColumn::addFilter()
	Adds the filter value. */
EXCEL_METHOD(FilterColumn, addFilter)
{
	zval *object = ZEND_THIS;
	FilterColumnHandle filtercolumn;
	zend_string *filtervalue_zs = NULL;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "S", &filtervalue_zs) == FAILURE) {
		RETURN_FALSE;
	}

	EXCEL_NUL_SAFE_STRING(filtervalue_zs)

	FILTERCOLUMN_FROM_OBJECT(filtercolumn, object);

	xlFilterColumnAddFilter(filtercolumn, ZSTR_VAL(filtervalue_zs));

	RETURN_TRUE;
}
/* }}} */

/* {{{ proto long FilterColumn::getTop10()
	Gets the number of top or bottom items: */
EXCEL_METHOD(FilterColumn, getTop10)
{
	zval *object = ZEND_THIS;
	FilterColumnHandle filtercolumn;
	double value;
	int top, percent;

	ZEND_PARSE_PARAMETERS_NONE();

	FILTERCOLUMN_FROM_OBJECT(filtercolumn, object);

	if (!xlFilterColumnGetTop10(filtercolumn, &value, &top, &percent)) {
		RETURN_FALSE;
	}

	array_init(return_value);
	add_assoc_double(return_value, "value", value);
	add_assoc_bool(return_value, "top", top);
	add_assoc_bool(return_value, "percent", percent);
}
/* }}} */

/* {{{ proto long FilterColumn::setTop10()
	Sets the number of top or bottom items: */
EXCEL_METHOD(FilterColumn, setTop10)
{
	zval *object = ZEND_THIS;
	FilterColumnHandle filtercolumn;
	double value;
	bool top = 1, percent = 0;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "d|bb", &value, &top, &percent) == FAILURE) {
		RETURN_FALSE;
	}

	EXCEL_VALIDATE_FINITE(value)
	FILTERCOLUMN_FROM_OBJECT(filtercolumn, object);

	xlFilterColumnSetTop10(filtercolumn, value, top, percent);

	RETURN_TRUE;
}
/* }}} */

/* {{{ proto long FilterColumn::getCustomFilter()
	Gets the custom filter criteria: */
EXCEL_METHOD(FilterColumn, getCustomFilter)
{
	zval *object = ZEND_THIS;
	FilterColumnHandle filtercolumn;
	int op1, op2, andOp;
	const char *v1 = NULL, *v2 = NULL;

	ZEND_PARSE_PARAMETERS_NONE();

	FILTERCOLUMN_FROM_OBJECT(filtercolumn, object);

	if (!xlFilterColumnGetCustomFilter(filtercolumn, &op1, &v1, &op2, &v2, &andOp)) {
		RETURN_FALSE;
	}

	array_init(return_value);
	add_assoc_long(return_value, "operator_1", op1);
	add_assoc_string(return_value, "value_1", v1 ? (char *)v1 : "");
	add_assoc_long(return_value, "operator_2", op2);
	add_assoc_string(return_value, "value_2", v2 ? (char *)v2 : "");
	add_assoc_bool(return_value, "and_operator", andOp);
}
/* }}} */

/* {{{ proto long FilterColumn::setCustomFilter()
	Sets the custom filter criteria: */
EXCEL_METHOD(FilterColumn, setCustomFilter)
{
	zval *object = ZEND_THIS;
	FilterColumnHandle filtercolumn;
	zend_long op1, op2 = -1;
	zend_string *v1 = NULL, *v2 = NULL;
	bool andOp = 0;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "lS|lS!b", &op1, &v1, &op2, &v2, &andOp) == FAILURE) {
		RETURN_FALSE;
	}

	EXCEL_VALIDATE_INT_RANGE(op1)
	/* op2 == -1 is the "no second criterion" sentinel; otherwise it is an
	 * operator enum that must fit in int. */
	if (op2 < -1 || op2 > INT_MAX) {
		php_error_docref(NULL, E_WARNING, "Argument out of int range");
		RETURN_FALSE;
	}

	FILTERCOLUMN_FROM_OBJECT(filtercolumn, object);

	EXCEL_NON_EMPTY_STRING(v1)
	EXCEL_NUL_SAFE_STRING(v1)

	if (op2 == -1 || !v2) {
		xlFilterColumnSetCustomFilter(filtercolumn, op1, ZSTR_VAL(v1));
		RETURN_TRUE;
	}

	EXCEL_NON_EMPTY_STRING(v2)
	EXCEL_NUL_SAFE_STRING(v2)

	xlFilterColumnSetCustomFilterEx(filtercolumn, op1, ZSTR_VAL(v1), op2, ZSTR_VAL(v2), andOp);
	RETURN_TRUE;
}
/* }}} */

/* {{{ proto long FilterColumn::clear()
	Clear the filter criteria. */
EXCEL_METHOD(FilterColumn, clear)
{
	zval *object = ZEND_THIS;
	FilterColumnHandle filtercolumn;

	ZEND_PARSE_PARAMETERS_NONE();

	FILTERCOLUMN_FROM_OBJECT(filtercolumn, object);

	xlFilterColumnClear(filtercolumn);

	RETURN_TRUE;
}
/* }}} */

/* {{{ proto long ExcelBook::addPictureAsLink(str filename, bool insert)
	Adds a picture to the workbook as link (only for xlsx files) */
EXCEL_METHOD(Book, addPictureAsLink)
{
	zval *object = ZEND_THIS;
	BookHandle book;
	zend_string *filename;
	bool insert = 0;
	zend_long result;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "S|b", &filename, &insert) == FAILURE) {
		RETURN_FALSE;
	}

	EXCEL_NON_EMPTY_STRING(filename)
	EXCEL_NUL_SAFE_STRING(filename)

	if (php_check_open_basedir(ZSTR_VAL(filename))) {
		RETURN_FALSE;
	}
	if (PG(open_basedir) && *PG(open_basedir)) {
		php_error_docref(NULL, E_WARNING, "addPictureAsLink is not available while open_basedir is active");
		RETURN_FALSE;
	}

	BOOK_FROM_OBJECT(book, object);

	result = xlBookAddPictureAsLink(book, ZSTR_VAL(filename), insert);

	if (-1 == result) {
		php_error_docref(NULL, E_WARNING, "Could not add picture as link.");
		RETURN_FALSE;
	}

	RETURN_LONG(result);
}
/* }}} */

/* {{{ proto bool ExcelBook::moveSheet(int src_index, int dest_index)
	Moves a sheet with specified index to a new position. Returns 0 if error occurs. */
EXCEL_METHOD(Book, moveSheet)
{
	BookHandle book;
	zval *object = ZEND_THIS;
	zend_long src_index, dest_index;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "ll", &src_index, &dest_index) == FAILURE) {
		RETURN_FALSE;
	}

	EXCEL_VALIDATE_INT_RANGE(src_index)
	EXCEL_VALIDATE_INT_RANGE(dest_index)

	BOOK_FROM_OBJECT(book, object);

	if (!xlBookMoveSheet(book, src_index, dest_index)) {
		RETURN_FALSE;
	}

	/* Existing Sheet wrappers point at libxl handles by index; moving a
	 * sheet shifts internal indices so the wrappers silently retarget to
	 * a different sheet. Bump sheet topology generation to invalidate them. */
	php_excel_book_bump_sheet_generation(object);
	RETURN_TRUE;
}
/* }}} */

/* {{{ proto bool Sheet::addDataValidation()
	Adds a data validation for the specified range (only for xlsx files). */
EXCEL_METHOD(Sheet, addDataValidation)
{
	zval *object = ZEND_THIS;
	SheetHandle sheet;

	zend_long type, op, row_first, row_last, col_first, col_last;
	zend_string *val_1, *val_2 = NULL;
	bool allow_blank = 1, hide_dropdown=0, show_inputmessage = 1, show_errormessage = 1;
	zend_string *prompt_title = NULL, *prompt = NULL;
	zend_string *error_title = NULL, *error = NULL;
	zend_long error_style = 1;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "llllllS|S!bbbbSSSSl", &type, &op, &row_first, &row_last, \
			&col_first, &col_last, &val_1, &val_2, &allow_blank, &hide_dropdown, &show_inputmessage, \
			&show_errormessage, &prompt_title, &prompt, &error_title, &error, &error_style) == FAILURE) {
		RETURN_FALSE;
	}

	EXCEL_VALIDATE_INT_RANGE(type)
	EXCEL_VALIDATE_INT_RANGE(op)
	EXCEL_VALIDATE_INT_RANGE(error_style)

	if (!val_1 || ZSTR_LEN(val_1) < 1) {
		php_error_docref(NULL, E_WARNING, "The first value can not be empty.");
		RETURN_FALSE;
	}
	EXCEL_NUL_SAFE_STRING(val_1)
	EXCEL_NUL_SAFE_STRING(val_2)
	EXCEL_NUL_SAFE_STRING(prompt_title)
	EXCEL_NUL_SAFE_STRING(prompt)
	EXCEL_NUL_SAFE_STRING(error_title)
	EXCEL_NUL_SAFE_STRING(error)
	EXCEL_VALIDATE_ROW_RANGE(row_first, row_last, object);
	EXCEL_VALIDATE_COL_RANGE(col_first, col_last, object);

	/* BETWEEN / NOT-BETWEEN need both endpoints. Reject not just "argument
	 * was omitted" (ZEND_NUM_ARGS() < 8) but also explicit null and empty
	 * string, so reflection callers replaying getDefaultValue() can't
	 * accidentally produce a one-sided rule. */
	if ((op == VALIDATION_OP_BETWEEN || op == VALIDATION_OP_NOTBETWEEN)
	    && (ZEND_NUM_ARGS() < 8 || !val_2 || ZSTR_LEN(val_2) < 1)) {
		php_error_docref(NULL, E_WARNING, "The second value can not be null when used with (not) between operator.");
		RETURN_FALSE;
	}

	SHEET_FROM_OBJECT(sheet, object);

	xlSheetAddDataValidationEx(sheet, type, op, row_first, row_last, col_first, col_last, ZSTR_VAL(val_1), \
			val_2 ? ZSTR_VAL(val_2) : "", allow_blank, hide_dropdown, show_inputmessage, show_errormessage, \
			prompt_title ? ZSTR_VAL(prompt_title) : "", prompt ? ZSTR_VAL(prompt) : "", \
			error_title ? ZSTR_VAL(error_title) : "", error ? ZSTR_VAL(error) : "", error_style);

	RETURN_TRUE;
}
/* }}} */

/* {{{ proto bool Sheet::addDataValidationDouble()
	Adds a data validation for the specified range with double or date values for the relational operator
	(only for xlsx files). See parameters in the xlSheetAddDataValidation() method. */
EXCEL_METHOD(Sheet, addDataValidationDouble)
{
	zval *object = ZEND_THIS;
	SheetHandle sheet;

	zend_long type, op, row_first, row_last, col_first, col_last;
	double val_1, val_2 = 0.0;
	bool val_2_is_null = true;  /* "argument omitted" defaults to "no second value" */
	bool allow_blank = 1, hide_dropdown=0, show_inputmessage = 1, show_errormessage = 1;
	zend_string *prompt_title = NULL, *prompt = NULL;
	zend_string *error_title = NULL, *error = NULL;
	zend_long error_style = 1;

	/* FAST_ZPP so val_2 keeps the standard `d` coercion path (numeric
	 * strings, bool, int) while still distinguishing explicit null
	 * (or omission) from a real value, which the BETWEEN guard needs. */
	ZEND_PARSE_PARAMETERS_START(7, 17)
		Z_PARAM_LONG(type)
		Z_PARAM_LONG(op)
		Z_PARAM_LONG(row_first)
		Z_PARAM_LONG(row_last)
		Z_PARAM_LONG(col_first)
		Z_PARAM_LONG(col_last)
		Z_PARAM_DOUBLE(val_1)
		Z_PARAM_OPTIONAL
		Z_PARAM_DOUBLE_OR_NULL(val_2, val_2_is_null)
		Z_PARAM_BOOL(allow_blank)
		Z_PARAM_BOOL(hide_dropdown)
		Z_PARAM_BOOL(show_inputmessage)
		Z_PARAM_BOOL(show_errormessage)
		Z_PARAM_STR(prompt_title)
		Z_PARAM_STR(prompt)
		Z_PARAM_STR(error_title)
		Z_PARAM_STR(error)
		Z_PARAM_LONG(error_style)
	ZEND_PARSE_PARAMETERS_END();

	EXCEL_VALIDATE_FINITE(val_1)
	if (!val_2_is_null) {
		EXCEL_VALIDATE_FINITE(val_2)
	}
	EXCEL_NUL_SAFE_STRING(prompt_title)
	EXCEL_NUL_SAFE_STRING(prompt)
	EXCEL_NUL_SAFE_STRING(error_title)
	EXCEL_NUL_SAFE_STRING(error)
	EXCEL_VALIDATE_ROW_RANGE(row_first, row_last, object);
	EXCEL_VALIDATE_COL_RANGE(col_first, col_last, object);
	EXCEL_VALIDATE_INT_RANGE(type)
	EXCEL_VALIDATE_INT_RANGE(op)
	EXCEL_VALIDATE_INT_RANGE(error_style)

	if ((op == VALIDATION_OP_BETWEEN || op == VALIDATION_OP_NOTBETWEEN) && val_2_is_null) {
		php_error_docref(NULL, E_WARNING, "The second value can not be null when used with (not) between operator.");
		RETURN_FALSE;
	}

	SHEET_FROM_OBJECT(sheet, object);

	xlSheetAddDataValidationDoubleEx(sheet, type, op, row_first, row_last, col_first, col_last, val_1, \
			val_2, allow_blank, hide_dropdown, show_inputmessage, show_errormessage, \
			prompt_title ? ZSTR_VAL(prompt_title) : "", prompt ? ZSTR_VAL(prompt) : "", \
			error_title ? ZSTR_VAL(error_title) : "", error ? ZSTR_VAL(error) : "", error_style);

	RETURN_TRUE;
}
/* }}} */

/* {{{ proto bool Sheet::removeDataValidations()
	Removes all data validations for the sheet (only for xlsx files). */
EXCEL_METHOD(Sheet, removeDataValidations)
{
	PHP_EXCEL_SHEET_VOID(RemoveDataValidations)
}
/* }}} */

#if LIBXL_VERSION >= 0x05020000
/* {{{ proto int Sheet::dataValidationSize()
	Returns the number of data validations in the sheet (only for xlsx files). */
EXCEL_METHOD(Sheet, dataValidationSize)
{
	PHP_EXCEL_INFO(DataValidationSize, IS_LONG)
}
/* }}} */

/* {{{ proto array Sheet::dataValidation(int $index)
	Returns the data validation at the zero-based index (only for xlsx files). */
EXCEL_METHOD(Sheet, dataValidation)
{
	zval *object = ZEND_THIS;
	SheetHandle sheet;
	zend_long index;
	int type, op, row_first, row_last, col_first, col_last;
	const char *value1 = NULL, *value2 = NULL;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "l", &index) == FAILURE) {
		RETURN_FALSE;
	}

	EXCEL_VALIDATE_INT_RANGE(index)

	SHEET_FROM_OBJECT(sheet, object);

	if (!xlSheetDataValidation(sheet, index, &type, &op, &row_first, &row_last, &col_first, &col_last, &value1, &value2)) {
		RETURN_FALSE;
	}

	array_init(return_value);
	add_assoc_long(return_value, "type", type);
	add_assoc_long(return_value, "op", op);
	add_assoc_long(return_value, "row_first", row_first);
	add_assoc_long(return_value, "row_last", row_last);
	add_assoc_long(return_value, "col_first", col_first);
	add_assoc_long(return_value, "col_last", col_last);
	add_assoc_string(return_value, "value1", value1 ? value1 : "");
	add_assoc_string(return_value, "value2", value2 ? value2 : "");
}
/* }}} */
#endif

EXCEL_METHOD(Sheet, firstFilledRow)
{
	PHP_EXCEL_INFO(FirstFilledRow, IS_LONG)
}

EXCEL_METHOD(Sheet, lastFilledRow)
{
	PHP_EXCEL_INFO(LastFilledRow, IS_LONG)
}

EXCEL_METHOD(Sheet, firstFilledCol)
{
	PHP_EXCEL_INFO(FirstFilledCol, IS_LONG)
}

EXCEL_METHOD(Sheet, lastFilledCol)
{
	PHP_EXCEL_INFO(LastFilledCol, IS_LONG)
}

EXCEL_METHOD(Sheet, removePicture)
{
	SheetHandle sheet;
	zval *object = ZEND_THIS;
	zend_long row, col;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "ll", &row, &col) == FAILURE) {
		RETURN_FALSE;
	}

	EXCEL_VALIDATE_ROW_COL(row, col, object);
	SHEET_FROM_OBJECT(sheet, object);

	RETURN_BOOL(xlSheetRemovePicture(sheet, row, col));
}

EXCEL_METHOD(Sheet, removePictureByIndex)
{
	SheetHandle sheet;
	zval *object = ZEND_THIS;
	zend_long index;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "l", &index) == FAILURE) {
		RETURN_FALSE;
	}

	EXCEL_VALIDATE_INT_RANGE(index)

	SHEET_FROM_OBJECT(sheet, object);

	RETURN_BOOL(xlSheetRemovePictureByIndex(sheet, index));
}

EXCEL_METHOD(Sheet, isRichStr)
{
	PHP_EXCEL_SHEET_GET_BOOL_STATE(IsRichStr)
}

EXCEL_METHOD(Sheet, readRichStr)
{
	zval *object = ZEND_THIS;
	SheetHandle sheet;
	BookHandle book;
	zend_long row, col;
	RichStringHandle rs;
	excel_richstring_object *ro;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "ll", &row, &col) == FAILURE) {
		RETURN_FALSE;
	}

	EXCEL_VALIDATE_ROW_COL(row, col, object);
	SHEET_AND_BOOK_FROM_OBJECT(sheet, book, object);

	rs = xlSheetReadRichStr(sheet, row, col, NULL);
	if (!rs) {
		RETURN_FALSE;
	}

	ZVAL_OBJ(return_value, excel_object_new_richstring(excel_ce_richstring));
	ro = Z_EXCEL_RICHSTRING_OBJ_P(return_value);
	ro->richstring = rs;
	ro->book = book;
	EXCEL_INIT_PARENT(ro, object);
}

EXCEL_METHOD(Sheet, writeRichStr)
{
	zval *object = ZEND_THIS;
	SheetHandle sheet;
	zend_long row, col;
	zval *zrs;
	zval *zfmt = NULL;
	FormatHandle format = NULL;
	RichStringHandle rs;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "llO|O!", &row, &col, &zrs, excel_ce_richstring, &zfmt, excel_ce_format) == FAILURE) {
		RETURN_FALSE;
	}

	EXCEL_VALIDATE_ROW_COL(row, col, object);
	SHEET_FROM_OBJECT(sheet, object);
	RICHSTRING_FROM_OBJECT(rs, zrs);
	EXCEL_REQUIRE_SAME_BOOK(zrs, object);
	if (zfmt) {
		FORMAT_FROM_OBJECT(format, zfmt);
		EXCEL_REQUIRE_SAME_BOOK(zfmt, object);
	}

	RETURN_BOOL(xlSheetWriteRichStr(sheet, row, col, rs, format));
}

EXCEL_METHOD(Sheet, formControlSize)
{
	PHP_EXCEL_INFO(FormControlSize, IS_LONG)
}

EXCEL_METHOD(Sheet, formControl)
{
	zval *object = ZEND_THIS;
	SheetHandle sheet;
	zend_long index;
	FormControlHandle fc;
	excel_formcontrol_object *fco;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "l", &index) == FAILURE) {
		RETURN_FALSE;
	}

	EXCEL_VALIDATE_INT_RANGE(index)

	SHEET_FROM_OBJECT(sheet, object);

	fc = xlSheetFormControl(sheet, index);
	if (!fc) {
		RETURN_FALSE;
	}

	ZVAL_OBJ(return_value, excel_object_new_formcontrol(excel_ce_formcontrol));
	fco = Z_EXCEL_FORMCONTROL_OBJ_P(return_value);
	fco->formcontrol = fc;
	fco->sheet = sheet;
	EXCEL_INIT_SHEET_PARENT(fco, object);
}

EXCEL_METHOD(Sheet, getActiveCell)
{
	SheetHandle sheet;
	zval *object = ZEND_THIS;
	int row = 0, col = 0;

	ZEND_PARSE_PARAMETERS_NONE();

	SHEET_FROM_OBJECT(sheet, object);

	if (!xlSheetGetActiveCell(sheet, &row, &col)) {
		RETURN_FALSE;
	}

	array_init(return_value);
	add_assoc_long(return_value, "row", row);
	add_assoc_long(return_value, "col", col);
}

EXCEL_METHOD(Sheet, setActiveCell)
{
	SheetHandle sheet;
	zval *object = ZEND_THIS;
	zend_long row, col;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "ll", &row, &col) == FAILURE) {
		RETURN_FALSE;
	}

	EXCEL_VALIDATE_ROW_COL(row, col, object);
	SHEET_FROM_OBJECT(sheet, object);

	xlSheetSetActiveCell(sheet, row, col);
	RETURN_TRUE;
}

EXCEL_METHOD(Sheet, selectionRange)
{
	SheetHandle sheet;
	zval *object = ZEND_THIS;
	const char *result;

	ZEND_PARSE_PARAMETERS_NONE();

	SHEET_FROM_OBJECT(sheet, object);

	result = xlSheetSelectionRange(sheet);
	PE_RETURN_IS_STRING(result)
}

EXCEL_METHOD(Sheet, addSelectionRange)
{
	SheetHandle sheet;
	zval *object = ZEND_THIS;
	zend_string *sqref;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "S", &sqref) == FAILURE) {
		RETURN_FALSE;
	}

	EXCEL_NON_EMPTY_STRING(sqref)
	EXCEL_NUL_SAFE_STRING(sqref)

	SHEET_FROM_OBJECT(sheet, object);

	xlSheetAddSelectionRange(sheet, ZSTR_VAL(sqref));
	RETURN_TRUE;
}

EXCEL_METHOD(Sheet, removeSelection)
{
	PHP_EXCEL_SHEET_VOID(RemoveSelection)
}

EXCEL_METHOD(Sheet, tabColor)
{
	PHP_EXCEL_INFO(TabColor, IS_LONG)
}

EXCEL_METHOD(Sheet, getTabRgbColor)
{
	SheetHandle sheet;
	zval *object = ZEND_THIS;
	int r = 0, g = 0, b = 0;

	ZEND_PARSE_PARAMETERS_NONE();

	SHEET_FROM_OBJECT(sheet, object);

	if (!xlSheetGetTabRgbColor(sheet, &r, &g, &b)) {
		RETURN_FALSE;
	}

	array_init(return_value);
	add_assoc_long(return_value, "red", r);
	add_assoc_long(return_value, "green", g);
	add_assoc_long(return_value, "blue", b);
}

EXCEL_METHOD(Sheet, setTabRgbColor)
{
	SheetHandle sheet;
	zval *object = ZEND_THIS;
	zend_long r, g, b;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "lll", &r, &g, &b) == FAILURE) {
		RETURN_FALSE;
	}

	EXCEL_VALIDATE_RGB(r)
	EXCEL_VALIDATE_RGB(g)
	EXCEL_VALIDATE_RGB(b)

	SHEET_FROM_OBJECT(sheet, object);

	xlSheetSetTabRgbColor(sheet, r, g, b);
	RETURN_TRUE;
}

EXCEL_METHOD(Sheet, hyperlinkIndex)
{
	SheetHandle sheet;
	zval *object = ZEND_THIS;
	zend_long row, col;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "ll", &row, &col) == FAILURE) {
		RETURN_FALSE;
	}

	EXCEL_VALIDATE_ROW_COL(row, col, object);
	SHEET_FROM_OBJECT(sheet, object);

	RETURN_LONG(xlSheetHyperlinkIndex(sheet, row, col));
}

EXCEL_METHOD(Sheet, colWidthPx)
{
	SheetHandle sheet;
	zval *object = ZEND_THIS;
	zend_long col;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "l", &col) == FAILURE) {
		RETURN_FALSE;
	}

	EXCEL_VALIDATE_COL(col, object);
	SHEET_FROM_OBJECT(sheet, object);

	RETURN_LONG(xlSheetColWidthPx(sheet, col));
}

EXCEL_METHOD(Sheet, rowHeightPx)
{
	SheetHandle sheet;
	zval *object = ZEND_THIS;
	zend_long row;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "l", &row) == FAILURE) {
		RETURN_FALSE;
	}

	EXCEL_VALIDATE_ROW(row, object);
	SHEET_FROM_OBJECT(sheet, object);

	RETURN_LONG(xlSheetRowHeightPx(sheet, row));
}

EXCEL_METHOD(Sheet, colFormat)
{
	zval *object = ZEND_THIS;
	SheetHandle sheet;
	BookHandle book;
	zend_long col;
	FormatHandle format;
	excel_format_object *fo;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "l", &col) == FAILURE) {
		RETURN_FALSE;
	}

	EXCEL_VALIDATE_COL(col, object);
	SHEET_AND_BOOK_FROM_OBJECT(sheet, book, object);

	format = xlSheetColFormat(sheet, col);
	if (!format) {
		RETURN_FALSE;
	}

	ZVAL_OBJ(return_value, excel_object_new_format(excel_ce_format));
	fo = Z_EXCEL_FORMAT_OBJ_P(return_value);
	fo->format = format;
	fo->book = book;
	EXCEL_INIT_PARENT(fo, object);
}

EXCEL_METHOD(Sheet, rowFormat)
{
	zval *object = ZEND_THIS;
	SheetHandle sheet;
	BookHandle book;
	zend_long row;
	FormatHandle format;
	excel_format_object *fo;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "l", &row) == FAILURE) {
		RETURN_FALSE;
	}

	EXCEL_VALIDATE_ROW(row, object);
	SHEET_AND_BOOK_FROM_OBJECT(sheet, book, object);

	format = xlSheetRowFormat(sheet, row);
	if (!format) {
		RETURN_FALSE;
	}

	ZVAL_OBJ(return_value, excel_object_new_format(excel_ce_format));
	fo = Z_EXCEL_FORMAT_OBJ_P(return_value);
	fo->format = format;
	fo->book = book;
	EXCEL_INIT_PARENT(fo, object);
}

EXCEL_METHOD(Sheet, setColPx)
{
	SheetHandle sheet;
	FormatHandle format = NULL;
	zval *object = ZEND_THIS;
	zend_long colFirst, colLast, widthPx;
	zval *f = NULL;
	bool hidden = 0;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "lll|O!b", &colFirst, &colLast, &widthPx, &f, excel_ce_format, &hidden) == FAILURE) {
		RETURN_FALSE;
	}

	EXCEL_VALIDATE_COL_RANGE(colFirst, colLast, object);
	EXCEL_VALIDATE_INT_RANGE(widthPx)
	SHEET_FROM_OBJECT(sheet, object);

	if (f) {
		FORMAT_FROM_OBJECT(format, f);
		EXCEL_REQUIRE_SAME_BOOK(f, object);
	}

	RETURN_BOOL(xlSheetSetColPx(sheet, colFirst, colLast, widthPx, format, hidden));
}

EXCEL_METHOD(Sheet, setRowPx)
{
	SheetHandle sheet;
	FormatHandle format = NULL;
	zval *object = ZEND_THIS;
	zend_long row, heightPx;
	zval *f = NULL;
	bool hidden = 0;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "ll|O!b", &row, &heightPx, &f, excel_ce_format, &hidden) == FAILURE) {
		RETURN_FALSE;
	}

	{
		excel_book_object *_vb = php_excel_resolve_book_obj(object);
		zend_long _maxr = (_vb && _vb->is_xlsx) ? EXCEL_MAX_ROW_XLSX : EXCEL_MAX_ROW_XLS;
		if (row < 0 || row > _maxr) {
			php_error_docref(NULL, E_WARNING, "Invalid row: " ZEND_LONG_FMT, row);
			RETURN_FALSE;
		}
	}
	EXCEL_VALIDATE_INT_RANGE(heightPx)
	SHEET_FROM_OBJECT(sheet, object);

	if (f) {
		FORMAT_FROM_OBJECT(format, f);
		EXCEL_REQUIRE_SAME_BOOK(f, object);
	}

	RETURN_BOOL(xlSheetSetRowPx(sheet, row, heightPx, format, hidden));
}

EXCEL_METHOD(Sheet, setBorder)
{
	SheetHandle sheet;
	zval *object = ZEND_THIS;
	zend_long rowFirst, rowLast, colFirst, colLast, borderStyle, borderColor;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "llllll", &rowFirst, &rowLast, &colFirst, &colLast, &borderStyle, &borderColor) == FAILURE) {
		RETURN_FALSE;
	}

	EXCEL_VALIDATE_ROW_RANGE(rowFirst, rowLast, object);
	EXCEL_VALIDATE_COL_RANGE(colFirst, colLast, object);
	EXCEL_VALIDATE_INT_RANGE(borderStyle)
	EXCEL_VALIDATE_INT_RANGE(borderColor)
	SHEET_FROM_OBJECT(sheet, object);

	RETURN_BOOL(xlSheetSetBorder(sheet, rowFirst, rowLast, colFirst, colLast, borderStyle, borderColor));
}

EXCEL_METHOD(Sheet, addTable)
{
	zval *object = ZEND_THIS;
	SheetHandle sheet;
	zend_string *name;
	zend_long rowFirst, rowLast, colFirst, colLast;
	bool hasHeaders = 1;
	zend_long style = 0;
	TableHandle th;
	excel_table_object *to;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "Sllll|bl", &name, &rowFirst, &rowLast, &colFirst, &colLast, &hasHeaders, &style) == FAILURE) {
		RETURN_FALSE;
	}

	EXCEL_NON_EMPTY_STRING(name)
	EXCEL_NUL_SAFE_STRING(name)
	EXCEL_VALIDATE_ROW_RANGE(rowFirst, rowLast, object);
	EXCEL_VALIDATE_COL_RANGE(colFirst, colLast, object);
	EXCEL_VALIDATE_INT_RANGE(style)

	SHEET_FROM_OBJECT(sheet, object);

	th = xlSheetAddTable(sheet, ZSTR_VAL(name), rowFirst, rowLast, colFirst, colLast, hasHeaders, style);
	if (!th) {
		RETURN_FALSE;
	}

	ZVAL_OBJ(return_value, excel_object_new_table(excel_ce_table));
	to = Z_EXCEL_TABLE_OBJ_P(return_value);
	to->table = th;
	to->sheet = sheet;
	EXCEL_INIT_SHEET_PARENT(to, object);
}

EXCEL_METHOD(Sheet, getTableByName)
{
	zval *object = ZEND_THIS;
	SheetHandle sheet;
	zend_string *name;
	TableHandle th;
	excel_table_object *to;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "S", &name) == FAILURE) {
		RETURN_FALSE;
	}

	EXCEL_NON_EMPTY_STRING(name)
	EXCEL_NUL_SAFE_STRING(name)

	SHEET_FROM_OBJECT(sheet, object);

	th = xlSheetGetTableByName(sheet, ZSTR_VAL(name));
	if (!th) {
		RETURN_FALSE;
	}

	ZVAL_OBJ(return_value, excel_object_new_table(excel_ce_table));
	to = Z_EXCEL_TABLE_OBJ_P(return_value);
	to->table = th;
	to->sheet = sheet;
	EXCEL_INIT_SHEET_PARENT(to, object);
}

EXCEL_METHOD(Sheet, getTableByIndex)
{
	zval *object = ZEND_THIS;
	SheetHandle sheet;
	zend_long index;
	TableHandle th;
	excel_table_object *to;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "l", &index) == FAILURE) {
		RETURN_FALSE;
	}

	EXCEL_VALIDATE_INT_RANGE(index)

	SHEET_FROM_OBJECT(sheet, object);

	th = xlSheetGetTableByIndex(sheet, index);
	if (!th) {
		RETURN_FALSE;
	}

	ZVAL_OBJ(return_value, excel_object_new_table(excel_ce_table));
	to = Z_EXCEL_TABLE_OBJ_P(return_value);
	to->table = th;
	to->sheet = sheet;
	EXCEL_INIT_SHEET_PARENT(to, object);
}

EXCEL_METHOD(Sheet, applyFilter2)
{
	zval *object = ZEND_THIS;
	SheetHandle sheet;
	zval *zaf;
	AutoFilterHandle autofilter;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "O", &zaf, excel_ce_autofilter) == FAILURE) {
		RETURN_FALSE;
	}

	SHEET_FROM_OBJECT(sheet, object);
	AUTOFILTER_FROM_OBJECT(autofilter, zaf);
	EXCEL_REQUIRE_SAME_BOOK(zaf, object);

	xlSheetApplyFilter2(sheet, autofilter);
	RETURN_TRUE;
}

EXCEL_METHOD(Sheet, addConditionalFormatting)
{
	zval *object = ZEND_THIS;
	SheetHandle sheet;
	ConditionalFormattingHandle cfh;
	excel_conditionalformatting_object *cfo;
#if LIBXL_VERSION >= 0x05010000
	zend_long rowFirst, rowLast, colFirst, colLast;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "llll", &rowFirst, &rowLast, &colFirst, &colLast) == FAILURE) {
		RETURN_FALSE;
	}

	EXCEL_VALIDATE_ROW_RANGE(rowFirst, rowLast, object);
	EXCEL_VALIDATE_COL_RANGE(colFirst, colLast, object);
	SHEET_FROM_OBJECT(sheet, object);

	cfh = xlSheetAddConditionalFormatting(sheet, rowFirst, rowLast, colFirst, colLast);
#else
	ZEND_PARSE_PARAMETERS_NONE();

	SHEET_FROM_OBJECT(sheet, object);

	cfh = xlSheetAddConditionalFormatting(sheet);
#endif
	if (!cfh) {
		RETURN_FALSE;
	}

	ZVAL_OBJ(return_value, excel_object_new_conditionalformatting(excel_ce_conditionalformatting));
	cfo = Z_EXCEL_CONDITIONALFORMATTING_OBJ_P(return_value);
	cfo->conditionalformatting = cfh;
	cfo->sheet = sheet;
	EXCEL_INIT_CONDITIONALFORMATTING_PARENT(cfo, object);
}

#if LIBXL_VERSION >= 0x05010000
EXCEL_METHOD(Sheet, conditionalFormatting)
{
	zval *object = ZEND_THIS;
	SheetHandle sheet;
	zend_long index;
	ConditionalFormattingHandle cfh;
	excel_conditionalformatting_object *cfo;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "l", &index) == FAILURE) {
		RETURN_FALSE;
	}

	EXCEL_VALIDATE_INT_RANGE(index)

	SHEET_FROM_OBJECT(sheet, object);

	cfh = xlSheetConditionalFormatting(sheet, index);
	if (!cfh) {
		RETURN_FALSE;
	}

	ZVAL_OBJ(return_value, excel_object_new_conditionalformatting(excel_ce_conditionalformatting));
	cfo = Z_EXCEL_CONDITIONALFORMATTING_OBJ_P(return_value);
	cfo->conditionalformatting = cfh;
	cfo->sheet = sheet;
	EXCEL_INIT_CONDITIONALFORMATTING_PARENT(cfo, object);
}

EXCEL_METHOD(Sheet, removeConditionalFormatting)
{
	SheetHandle sheet;
	zval *object = ZEND_THIS;
	zend_long index;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "l", &index) == FAILURE) {
		RETURN_FALSE;
	}

	EXCEL_VALIDATE_INT_RANGE(index)

	SHEET_FROM_OBJECT(sheet, object);

	{
		int ret = xlSheetRemoveConditionalFormatting(sheet, index);
		if (ret) {
			php_excel_book_bump_conditional_formatting_generation(object);
		}
		RETURN_BOOL(ret);
	}
}

EXCEL_METHOD(Sheet, conditionalFormattingSize)
{
	PHP_EXCEL_INFO(ConditionalFormattingSize, IS_LONG)
}
#endif

/* RichString methods */

EXCEL_METHOD(RichString, __construct)
{
	BookHandle book;
	RichStringHandle rs;
	zval *object = ZEND_THIS;
	excel_richstring_object *obj;
	zval *zbook;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "O", &zbook, excel_ce_book) == FAILURE) {
		RETURN_THROWS();
	}

	BOOK_FROM_OBJECT_THROW(book, zbook);

	obj = Z_EXCEL_RICHSTRING_OBJ_P(object);
	EXCEL_REJECT_RECONSTRUCTION(obj, richstring);

	rs = xlBookAddRichString(book);
	if (!rs) {
		zend_throw_exception(NULL, "Failed to create rich string", 0);
		RETURN_THROWS();
	}

	obj->richstring = rs;
	obj->book = book;
	EXCEL_INIT_PARENT(obj, zbook);
}

EXCEL_METHOD(RichString, addFont)
{
	zval *object = ZEND_THIS;
	RichStringHandle rs;
	FontHandle font = NULL;
	FontHandle nfont;
	zval *zfont = NULL;
	excel_font_object *fo;
	excel_richstring_object *ro;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "|O!", &zfont, excel_ce_font) == FAILURE) {
		RETURN_FALSE;
	}

	RICHSTRING_FROM_OBJECT(rs, object);

	if (zfont) {
		FONT_FROM_OBJECT(font, zfont);
	}

	nfont = xlRichStringAddFont(rs, font);
	if (!nfont) {
		RETURN_FALSE;
	}

	ro = Z_EXCEL_RICHSTRING_OBJ_P(object);

	ZVAL_OBJ(return_value, excel_object_new_font(excel_ce_font));
	fo = Z_EXCEL_FONT_OBJ_P(return_value);
	fo->font = nfont;
	fo->book = ro->book;
	EXCEL_INIT_PARENT(fo, object);
}

EXCEL_METHOD(RichString, addText)
{
	zval *object = ZEND_THIS;
	RichStringHandle rs;
	zend_string *text;
	FontHandle font = NULL;
	zval *zfont = NULL;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "S|O!", &text, &zfont, excel_ce_font) == FAILURE) {
		RETURN_FALSE;
	}

	EXCEL_NUL_SAFE_STRING(text)

	RICHSTRING_FROM_OBJECT(rs, object);

	if (zfont) {
		FONT_FROM_OBJECT(font, zfont);
	}

	xlRichStringAddText(rs, ZSTR_VAL(text), font);
	RETURN_TRUE;
}

EXCEL_METHOD(RichString, getText)
{
	zval *object = ZEND_THIS;
	RichStringHandle rs;
	zend_long index;
	FontHandle font = NULL;
	const char *text;
	excel_font_object *fo;
	excel_richstring_object *ro;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "l", &index) == FAILURE) {
		RETURN_FALSE;
	}

	EXCEL_VALIDATE_INT_RANGE(index)

	RICHSTRING_FROM_OBJECT(rs, object);

	text = xlRichStringGetText(rs, index, &font);
	if (!text) {
		RETURN_FALSE;
	}

	ro = Z_EXCEL_RICHSTRING_OBJ_P(object);

	array_init(return_value);
	add_assoc_string(return_value, "text", (char *)text);

	if (font) {
		zval zfont;
		ZVAL_OBJ(&zfont, excel_object_new_font(excel_ce_font));
		fo = Z_EXCEL_FONT_OBJ_P(&zfont);
		fo->font = font;
		fo->book = ro->book;
		EXCEL_INIT_PARENT(fo, object);
		add_assoc_zval(return_value, "font", &zfont);
	} else {
		add_assoc_null(return_value, "font");
	}
}

EXCEL_METHOD(RichString, textSize)
{
	zval *object = ZEND_THIS;
	RichStringHandle rs;

	ZEND_PARSE_PARAMETERS_NONE();

	RICHSTRING_FROM_OBJECT(rs, object);

	RETURN_LONG(xlRichStringTextSize(rs));
}

/* FormControl methods */

EXCEL_METHOD(FormControl, __construct)
{
	SheetHandle sheet;
	FormControlHandle fc;
	zval *object = ZEND_THIS;
	excel_formcontrol_object *obj;
	zval *zsheet;
	zend_long index;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "Ol", &zsheet, excel_ce_sheet, &index) == FAILURE) {
		RETURN_THROWS();
	}

	if (index < 0 || index > INT_MAX) {
		zend_throw_exception(NULL, "Argument out of int range", 0);
		RETURN_THROWS();
	}

	SHEET_FROM_OBJECT_THROW(sheet, zsheet);

	obj = Z_EXCEL_FORMCONTROL_OBJ_P(object);
	EXCEL_REJECT_RECONSTRUCTION(obj, formcontrol);

	fc = xlSheetFormControl(sheet, index);
	if (!fc) {
		zend_throw_exception(NULL, "Failed to get form control", 0);
		RETURN_THROWS();
	}

	obj->formcontrol = fc;
	obj->sheet = sheet;
	EXCEL_INIT_SHEET_PARENT(obj, zsheet);
}

#define FORMCONTROL_LONG_GETTER(method_name, api_func) \
EXCEL_METHOD(FormControl, method_name) \
{ \
	zval *object = ZEND_THIS; \
	FormControlHandle fc; \
	ZEND_PARSE_PARAMETERS_NONE(); \
	FORMCONTROL_FROM_OBJECT(fc, object); \
	RETURN_LONG(api_func(fc)); \
}

#define FORMCONTROL_LONG_SETTER(method_name, api_func) \
EXCEL_METHOD(FormControl, method_name) \
{ \
	zval *object = ZEND_THIS; \
	FormControlHandle fc; \
	zend_long val; \
	if (zend_parse_parameters(ZEND_NUM_ARGS(), "l", &val) == FAILURE) { \
		RETURN_FALSE; \
	} \
	EXCEL_VALIDATE_INT_RANGE(val) \
	FORMCONTROL_FROM_OBJECT(fc, object); \
	api_func(fc, val); \
	RETURN_TRUE; \
}

#define FORMCONTROL_STRING_GETTER(method_name, api_func) \
EXCEL_METHOD(FormControl, method_name) \
{ \
	zval *object = ZEND_THIS; \
	FormControlHandle fc; \
	const char *result; \
	ZEND_PARSE_PARAMETERS_NONE(); \
	FORMCONTROL_FROM_OBJECT(fc, object); \
	result = api_func(fc); \
	PE_RETURN_IS_STRING(result) \
}

#define FORMCONTROL_STRING_SETTER(method_name, api_func) \
EXCEL_METHOD(FormControl, method_name) \
{ \
	zval *object = ZEND_THIS; \
	FormControlHandle fc; \
	zend_string *val; \
	if (zend_parse_parameters(ZEND_NUM_ARGS(), "S", &val) == FAILURE) { \
		RETURN_FALSE; \
	} \
	EXCEL_NUL_SAFE_STRING(val) \
	FORMCONTROL_FROM_OBJECT(fc, object); \
	api_func(fc, ZSTR_VAL(val)); \
	RETURN_TRUE; \
}

#define FORMCONTROL_BOOL_GETTER(method_name, api_func) \
EXCEL_METHOD(FormControl, method_name) \
{ \
	zval *object = ZEND_THIS; \
	FormControlHandle fc; \
	ZEND_PARSE_PARAMETERS_NONE(); \
	FORMCONTROL_FROM_OBJECT(fc, object); \
	RETURN_BOOL(api_func(fc)); \
}

#define FORMCONTROL_BOOL_SETTER(method_name, api_func) \
EXCEL_METHOD(FormControl, method_name) \
{ \
	zval *object = ZEND_THIS; \
	FormControlHandle fc; \
	bool val; \
	if (zend_parse_parameters(ZEND_NUM_ARGS(), "b", &val) == FAILURE) { \
		RETURN_FALSE; \
	} \
	FORMCONTROL_FROM_OBJECT(fc, object); \
	api_func(fc, val); \
	RETURN_TRUE; \
}

FORMCONTROL_LONG_GETTER(objectType, xlFormControlObjectType)
FORMCONTROL_LONG_GETTER(checked, xlFormControlChecked)
FORMCONTROL_LONG_SETTER(setChecked, xlFormControlSetChecked)
FORMCONTROL_STRING_GETTER(fmlaGroup, xlFormControlFmlaGroup)
FORMCONTROL_STRING_SETTER(setFmlaGroup, xlFormControlSetFmlaGroup)
FORMCONTROL_STRING_GETTER(fmlaLink, xlFormControlFmlaLink)
FORMCONTROL_STRING_SETTER(setFmlaLink, xlFormControlSetFmlaLink)
FORMCONTROL_STRING_GETTER(fmlaRange, xlFormControlFmlaRange)
FORMCONTROL_STRING_SETTER(setFmlaRange, xlFormControlSetFmlaRange)
FORMCONTROL_STRING_GETTER(fmlaTxbx, xlFormControlFmlaTxbx)
FORMCONTROL_STRING_SETTER(setFmlaTxbx, xlFormControlSetFmlaTxbx)
FORMCONTROL_STRING_GETTER(name, xlFormControlName)
FORMCONTROL_STRING_GETTER(linkedCell, xlFormControlLinkedCell)
FORMCONTROL_STRING_GETTER(listFillRange, xlFormControlListFillRange)
FORMCONTROL_STRING_GETTER(macro, xlFormControlMacro)
FORMCONTROL_STRING_GETTER(altText, xlFormControlAltText)
FORMCONTROL_BOOL_GETTER(locked, xlFormControlLocked)
FORMCONTROL_BOOL_GETTER(defaultSize, xlFormControlDefaultSize)
FORMCONTROL_BOOL_GETTER(print, xlFormControlPrint)
FORMCONTROL_BOOL_GETTER(disabled, xlFormControlDisabled)

EXCEL_METHOD(FormControl, item)
{
	zval *object = ZEND_THIS;
	FormControlHandle fc;
	zend_long index;
	const char *result;
	if (zend_parse_parameters(ZEND_NUM_ARGS(), "l", &index) == FAILURE) {
		RETURN_FALSE;
	}
	EXCEL_VALIDATE_INT_RANGE(index)
	FORMCONTROL_FROM_OBJECT(fc, object);
	result = xlFormControlItem(fc, index);
	PE_RETURN_IS_STRING(result)
}

FORMCONTROL_LONG_GETTER(itemSize, xlFormControlItemSize)

EXCEL_METHOD(FormControl, addItem)
{
	zval *object = ZEND_THIS;
	FormControlHandle fc;
	zend_string *val;
	if (zend_parse_parameters(ZEND_NUM_ARGS(), "S", &val) == FAILURE) {
		RETURN_FALSE;
	}
	EXCEL_NUL_SAFE_STRING(val)
	FORMCONTROL_FROM_OBJECT(fc, object);
	xlFormControlAddItem(fc, ZSTR_VAL(val));
	RETURN_TRUE;
}

EXCEL_METHOD(FormControl, insertItem)
{
	zval *object = ZEND_THIS;
	FormControlHandle fc;
	zend_long index;
	zend_string *val;
	if (zend_parse_parameters(ZEND_NUM_ARGS(), "lS", &index, &val) == FAILURE) {
		RETURN_FALSE;
	}
	EXCEL_NUL_SAFE_STRING(val)
	EXCEL_VALIDATE_INT_RANGE(index)
	FORMCONTROL_FROM_OBJECT(fc, object);
	xlFormControlInsertItem(fc, index, ZSTR_VAL(val));
	RETURN_TRUE;
}

EXCEL_METHOD(FormControl, clearItems)
{
	zval *object = ZEND_THIS;
	FormControlHandle fc;
	ZEND_PARSE_PARAMETERS_NONE();

	FORMCONTROL_FROM_OBJECT(fc, object);
	xlFormControlClearItems(fc);
	RETURN_TRUE;
}

FORMCONTROL_LONG_GETTER(dropLines, xlFormControlDropLines)
FORMCONTROL_LONG_SETTER(setDropLines, xlFormControlSetDropLines)
FORMCONTROL_LONG_GETTER(dx, xlFormControlDx)
FORMCONTROL_LONG_SETTER(setDx, xlFormControlSetDx)
FORMCONTROL_BOOL_GETTER(firstButton, xlFormControlFirstButton)
FORMCONTROL_BOOL_SETTER(setFirstButton, xlFormControlSetFirstButton)
FORMCONTROL_BOOL_GETTER(horiz, xlFormControlHoriz)
FORMCONTROL_BOOL_SETTER(setHoriz, xlFormControlSetHoriz)
FORMCONTROL_LONG_GETTER(inc, xlFormControlInc)
FORMCONTROL_LONG_SETTER(setInc, xlFormControlSetInc)
FORMCONTROL_LONG_GETTER(getMax, xlFormControlGetMax)
FORMCONTROL_LONG_SETTER(setMax, xlFormControlSetMax)
FORMCONTROL_LONG_GETTER(getMin, xlFormControlGetMin)
FORMCONTROL_LONG_SETTER(setMin, xlFormControlSetMin)
FORMCONTROL_STRING_GETTER(multiSel, xlFormControlMultiSel)
FORMCONTROL_STRING_SETTER(setMultiSel, xlFormControlSetMultiSel)
FORMCONTROL_LONG_GETTER(sel, xlFormControlSel)
FORMCONTROL_LONG_SETTER(setSel, xlFormControlSetSel)

EXCEL_METHOD(FormControl, fromAnchor)
{
	zval *object = ZEND_THIS;
	FormControlHandle fc;
	int col = 0, colOff = 0, row = 0, rowOff = 0;

	ZEND_PARSE_PARAMETERS_NONE();

	FORMCONTROL_FROM_OBJECT(fc, object);

	if (!xlFormControlFromAnchor(fc, &col, &colOff, &row, &rowOff)) {
		RETURN_FALSE;
	}

	array_init(return_value);
	add_assoc_long(return_value, "col", col);
	add_assoc_long(return_value, "colOff", colOff);
	add_assoc_long(return_value, "row", row);
	add_assoc_long(return_value, "rowOff", rowOff);
}

EXCEL_METHOD(FormControl, toAnchor)
{
	zval *object = ZEND_THIS;
	FormControlHandle fc;
	int col = 0, colOff = 0, row = 0, rowOff = 0;

	ZEND_PARSE_PARAMETERS_NONE();

	FORMCONTROL_FROM_OBJECT(fc, object);

	if (!xlFormControlToAnchor(fc, &col, &colOff, &row, &rowOff)) {
		RETURN_FALSE;
	}

	array_init(return_value);
	add_assoc_long(return_value, "col", col);
	add_assoc_long(return_value, "colOff", colOff);
	add_assoc_long(return_value, "row", row);
	add_assoc_long(return_value, "rowOff", rowOff);
}

/* ConditionalFormat methods */

EXCEL_METHOD(ConditionalFormat, __construct)
{
	BookHandle book;
	ConditionalFormatHandle cf;
	zval *object = ZEND_THIS;
	excel_conditionalformat_object *obj;
	zval *zbook;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "O", &zbook, excel_ce_book) == FAILURE) {
		RETURN_THROWS();
	}

	BOOK_FROM_OBJECT_THROW(book, zbook);

	obj = Z_EXCEL_CONDITIONALFORMAT_OBJ_P(object);
	EXCEL_REJECT_RECONSTRUCTION(obj, conditionalformat);

	cf = xlBookAddConditionalFormat(book);
	if (!cf) {
		zend_throw_exception(NULL, "Failed to create conditional format", 0);
		RETURN_THROWS();
	}

	obj->conditionalformat = cf;
	obj->book = book;
	EXCEL_INIT_PARENT(obj, zbook);
}

EXCEL_METHOD(ConditionalFormat, font)
{
	zval *object = ZEND_THIS;
	ConditionalFormatHandle cf;
	FontHandle font;
	excel_font_object *fo;
	excel_conditionalformat_object *cfo;

	ZEND_PARSE_PARAMETERS_NONE();

	CONDITIONALFORMAT_FROM_OBJECT(cf, object);

	font = xlConditionalFormatFont(cf);
	if (!font) {
		RETURN_FALSE;
	}

	cfo = Z_EXCEL_CONDITIONALFORMAT_OBJ_P(object);

	ZVAL_OBJ(return_value, excel_object_new_font(excel_ce_font));
	fo = Z_EXCEL_FONT_OBJ_P(return_value);
	fo->font = font;
	fo->book = cfo->book;
	EXCEL_INIT_PARENT(fo, object);
}

#define CF_LONG_GETTER(method_name, api_func) \
EXCEL_METHOD(ConditionalFormat, method_name) \
{ \
	zval *object = ZEND_THIS; \
	ConditionalFormatHandle cf; \
	ZEND_PARSE_PARAMETERS_NONE(); \
	CONDITIONALFORMAT_FROM_OBJECT(cf, object); \
	RETURN_LONG(api_func(cf)); \
}

#define CF_LONG_SETTER(method_name, api_func) \
EXCEL_METHOD(ConditionalFormat, method_name) \
{ \
	zval *object = ZEND_THIS; \
	ConditionalFormatHandle cf; \
	zend_long val; \
	if (zend_parse_parameters(ZEND_NUM_ARGS(), "l", &val) == FAILURE) { \
		RETURN_FALSE; \
	} \
	EXCEL_VALIDATE_INT_RANGE(val) \
	CONDITIONALFORMAT_FROM_OBJECT(cf, object); \
	api_func(cf, val); \
	RETURN_TRUE; \
}

#define CF_STRING_GETTER(method_name, api_func) \
EXCEL_METHOD(ConditionalFormat, method_name) \
{ \
	zval *object = ZEND_THIS; \
	ConditionalFormatHandle cf; \
	const char *result; \
	ZEND_PARSE_PARAMETERS_NONE(); \
	CONDITIONALFORMAT_FROM_OBJECT(cf, object); \
	result = api_func(cf); \
	PE_RETURN_IS_STRING(result) \
}

#define CF_STRING_SETTER(method_name, api_func) \
EXCEL_METHOD(ConditionalFormat, method_name) \
{ \
	zval *object = ZEND_THIS; \
	ConditionalFormatHandle cf; \
	zend_string *val; \
	if (zend_parse_parameters(ZEND_NUM_ARGS(), "S", &val) == FAILURE) { \
		RETURN_FALSE; \
	} \
	EXCEL_NUL_SAFE_STRING(val) \
	CONDITIONALFORMAT_FROM_OBJECT(cf, object); \
	api_func(cf, ZSTR_VAL(val)); \
	RETURN_TRUE; \
}

CF_LONG_GETTER(numFormat, xlConditionalFormatNumFormat)
CF_LONG_SETTER(setNumFormat, xlConditionalFormatSetNumFormat)
CF_STRING_GETTER(customNumFormat, xlConditionalFormatCustomNumFormat)
CF_STRING_SETTER(setCustomNumFormat, xlConditionalFormatSetCustomNumFormat)
CF_LONG_SETTER(setBorder, xlConditionalFormatSetBorder)
CF_LONG_SETTER(setBorderColor, xlConditionalFormatSetBorderColor)
CF_LONG_GETTER(borderLeft, xlConditionalFormatBorderLeft)
CF_LONG_SETTER(setBorderLeft, xlConditionalFormatSetBorderLeft)
CF_LONG_GETTER(borderRight, xlConditionalFormatBorderRight)
CF_LONG_SETTER(setBorderRight, xlConditionalFormatSetBorderRight)
CF_LONG_GETTER(borderTop, xlConditionalFormatBorderTop)
CF_LONG_SETTER(setBorderTop, xlConditionalFormatSetBorderTop)
CF_LONG_GETTER(borderBottom, xlConditionalFormatBorderBottom)
CF_LONG_SETTER(setBorderBottom, xlConditionalFormatSetBorderBottom)
CF_LONG_GETTER(borderLeftColor, xlConditionalFormatBorderLeftColor)
CF_LONG_SETTER(setBorderLeftColor, xlConditionalFormatSetBorderLeftColor)
CF_LONG_GETTER(borderRightColor, xlConditionalFormatBorderRightColor)
CF_LONG_SETTER(setBorderRightColor, xlConditionalFormatSetBorderRightColor)
CF_LONG_GETTER(borderTopColor, xlConditionalFormatBorderTopColor)
CF_LONG_SETTER(setBorderTopColor, xlConditionalFormatSetBorderTopColor)
CF_LONG_GETTER(borderBottomColor, xlConditionalFormatBorderBottomColor)
CF_LONG_SETTER(setBorderBottomColor, xlConditionalFormatSetBorderBottomColor)
CF_LONG_GETTER(fillPattern, xlConditionalFormatFillPattern)
CF_LONG_SETTER(setFillPattern, xlConditionalFormatSetFillPattern)
CF_LONG_GETTER(patternForegroundColor, xlConditionalFormatPatternForegroundColor)
CF_LONG_SETTER(setPatternForegroundColor, xlConditionalFormatSetPatternForegroundColor)
CF_LONG_GETTER(patternBackgroundColor, xlConditionalFormatPatternBackgroundColor)
CF_LONG_SETTER(setPatternBackgroundColor, xlConditionalFormatSetPatternBackgroundColor)

/* ConditionalFormatting methods */

EXCEL_METHOD(ConditionalFormatting, __construct)
{
	SheetHandle sheet;
	ConditionalFormattingHandle cfh;
	zval *object = ZEND_THIS;
	excel_conditionalformatting_object *obj;
	zval *zsheet;
#if LIBXL_VERSION >= 0x05010000
	zend_long rowFirst, rowLast, colFirst, colLast;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "Ollll", &zsheet, excel_ce_sheet, &rowFirst, &rowLast, &colFirst, &colLast) == FAILURE) {
		RETURN_THROWS();
	}

	{
		/* PHP ignores constructor return values, so throw on bad coordinates
		 * instead of leaving the caller with an uninitialized wrapper. */
		excel_book_object *_vb = php_excel_resolve_book_obj(zsheet);
		zend_long _maxr = (_vb && _vb->is_xlsx) ? EXCEL_MAX_ROW_XLSX : EXCEL_MAX_ROW_XLS;
		zend_long _maxc = (_vb && _vb->is_xlsx) ? EXCEL_MAX_COL_XLSX : EXCEL_MAX_COL_XLS;
		if (rowFirst < 0 || rowFirst > _maxr || rowLast < 0 || rowLast > _maxr) {
			zend_throw_exception_ex(NULL, 0,
				"Invalid row range: first=" ZEND_LONG_FMT ", last=" ZEND_LONG_FMT,
				rowFirst, rowLast);
			RETURN_THROWS();
		}
		if (rowFirst > rowLast) {
			zend_throw_exception_ex(NULL, 0,
				"Invalid row range: first=" ZEND_LONG_FMT ", last=" ZEND_LONG_FMT,
				rowFirst, rowLast);
			RETURN_THROWS();
		}
		if (colFirst < 0 || colFirst > _maxc || colLast < 0 || colLast > _maxc) {
			zend_throw_exception_ex(NULL, 0,
				"Invalid column range: first=" ZEND_LONG_FMT ", last=" ZEND_LONG_FMT,
				colFirst, colLast);
			RETURN_THROWS();
		}
		if (colFirst > colLast) {
			zend_throw_exception_ex(NULL, 0,
				"Invalid column range: first=" ZEND_LONG_FMT ", last=" ZEND_LONG_FMT,
				colFirst, colLast);
			RETURN_THROWS();
		}
	}
#else
	if (zend_parse_parameters(ZEND_NUM_ARGS(), "O", &zsheet, excel_ce_sheet) == FAILURE) {
		RETURN_THROWS();
	}
#endif

	SHEET_FROM_OBJECT_THROW(sheet, zsheet);

	obj = Z_EXCEL_CONDITIONALFORMATTING_OBJ_P(object);
	EXCEL_REJECT_RECONSTRUCTION(obj, conditionalformatting);

#if LIBXL_VERSION >= 0x05010000
	cfh = xlSheetAddConditionalFormatting(sheet, rowFirst, rowLast, colFirst, colLast);
#else
	cfh = xlSheetAddConditionalFormatting(sheet);
#endif
	if (!cfh) {
		zend_throw_exception(NULL, "Failed to create conditional formatting", 0);
		RETURN_THROWS();
	}

	obj->conditionalformatting = cfh;
	obj->sheet = sheet;
	EXCEL_INIT_CONDITIONALFORMATTING_PARENT(obj, zsheet);
}

EXCEL_METHOD(ConditionalFormatting, addRange)
{
	zval *object = ZEND_THIS;
	ConditionalFormattingHandle cfing;
	zend_long rowFirst, rowLast, colFirst, colLast;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "llll", &rowFirst, &rowLast, &colFirst, &colLast) == FAILURE) {
		RETURN_FALSE;
	}

	EXCEL_VALIDATE_ROW_RANGE(rowFirst, rowLast, object);
	EXCEL_VALIDATE_COL_RANGE(colFirst, colLast, object);
	CONDITIONALFORMATTING_FROM_OBJECT(cfing, object);

	xlConditionalFormattingAddRange(cfing, rowFirst, rowLast, colFirst, colLast);
	RETURN_TRUE;
}

EXCEL_METHOD(ConditionalFormatting, addRule)
{
	zval *object = ZEND_THIS;
	ConditionalFormattingHandle cfing;
	ConditionalFormatHandle cf;
	zend_long type;
	zval *zcf;
	zend_string *value;
	bool stopIfTrue = 0;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "lOS|b", &type, &zcf, excel_ce_conditionalformat, &value, &stopIfTrue) == FAILURE) {
		RETURN_FALSE;
	}

	EXCEL_VALIDATE_INT_RANGE(type)
	EXCEL_NUL_SAFE_STRING(value)

	CONDITIONALFORMATTING_FROM_OBJECT(cfing, object);
	CONDITIONALFORMAT_FROM_OBJECT(cf, zcf);
	EXCEL_REQUIRE_SAME_BOOK(zcf, object);

	xlConditionalFormattingAddRule(cfing, type, cf, ZSTR_VAL(value), stopIfTrue);
	RETURN_TRUE;
}

EXCEL_METHOD(ConditionalFormatting, addTopRule)
{
	zval *object = ZEND_THIS;
	ConditionalFormattingHandle cfing;
	ConditionalFormatHandle cf;
	zval *zcf;
	zend_long value;
	bool bottom, percent, stopIfTrue = 0;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "Olbb|b", &zcf, excel_ce_conditionalformat, &value, &bottom, &percent, &stopIfTrue) == FAILURE) {
		RETURN_FALSE;
	}

	EXCEL_VALIDATE_INT_RANGE(value)

	CONDITIONALFORMATTING_FROM_OBJECT(cfing, object);
	CONDITIONALFORMAT_FROM_OBJECT(cf, zcf);
	EXCEL_REQUIRE_SAME_BOOK(zcf, object);

	xlConditionalFormattingAddTopRule(cfing, cf, value, bottom, percent, stopIfTrue);
	RETURN_TRUE;
}

EXCEL_METHOD(ConditionalFormatting, addOpNumRule)
{
	zval *object = ZEND_THIS;
	ConditionalFormattingHandle cfing;
	ConditionalFormatHandle cf;
	zend_long op;
	zval *zcf;
	double v1, v2;
	bool stopIfTrue = 0;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "lOdd|b", &op, &zcf, excel_ce_conditionalformat, &v1, &v2, &stopIfTrue) == FAILURE) {
		RETURN_FALSE;
	}

	EXCEL_VALIDATE_FINITE(v1)
	EXCEL_VALIDATE_FINITE(v2)
	EXCEL_VALIDATE_INT_RANGE(op)

	CONDITIONALFORMATTING_FROM_OBJECT(cfing, object);
	CONDITIONALFORMAT_FROM_OBJECT(cf, zcf);
	EXCEL_REQUIRE_SAME_BOOK(zcf, object);

	xlConditionalFormattingAddOpNumRule(cfing, op, cf, v1, v2, stopIfTrue);
	RETURN_TRUE;
}

EXCEL_METHOD(ConditionalFormatting, addOpStrRule)
{
	zval *object = ZEND_THIS;
	ConditionalFormattingHandle cfing;
	ConditionalFormatHandle cf;
	zend_long op;
	zval *zcf;
	zend_string *v1, *v2;
	bool stopIfTrue = 0;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "lOSS|b", &op, &zcf, excel_ce_conditionalformat, &v1, &v2, &stopIfTrue) == FAILURE) {
		RETURN_FALSE;
	}

	EXCEL_VALIDATE_INT_RANGE(op)
	EXCEL_NUL_SAFE_STRING(v1)
	EXCEL_NUL_SAFE_STRING(v2)

	CONDITIONALFORMATTING_FROM_OBJECT(cfing, object);
	CONDITIONALFORMAT_FROM_OBJECT(cf, zcf);
	EXCEL_REQUIRE_SAME_BOOK(zcf, object);

	xlConditionalFormattingAddOpStrRule(cfing, op, cf, ZSTR_VAL(v1), ZSTR_VAL(v2), stopIfTrue);
	RETURN_TRUE;
}

EXCEL_METHOD(ConditionalFormatting, addAboveAverageRule)
{
	zval *object = ZEND_THIS;
	ConditionalFormattingHandle cfing;
	ConditionalFormatHandle cf;
	zval *zcf;
	bool above, equal;
	zend_long stdDev;
	bool stopIfTrue = 0;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "Obbl|b", &zcf, excel_ce_conditionalformat, &above, &equal, &stdDev, &stopIfTrue) == FAILURE) {
		RETURN_FALSE;
	}

	EXCEL_VALIDATE_INT_RANGE(stdDev)

	CONDITIONALFORMATTING_FROM_OBJECT(cfing, object);
	CONDITIONALFORMAT_FROM_OBJECT(cf, zcf);
	EXCEL_REQUIRE_SAME_BOOK(zcf, object);

	xlConditionalFormattingAddAboveAverageRule(cfing, cf, above, equal, stdDev, stopIfTrue);
	RETURN_TRUE;
}

EXCEL_METHOD(ConditionalFormatting, addTimePeriodRule)
{
	zval *object = ZEND_THIS;
	ConditionalFormattingHandle cfing;
	ConditionalFormatHandle cf;
	zval *zcf;
	zend_long timePeriod;
	bool stopIfTrue = 0;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "Ol|b", &zcf, excel_ce_conditionalformat, &timePeriod, &stopIfTrue) == FAILURE) {
		RETURN_FALSE;
	}

	EXCEL_VALIDATE_INT_RANGE(timePeriod)

	CONDITIONALFORMATTING_FROM_OBJECT(cfing, object);
	CONDITIONALFORMAT_FROM_OBJECT(cf, zcf);
	EXCEL_REQUIRE_SAME_BOOK(zcf, object);

	xlConditionalFormattingAddTimePeriodRule(cfing, cf, timePeriod, stopIfTrue);
	RETURN_TRUE;
}

EXCEL_METHOD(ConditionalFormatting, add2ColorScaleRule)
{
	zval *object = ZEND_THIS;
	ConditionalFormattingHandle cfing;
	zend_long minColor, maxColor, minType, maxType;
	double minVal, maxVal;
	bool stopIfTrue = 0;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "llldld|b", &minColor, &maxColor, &minType, &minVal, &maxType, &maxVal, &stopIfTrue) == FAILURE) {
		RETURN_FALSE;
	}

	EXCEL_VALIDATE_FINITE(minVal)
	EXCEL_VALIDATE_FINITE(maxVal)
	EXCEL_VALIDATE_INT_RANGE(minColor)
	EXCEL_VALIDATE_INT_RANGE(maxColor)
	EXCEL_VALIDATE_INT_RANGE(minType)
	EXCEL_VALIDATE_INT_RANGE(maxType)

	CONDITIONALFORMATTING_FROM_OBJECT(cfing, object);

	xlConditionalFormattingAdd2ColorScaleRule(cfing, minColor, maxColor, minType, minVal, maxType, maxVal, stopIfTrue);
	RETURN_TRUE;
}

EXCEL_METHOD(ConditionalFormatting, add2ColorScaleFormulaRule)
{
	zval *object = ZEND_THIS;
	ConditionalFormattingHandle cfing;
	zend_long minColor, maxColor, minType, maxType;
	zend_string *minVal, *maxVal;
	bool stopIfTrue = 0;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "lllSlS|b", &minColor, &maxColor, &minType, &minVal, &maxType, &maxVal, &stopIfTrue) == FAILURE) {
		RETURN_FALSE;
	}

	EXCEL_VALIDATE_INT_RANGE(minColor)
	EXCEL_VALIDATE_INT_RANGE(maxColor)
	EXCEL_VALIDATE_INT_RANGE(minType)
	EXCEL_VALIDATE_INT_RANGE(maxType)
	EXCEL_NUL_SAFE_STRING(minVal)
	EXCEL_NUL_SAFE_STRING(maxVal)

	CONDITIONALFORMATTING_FROM_OBJECT(cfing, object);

	xlConditionalFormattingAdd2ColorScaleFormulaRule(cfing, minColor, maxColor, minType, ZSTR_VAL(minVal), maxType, ZSTR_VAL(maxVal), stopIfTrue);
	RETURN_TRUE;
}

EXCEL_METHOD(ConditionalFormatting, add3ColorScaleRule)
{
	zval *object = ZEND_THIS;
	ConditionalFormattingHandle cfing;
	zend_long minColor, midColor, maxColor, minType, midType, maxType;
	double minVal, midVal, maxVal;
	bool stopIfTrue = 0;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "lllldldld|b", &minColor, &midColor, &maxColor, &minType, &minVal, &midType, &midVal, &maxType, &maxVal, &stopIfTrue) == FAILURE) {
		RETURN_FALSE;
	}

	EXCEL_VALIDATE_FINITE(minVal)
	EXCEL_VALIDATE_FINITE(midVal)
	EXCEL_VALIDATE_FINITE(maxVal)
	EXCEL_VALIDATE_INT_RANGE(minColor)
	EXCEL_VALIDATE_INT_RANGE(midColor)
	EXCEL_VALIDATE_INT_RANGE(maxColor)
	EXCEL_VALIDATE_INT_RANGE(minType)
	EXCEL_VALIDATE_INT_RANGE(midType)
	EXCEL_VALIDATE_INT_RANGE(maxType)

	CONDITIONALFORMATTING_FROM_OBJECT(cfing, object);

	xlConditionalFormattingAdd3ColorScaleRule(cfing, minColor, midColor, maxColor, minType, minVal, midType, midVal, maxType, maxVal, stopIfTrue);
	RETURN_TRUE;
}

EXCEL_METHOD(ConditionalFormatting, add3ColorScaleFormulaRule)
{
	zval *object = ZEND_THIS;
	ConditionalFormattingHandle cfing;
	zend_long minColor, midColor, maxColor, minType, midType, maxType;
	zend_string *minVal, *midVal, *maxVal;
	bool stopIfTrue = 0;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "llllSlSlS|b", &minColor, &midColor, &maxColor, &minType, &minVal, &midType, &midVal, &maxType, &maxVal, &stopIfTrue) == FAILURE) {
		RETURN_FALSE;
	}

	EXCEL_VALIDATE_INT_RANGE(minColor)
	EXCEL_VALIDATE_INT_RANGE(midColor)
	EXCEL_VALIDATE_INT_RANGE(maxColor)
	EXCEL_VALIDATE_INT_RANGE(minType)
	EXCEL_VALIDATE_INT_RANGE(midType)
	EXCEL_VALIDATE_INT_RANGE(maxType)
	EXCEL_NUL_SAFE_STRING(minVal)
	EXCEL_NUL_SAFE_STRING(midVal)
	EXCEL_NUL_SAFE_STRING(maxVal)

	CONDITIONALFORMATTING_FROM_OBJECT(cfing, object);

	xlConditionalFormattingAdd3ColorScaleFormulaRule(cfing, minColor, midColor, maxColor, minType, ZSTR_VAL(minVal), midType, ZSTR_VAL(midVal), maxType, ZSTR_VAL(maxVal), stopIfTrue);
	RETURN_TRUE;
}

/* CoreProperties methods */

EXCEL_METHOD(CoreProperties, __construct)
{
	BookHandle book;
	CorePropertiesHandle cp;
	zval *object = ZEND_THIS;
	excel_coreproperties_object *obj;
	zval *zbook;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "O", &zbook, excel_ce_book) == FAILURE) {
		RETURN_THROWS();
	}

	BOOK_FROM_OBJECT_THROW(book, zbook);

	obj = Z_EXCEL_COREPROPERTIES_OBJ_P(object);
	EXCEL_REJECT_RECONSTRUCTION(obj, coreproperties);

	cp = xlBookCoreProperties(book);
	if (!cp) {
		zend_throw_exception(NULL, "Failed to get core properties", 0);
		RETURN_THROWS();
	}

	obj->coreproperties = cp;
	obj->book = book;
	EXCEL_INIT_PARENT(obj, zbook);
}

#define COREPROPERTIES_STRING_GETTER(method_name, api_func) \
EXCEL_METHOD(CoreProperties, method_name) \
{ \
	zval *object = ZEND_THIS; \
	CorePropertiesHandle cp; \
	const char *result; \
	ZEND_PARSE_PARAMETERS_NONE(); \
	COREPROPERTIES_FROM_OBJECT(cp, object); \
	result = api_func(cp); \
	PE_RETURN_IS_STRING(result) \
}

#define COREPROPERTIES_STRING_SETTER(method_name, api_func) \
EXCEL_METHOD(CoreProperties, method_name) \
{ \
	zval *object = ZEND_THIS; \
	CorePropertiesHandle cp; \
	zend_string *val; \
	if (zend_parse_parameters(ZEND_NUM_ARGS(), "S", &val) == FAILURE) { \
		RETURN_FALSE; \
	} \
	EXCEL_NUL_SAFE_STRING(val) \
	COREPROPERTIES_FROM_OBJECT(cp, object); \
	api_func(cp, ZSTR_VAL(val)); \
	RETURN_TRUE; \
}

COREPROPERTIES_STRING_GETTER(title, xlCorePropertiesTitle)
COREPROPERTIES_STRING_SETTER(setTitle, xlCorePropertiesSetTitle)
COREPROPERTIES_STRING_GETTER(subject, xlCorePropertiesSubject)
COREPROPERTIES_STRING_SETTER(setSubject, xlCorePropertiesSetSubject)
COREPROPERTIES_STRING_GETTER(creator, xlCorePropertiesCreator)
COREPROPERTIES_STRING_SETTER(setCreator, xlCorePropertiesSetCreator)
COREPROPERTIES_STRING_GETTER(lastModifiedBy, xlCorePropertiesLastModifiedBy)
COREPROPERTIES_STRING_SETTER(setLastModifiedBy, xlCorePropertiesSetLastModifiedBy)
COREPROPERTIES_STRING_GETTER(created, xlCorePropertiesCreated)
COREPROPERTIES_STRING_SETTER(setCreated, xlCorePropertiesSetCreated)
COREPROPERTIES_STRING_GETTER(modified, xlCorePropertiesModified)
COREPROPERTIES_STRING_SETTER(setModified, xlCorePropertiesSetModified)
COREPROPERTIES_STRING_GETTER(tags, xlCorePropertiesTags)
COREPROPERTIES_STRING_SETTER(setTags, xlCorePropertiesSetTags)
COREPROPERTIES_STRING_GETTER(categories, xlCorePropertiesCategories)
COREPROPERTIES_STRING_SETTER(setCategories, xlCorePropertiesSetCategories)
COREPROPERTIES_STRING_GETTER(comments, xlCorePropertiesComments)
COREPROPERTIES_STRING_SETTER(setComments, xlCorePropertiesSetComments)

EXCEL_METHOD(CoreProperties, createdAsDouble)
{
	zval *object = ZEND_THIS;
	CorePropertiesHandle cp;
	ZEND_PARSE_PARAMETERS_NONE();

	COREPROPERTIES_FROM_OBJECT(cp, object);
	RETURN_DOUBLE(xlCorePropertiesCreatedAsDouble(cp));
}

EXCEL_METHOD(CoreProperties, setCreatedAsDouble)
{
	zval *object = ZEND_THIS;
	CorePropertiesHandle cp;
	double val;
	if (zend_parse_parameters(ZEND_NUM_ARGS(), "d", &val) == FAILURE) {
		RETURN_FALSE;
	}
	EXCEL_VALIDATE_FINITE(val)
	COREPROPERTIES_FROM_OBJECT(cp, object);
	xlCorePropertiesSetCreatedAsDouble(cp, val);
	RETURN_TRUE;
}

EXCEL_METHOD(CoreProperties, modifiedAsDouble)
{
	zval *object = ZEND_THIS;
	CorePropertiesHandle cp;
	ZEND_PARSE_PARAMETERS_NONE();

	COREPROPERTIES_FROM_OBJECT(cp, object);
	RETURN_DOUBLE(xlCorePropertiesModifiedAsDouble(cp));
}

EXCEL_METHOD(CoreProperties, setModifiedAsDouble)
{
	zval *object = ZEND_THIS;
	CorePropertiesHandle cp;
	double val;
	if (zend_parse_parameters(ZEND_NUM_ARGS(), "d", &val) == FAILURE) {
		RETURN_FALSE;
	}
	EXCEL_VALIDATE_FINITE(val)
	COREPROPERTIES_FROM_OBJECT(cp, object);
	xlCorePropertiesSetModifiedAsDouble(cp, val);
	RETURN_TRUE;
}

EXCEL_METHOD(CoreProperties, removeAll)
{
	zval *object = ZEND_THIS;
	CorePropertiesHandle cp;
	ZEND_PARSE_PARAMETERS_NONE();

	COREPROPERTIES_FROM_OBJECT(cp, object);
	xlCorePropertiesRemoveAll(cp);
	RETURN_TRUE;
}

/* Table methods */

EXCEL_METHOD(Table, __construct)
{
	SheetHandle sheet;
	TableHandle th;
	zval *object = ZEND_THIS;
	excel_table_object *obj;
	zval *zsheet;
	zend_string *name;
	zend_long rowFirst, rowLast, colFirst, colLast;
	bool hasHeaders = 1;
	zend_long style = 0;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "OSllll|bl", &zsheet, excel_ce_sheet, &name, &rowFirst, &rowLast, &colFirst, &colLast, &hasHeaders, &style) == FAILURE) {
		return;
	}

	if (!name || ZSTR_LEN(name) < 1) {
		zend_throw_exception(NULL, "Table name cannot be empty", 0);
		RETURN_THROWS();
	}
	if (ZSTR_LEN(name) != strlen(ZSTR_VAL(name))) {
		zend_throw_exception(NULL, "Table name must not contain NUL bytes", 0);
		RETURN_THROWS();
	}
	{
		excel_book_object *_vb = php_excel_resolve_book_obj(zsheet);
		zend_long _maxr = (_vb && _vb->is_xlsx) ? EXCEL_MAX_ROW_XLSX : EXCEL_MAX_ROW_XLS;
		zend_long _maxc = (_vb && _vb->is_xlsx) ? EXCEL_MAX_COL_XLSX : EXCEL_MAX_COL_XLS;
		if (rowFirst < 0 || rowFirst > _maxr || rowLast < 0 || rowLast > _maxr) {
			zend_throw_exception_ex(NULL, 0,
				"Invalid row range: first=" ZEND_LONG_FMT ", last=" ZEND_LONG_FMT,
				rowFirst, rowLast);
			RETURN_THROWS();
		}
		if (colFirst < 0 || colFirst > _maxc || colLast < 0 || colLast > _maxc) {
			zend_throw_exception_ex(NULL, 0,
				"Invalid column range: first=" ZEND_LONG_FMT ", last=" ZEND_LONG_FMT,
				colFirst, colLast);
			RETURN_THROWS();
		}
		if (rowFirst > rowLast) {
			zend_throw_exception(NULL, "Table row start cannot be greater than row end", 0);
			RETURN_THROWS();
		}
		if (colFirst > colLast) {
			zend_throw_exception(NULL, "Table column start cannot be greater than column end", 0);
			RETURN_THROWS();
		}
	}

	if (style < 0 || style > INT_MAX) {
		zend_throw_exception(NULL, "Table style out of int range", 0);
		RETURN_THROWS();
	}

	SHEET_FROM_OBJECT_THROW(sheet, zsheet);

	obj = Z_EXCEL_TABLE_OBJ_P(object);
	EXCEL_REJECT_RECONSTRUCTION(obj, table);

	th = xlSheetAddTable(sheet, ZSTR_VAL(name), rowFirst, rowLast, colFirst, colLast, hasHeaders, style);
	if (!th) {
		zend_throw_exception(NULL, "Failed to create table", 0);
		RETURN_THROWS();
	}

	obj->table = th;
	obj->sheet = sheet;
	EXCEL_INIT_SHEET_PARENT(obj, zsheet);
}


#define PHP_EXCEL_TABLE_INFO(func_name, type) \
{ \
	zval *object = ZEND_THIS; \
	TableHandle table; \
	ZEND_PARSE_PARAMETERS_NONE(); \
	TABLE_FROM_OBJECT(table, object); \
	PE_RETURN_ ## type (xlTable ## func_name (table)); \
}

#define PHP_EXCEL_TABLE_SET_BOOL(func_name) \
{ \
	zval *object = ZEND_THIS; \
	TableHandle table; \
	bool val; \
	if (zend_parse_parameters(ZEND_NUM_ARGS(), "b", &val) == FAILURE) { \
		RETURN_FALSE; \
	} \
	TABLE_FROM_OBJECT(table, object); \
	xlTable ## func_name (table, val); \
	RETURN_TRUE; \
}

#define PHP_EXCEL_TABLE_SET_LONG(func_name) \
{ \
	zval *object = ZEND_THIS; \
	TableHandle table; \
	zend_long val; \
	if (zend_parse_parameters(ZEND_NUM_ARGS(), "l", &val) == FAILURE) { \
		RETURN_FALSE; \
	} \
	EXCEL_VALIDATE_INT_RANGE(val) \
	TABLE_FROM_OBJECT(table, object); \
	xlTable ## func_name (table, val); \
	RETURN_TRUE; \
}

EXCEL_METHOD(Table, name)
{
	zval *object = ZEND_THIS;
	TableHandle table;
	const char *result;
	ZEND_PARSE_PARAMETERS_NONE();

	TABLE_FROM_OBJECT(table, object);
	result = xlTableName(table);
	PE_RETURN_IS_STRING(result)
}

EXCEL_METHOD(Table, setName)
{
	zval *object = ZEND_THIS;
	TableHandle table;
	zend_string *val;
	if (zend_parse_parameters(ZEND_NUM_ARGS(), "S", &val) == FAILURE) {
		RETURN_FALSE;
	}
	EXCEL_NON_EMPTY_STRING(val)
	EXCEL_NUL_SAFE_STRING(val)
	TABLE_FROM_OBJECT(table, object);
	xlTableSetName(table, ZSTR_VAL(val));
	RETURN_TRUE;
}

EXCEL_METHOD(Table, ref)
{
	zval *object = ZEND_THIS;
	TableHandle table;
	const char *result;
	ZEND_PARSE_PARAMETERS_NONE();

	TABLE_FROM_OBJECT(table, object);
	result = xlTableRef(table);
	PE_RETURN_IS_STRING(result)
}

EXCEL_METHOD(Table, setRef)
{
	zval *object = ZEND_THIS;
	TableHandle table;
	zend_string *val;
	if (zend_parse_parameters(ZEND_NUM_ARGS(), "S", &val) == FAILURE) {
		RETURN_FALSE;
	}
	EXCEL_NON_EMPTY_STRING(val)
	EXCEL_NUL_SAFE_STRING(val)
	TABLE_FROM_OBJECT(table, object);
	xlTableSetRef(table, ZSTR_VAL(val));
	RETURN_TRUE;
}

EXCEL_METHOD(Table, autoFilter)
{
	zval *object = ZEND_THIS;
	TableHandle table;
	AutoFilterHandle afh;
	excel_autofilter_object *aobj;
	excel_table_object *tobj;

	ZEND_PARSE_PARAMETERS_NONE();

	TABLE_FROM_OBJECT(table, object);

	afh = xlTableAutoFilter(table);
	if (!afh) {
		RETURN_FALSE;
	}

	tobj = Z_EXCEL_TABLE_OBJ_P(object);

	ZVAL_OBJ(return_value, excel_object_new_autofilter(excel_ce_autofilter));
	aobj = Z_EXCEL_AUTOFILTER_OBJ_P(return_value);
	aobj->autofilter = afh;
	aobj->sheet = tobj->sheet;
	EXCEL_INIT_AUTOFILTER_PARENT(aobj, object);
}

#if LIBXL_VERSION >= 0x05020000
EXCEL_METHOD(Table, isAutoFilter)
{
	PHP_EXCEL_TABLE_INFO(IsAutoFilter, IS_BOOL)
}

EXCEL_METHOD(Table, removeFilter)
{
	zval *object = ZEND_THIS;
	TableHandle table;
	ZEND_PARSE_PARAMETERS_NONE();

	TABLE_FROM_OBJECT(table, object);
	xlTableRemoveFilter(table);
	php_excel_book_bump_autofilter_generation(object);
	RETURN_TRUE;
}
#endif

EXCEL_METHOD(Table, style)
{
	PHP_EXCEL_TABLE_INFO(Style, IS_LONG)
}

EXCEL_METHOD(Table, setStyle)
{
	PHP_EXCEL_TABLE_SET_LONG(SetStyle)
}

EXCEL_METHOD(Table, showRowStripes)
{
	PHP_EXCEL_TABLE_INFO(ShowRowStripes, IS_BOOL)
}

EXCEL_METHOD(Table, setShowRowStripes)
{
	PHP_EXCEL_TABLE_SET_BOOL(SetShowRowStripes)
}

EXCEL_METHOD(Table, showColumnStripes)
{
	PHP_EXCEL_TABLE_INFO(ShowColumnStripes, IS_BOOL)
}

EXCEL_METHOD(Table, setShowColumnStripes)
{
	PHP_EXCEL_TABLE_SET_BOOL(SetShowColumnStripes)
}

EXCEL_METHOD(Table, showFirstColumn)
{
	PHP_EXCEL_TABLE_INFO(ShowFirstColumn, IS_BOOL)
}

EXCEL_METHOD(Table, setShowFirstColumn)
{
	PHP_EXCEL_TABLE_SET_BOOL(SetShowFirstColumn)
}

EXCEL_METHOD(Table, showLastColumn)
{
	PHP_EXCEL_TABLE_INFO(ShowLastColumn, IS_BOOL)
}

EXCEL_METHOD(Table, setShowLastColumn)
{
	PHP_EXCEL_TABLE_SET_BOOL(SetShowLastColumn)
}

EXCEL_METHOD(Table, columnSize)
{
	PHP_EXCEL_TABLE_INFO(ColumnSize, IS_LONG)
}

EXCEL_METHOD(Table, columnName)
{
	zval *object = ZEND_THIS;
	TableHandle table;
	zend_long index;
	const char *result;
	if (zend_parse_parameters(ZEND_NUM_ARGS(), "l", &index) == FAILURE) {
		RETURN_FALSE;
	}
	EXCEL_VALIDATE_INT_RANGE(index)
	TABLE_FROM_OBJECT(table, object);
	result = xlTableColumnName(table, index);
	PE_RETURN_IS_STRING(result)
}

EXCEL_METHOD(Table, setColumnName)
{
	zval *object = ZEND_THIS;
	TableHandle table;
	zend_long index;
	zend_string *name;
	if (zend_parse_parameters(ZEND_NUM_ARGS(), "lS", &index, &name) == FAILURE) {
		RETURN_FALSE;
	}
	EXCEL_NON_EMPTY_STRING(name)
	EXCEL_NUL_SAFE_STRING(name)
	EXCEL_VALIDATE_INT_RANGE(index)
	TABLE_FROM_OBJECT(table, object);
	RETURN_BOOL(xlTableSetColumnName(table, index, ZSTR_VAL(name)));
}

#include "excel_arginfo.h"

/* {{{ PHP_MINIT_FUNCTION
 */
PHP_MINIT_FUNCTION(excel)
{
	REGISTER_INI_ENTRIES();

	REGISTER_EXCEL_CLASS(Book,			book,			NULL);
	REGISTER_EXCEL_CLASS(Sheet,			sheet,			NULL);
	REGISTER_EXCEL_CLASS(Format,		format,			excel_format_object_clone);
	REGISTER_EXCEL_CLASS(Font,			font,			excel_font_object_clone);
	REGISTER_EXCEL_CLASS(AutoFilter,	autofilter,		NULL);
	REGISTER_EXCEL_CLASS(FilterColumn,	filtercolumn,	NULL);
	REGISTER_EXCEL_CLASS(RichString,	richstring,		NULL);
	REGISTER_EXCEL_CLASS(FormControl,	formcontrol,	NULL);
	REGISTER_EXCEL_CLASS(ConditionalFormat,	conditionalformat,	NULL);
	REGISTER_EXCEL_CLASS(ConditionalFormatting,	conditionalformatting,	NULL);
	REGISTER_EXCEL_CLASS(CoreProperties,	coreproperties,	NULL);
	REGISTER_EXCEL_CLASS(Table,			table,			NULL);

	EXCEL_SET_GC(sheet);
	EXCEL_SET_GC(font);
	EXCEL_SET_GC(format);
	EXCEL_SET_GC(autofilter);
	EXCEL_SET_GC(filtercolumn);
	EXCEL_SET_GC(richstring);
	EXCEL_SET_GC(formcontrol);
	EXCEL_SET_GC(conditionalformat);
	EXCEL_SET_GC(conditionalformatting);
	EXCEL_SET_GC(coreproperties);
	EXCEL_SET_GC(table);

	REGISTER_EXCEL_CLASS_CONST_LONG(font, "NORMAL", SCRIPT_NORMAL);
	REGISTER_EXCEL_CLASS_CONST_LONG(font, "SUBSCRIPT", SCRIPT_SUB);
	REGISTER_EXCEL_CLASS_CONST_LONG(font, "SUPERSCRIPT", SCRIPT_SUPER);

	REGISTER_EXCEL_CLASS_CONST_LONG(font, "UNDERLINE_NONE", UNDERLINE_NONE);
	REGISTER_EXCEL_CLASS_CONST_LONG(font, "UNDERLINE_SINGLE", UNDERLINE_SINGLE);
	REGISTER_EXCEL_CLASS_CONST_LONG(font, "UNDERLINE_DOUBLE", UNDERLINE_DOUBLE);
	REGISTER_EXCEL_CLASS_CONST_LONG(font, "UNDERLINE_SINGLEACC", UNDERLINE_SINGLEACC);
	REGISTER_EXCEL_CLASS_CONST_LONG(font, "UNDERLINE_DOUBLEACC", UNDERLINE_DOUBLEACC);

	REGISTER_EXCEL_CLASS_CONST_LONG(format, "AS_DATE", PHP_EXCEL_DATE);
	REGISTER_EXCEL_CLASS_CONST_LONG(format, "AS_FORMULA", PHP_EXCEL_FORMULA);
	REGISTER_EXCEL_CLASS_CONST_LONG(format, "AS_TEXT", PHP_EXCEL_TEXT);
	REGISTER_EXCEL_CLASS_CONST_LONG(format, "AS_NUMERIC_STRING", PHP_EXCEL_NUMERIC_STRING);

	REGISTER_EXCEL_CLASS_CONST_LONG(format, "COLOR_BLACK", COLOR_BLACK);
	REGISTER_EXCEL_CLASS_CONST_LONG(format, "COLOR_WHITE", COLOR_WHITE);
	REGISTER_EXCEL_CLASS_CONST_LONG(format, "COLOR_RED", COLOR_RED);
	REGISTER_EXCEL_CLASS_CONST_LONG(format, "COLOR_BRIGHTGREEN", COLOR_BRIGHTGREEN);
	REGISTER_EXCEL_CLASS_CONST_LONG(format, "COLOR_BLUE", COLOR_BLUE);
	REGISTER_EXCEL_CLASS_CONST_LONG(format, "COLOR_YELLOW", COLOR_YELLOW);
	REGISTER_EXCEL_CLASS_CONST_LONG(format, "COLOR_PINK", COLOR_PINK);
	REGISTER_EXCEL_CLASS_CONST_LONG(format, "COLOR_TURQUOISE", COLOR_TURQUOISE);
	REGISTER_EXCEL_CLASS_CONST_LONG(format, "COLOR_DARKRED", COLOR_DARKRED);
	REGISTER_EXCEL_CLASS_CONST_LONG(format, "COLOR_GREEN", COLOR_GREEN);
	REGISTER_EXCEL_CLASS_CONST_LONG(format, "COLOR_DARKBLUE", COLOR_DARKBLUE);
	REGISTER_EXCEL_CLASS_CONST_LONG(format, "COLOR_DARKYELLOW", COLOR_DARKYELLOW);
	REGISTER_EXCEL_CLASS_CONST_LONG(format, "COLOR_VIOLET", COLOR_VIOLET);
	REGISTER_EXCEL_CLASS_CONST_LONG(format, "COLOR_TEAL", COLOR_TEAL);
	REGISTER_EXCEL_CLASS_CONST_LONG(format, "COLOR_GRAY25", COLOR_GRAY25);
	REGISTER_EXCEL_CLASS_CONST_LONG(format, "COLOR_GRAY50", COLOR_GRAY50);
	REGISTER_EXCEL_CLASS_CONST_LONG(format, "COLOR_PERIWINKLE_CF", COLOR_PERIWINKLE_CF);
	REGISTER_EXCEL_CLASS_CONST_LONG(format, "COLOR_PLUM_CF", COLOR_PLUM_CF);
	REGISTER_EXCEL_CLASS_CONST_LONG(format, "COLOR_IVORY_CF", COLOR_IVORY_CF);
	REGISTER_EXCEL_CLASS_CONST_LONG(format, "COLOR_LIGHTTURQUOISE_CF", COLOR_LIGHTTURQUOISE_CF);
	REGISTER_EXCEL_CLASS_CONST_LONG(format, "COLOR_DARKPURPLE_CF", COLOR_DARKPURPLE_CF);
	REGISTER_EXCEL_CLASS_CONST_LONG(format, "COLOR_CORAL_CF", COLOR_CORAL_CF);
	REGISTER_EXCEL_CLASS_CONST_LONG(format, "COLOR_OCEANBLUE_CF", COLOR_OCEANBLUE_CF);
	REGISTER_EXCEL_CLASS_CONST_LONG(format, "COLOR_ICEBLUE_CF", COLOR_ICEBLUE_CF);
	REGISTER_EXCEL_CLASS_CONST_LONG(format, "COLOR_DARKBLUE_CL", COLOR_DARKBLUE_CL);
	REGISTER_EXCEL_CLASS_CONST_LONG(format, "COLOR_PINK_CL", COLOR_PINK_CL);
	REGISTER_EXCEL_CLASS_CONST_LONG(format, "COLOR_YELLOW_CL", COLOR_YELLOW_CL);
	REGISTER_EXCEL_CLASS_CONST_LONG(format, "COLOR_TURQUOISE_CL", COLOR_TURQUOISE_CL);
	REGISTER_EXCEL_CLASS_CONST_LONG(format, "COLOR_VIOLET_CL", COLOR_VIOLET_CL);
	REGISTER_EXCEL_CLASS_CONST_LONG(format, "COLOR_DARKRED_CL", COLOR_DARKRED_CL);
	REGISTER_EXCEL_CLASS_CONST_LONG(format, "COLOR_TEAL_CL", COLOR_TEAL_CL);
	REGISTER_EXCEL_CLASS_CONST_LONG(format, "COLOR_BLUE_CL", COLOR_BLUE_CL);
	REGISTER_EXCEL_CLASS_CONST_LONG(format, "COLOR_SKYBLUE", COLOR_SKYBLUE);
	REGISTER_EXCEL_CLASS_CONST_LONG(format, "COLOR_LIGHTTURQUOISE", COLOR_LIGHTTURQUOISE);
	REGISTER_EXCEL_CLASS_CONST_LONG(format, "COLOR_LIGHTGREEN", COLOR_LIGHTGREEN);
	REGISTER_EXCEL_CLASS_CONST_LONG(format, "COLOR_LIGHTYELLOW", COLOR_LIGHTYELLOW);
	REGISTER_EXCEL_CLASS_CONST_LONG(format, "COLOR_PALEBLUE", COLOR_PALEBLUE);
	REGISTER_EXCEL_CLASS_CONST_LONG(format, "COLOR_ROSE", COLOR_ROSE);
	REGISTER_EXCEL_CLASS_CONST_LONG(format, "COLOR_LAVENDER", COLOR_LAVENDER);
	REGISTER_EXCEL_CLASS_CONST_LONG(format, "COLOR_TAN", COLOR_TAN);
	REGISTER_EXCEL_CLASS_CONST_LONG(format, "COLOR_LIGHTBLUE", COLOR_LIGHTBLUE);
	REGISTER_EXCEL_CLASS_CONST_LONG(format, "COLOR_AQUA", COLOR_AQUA);
	REGISTER_EXCEL_CLASS_CONST_LONG(format, "COLOR_LIME", COLOR_LIME);
	REGISTER_EXCEL_CLASS_CONST_LONG(format, "COLOR_GOLD", COLOR_GOLD);
	REGISTER_EXCEL_CLASS_CONST_LONG(format, "COLOR_LIGHTORANGE", COLOR_LIGHTORANGE);
	REGISTER_EXCEL_CLASS_CONST_LONG(format, "COLOR_ORANGE", COLOR_ORANGE);
	REGISTER_EXCEL_CLASS_CONST_LONG(format, "COLOR_BLUEGRAY", COLOR_BLUEGRAY);
	REGISTER_EXCEL_CLASS_CONST_LONG(format, "COLOR_GRAY40", COLOR_GRAY40);
	REGISTER_EXCEL_CLASS_CONST_LONG(format, "COLOR_DARKTEAL", COLOR_DARKTEAL);
	REGISTER_EXCEL_CLASS_CONST_LONG(format, "COLOR_SEAGREEN", COLOR_SEAGREEN);
	REGISTER_EXCEL_CLASS_CONST_LONG(format, "COLOR_DARKGREEN", COLOR_DARKGREEN);
	REGISTER_EXCEL_CLASS_CONST_LONG(format, "COLOR_OLIVEGREEN", COLOR_OLIVEGREEN);
	REGISTER_EXCEL_CLASS_CONST_LONG(format, "COLOR_BROWN", COLOR_BROWN);
	REGISTER_EXCEL_CLASS_CONST_LONG(format, "COLOR_PLUM", COLOR_PLUM);
	REGISTER_EXCEL_CLASS_CONST_LONG(format, "COLOR_INDIGO", COLOR_INDIGO);
	REGISTER_EXCEL_CLASS_CONST_LONG(format, "COLOR_GRAY80", COLOR_GRAY80);
	REGISTER_EXCEL_CLASS_CONST_LONG(format, "COLOR_DEFAULT_FOREGROUND", COLOR_DEFAULT_FOREGROUND);
	REGISTER_EXCEL_CLASS_CONST_LONG(format, "COLOR_DEFAULT_BACKGROUND", COLOR_DEFAULT_BACKGROUND);

	REGISTER_EXCEL_CLASS_CONST_LONG(format, "NUMFORMAT_GENERAL", NUMFORMAT_GENERAL);
	REGISTER_EXCEL_CLASS_CONST_LONG(format, "NUMFORMAT_NUMBER", NUMFORMAT_NUMBER);
	REGISTER_EXCEL_CLASS_CONST_LONG(format, "NUMFORMAT_NUMBER_D2", NUMFORMAT_NUMBER_D2);
	REGISTER_EXCEL_CLASS_CONST_LONG(format, "NUMFORMAT_NUMBER_SEP", NUMFORMAT_NUMBER_SEP);
	REGISTER_EXCEL_CLASS_CONST_LONG(format, "NUMFORMAT_NUMBER_SEP_D2", NUMFORMAT_NUMBER_SEP_D2);
	REGISTER_EXCEL_CLASS_CONST_LONG(format, "NUMFORMAT_CURRENCY_NEGBRA", NUMFORMAT_CURRENCY_NEGBRA);
	REGISTER_EXCEL_CLASS_CONST_LONG(format, "NUMFORMAT_CURRENCY_NEGBRARED", NUMFORMAT_CURRENCY_NEGBRARED);
	REGISTER_EXCEL_CLASS_CONST_LONG(format, "NUMFORMAT_CURRENCY_D2_NEGBRA", NUMFORMAT_CURRENCY_D2_NEGBRA);
	REGISTER_EXCEL_CLASS_CONST_LONG(format, "NUMFORMAT_CURRENCY_D2_NEGBRARED", NUMFORMAT_CURRENCY_D2_NEGBRARED);
	REGISTER_EXCEL_CLASS_CONST_LONG(format, "NUMFORMAT_PERCENT", NUMFORMAT_PERCENT);
	REGISTER_EXCEL_CLASS_CONST_LONG(format, "NUMFORMAT_PERCENT_D2", NUMFORMAT_PERCENT_D2);
	REGISTER_EXCEL_CLASS_CONST_LONG(format, "NUMFORMAT_SCIENTIFIC_D2", NUMFORMAT_SCIENTIFIC_D2);
	REGISTER_EXCEL_CLASS_CONST_LONG(format, "NUMFORMAT_FRACTION_ONEDIG", NUMFORMAT_FRACTION_ONEDIG);
	REGISTER_EXCEL_CLASS_CONST_LONG(format, "NUMFORMAT_FRACTION_TWODIG", NUMFORMAT_FRACTION_TWODIG);
	REGISTER_EXCEL_CLASS_CONST_LONG(format, "NUMFORMAT_DATE", NUMFORMAT_DATE);
	REGISTER_EXCEL_CLASS_CONST_LONG(format, "NUMFORMAT_CUSTOM_D_MON_YY", NUMFORMAT_CUSTOM_D_MON_YY);
	REGISTER_EXCEL_CLASS_CONST_LONG(format, "NUMFORMAT_CUSTOM_D_MON", NUMFORMAT_CUSTOM_D_MON);
	REGISTER_EXCEL_CLASS_CONST_LONG(format, "NUMFORMAT_CUSTOM_MON_YY", NUMFORMAT_CUSTOM_MON_YY);
	REGISTER_EXCEL_CLASS_CONST_LONG(format, "NUMFORMAT_CUSTOM_HMM_AM", NUMFORMAT_CUSTOM_HMM_AM);
	REGISTER_EXCEL_CLASS_CONST_LONG(format, "NUMFORMAT_CUSTOM_HMMSS_AM", NUMFORMAT_CUSTOM_HMMSS_AM);
	REGISTER_EXCEL_CLASS_CONST_LONG(format, "NUMFORMAT_CUSTOM_HMM", NUMFORMAT_CUSTOM_HMM);
	REGISTER_EXCEL_CLASS_CONST_LONG(format, "NUMFORMAT_CUSTOM_HMMSS", NUMFORMAT_CUSTOM_HMMSS);
	REGISTER_EXCEL_CLASS_CONST_LONG(format, "NUMFORMAT_CUSTOM_MDYYYY_HMM", NUMFORMAT_CUSTOM_MDYYYY_HMM);
	REGISTER_EXCEL_CLASS_CONST_LONG(format, "NUMFORMAT_NUMBER_SEP_NEGBRA", NUMFORMAT_NUMBER_SEP_NEGBRA);
	REGISTER_EXCEL_CLASS_CONST_LONG(format, "NUMFORMAT_NUMBER_SEP_NEGBRARED", NUMFORMAT_NUMBER_SEP_NEGBRARED);
	REGISTER_EXCEL_CLASS_CONST_LONG(format, "NUMFORMAT_NUMBER_D2_SEP_NEGBRA", NUMFORMAT_NUMBER_D2_SEP_NEGBRA);
	REGISTER_EXCEL_CLASS_CONST_LONG(format, "NUMFORMAT_NUMBER_D2_SEP_NEGBRARED", NUMFORMAT_NUMBER_D2_SEP_NEGBRARED);
	REGISTER_EXCEL_CLASS_CONST_LONG(format, "NUMFORMAT_ACCOUNT", NUMFORMAT_ACCOUNT);
	REGISTER_EXCEL_CLASS_CONST_LONG(format, "NUMFORMAT_ACCOUNTCUR", NUMFORMAT_ACCOUNTCUR);
	REGISTER_EXCEL_CLASS_CONST_LONG(format, "NUMFORMAT_ACCOUNT_D2", NUMFORMAT_ACCOUNT_D2);
	REGISTER_EXCEL_CLASS_CONST_LONG(format, "NUMFORMAT_ACCOUNT_D2_CUR", NUMFORMAT_ACCOUNT_D2_CUR);
	REGISTER_EXCEL_CLASS_CONST_LONG(format, "NUMFORMAT_CUSTOM_MMSS", NUMFORMAT_CUSTOM_MMSS);
	REGISTER_EXCEL_CLASS_CONST_LONG(format, "NUMFORMAT_CUSTOM_H0MMSS", NUMFORMAT_CUSTOM_H0MMSS);
	REGISTER_EXCEL_CLASS_CONST_LONG(format, "NUMFORMAT_CUSTOM_MMSS0", NUMFORMAT_CUSTOM_MMSS0);
	REGISTER_EXCEL_CLASS_CONST_LONG(format, "NUMFORMAT_CUSTOM_000P0E_PLUS0", NUMFORMAT_CUSTOM_000P0E_PLUS0);
	REGISTER_EXCEL_CLASS_CONST_LONG(format, "NUMFORMAT_TEXT", NUMFORMAT_TEXT);

	REGISTER_EXCEL_CLASS_CONST_LONG(format, "ALIGNH_GENERAL", ALIGNH_GENERAL);
	REGISTER_EXCEL_CLASS_CONST_LONG(format, "ALIGNH_LEFT", ALIGNH_LEFT);
	REGISTER_EXCEL_CLASS_CONST_LONG(format, "ALIGNH_CENTER", ALIGNH_CENTER);
	REGISTER_EXCEL_CLASS_CONST_LONG(format, "ALIGNH_RIGHT", ALIGNH_RIGHT);
	REGISTER_EXCEL_CLASS_CONST_LONG(format, "ALIGNH_FILL", ALIGNH_FILL);
	REGISTER_EXCEL_CLASS_CONST_LONG(format, "ALIGNH_JUSTIFY", ALIGNH_JUSTIFY);
	REGISTER_EXCEL_CLASS_CONST_LONG(format, "ALIGNH_MERGE", ALIGNH_MERGE);
	REGISTER_EXCEL_CLASS_CONST_LONG(format, "ALIGNH_DISTRIBUTED", ALIGNH_DISTRIBUTED);
	REGISTER_EXCEL_CLASS_CONST_LONG(format, "ALIGNV_TOP", ALIGNV_TOP);
	REGISTER_EXCEL_CLASS_CONST_LONG(format, "ALIGNV_CENTER", ALIGNV_CENTER);
	REGISTER_EXCEL_CLASS_CONST_LONG(format, "ALIGNV_BOTTOM", ALIGNV_BOTTOM);
	REGISTER_EXCEL_CLASS_CONST_LONG(format, "ALIGNV_JUSTIFY", ALIGNV_JUSTIFY);
	REGISTER_EXCEL_CLASS_CONST_LONG(format, "ALIGNV_DISTRIBUTED", ALIGNV_DISTRIBUTED);

	REGISTER_EXCEL_CLASS_CONST_LONG(format, "BORDERSTYLE_NONE", BORDERSTYLE_NONE);
	REGISTER_EXCEL_CLASS_CONST_LONG(format, "BORDERSTYLE_THIN", BORDERSTYLE_THIN);
	REGISTER_EXCEL_CLASS_CONST_LONG(format, "BORDERSTYLE_MEDIUM", BORDERSTYLE_MEDIUM);
	REGISTER_EXCEL_CLASS_CONST_LONG(format, "BORDERSTYLE_DASHED", BORDERSTYLE_DASHED);
	REGISTER_EXCEL_CLASS_CONST_LONG(format, "BORDERSTYLE_DOTTED", BORDERSTYLE_DOTTED);
	REGISTER_EXCEL_CLASS_CONST_LONG(format, "BORDERSTYLE_THICK", BORDERSTYLE_THICK);
	REGISTER_EXCEL_CLASS_CONST_LONG(format, "BORDERSTYLE_DOUBLE", BORDERSTYLE_DOUBLE);
	REGISTER_EXCEL_CLASS_CONST_LONG(format, "BORDERSTYLE_HAIR", BORDERSTYLE_HAIR);
	REGISTER_EXCEL_CLASS_CONST_LONG(format, "BORDERSTYLE_MEDIUMDASHED", BORDERSTYLE_MEDIUMDASHED);
	REGISTER_EXCEL_CLASS_CONST_LONG(format, "BORDERSTYLE_DASHDOT", BORDERSTYLE_DASHDOT);
	REGISTER_EXCEL_CLASS_CONST_LONG(format, "BORDERSTYLE_MEDIUMDASHDOT", BORDERSTYLE_MEDIUMDASHDOT);
	REGISTER_EXCEL_CLASS_CONST_LONG(format, "BORDERSTYLE_DASHDOTDOT", BORDERSTYLE_DASHDOTDOT);
	REGISTER_EXCEL_CLASS_CONST_LONG(format, "BORDERSTYLE_MEDIUMDASHDOTDOT", BORDERSTYLE_MEDIUMDASHDOTDOT);
	REGISTER_EXCEL_CLASS_CONST_LONG(format, "BORDERSTYLE_SLANTDASHDOT", BORDERSTYLE_SLANTDASHDOT);

	REGISTER_EXCEL_CLASS_CONST_LONG(format, "BORDERDIAGONAL_NONE", BORDERDIAGONAL_NONE);
	REGISTER_EXCEL_CLASS_CONST_LONG(format, "BORDERDIAGONAL_DOWN", BORDERDIAGONAL_DOWN);
	REGISTER_EXCEL_CLASS_CONST_LONG(format, "BORDERDIAGONAL_UP", BORDERDIAGONAL_UP);
	REGISTER_EXCEL_CLASS_CONST_LONG(format, "BORDERDIAGONAL_BOTH", BORDERDIAGONAL_BOTH);

	REGISTER_EXCEL_CLASS_CONST_LONG(format, "FILLPATTERN_NONE", FILLPATTERN_NONE);
	REGISTER_EXCEL_CLASS_CONST_LONG(format, "FILLPATTERN_SOLID", FILLPATTERN_SOLID);
	REGISTER_EXCEL_CLASS_CONST_LONG(format, "FILLPATTERN_GRAY50", FILLPATTERN_GRAY50);
	REGISTER_EXCEL_CLASS_CONST_LONG(format, "FILLPATTERN_GRAY75", FILLPATTERN_GRAY75);
	REGISTER_EXCEL_CLASS_CONST_LONG(format, "FILLPATTERN_GRAY25", FILLPATTERN_GRAY25);
	REGISTER_EXCEL_CLASS_CONST_LONG(format, "FILLPATTERN_HORSTRIPE", FILLPATTERN_HORSTRIPE);
	REGISTER_EXCEL_CLASS_CONST_LONG(format, "FILLPATTERN_VERSTRIPE", FILLPATTERN_VERSTRIPE);
	REGISTER_EXCEL_CLASS_CONST_LONG(format, "FILLPATTERN_REVDIAGSTRIPE", FILLPATTERN_REVDIAGSTRIPE);
	REGISTER_EXCEL_CLASS_CONST_LONG(format, "FILLPATTERN_DIAGSTRIPE", FILLPATTERN_DIAGSTRIPE);
	REGISTER_EXCEL_CLASS_CONST_LONG(format, "FILLPATTERN_DIAGCROSSHATCH", FILLPATTERN_DIAGCROSSHATCH);
	REGISTER_EXCEL_CLASS_CONST_LONG(format, "FILLPATTERN_THICKDIAGCROSSHATCH", FILLPATTERN_THICKDIAGCROSSHATCH);
	REGISTER_EXCEL_CLASS_CONST_LONG(format, "FILLPATTERN_THINHORSTRIPE", FILLPATTERN_THINHORSTRIPE);
	REGISTER_EXCEL_CLASS_CONST_LONG(format, "FILLPATTERN_THINVERSTRIPE", FILLPATTERN_THINVERSTRIPE);
	REGISTER_EXCEL_CLASS_CONST_LONG(format, "FILLPATTERN_THINREVDIAGSTRIPE", FILLPATTERN_THINREVDIAGSTRIPE);
	REGISTER_EXCEL_CLASS_CONST_LONG(format, "FILLPATTERN_THINDIAGSTRIPE", FILLPATTERN_THINDIAGSTRIPE);
	REGISTER_EXCEL_CLASS_CONST_LONG(format, "FILLPATTERN_THINHORCROSSHATCH", FILLPATTERN_THINHORCROSSHATCH);
	REGISTER_EXCEL_CLASS_CONST_LONG(format, "FILLPATTERN_THINDIAGCROSSHATCH", FILLPATTERN_THINDIAGCROSSHATCH);
	REGISTER_EXCEL_CLASS_CONST_LONG(format, "FILLPATTERN_GRAY12P5", FILLPATTERN_GRAY12P5);
	REGISTER_EXCEL_CLASS_CONST_LONG(format, "FILLPATTERN_GRAY6P25", FILLPATTERN_GRAY6P25);

	REGISTER_EXCEL_CLASS_CONST_LONG(sheet, "CELLTYPE_EMPTY", CELLTYPE_EMPTY);
	REGISTER_EXCEL_CLASS_CONST_LONG(sheet, "CELLTYPE_NUMBER", CELLTYPE_NUMBER);
	REGISTER_EXCEL_CLASS_CONST_LONG(sheet, "CELLTYPE_STRING", CELLTYPE_STRING);
	REGISTER_EXCEL_CLASS_CONST_LONG(sheet, "CELLTYPE_BOOLEAN", CELLTYPE_BOOLEAN);
	REGISTER_EXCEL_CLASS_CONST_LONG(sheet, "CELLTYPE_BLANK", CELLTYPE_BLANK);
	REGISTER_EXCEL_CLASS_CONST_LONG(sheet, "CELLTYPE_ERROR", CELLTYPE_ERROR);
	REGISTER_EXCEL_CLASS_CONST_LONG(sheet, "CELLTYPE_STRICTDATE", CELLTYPE_STRICTDATE);

	REGISTER_EXCEL_CLASS_CONST_LONG(sheet, "ERRORTYPE_NA", ERRORTYPE_NA);
	REGISTER_EXCEL_CLASS_CONST_LONG(sheet, "ERRORTYPE_NUM", ERRORTYPE_NUM);
	REGISTER_EXCEL_CLASS_CONST_LONG(sheet, "ERRORTYPE_NAME", ERRORTYPE_NAME);
	REGISTER_EXCEL_CLASS_CONST_LONG(sheet, "ERRORTYPE_REF", ERRORTYPE_REF);
	REGISTER_EXCEL_CLASS_CONST_LONG(sheet, "ERRORTYPE_VALUE", ERRORTYPE_VALUE);
	REGISTER_EXCEL_CLASS_CONST_LONG(sheet, "ERRORTYPE_DIV_0", ERRORTYPE_DIV_0);
	REGISTER_EXCEL_CLASS_CONST_LONG(sheet, "ERRORTYPE_NULL", ERRORTYPE_NULL);
	REGISTER_EXCEL_CLASS_CONST_LONG(sheet, "ERRORTYPE_NOERROR", ERRORTYPE_NOERROR);

	REGISTER_EXCEL_CLASS_CONST_LONG(sheet, "PAPER_DEFAULT", PAPER_DEFAULT);
	REGISTER_EXCEL_CLASS_CONST_LONG(sheet, "PAPER_LETTER", PAPER_LETTER);
	REGISTER_EXCEL_CLASS_CONST_LONG(sheet, "PAPER_LETTERSMALL", PAPER_LETTERSMALL);
	REGISTER_EXCEL_CLASS_CONST_LONG(sheet, "PAPER_TABLOID", PAPER_TABLOID);
	REGISTER_EXCEL_CLASS_CONST_LONG(sheet, "PAPER_LEDGER", PAPER_LEDGER);
	REGISTER_EXCEL_CLASS_CONST_LONG(sheet, "PAPER_LEGAL", PAPER_LEGAL);
	REGISTER_EXCEL_CLASS_CONST_LONG(sheet, "PAPER_STATEMENT", PAPER_STATEMENT);
	REGISTER_EXCEL_CLASS_CONST_LONG(sheet, "PAPER_EXECUTIVE", PAPER_EXECUTIVE);
	REGISTER_EXCEL_CLASS_CONST_LONG(sheet, "PAPER_A3", PAPER_A3);
	REGISTER_EXCEL_CLASS_CONST_LONG(sheet, "PAPER_A4", PAPER_A4);
	REGISTER_EXCEL_CLASS_CONST_LONG(sheet, "PAPER_A4SMALL", PAPER_A4SMALL);
	REGISTER_EXCEL_CLASS_CONST_LONG(sheet, "PAPER_A5", PAPER_A5);
	REGISTER_EXCEL_CLASS_CONST_LONG(sheet, "PAPER_B4", PAPER_B4);
	REGISTER_EXCEL_CLASS_CONST_LONG(sheet, "PAPER_B5", PAPER_B5);
	REGISTER_EXCEL_CLASS_CONST_LONG(sheet, "PAPER_FOLIO", PAPER_FOLIO);
	REGISTER_EXCEL_CLASS_CONST_LONG(sheet, "PAPER_QUATRO", PAPER_QUATRO);
	REGISTER_EXCEL_CLASS_CONST_LONG(sheet, "PAPER_10x14", PAPER_10x14);
	REGISTER_EXCEL_CLASS_CONST_LONG(sheet, "PAPER_10x17", PAPER_10x17);
	REGISTER_EXCEL_CLASS_CONST_LONG(sheet, "PAPER_NOTE", PAPER_NOTE);
	REGISTER_EXCEL_CLASS_CONST_LONG(sheet, "PAPER_ENVELOPE_9", PAPER_ENVELOPE_9);
	REGISTER_EXCEL_CLASS_CONST_LONG(sheet, "PAPER_ENVELOPE_10", PAPER_ENVELOPE_10);
	REGISTER_EXCEL_CLASS_CONST_LONG(sheet, "PAPER_ENVELOPE_11", PAPER_ENVELOPE_11);
	REGISTER_EXCEL_CLASS_CONST_LONG(sheet, "PAPER_ENVELOPE_12", PAPER_ENVELOPE_12);
	REGISTER_EXCEL_CLASS_CONST_LONG(sheet, "PAPER_ENVELOPE_14", PAPER_ENVELOPE_14);
	REGISTER_EXCEL_CLASS_CONST_LONG(sheet, "PAPER_C_SIZE", PAPER_C_SIZE);
	REGISTER_EXCEL_CLASS_CONST_LONG(sheet, "PAPER_D_SIZE", PAPER_D_SIZE);
	REGISTER_EXCEL_CLASS_CONST_LONG(sheet, "PAPER_E_SIZE", PAPER_E_SIZE);
	REGISTER_EXCEL_CLASS_CONST_LONG(sheet, "PAPER_ENVELOPE_DL", PAPER_ENVELOPE_DL);
	REGISTER_EXCEL_CLASS_CONST_LONG(sheet, "PAPER_ENVELOPE_C5", PAPER_ENVELOPE_C5);
	REGISTER_EXCEL_CLASS_CONST_LONG(sheet, "PAPER_ENVELOPE_C3", PAPER_ENVELOPE_C3);
	REGISTER_EXCEL_CLASS_CONST_LONG(sheet, "PAPER_ENVELOPE_C4", PAPER_ENVELOPE_C4);
	REGISTER_EXCEL_CLASS_CONST_LONG(sheet, "PAPER_ENVELOPE_C6", PAPER_ENVELOPE_C6);
	REGISTER_EXCEL_CLASS_CONST_LONG(sheet, "PAPER_ENVELOPE_C65", PAPER_ENVELOPE_C65);
	REGISTER_EXCEL_CLASS_CONST_LONG(sheet, "PAPER_ENVELOPE_B4", PAPER_ENVELOPE_B4);
	REGISTER_EXCEL_CLASS_CONST_LONG(sheet, "PAPER_ENVELOPE_B5", PAPER_ENVELOPE_B5);
	REGISTER_EXCEL_CLASS_CONST_LONG(sheet, "PAPER_ENVELOPE_B6", PAPER_ENVELOPE_B6);
	REGISTER_EXCEL_CLASS_CONST_LONG(sheet, "PAPER_ENVELOPE", PAPER_ENVELOPE);
	REGISTER_EXCEL_CLASS_CONST_LONG(sheet, "PAPER_ENVELOPE_MONARCH", PAPER_ENVELOPE_MONARCH);
	REGISTER_EXCEL_CLASS_CONST_LONG(sheet, "PAPER_US_ENVELOPE", PAPER_US_ENVELOPE);
	REGISTER_EXCEL_CLASS_CONST_LONG(sheet, "PAPER_FANFOLD", PAPER_FANFOLD);
	REGISTER_EXCEL_CLASS_CONST_LONG(sheet, "PAPER_GERMAN_STD_FANFOLD", PAPER_GERMAN_STD_FANFOLD);
	REGISTER_EXCEL_CLASS_CONST_LONG(sheet, "PAPER_GERMAN_LEGAL_FANFOLD", PAPER_GERMAN_LEGAL_FANFOLD);

	REGISTER_EXCEL_CLASS_CONST_LONG(book, "PICTURETYPE_PNG", PICTURETYPE_PNG);
	REGISTER_EXCEL_CLASS_CONST_LONG(book, "PICTURETYPE_JPEG", PICTURETYPE_JPEG);
	REGISTER_EXCEL_CLASS_CONST_LONG(book, "PICTURETYPE_WMF", PICTURETYPE_WMF);
	REGISTER_EXCEL_CLASS_CONST_LONG(book, "PICTURETYPE_DIB", PICTURETYPE_DIB);
	REGISTER_EXCEL_CLASS_CONST_LONG(book, "PICTURETYPE_EMF", PICTURETYPE_EMF);
	REGISTER_EXCEL_CLASS_CONST_LONG(book, "PICTURETYPE_PICT", PICTURETYPE_PICT);
	REGISTER_EXCEL_CLASS_CONST_LONG(book, "PICTURETYPE_TIFF", PICTURETYPE_TIFF);
	REGISTER_EXCEL_CLASS_CONST_LONG(book, "PICTURETYPE_GIF", PICTURETYPE_GIF);
	REGISTER_EXCEL_CLASS_CONST_LONG(book, "PICTURETYPE_SVG", PICTURETYPE_SVG);

	REGISTER_EXCEL_CLASS_CONST_LONG(book, "SCOPE_UNDEFINED", SCOPE_UNDEFINED);
	REGISTER_EXCEL_CLASS_CONST_LONG(book, "SCOPE_WORKBOOK", SCOPE_WORKBOOK);

	REGISTER_EXCEL_CLASS_CONST_LONG(sheet, "RIGHT_TO_LEFT", 1);
	REGISTER_EXCEL_CLASS_CONST_LONG(sheet, "LEFT_TO_RIGHT", 0);

	REGISTER_EXCEL_CLASS_CONST_LONG(book, "SHEETTYPE_SHEET", SHEETTYPE_SHEET);
	REGISTER_EXCEL_CLASS_CONST_LONG(book, "SHEETTYPE_CHART", SHEETTYPE_CHART);
	REGISTER_EXCEL_CLASS_CONST_LONG(book, "SHEETTYPE_UNKNOWN", SHEETTYPE_UNKNOWN);

	REGISTER_EXCEL_CLASS_CONST_LONG(book, "POSITION_MOVE_AND_SIZE", POSITION_MOVE_AND_SIZE);
	REGISTER_EXCEL_CLASS_CONST_LONG(book, "POSITION_ONLY_MOVE", POSITION_ONLY_MOVE);
	REGISTER_EXCEL_CLASS_CONST_LONG(book, "POSITION_ABSOLUTE", POSITION_ABSOLUTE);

	REGISTER_EXCEL_CLASS_CONST_LONG(sheet, "PROT_DEFAULT", PROT_DEFAULT);
	REGISTER_EXCEL_CLASS_CONST_LONG(sheet, "PROT_ALL", PROT_ALL);
	REGISTER_EXCEL_CLASS_CONST_LONG(sheet, "PROT_OBJECTS", PROT_OBJECTS);
	REGISTER_EXCEL_CLASS_CONST_LONG(sheet, "PROT_SCENARIOS", PROT_SCENARIOS);
	REGISTER_EXCEL_CLASS_CONST_LONG(sheet, "PROT_FORMAT_CELLS", PROT_FORMAT_CELLS);
	REGISTER_EXCEL_CLASS_CONST_LONG(sheet, "PROT_FORMAT_COLUMNS", PROT_FORMAT_COLUMNS);
	REGISTER_EXCEL_CLASS_CONST_LONG(sheet, "PROT_FORMAT_ROWS", PROT_FORMAT_ROWS);
	REGISTER_EXCEL_CLASS_CONST_LONG(sheet, "PROT_INSERT_COLUMNS", PROT_INSERT_COLUMNS);
	REGISTER_EXCEL_CLASS_CONST_LONG(sheet, "PROT_INSERT_ROWS", PROT_INSERT_ROWS);
	REGISTER_EXCEL_CLASS_CONST_LONG(sheet, "PROT_INSERT_HYPERLINKS", PROT_INSERT_HYPERLINKS);
	REGISTER_EXCEL_CLASS_CONST_LONG(sheet, "PROT_DELETE_COLUMNS", PROT_DELETE_COLUMNS);
	REGISTER_EXCEL_CLASS_CONST_LONG(sheet, "PROT_DELETE_ROWS", PROT_DELETE_ROWS);
	REGISTER_EXCEL_CLASS_CONST_LONG(sheet, "PROT_SEL_LOCKED_CELLS", PROT_SEL_LOCKED_CELLS);
	REGISTER_EXCEL_CLASS_CONST_LONG(sheet, "PROT_SORT", PROT_SORT);
	REGISTER_EXCEL_CLASS_CONST_LONG(sheet, "PROT_AUTOFILTER", PROT_AUTOFILTER);
	REGISTER_EXCEL_CLASS_CONST_LONG(sheet, "PROT_PIVOTTABLES", PROT_PIVOTTABLES);
	REGISTER_EXCEL_CLASS_CONST_LONG(sheet, "PROT_SEL_UNLOCKED_CELLS", PROT_SEL_UNLOCKED_CELLS);

	REGISTER_EXCEL_CLASS_CONST_LONG(sheet, "SHEETSTATE_VISIBLE", SHEETSTATE_VISIBLE);
	REGISTER_EXCEL_CLASS_CONST_LONG(sheet, "SHEETSTATE_HIDDEN", SHEETSTATE_HIDDEN);
	REGISTER_EXCEL_CLASS_CONST_LONG(sheet, "SHEETSTATE_VERYHIDDEN", SHEETSTATE_VERYHIDDEN);

	REGISTER_EXCEL_CLASS_CONST_LONG(sheet, "IERR_EVAL_ERROR", IERR_EVAL_ERROR);
	REGISTER_EXCEL_CLASS_CONST_LONG(sheet, "IERR_EMPTY_CELLREF", IERR_EMPTY_CELLREF);
	REGISTER_EXCEL_CLASS_CONST_LONG(sheet, "IERR_NUMBER_STORED_AS_TEXT", IERR_NUMBER_STORED_AS_TEXT);
	REGISTER_EXCEL_CLASS_CONST_LONG(sheet, "IERR_INCONSIST_RANGE", IERR_INCONSIST_RANGE);
	REGISTER_EXCEL_CLASS_CONST_LONG(sheet, "IERR_INCONSIST_FMLA", IERR_INCONSIST_FMLA);
	REGISTER_EXCEL_CLASS_CONST_LONG(sheet, "IERR_TWODIG_TEXTYEAR", IERR_TWODIG_TEXTYEAR);
	REGISTER_EXCEL_CLASS_CONST_LONG(sheet, "IERR_UNLOCK_FMLA", IERR_UNLOCK_FMLA);
	REGISTER_EXCEL_CLASS_CONST_LONG(sheet, "IERR_DATA_VALIDATION", IERR_DATA_VALIDATION);

	REGISTER_EXCEL_CLASS_CONST_LONG(book, "CALCMODE_MANUAL", CALCMODE_MANUAL);
	REGISTER_EXCEL_CLASS_CONST_LONG(book, "CALCMODE_AUTO", CALCMODE_AUTO);
	REGISTER_EXCEL_CLASS_CONST_LONG(book, "CALCMODE_AUTONOTABLE", CALCMODE_AUTONOTABLE);

	REGISTER_EXCEL_CLASS_CONST_LONG(formcontrol, "CHECKEDTYPE_UNCHECKED", CHECKEDTYPE_UNCHECKED);
	REGISTER_EXCEL_CLASS_CONST_LONG(formcontrol, "CHECKEDTYPE_CHECKED", CHECKEDTYPE_CHECKED);
	REGISTER_EXCEL_CLASS_CONST_LONG(formcontrol, "CHECKEDTYPE_MIXED", CHECKEDTYPE_MIXED);

	REGISTER_EXCEL_CLASS_CONST_LONG(formcontrol, "OBJECT_UNKNOWN", OBJECT_UNKNOWN);
	REGISTER_EXCEL_CLASS_CONST_LONG(formcontrol, "OBJECT_BUTTON", OBJECT_BUTTON);
	REGISTER_EXCEL_CLASS_CONST_LONG(formcontrol, "OBJECT_CHECKBOX", OBJECT_CHECKBOX);
	REGISTER_EXCEL_CLASS_CONST_LONG(formcontrol, "OBJECT_DROP", OBJECT_DROP);
	REGISTER_EXCEL_CLASS_CONST_LONG(formcontrol, "OBJECT_GBOX", OBJECT_GBOX);
	REGISTER_EXCEL_CLASS_CONST_LONG(formcontrol, "OBJECT_LABEL", OBJECT_LABEL);
	REGISTER_EXCEL_CLASS_CONST_LONG(formcontrol, "OBJECT_LIST", OBJECT_LIST);
	REGISTER_EXCEL_CLASS_CONST_LONG(formcontrol, "OBJECT_RADIO", OBJECT_RADIO);
	REGISTER_EXCEL_CLASS_CONST_LONG(formcontrol, "OBJECT_SCROLL", OBJECT_SCROLL);
	REGISTER_EXCEL_CLASS_CONST_LONG(formcontrol, "OBJECT_SPIN", OBJECT_SPIN);
	REGISTER_EXCEL_CLASS_CONST_LONG(formcontrol, "OBJECT_EDITBOX", OBJECT_EDITBOX);
	REGISTER_EXCEL_CLASS_CONST_LONG(formcontrol, "OBJECT_DIALOG", OBJECT_DIALOG);

	REGISTER_EXCEL_CLASS_CONST_LONG(conditionalformatting, "CFORMAT_BEGINWITH", CFORMAT_BEGINWITH);
	REGISTER_EXCEL_CLASS_CONST_LONG(conditionalformatting, "CFORMAT_CONTAINSBLANKS", CFORMAT_CONTAINSBLANKS);
	REGISTER_EXCEL_CLASS_CONST_LONG(conditionalformatting, "CFORMAT_CONTAINSERRORS", CFORMAT_CONTAINSERRORS);
	REGISTER_EXCEL_CLASS_CONST_LONG(conditionalformatting, "CFORMAT_CONTAINSTEXT", CFORMAT_CONTAINSTEXT);
	REGISTER_EXCEL_CLASS_CONST_LONG(conditionalformatting, "CFORMAT_DUPLICATEVALUES", CFORMAT_DUPLICATEVALUES);
	REGISTER_EXCEL_CLASS_CONST_LONG(conditionalformatting, "CFORMAT_ENDSWITH", CFORMAT_ENDSWITH);
	REGISTER_EXCEL_CLASS_CONST_LONG(conditionalformatting, "CFORMAT_EXPRESSION", CFORMAT_EXPRESSION);
	REGISTER_EXCEL_CLASS_CONST_LONG(conditionalformatting, "CFORMAT_NOTCONTAINSBLANKS", CFORMAT_NOTCONTAINSBLANKS);
	REGISTER_EXCEL_CLASS_CONST_LONG(conditionalformatting, "CFORMAT_NOTCONTAINSERRORS", CFORMAT_NOTCONTAINSERRORS);
	REGISTER_EXCEL_CLASS_CONST_LONG(conditionalformatting, "CFORMAT_NOTCONTAINSTEXT", CFORMAT_NOTCONTAINSTEXT);
	REGISTER_EXCEL_CLASS_CONST_LONG(conditionalformatting, "CFORMAT_UNIQUEVALUES", CFORMAT_UNIQUEVALUES);

	REGISTER_EXCEL_CLASS_CONST_LONG(conditionalformatting, "CFOPERATOR_LESSTHAN", CFOPERATOR_LESSTHAN);
	REGISTER_EXCEL_CLASS_CONST_LONG(conditionalformatting, "CFOPERATOR_LESSTHANOREQUAL", CFOPERATOR_LESSTHANOREQUAL);
	REGISTER_EXCEL_CLASS_CONST_LONG(conditionalformatting, "CFOPERATOR_EQUAL", CFOPERATOR_EQUAL);
	REGISTER_EXCEL_CLASS_CONST_LONG(conditionalformatting, "CFOPERATOR_NOTEQUAL", CFOPERATOR_NOTEQUAL);
	REGISTER_EXCEL_CLASS_CONST_LONG(conditionalformatting, "CFOPERATOR_GREATERTHANOREQUAL", CFOPERATOR_GREATERTHANOREQUAL);
	REGISTER_EXCEL_CLASS_CONST_LONG(conditionalformatting, "CFOPERATOR_GREATERTHAN", CFOPERATOR_GREATERTHAN);
	REGISTER_EXCEL_CLASS_CONST_LONG(conditionalformatting, "CFOPERATOR_BETWEEN", CFOPERATOR_BETWEEN);
	REGISTER_EXCEL_CLASS_CONST_LONG(conditionalformatting, "CFOPERATOR_NOTBETWEEN", CFOPERATOR_NOTBETWEEN);
	REGISTER_EXCEL_CLASS_CONST_LONG(conditionalformatting, "CFOPERATOR_CONTAINSTEXT", CFOPERATOR_CONTAINSTEXT);
	REGISTER_EXCEL_CLASS_CONST_LONG(conditionalformatting, "CFOPERATOR_NOTCONTAINS", CFOPERATOR_NOTCONTAINS);
	REGISTER_EXCEL_CLASS_CONST_LONG(conditionalformatting, "CFOPERATOR_BEGINSWITH", CFOPERATOR_BEGINSWITH);
	REGISTER_EXCEL_CLASS_CONST_LONG(conditionalformatting, "CFOPERATOR_ENDSWITH", CFOPERATOR_ENDSWITH);

	REGISTER_EXCEL_CLASS_CONST_LONG(conditionalformatting, "CFTP_LAST7DAYS", CFTP_LAST7DAYS);
	REGISTER_EXCEL_CLASS_CONST_LONG(conditionalformatting, "CFTP_LASTMONTH", CFTP_LASTMONTH);
	REGISTER_EXCEL_CLASS_CONST_LONG(conditionalformatting, "CFTP_LASTWEEK", CFTP_LASTWEEK);
	REGISTER_EXCEL_CLASS_CONST_LONG(conditionalformatting, "CFTP_NEXTMONTH", CFTP_NEXTMONTH);
	REGISTER_EXCEL_CLASS_CONST_LONG(conditionalformatting, "CFTP_NEXTWEEK", CFTP_NEXTWEEK);
	REGISTER_EXCEL_CLASS_CONST_LONG(conditionalformatting, "CFTP_THISMONTH", CFTP_THISMONTH);
	REGISTER_EXCEL_CLASS_CONST_LONG(conditionalformatting, "CFTP_THISWEEK", CFTP_THISWEEK);
	REGISTER_EXCEL_CLASS_CONST_LONG(conditionalformatting, "CFTP_TODAY", CFTP_TODAY);
	REGISTER_EXCEL_CLASS_CONST_LONG(conditionalformatting, "CFTP_TOMORROW", CFTP_TOMORROW);
	REGISTER_EXCEL_CLASS_CONST_LONG(conditionalformatting, "CFTP_YESTERDAY", CFTP_YESTERDAY);

	REGISTER_EXCEL_CLASS_CONST_LONG(conditionalformatting, "CFVO_MIN", CFVO_MIN);
	REGISTER_EXCEL_CLASS_CONST_LONG(conditionalformatting, "CFVO_MAX", CFVO_MAX);
	REGISTER_EXCEL_CLASS_CONST_LONG(conditionalformatting, "CFVO_FORMULA", CFVO_FORMULA);
	REGISTER_EXCEL_CLASS_CONST_LONG(conditionalformatting, "CFVO_NUMBER", CFVO_NUMBER);
	REGISTER_EXCEL_CLASS_CONST_LONG(conditionalformatting, "CFVO_PERCENT", CFVO_PERCENT);
	REGISTER_EXCEL_CLASS_CONST_LONG(conditionalformatting, "CFVO_PERCENTILE", CFVO_PERCENTILE);

	REGISTER_EXCEL_CLASS_CONST_LONG(book, "CELLSTYLE_NORMAL", CELLSTYLE_NORMAL);
	REGISTER_EXCEL_CLASS_CONST_LONG(book, "CELLSTYLE_BAD", CELLSTYLE_BAD);
	REGISTER_EXCEL_CLASS_CONST_LONG(book, "CELLSTYLE_GOOD", CELLSTYLE_GOOD);
	REGISTER_EXCEL_CLASS_CONST_LONG(book, "CELLSTYLE_NEUTRAL", CELLSTYLE_NEUTRAL);
	REGISTER_EXCEL_CLASS_CONST_LONG(book, "CELLSTYLE_CALC", CELLSTYLE_CALC);
	REGISTER_EXCEL_CLASS_CONST_LONG(book, "CELLSTYLE_CHECKCELL", CELLSTYLE_CHECKCELL);
	REGISTER_EXCEL_CLASS_CONST_LONG(book, "CELLSTYLE_EXPLANATORY", CELLSTYLE_EXPLANATORY);
	REGISTER_EXCEL_CLASS_CONST_LONG(book, "CELLSTYLE_INPUT", CELLSTYLE_INPUT);
	REGISTER_EXCEL_CLASS_CONST_LONG(book, "CELLSTYLE_OUTPUT", CELLSTYLE_OUTPUT);
	REGISTER_EXCEL_CLASS_CONST_LONG(book, "CELLSTYLE_HYPERLINK", CELLSTYLE_HYPERLINK);
	REGISTER_EXCEL_CLASS_CONST_LONG(book, "CELLSTYLE_LINKEDCELL", CELLSTYLE_LINKEDCELL);
	REGISTER_EXCEL_CLASS_CONST_LONG(book, "CELLSTYLE_NOTE", CELLSTYLE_NOTE);
	REGISTER_EXCEL_CLASS_CONST_LONG(book, "CELLSTYLE_WARNING", CELLSTYLE_WARNING);
	REGISTER_EXCEL_CLASS_CONST_LONG(book, "CELLSTYLE_TITLE", CELLSTYLE_TITLE);
	REGISTER_EXCEL_CLASS_CONST_LONG(book, "CELLSTYLE_HEADING1", CELLSTYLE_HEADING1);
	REGISTER_EXCEL_CLASS_CONST_LONG(book, "CELLSTYLE_HEADING2", CELLSTYLE_HEADING2);
	REGISTER_EXCEL_CLASS_CONST_LONG(book, "CELLSTYLE_HEADING3", CELLSTYLE_HEADING3);
	REGISTER_EXCEL_CLASS_CONST_LONG(book, "CELLSTYLE_HEADING4", CELLSTYLE_HEADING4);
	REGISTER_EXCEL_CLASS_CONST_LONG(book, "CELLSTYLE_TOTAL", CELLSTYLE_TOTAL);
	REGISTER_EXCEL_CLASS_CONST_LONG(book, "CELLSTYLE_COMMA", CELLSTYLE_COMMA);
	REGISTER_EXCEL_CLASS_CONST_LONG(book, "CELLSTYLE_COMMA0", CELLSTYLE_COMMA0);
	REGISTER_EXCEL_CLASS_CONST_LONG(book, "CELLSTYLE_CURRENCY", CELLSTYLE_CURRENCY);
	REGISTER_EXCEL_CLASS_CONST_LONG(book, "CELLSTYLE_CURRENCY0", CELLSTYLE_CURRENCY0);
	REGISTER_EXCEL_CLASS_CONST_LONG(book, "CELLSTYLE_PERCENT", CELLSTYLE_PERCENT);

	REGISTER_EXCEL_CLASS_CONST_LONG(table, "TABLESTYLE_NONE", TABLESTYLE_NONE);
	REGISTER_EXCEL_CLASS_CONST_LONG(table, "TABLESTYLE_LIGHT1", TABLESTYLE_LIGHT1);
	REGISTER_EXCEL_CLASS_CONST_LONG(table, "TABLESTYLE_LIGHT2", TABLESTYLE_LIGHT2);
	REGISTER_EXCEL_CLASS_CONST_LONG(table, "TABLESTYLE_LIGHT3", TABLESTYLE_LIGHT3);
	REGISTER_EXCEL_CLASS_CONST_LONG(table, "TABLESTYLE_LIGHT4", TABLESTYLE_LIGHT4);
	REGISTER_EXCEL_CLASS_CONST_LONG(table, "TABLESTYLE_LIGHT5", TABLESTYLE_LIGHT5);
	REGISTER_EXCEL_CLASS_CONST_LONG(table, "TABLESTYLE_LIGHT6", TABLESTYLE_LIGHT6);
	REGISTER_EXCEL_CLASS_CONST_LONG(table, "TABLESTYLE_LIGHT7", TABLESTYLE_LIGHT7);
	REGISTER_EXCEL_CLASS_CONST_LONG(table, "TABLESTYLE_LIGHT8", TABLESTYLE_LIGHT8);
	REGISTER_EXCEL_CLASS_CONST_LONG(table, "TABLESTYLE_LIGHT9", TABLESTYLE_LIGHT9);
	REGISTER_EXCEL_CLASS_CONST_LONG(table, "TABLESTYLE_LIGHT10", TABLESTYLE_LIGHT10);
	REGISTER_EXCEL_CLASS_CONST_LONG(table, "TABLESTYLE_MEDIUM1", TABLESTYLE_MEDIUM1);
	REGISTER_EXCEL_CLASS_CONST_LONG(table, "TABLESTYLE_MEDIUM2", TABLESTYLE_MEDIUM2);
	REGISTER_EXCEL_CLASS_CONST_LONG(table, "TABLESTYLE_MEDIUM3", TABLESTYLE_MEDIUM3);
	REGISTER_EXCEL_CLASS_CONST_LONG(table, "TABLESTYLE_DARK1", TABLESTYLE_DARK1);
	REGISTER_EXCEL_CLASS_CONST_LONG(table, "TABLESTYLE_DARK2", TABLESTYLE_DARK2);
	REGISTER_EXCEL_CLASS_CONST_LONG(table, "TABLESTYLE_DARK3", TABLESTYLE_DARK3);

	REGISTER_EXCEL_CLASS_CONST_LONG(filtercolumn, "OPERATOR_EQUAL", OPERATOR_EQUAL);
	REGISTER_EXCEL_CLASS_CONST_LONG(filtercolumn, "OPERATOR_GREATER_THAN", OPERATOR_GREATER_THAN);
	REGISTER_EXCEL_CLASS_CONST_LONG(filtercolumn, "OPERATOR_GREATER_THAN_OR_EQUAL", OPERATOR_GREATER_THAN_OR_EQUAL);
	REGISTER_EXCEL_CLASS_CONST_LONG(filtercolumn, "OPERATOR_LESS_THAN", OPERATOR_LESS_THAN);
	REGISTER_EXCEL_CLASS_CONST_LONG(filtercolumn, "OPERATOR_LESS_THAN_OR_EQUAL", OPERATOR_LESS_THAN_OR_EQUAL);
	REGISTER_EXCEL_CLASS_CONST_LONG(filtercolumn, "OPERATOR_NOT_EQUAL", OPERATOR_NOT_EQUAL);

	REGISTER_EXCEL_CLASS_CONST_LONG(filtercolumn, "FILTER_VALUE", FILTER_VALUE);
	REGISTER_EXCEL_CLASS_CONST_LONG(filtercolumn, "FILTER_TOP10", FILTER_TOP10);
	REGISTER_EXCEL_CLASS_CONST_LONG(filtercolumn, "FILTER_CUSTOM", FILTER_CUSTOM);
	REGISTER_EXCEL_CLASS_CONST_LONG(filtercolumn, "FILTER_DYNAMIC", FILTER_DYNAMIC);
	REGISTER_EXCEL_CLASS_CONST_LONG(filtercolumn, "FILTER_COLOR", FILTER_COLOR);
	REGISTER_EXCEL_CLASS_CONST_LONG(filtercolumn, "FILTER_ICON", FILTER_ICON);
	REGISTER_EXCEL_CLASS_CONST_LONG(filtercolumn, "FILTER_EXT", FILTER_EXT);
	REGISTER_EXCEL_CLASS_CONST_LONG(filtercolumn, "FILTER_NOT_SET", FILTER_NOT_SET);

	REGISTER_EXCEL_CLASS_CONST_LONG(sheet, "VALIDATION_TYPE_NONE", VALIDATION_TYPE_NONE);
	REGISTER_EXCEL_CLASS_CONST_LONG(sheet, "VALIDATION_TYPE_WHOLE", VALIDATION_TYPE_WHOLE);
	REGISTER_EXCEL_CLASS_CONST_LONG(sheet, "VALIDATION_TYPE_DECIMAL", VALIDATION_TYPE_DECIMAL);
	REGISTER_EXCEL_CLASS_CONST_LONG(sheet, "VALIDATION_TYPE_LIST", VALIDATION_TYPE_LIST);
	REGISTER_EXCEL_CLASS_CONST_LONG(sheet, "VALIDATION_TYPE_DATE", VALIDATION_TYPE_DATE);
	REGISTER_EXCEL_CLASS_CONST_LONG(sheet, "VALIDATION_TYPE_TIME", VALIDATION_TYPE_TIME);
	REGISTER_EXCEL_CLASS_CONST_LONG(sheet, "VALIDATION_TYPE_TEXTLENGTH", VALIDATION_TYPE_TEXTLENGTH);
	REGISTER_EXCEL_CLASS_CONST_LONG(sheet, "VALIDATION_TYPE_CUSTOM", VALIDATION_TYPE_CUSTOM);

	REGISTER_EXCEL_CLASS_CONST_LONG(sheet, "VALIDATION_OP_BETWEEN", VALIDATION_OP_BETWEEN);
	REGISTER_EXCEL_CLASS_CONST_LONG(sheet, "VALIDATION_OP_NOTBETWEEN", VALIDATION_OP_NOTBETWEEN);
	REGISTER_EXCEL_CLASS_CONST_LONG(sheet, "VALIDATION_OP_EQUAL", VALIDATION_OP_EQUAL);
	REGISTER_EXCEL_CLASS_CONST_LONG(sheet, "VALIDATION_OP_NOTEQUAL", VALIDATION_OP_NOTEQUAL);
	REGISTER_EXCEL_CLASS_CONST_LONG(sheet, "VALIDATION_OP_LESSTHAN", VALIDATION_OP_LESSTHAN);
	REGISTER_EXCEL_CLASS_CONST_LONG(sheet, "VALIDATION_OP_LESSTHANOREQUAL", VALIDATION_OP_LESSTHANOREQUAL);
	REGISTER_EXCEL_CLASS_CONST_LONG(sheet, "VALIDATION_OP_GREATERTHAN", VALIDATION_OP_GREATERTHAN);
	REGISTER_EXCEL_CLASS_CONST_LONG(sheet, "VALIDATION_OP_GREATERTHANOREQUAL", VALIDATION_OP_GREATERTHANOREQUAL);

	REGISTER_EXCEL_CLASS_CONST_LONG(sheet, "VALIDATION_ERRSTYLE_STOP", VALIDATION_ERRSTYLE_STOP);
	REGISTER_EXCEL_CLASS_CONST_LONG(sheet, "VALIDATION_ERRSTYLE_WARNING", VALIDATION_ERRSTYLE_WARNING);
	REGISTER_EXCEL_CLASS_CONST_LONG(sheet, "VALIDATION_ERRSTYLE_INFORMATION", VALIDATION_ERRSTYLE_INFORMATION);

	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MSHUTDOWN_FUNCTION
 */
PHP_MSHUTDOWN_FUNCTION(excel)
{
	UNREGISTER_INI_ENTRIES();
	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MINFO_FUNCTION
 */
PHP_MINFO_FUNCTION(excel)
{
	char temp_api[25];

	php_info_print_table_start();
	php_info_print_table_header(2, "excel support", "enabled");
	php_info_print_table_row(2, "Excel Version", PHP_EXCEL_VERSION);
	snprintf(temp_api, sizeof(temp_api), "%x", LIBXL_VERSION);
	php_info_print_table_row(2, "LibXL Version", temp_api);
	php_info_print_table_end();
}
/* }}} */

/* {{{ PHP_GINIT_FUNCTION
 */
static PHP_GINIT_FUNCTION(excel)
{
#if defined(ZTS) && defined(COMPILE_DL_EXCEL)
	ZEND_TSRMLS_CACHE_UPDATE();
#endif
	memset(excel_globals, 0, sizeof(*excel_globals));
}
/* }}} */

/* {{{ excel_functions[]
 */
const zend_function_entry excel_functions[] = {
	PHP_FE_END
};
/* }}} */

/* {{{ excel_module_entry
 */
zend_module_entry excel_module_entry = {
	STANDARD_MODULE_HEADER,
	"excel",
	excel_functions,
	PHP_MINIT(excel),
	PHP_MSHUTDOWN(excel),
	NULL,
	NULL,
	PHP_MINFO(excel),
	PHP_EXCEL_VERSION,
	PHP_MODULE_GLOBALS(excel),
	PHP_GINIT(excel),
	NULL,
	NULL,
	STANDARD_MODULE_PROPERTIES_EX
};
/* }}} */

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
