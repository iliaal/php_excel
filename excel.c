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
#include "ext/standard/info.h"
#include "ext/date/php_date.h"

#include "php_excel.h"
#include "zend_exceptions.h"

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

#define PHP_EXCEL_VERSION "2.0.1"

#ifdef COMPILE_DL_EXCEL
ZEND_GET_MODULE(excel)
#endif

ZEND_DECLARE_MODULE_GLOBALS(excel)

static PHP_GINIT_FUNCTION(excel);

PHP_INI_BEGIN()
#if defined(HAVE_LIBXL_SETKEY)
	STD_PHP_INI_ENTRY("excel.license_name", NULL, PHP_INI_ALL, OnUpdateString, ini_license_name, zend_excel_globals, excel_globals)
	STD_PHP_INI_ENTRY("excel.license_key", NULL, PHP_INI_ALL, OnUpdateString, ini_license_key, zend_excel_globals, excel_globals)
#endif
	STD_PHP_INI_ENTRY("excel.skip_empty", "0", PHP_INI_ALL, OnUpdateLong, ini_skip_empty, zend_excel_globals, excel_globals)
PHP_INI_END()

/* {{{ OO init/structure stuff */
/* Class registration uses the gen_stub-generated register_class_*()
 * functions from excel_arginfo.h; this macro layers on create_object,
 * NOT_SERIALIZABLE, and per-class object_handlers. */
#define REGISTER_EXCEL_CLASS(name, c_name, clone) \
	{ \
		excel_ce_ ## c_name = register_class_Excel ## name(); \
		excel_ce_ ## c_name->ce_flags |= ZEND_ACC_NOT_SERIALIZABLE; \
		excel_ce_ ## c_name->create_object = excel_object_new_ ## c_name; \
		memcpy(&excel_object_handlers_ ## c_name, zend_get_std_object_handlers(), sizeof(zend_object_handlers)); \
		excel_ce_ ## c_name->default_object_handlers = &excel_object_handlers_ ## c_name; \
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
	/* Bumped whenever the underlying libxl book state is reset or a child-
	 * invalidating mutation runs (load, loadFile, clear, deleteSheet,
	 * insertSheet, copySheet, moveSheet, manual __construct reuse). Child
	 * wrappers stamp this at creation and CHECK_BOOK_GENERATION refuses to
	 * use them once the value diverges. */
	uint64_t generation;
	/* True for xlCreateXMLBook() (XLSX, 1048576x16384), false for
	 * xlCreateBook() (XLS, 65536x256). Used by EXCEL_VALIDATE_ROW_COL to
	 * reject impossible coordinates per book type. */
	bool is_xlsx;
	/* Lazily-allocated default format used by IS_DATE writes when the
	 * caller doesn't supply an explicit format, so bulk date exports
	 * don't create one new format per cell and bloat the style table.
	 * Reset along with `book` on __construct reuse. */
	FormatHandle default_date_format;
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

typedef struct _excel_sheet_object {
	SheetHandle	sheet;
	BookHandle book;
	uint64_t book_generation;
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
		CHECK_BOOK_GENERATION(obj); \
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
		CHECK_BOOK_GENERATION(obj); \
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
		CHECK_BOOK_GENERATION(obj); \
	}

#define FILTERCOLUMN_FROM_OBJECT(filtercolumn, object) \
	{ \
		excel_filtercolumn_object *obj = Z_EXCEL_FILTERCOLUMN_OBJ_P(object); \
		filtercolumn = obj->filtercolumn; \
		if (!filtercolumn) { \
			php_error_docref(NULL, E_WARNING, "The filtercolumn wasn't initialized"); \
			RETURN_FALSE; \
		} \
		CHECK_BOOK_GENERATION(obj); \
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
		CHECK_BOOK_GENERATION(obj); \
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
		CHECK_BOOK_GENERATION(obj); \
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
		CHECK_BOOK_GENERATION(obj); \
	}

/* Walk the parent zval chain up to the owning ExcelBook so that any descendant
 * (sheet, format, autofilter, filtercolumn, table, ...) can compare its
 * book_generation against the book's current generation counter. */
static inline excel_book_object *php_excel_resolve_book_obj(zval *parent_zv) {
	int hops = 0;
	while (parent_zv && Z_TYPE_P(parent_zv) == IS_OBJECT && hops++ < 8) {
		zend_class_entry *ce = Z_OBJCE_P(parent_zv);
		if (ce == excel_ce_book) {
			return php_excel_book_object_fetch_object(Z_OBJ_P(parent_zv));
		} else if (ce == excel_ce_sheet) {
			parent_zv = &php_excel_sheet_object_fetch_object(Z_OBJ_P(parent_zv))->parent;
		} else if (ce == excel_ce_format) {
			parent_zv = &php_excel_format_object_fetch_object(Z_OBJ_P(parent_zv))->parent;
		} else if (ce == excel_ce_font) {
			parent_zv = &php_excel_font_object_fetch_object(Z_OBJ_P(parent_zv))->parent;
		} else if (ce == excel_ce_autofilter) {
			parent_zv = &php_excel_autofilter_object_fetch_object(Z_OBJ_P(parent_zv))->parent;
		} else if (ce == excel_ce_filtercolumn) {
			parent_zv = &php_excel_filtercolumn_object_fetch_object(Z_OBJ_P(parent_zv))->parent;
		} else if (ce == excel_ce_richstring) {
			parent_zv = &php_excel_richstring_object_fetch_object(Z_OBJ_P(parent_zv))->parent;
		} else if (ce == excel_ce_formcontrol) {
			parent_zv = &php_excel_formcontrol_object_fetch_object(Z_OBJ_P(parent_zv))->parent;
		} else if (ce == excel_ce_conditionalformat) {
			parent_zv = &php_excel_conditionalformat_object_fetch_object(Z_OBJ_P(parent_zv))->parent;
		} else if (ce == excel_ce_conditionalformatting) {
			parent_zv = &php_excel_conditionalformatting_object_fetch_object(Z_OBJ_P(parent_zv))->parent;
		} else if (ce == excel_ce_coreproperties) {
			parent_zv = &php_excel_coreproperties_object_fetch_object(Z_OBJ_P(parent_zv))->parent;
		} else if (ce == excel_ce_table) {
			parent_zv = &php_excel_table_object_fetch_object(Z_OBJ_P(parent_zv))->parent;
		} else {
			return NULL;
		}
	}
	return NULL;
}

#define CHECK_BOOK_GENERATION(child_obj) \
	do { \
		excel_book_object *_b = php_excel_resolve_book_obj(&(child_obj)->parent); \
		if (!_b || !_b->book || _b->generation != (child_obj)->book_generation) { \
			php_error_docref(NULL, E_WARNING, \
				"Underlying ExcelBook handle is stale (parent was reloaded, cleared, or reinitialized)"); \
			RETURN_FALSE; \
		} \
	} while (0)

/* Preresolved variant of CHECK_BOOK_GENERATION. resolve_book_obj(child) and
 * resolve_book_obj(&child->parent) return the same owning book, so single-cell
 * hot paths can resolve the book once and feed it to both the stale check and
 * the coordinate-limit lookups instead of walking the parent chain each time. */
#define CHECK_BOOK_GENERATION_PR(child_obj, vb) \
	do { \
		if (!(vb) || !(vb)->book || (vb)->generation != (child_obj)->book_generation) { \
			php_error_docref(NULL, E_WARNING, \
				"Underlying ExcelBook handle is stale (parent was reloaded, cleared, or reinitialized)"); \
			RETURN_FALSE; \
		} \
	} while (0)

/* Stamp child wrapper with current book generation and copy parent zval.
 * Use at every child creation site instead of a bare ZVAL_COPY(&x->parent, p). */
#define EXCEL_INIT_PARENT(child_obj, parent_zv) \
	do { \
		excel_book_object *_bg = php_excel_resolve_book_obj(parent_zv); \
		(child_obj)->book_generation = _bg ? _bg->generation : 0; \
		ZVAL_COPY(&(child_obj)->parent, parent_zv); \
	} while (0)

/* Bump the generation counter so existing child wrappers fail their
 * stale-check. Use after libxl operations that shift internal child
 * indices (deleteSheet, moveSheet) — formats remain valid, so the
 * cached default date format must be preserved. */
static inline void php_excel_book_bump_generation(zval *book_zv) {
	excel_book_object *bobj = php_excel_book_object_fetch_object(Z_OBJ_P(book_zv));
	bobj->generation++;
}

/* Full state reset: bump generation AND drop the cached default date
 * format. Use after libxl operations that free the internal format
 * table (load, loadFile, loadInfo, loadInfoRaw, clear, __construct
 * reuse). Reusing the stale FormatHandle from default_date_format
 * after one of these would silently write cells with no format,
 * corrupting AS_DATE-typed output. */
static inline void php_excel_book_reset_state(zval *book_zv) {
	excel_book_object *bobj = php_excel_book_object_fetch_object(Z_OBJ_P(book_zv));
	bobj->generation++;
	bobj->default_date_format = NULL;
}

/* Throw-on-stale variant for code paths that cannot use RETURN_FALSE — most
 * importantly clone handlers, which must always produce an object and signal
 * failure via exception. Returns 1 on valid, 0 (and throws) on stale. */
static inline int php_excel_check_book_generation_throw(zval *parent_zv, uint64_t stamped) {
	excel_book_object *b = php_excel_resolve_book_obj(parent_zv);
	if (!b || !b->book || b->generation != stamped) {
		zend_throw_exception(NULL,
			"Underlying ExcelBook handle is stale (parent was reloaded, cleared, or reinitialized)",
			0);
		return 0;
	}
	return 1;
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
		if (!php_excel_check_book_generation_throw(&_obj->parent, _obj->book_generation)) { \
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
		if (!php_excel_check_book_generation_throw(&_obj->parent, _obj->book_generation)) { \
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
	intern->is_xlsx = false;
	intern->default_date_format = NULL;
	intern->std.handlers = &excel_object_handlers_book;

	return &intern->std;
}

static void excel_sheet_object_free_storage(zend_object *object)
{
	excel_sheet_object *intern = php_excel_sheet_object_fetch_object(object);
	zval_ptr_dtor(&intern->parent);
	zend_object_std_dtor(&intern->std);
}

static zend_object *excel_object_new_sheet(zend_class_entry *class_type)
{
	excel_sheet_object *intern;

	intern = zend_object_alloc(sizeof(excel_sheet_object), class_type);

	zend_object_std_init(&intern->std, class_type);
	object_properties_init(&intern->std, class_type);

	intern->std.handlers = &excel_object_handlers_sheet;

	return &intern->std;
}

static void excel_font_object_free_storage(zend_object *object)
{
	excel_font_object *intern = php_excel_font_object_fetch_object(object);
	zval_ptr_dtor(&intern->parent);
	zend_object_std_dtor(&intern->std);
}

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

static void excel_format_object_free_storage(zend_object *object)
{
	excel_format_object *intern = php_excel_format_object_fetch_object(object);
	zval_ptr_dtor(&intern->parent);
	zend_object_std_dtor(&intern->std);
}

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

static void excel_autofilter_object_free_storage(zend_object *object)
{
	excel_autofilter_object *intern = php_excel_autofilter_object_fetch_object(object);
	zval_ptr_dtor(&intern->parent);
	zend_object_std_dtor(&intern->std);
}

static zend_object *excel_object_new_autofilter(zend_class_entry *class_type)
{
	excel_autofilter_object *intern;

	intern = zend_object_alloc(sizeof(excel_autofilter_object), class_type);

	zend_object_std_init(&intern->std, class_type);
	object_properties_init(&intern->std, class_type);

	intern->std.handlers = &excel_object_handlers_autofilter;

	return &intern->std;
}

static void excel_filtercolumn_object_free_storage(zend_object *object)
{
	excel_filtercolumn_object *intern = php_excel_filtercolumn_object_fetch_object(object);
	zval_ptr_dtor(&intern->parent);
	zend_object_std_dtor(&intern->std);
}

static zend_object *excel_object_new_filtercolumn(zend_class_entry *class_type)
{
	excel_filtercolumn_object *intern;

	intern = zend_object_alloc(sizeof(excel_filtercolumn_object), class_type);

	zend_object_std_init(&intern->std, class_type);
	object_properties_init(&intern->std, class_type);

	intern->std.handlers = &excel_object_handlers_filtercolumn;

	return &intern->std;
}

static void excel_richstring_object_free_storage(zend_object *object)
{
	excel_richstring_object *intern = php_excel_richstring_object_fetch_object(object);
	zval_ptr_dtor(&intern->parent);
	zend_object_std_dtor(&intern->std);
}

static zend_object *excel_object_new_richstring(zend_class_entry *class_type)
{
	excel_richstring_object *intern;

	intern = zend_object_alloc(sizeof(excel_richstring_object), class_type);

	zend_object_std_init(&intern->std, class_type);
	object_properties_init(&intern->std, class_type);

	intern->std.handlers = &excel_object_handlers_richstring;

	return &intern->std;
}

static void excel_formcontrol_object_free_storage(zend_object *object)
{
	excel_formcontrol_object *intern = php_excel_formcontrol_object_fetch_object(object);
	zval_ptr_dtor(&intern->parent);
	zend_object_std_dtor(&intern->std);
}

static zend_object *excel_object_new_formcontrol(zend_class_entry *class_type)
{
	excel_formcontrol_object *intern;

	intern = zend_object_alloc(sizeof(excel_formcontrol_object), class_type);

	zend_object_std_init(&intern->std, class_type);
	object_properties_init(&intern->std, class_type);

	intern->std.handlers = &excel_object_handlers_formcontrol;

	return &intern->std;
}

static void excel_conditionalformat_object_free_storage(zend_object *object)
{
	excel_conditionalformat_object *intern = php_excel_conditionalformat_object_fetch_object(object);
	zval_ptr_dtor(&intern->parent);
	zend_object_std_dtor(&intern->std);
}

static zend_object *excel_object_new_conditionalformat(zend_class_entry *class_type)
{
	excel_conditionalformat_object *intern;

	intern = zend_object_alloc(sizeof(excel_conditionalformat_object), class_type);

	zend_object_std_init(&intern->std, class_type);
	object_properties_init(&intern->std, class_type);

	intern->std.handlers = &excel_object_handlers_conditionalformat;

	return &intern->std;
}

static void excel_conditionalformatting_object_free_storage(zend_object *object)
{
	excel_conditionalformatting_object *intern = php_excel_conditionalformatting_object_fetch_object(object);
	zval_ptr_dtor(&intern->parent);
	zend_object_std_dtor(&intern->std);
}

static zend_object *excel_object_new_conditionalformatting(zend_class_entry *class_type)
{
	excel_conditionalformatting_object *intern;

	intern = zend_object_alloc(sizeof(excel_conditionalformatting_object), class_type);

	zend_object_std_init(&intern->std, class_type);
	object_properties_init(&intern->std, class_type);

	intern->std.handlers = &excel_object_handlers_conditionalformatting;

	return &intern->std;
}

static void excel_coreproperties_object_free_storage(zend_object *object)
{
	excel_coreproperties_object *intern = php_excel_coreproperties_object_fetch_object(object);
	zval_ptr_dtor(&intern->parent);
	zend_object_std_dtor(&intern->std);
}

static zend_object *excel_object_new_coreproperties(zend_class_entry *class_type)
{
	excel_coreproperties_object *intern;

	intern = zend_object_alloc(sizeof(excel_coreproperties_object), class_type);

	zend_object_std_init(&intern->std, class_type);
	object_properties_init(&intern->std, class_type);

	intern->std.handlers = &excel_object_handlers_coreproperties;

	return &intern->std;
}

static void excel_table_object_free_storage(zend_object *object)
{
	excel_table_object *intern = php_excel_table_object_fetch_object(object);
	zval_ptr_dtor(&intern->parent);
	zend_object_std_dtor(&intern->std);
}

static zend_object *excel_object_new_table(zend_class_entry *class_type)
{
	excel_table_object *intern;

	intern = zend_object_alloc(sizeof(excel_table_object), class_type);

	zend_object_std_init(&intern->std, class_type);
	object_properties_init(&intern->std, class_type);

	intern->std.handlers = &excel_object_handlers_table;

	return &intern->std;
}

#define EXCEL_METHOD(class_name, function_name) \
	PHP_METHOD(Excel ## class_name, function_name)

#define EXCEL_NON_EMPTY_STRING(string_zval) \
	if (!string_zval || ZSTR_LEN(string_zval) < 1) {	\
		RETURN_FALSE;	\
	}

/* PHP zend_string is binary-safe; libxl C ABI is NUL-terminated.
 * Reject embedded NUL bytes so libxl never sees a silently-truncated
 * value while the application-side validator saw the full string. */
#define EXCEL_NUL_SAFE_STRING(string_zval) \
	if ((string_zval) && ZSTR_LEN(string_zval) != strlen(ZSTR_VAL(string_zval))) { \
		php_error_docref(NULL, E_WARNING, "String must not contain NUL bytes"); \
		RETURN_FALSE; \
	}

/* libxl APIs take int for row/col/dimension args; zend_long is 64-bit.
 * Reject out-of-int-range values before the implicit narrowing cast. */
#define EXCEL_VALIDATE_INT_RANGE(arg) \
	if ((arg) < 0 || (arg) > INT_MAX) { \
		php_error_docref(NULL, E_WARNING, "Argument out of int range"); \
		RETURN_FALSE; \
	}

/* Coordinate validation for sheet read/write paths. Limits depend on book
 * type: XLSX is 1048576 rows x 16384 cols; XLS is 65536 rows x 256 cols.
 * libxl write paths reject out-of-range writes themselves, but read paths
 * silently return empty cells, so the macro must run before either. */
#define EXCEL_MAX_ROW_XLSX 1048575
#define EXCEL_MAX_COL_XLSX 16383
#define EXCEL_MAX_ROW_XLS  65535
#define EXCEL_MAX_COL_XLS  255
#define EXCEL_VALIDATE_ROW_COL(r, c, parent_zv) \
	do { \
		excel_book_object *_vb = php_excel_resolve_book_obj(parent_zv); \
		zend_long _maxr = (_vb && _vb->is_xlsx) ? EXCEL_MAX_ROW_XLSX : EXCEL_MAX_ROW_XLS; \
		zend_long _maxc = (_vb && _vb->is_xlsx) ? EXCEL_MAX_COL_XLSX : EXCEL_MAX_COL_XLS; \
		if ((r) < 0 || (r) > _maxr || (c) < 0 || (c) > _maxc) { \
			php_error_docref(NULL, E_WARNING, \
				"Invalid coordinates: row=" ZEND_LONG_FMT ", column=" ZEND_LONG_FMT, \
				(zend_long)(r), (zend_long)(c)); \
			RETURN_FALSE; \
		} \
	} while (0)

/* Preresolved variant: the caller has already resolved the owning book object
 * (see CHECK_BOOK_GENERATION_PR). Identical limits and error message, no walk. */
#define EXCEL_VALIDATE_ROW_COL_PR(r, c, vb) \
	do { \
		zend_long _maxr = ((vb) && (vb)->is_xlsx) ? EXCEL_MAX_ROW_XLSX : EXCEL_MAX_ROW_XLS; \
		zend_long _maxc = ((vb) && (vb)->is_xlsx) ? EXCEL_MAX_COL_XLSX : EXCEL_MAX_COL_XLS; \
		if ((r) < 0 || (r) > _maxr || (c) < 0 || (c) > _maxc) { \
			php_error_docref(NULL, E_WARNING, \
				"Invalid coordinates: row=" ZEND_LONG_FMT ", column=" ZEND_LONG_FMT, \
				(zend_long)(r), (zend_long)(c)); \
			RETURN_FALSE; \
		} \
	} while (0)

/* Row-range validation for insertRow/removeRow, where both arguments are
 * row indices (row_first, row_last). The cell-coordinate macro above would
 * incorrectly check the second argument against the column limit. */
#define EXCEL_VALIDATE_ROW_RANGE(rfirst, rlast, parent_zv) \
	do { \
		excel_book_object *_vb = php_excel_resolve_book_obj(parent_zv); \
		zend_long _maxr = (_vb && _vb->is_xlsx) ? EXCEL_MAX_ROW_XLSX : EXCEL_MAX_ROW_XLS; \
		if ((rfirst) < 0 || (rfirst) > _maxr || (rlast) < 0 || (rlast) > _maxr) { \
			php_error_docref(NULL, E_WARNING, \
				"Invalid row range: first=" ZEND_LONG_FMT ", last=" ZEND_LONG_FMT, \
				(zend_long)(rfirst), (zend_long)(rlast)); \
			RETURN_FALSE; \
		} \
	} while (0)

/* Column-range validation for insertCol/removeCol. */
#define EXCEL_VALIDATE_COL_RANGE(cfirst, clast, parent_zv) \
	do { \
		excel_book_object *_vb = php_excel_resolve_book_obj(parent_zv); \
		zend_long _maxc = (_vb && _vb->is_xlsx) ? EXCEL_MAX_COL_XLSX : EXCEL_MAX_COL_XLS; \
		if ((cfirst) < 0 || (cfirst) > _maxc || (clast) < 0 || (clast) > _maxc) { \
			php_error_docref(NULL, E_WARNING, \
				"Invalid column range: first=" ZEND_LONG_FMT ", last=" ZEND_LONG_FMT, \
				(zend_long)(cfirst), (zend_long)(clast)); \
			RETURN_FALSE; \
		} \
	} while (0)

/* Single-axis validators for methods that take just one coordinate
 * (colWidth/rowHeight/setRowHidden/setColHidden/colWidthPx/rowHeightPx/...). */
#define EXCEL_VALIDATE_ROW(r, parent_zv) \
	do { \
		excel_book_object *_vb = php_excel_resolve_book_obj(parent_zv); \
		zend_long _maxr = (_vb && _vb->is_xlsx) ? EXCEL_MAX_ROW_XLSX : EXCEL_MAX_ROW_XLS; \
		if ((r) < 0 || (r) > _maxr) { \
			php_error_docref(NULL, E_WARNING, "Invalid row: " ZEND_LONG_FMT, (zend_long)(r)); \
			RETURN_FALSE; \
		} \
	} while (0)

#define EXCEL_VALIDATE_COL(c, parent_zv) \
	do { \
		excel_book_object *_vb = php_excel_resolve_book_obj(parent_zv); \
		zend_long _maxc = (_vb && _vb->is_xlsx) ? EXCEL_MAX_COL_XLSX : EXCEL_MAX_COL_XLS; \
		if ((c) < 0 || (c) > _maxc) { \
			php_error_docref(NULL, E_WARNING, "Invalid column: " ZEND_LONG_FMT, (zend_long)(c)); \
			RETURN_FALSE; \
		} \
	} while (0)

/* {{{ proto bool ExcelBook::requiresKey()
	true if license key is required. */
EXCEL_METHOD(Book, requiresKey)
{
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

	BOOK_FROM_OBJECT(book, object);

	php_excel_book_reset_state(object);
	RETURN_BOOL(xlBookLoadRaw(book, ZSTR_VAL(data_zs), ZSTR_LEN(data_zs)));
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

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "S", &filename_zs) == FAILURE) {
		RETURN_FALSE;
	}

	EXCEL_NON_EMPTY_STRING(filename_zs)
	EXCEL_NUL_SAFE_STRING(filename_zs)

	BOOK_FROM_OBJECT(book, object);

	/* For plain filesystem paths (no stream wrapper, passes open_basedir),
	 * hand the path straight to libxl so we don't double-buffer the workbook
	 * in PHP memory. Stream paths (phar://, php://, ...) still go through
	 * the wrapper machinery. */
	if (!strstr(ZSTR_VAL(filename_zs), "://")
	    && !php_check_open_basedir(ZSTR_VAL(filename_zs))) {
		php_excel_book_reset_state(object);
		RETURN_BOOL(xlBookLoad(book, ZSTR_VAL(filename_zs)));
	}

	stream = php_stream_open_wrapper(ZSTR_VAL(filename_zs), "rb", REPORT_ERRORS, NULL);

	if (!stream) {
		RETURN_FALSE;
	}

	contents = php_stream_copy_to_mem(stream, PHP_STREAM_COPY_ALL, 0);
	php_stream_close(stream);

	if (!contents) {
		php_error_docref(NULL, E_WARNING, "Source file is empty");
		RETURN_FALSE;
	}

	if (ZSTR_LEN(contents) < 1) {
		php_error_docref(NULL, E_WARNING, "Source file is empty");
		zend_string_release(contents);
		RETURN_FALSE;
	}

	php_excel_book_reset_state(object);
	RETVAL_BOOL(xlBookLoadRaw(book, ZSTR_VAL(contents), ZSTR_LEN(contents)));
	zend_string_release(contents);
}
/* }}} */

/* {{{ proto mixed ExcelBook::save([string filename])
	Save Excel file. */
EXCEL_METHOD(Book, save)
{
	BookHandle book;
	zval *object = ZEND_THIS;
	zend_string *filename_zs = NULL;
	unsigned int len = 0;
	char *contents = NULL;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "|S", &filename_zs) == FAILURE) {
		RETURN_FALSE;
	}

	if (filename_zs && ZSTR_LEN(filename_zs) > 0) {
		EXCEL_NUL_SAFE_STRING(filename_zs)
	}

	BOOK_FROM_OBJECT(book, object);

	/* Plain filesystem path: hand straight to libxl, no in-memory copy. */
	if (filename_zs && ZSTR_LEN(filename_zs) > 0
	    && !strstr(ZSTR_VAL(filename_zs), "://")
	    && !php_check_open_basedir(ZSTR_VAL(filename_zs))) {
		RETURN_BOOL(xlBookSave(book, ZSTR_VAL(filename_zs)));
	}

	if (!xlBookSaveRaw(book, (const char **) &contents, &len)) {
		RETURN_FALSE;
	}

	if (filename_zs && ZSTR_LEN(filename_zs) > 0) {
		ssize_t numbytes;
		php_stream *stream = php_stream_open_wrapper(ZSTR_VAL(filename_zs), "wb", REPORT_ERRORS, NULL);

		if (!stream) {
			RETURN_FALSE;
		}

		if ((numbytes = php_stream_write(stream, contents, len)) != (ssize_t)len) {
			php_stream_close(stream);
			php_error_docref(NULL, E_WARNING, "Only %zd of %u bytes written, possibly out of free disk space", numbytes, len);
			RETURN_FALSE;
		}

		php_stream_close(stream);
		RETURN_TRUE;
	} else {
		RETURN_STRINGL(contents, len);
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
	EXCEL_INIT_PARENT(fo, object);
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
					EXCEL_INIT_PARENT(fo, object);
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
		 * "pure virtual method called" on next use. Bump the generation
		 * so any outstanding wrapper fails the stale-check instead.
		 * Sibling sheets must be re-fetched via getSheet/getSheetByName. */
		php_excel_book_bump_generation(object);
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
	EXCEL_INIT_PARENT(fo, object);
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
	EXCEL_INIT_PARENT(fo, object);
}
/* }}} */

/* {{{ proto int ExcelBook::sheetCount()
	Get the number of sheets inside a file. */
EXCEL_METHOD(Book, sheetCount)
{
	BookHandle book;
	zval *object = ZEND_THIS;

	ZEND_PARSE_PARAMETERS_NONE();

	BOOK_FROM_OBJECT(book, object);

	RETURN_LONG(xlBookSheetCount(book));
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

	if (id < 1) {
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

	if (ts < 1) {
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
		if (year < 1) {
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

	if (dt < 1) {
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
	BookHandle book;
	zval *object = ZEND_THIS;

	ZEND_PARSE_PARAMETERS_NONE();

	BOOK_FROM_OBJECT(book, object);

	RETURN_BOOL(xlBookIsDate1904(book));
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
	BookHandle book;
	zval *object = ZEND_THIS;

	ZEND_PARSE_PARAMETERS_NONE();

	BOOK_FROM_OBJECT(book, object);

	RETURN_LONG(xlBookActiveSheet(book));
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

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "Sl", &font_zs, &font_size) == FAILURE || font_size < 1) {
		RETURN_FALSE;
	}

	EXCEL_NON_EMPTY_STRING(font_zs)
	EXCEL_NUL_SAFE_STRING(font_zs)

	BOOK_FROM_OBJECT(book, object);

	xlBookSetDefaultFont(book, ZSTR_VAL(font_zs), (int)font_size);
}
/* }}} */

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

	xlBookSetLocale(book, ZSTR_VAL(locale_zs));
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
		}
		obj->book = book;
		obj->is_xlsx = new_excel;
		obj->default_date_format = NULL;
	}

#if defined(HAVE_LIBXL_SETKEY)

	if (name_len == 0 && EXCEL_G(ini_license_name) && EXCEL_G(ini_license_key)) {
		name = EXCEL_G(ini_license_name);
		name_len = strlen(name);
		key = EXCEL_G(ini_license_key);
		key_len = strlen(key);
	}

	if (!name || name_len < 1 || !key || key_len < 1) {
		RETURN_FALSE;
	}

	if (name_len != strlen(name) || key_len != strlen(key)) {
		php_error_docref(NULL, E_WARNING, "License name/key must not contain NUL bytes");
		RETURN_FALSE;
	}

	xlBookSetKey(book, name, key);

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

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "S", &data_zs) == FAILURE) {
		RETURN_FALSE;
	}

	EXCEL_NON_EMPTY_STRING(data_zs)

	BOOK_FROM_OBJECT(book, object);

	if (mode == 1) {
		ret = xlBookAddPicture2(book, ZSTR_VAL(data_zs), ZSTR_LEN(data_zs));
	} else {
		/* path-mode: data_zs is a filesystem path. NUL truncation here
		 * silently opens a different file than the caller validated. */
		EXCEL_NUL_SAFE_STRING(data_zs)
		/* Plain filesystem path: skip the stream double-buffer. */
		if (!strstr(ZSTR_VAL(data_zs), "://")
		    && !php_check_open_basedir(ZSTR_VAL(data_zs))) {
			ret = xlBookAddPicture(book, ZSTR_VAL(data_zs));
			goto picture_done;
		}
		stream = php_stream_open_wrapper(ZSTR_VAL(data_zs), "rb", REPORT_ERRORS, NULL);

		if (!stream) {
			RETURN_FALSE;
		}

		contents = php_stream_copy_to_mem(stream, PHP_STREAM_COPY_ALL, 0);
		php_stream_close(stream);

		if (!contents || ZSTR_LEN(contents) < 1) {
			if (contents) {
				zend_string_release(contents);
			}
			RETURN_FALSE;
		}
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
	BookHandle book;
	zval *object = ZEND_THIS;

	ZEND_PARSE_PARAMETERS_NONE();

	BOOK_FROM_OBJECT(book, object);

	RETURN_BOOL(xlBookRgbMode(book));
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

	if (color <= 0) {
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
	snprintf(libxl_api, sizeof(libxl_api), "%x", LIBXL_VERSION);
	RETURN_STRING(libxl_api);
}
/* }}} */

/* {{{ proto string ExcelBook::getPhpExcelVersion()
	Returns the version of PHP Excel extension */
EXCEL_METHOD(Book, getPhpExcelVersion)
{
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

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "S", &filename_zs) == FAILURE) {
		RETURN_FALSE;
	}

	EXCEL_NON_EMPTY_STRING(filename_zs)
	EXCEL_NUL_SAFE_STRING(filename_zs)

	if (php_check_open_basedir(ZSTR_VAL(filename_zs))) {
		RETURN_FALSE;
	}

	BOOK_FROM_OBJECT(book, object);

	php_excel_book_reset_state(object);
	RETURN_BOOL(xlBookLoadInfo(book, ZSTR_VAL(filename_zs)));
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
	BookHandle book;
	zval *object = ZEND_THIS;

	ZEND_PARSE_PARAMETERS_NONE();

	BOOK_FROM_OBJECT(book, object);

	RETURN_LONG(xlBookCalcMode(book));
}

EXCEL_METHOD(Book, setCalcMode)
{
	BookHandle book;
	zval *object = ZEND_THIS;
	zend_long mode;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "l", &mode) == FAILURE) {
		RETURN_FALSE;
	}

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
	BookHandle book;
	zval *object = ZEND_THIS;

	ZEND_PARSE_PARAMETERS_NONE();

	BOOK_FROM_OBJECT(book, object);

	RETURN_BOOL(xlBookRemoveVBA(book));
}

EXCEL_METHOD(Book, removePrinterSettings)
{
	BookHandle book;
	zval *object = ZEND_THIS;

	ZEND_PARSE_PARAMETERS_NONE();

	BOOK_FROM_OBJECT(book, object);

	RETURN_BOOL(xlBookRemovePrinterSettings(book));
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
	BookHandle book;
	zval *object = ZEND_THIS;

	ZEND_PARSE_PARAMETERS_NONE();

	BOOK_FROM_OBJECT(book, object);

	RETURN_LONG(xlBookDpiAwareness(book));
}

EXCEL_METHOD(Book, setDpiAwareness)
{
	BookHandle book;
	zval *object = ZEND_THIS;
	zend_long val;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "l", &val) == FAILURE) {
		RETURN_FALSE;
	}

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

	BOOK_FROM_OBJECT(book, object);

	php_excel_book_reset_state(object);
	RETURN_BOOL(xlBookLoadInfoRaw(book, ZSTR_VAL(data), ZSTR_LEN(data)));
}
#endif

#if LIBXL_VERSION >= 0x05010000
EXCEL_METHOD(Book, errorCode)
{
	BookHandle book;
	zval *object = ZEND_THIS;

	ZEND_PARSE_PARAMETERS_NONE();

	BOOK_FROM_OBJECT(book, object);

	RETURN_LONG(xlBookErrorCode(book));
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
	BookHandle book;
	zval *object = ZEND_THIS;

	ZEND_PARSE_PARAMETERS_NONE();

	BOOK_FROM_OBJECT(book, object);

	RETURN_LONG(xlBookConditionalFormatSize(book));
}

EXCEL_METHOD(Book, clear)
{
	BookHandle book;
	zval *object = ZEND_THIS;

	ZEND_PARSE_PARAMETERS_NONE();

	BOOK_FROM_OBJECT(book, object);

	php_excel_book_reset_state(object);
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

	if (!size_is_null && size > 0) {
		if (size > INT_MAX) {
			php_error_docref(NULL, E_WARNING, "Argument out of int range");
			RETURN_FALSE;
		}
		xlFontSetSize(font, (int)size);
	}

	RETURN_LONG(xlFontSize(font));
}
/* }}} */

/* {{{ proto bool ExcelFont::italics([bool italics])
	Get or set the if italics are enabled */
EXCEL_METHOD(Font, italics)
{
	zval *object = ZEND_THIS;
	FontHandle font;
	bool italics = 0;
	bool italics_is_null = 1;

	ZEND_PARSE_PARAMETERS_START(0, 1)
		Z_PARAM_OPTIONAL
		Z_PARAM_BOOL_OR_NULL(italics, italics_is_null)
	ZEND_PARSE_PARAMETERS_END_EX(RETURN_FALSE);

	FONT_FROM_OBJECT(font, object);

	if (!italics_is_null) {
		xlFontSetItalic(font, italics);
	}

	RETURN_BOOL(xlFontItalic(font));
}
/* }}} */

/* {{{ proto bool ExcelFont::strike([bool strike])
	Get or set the font strike-through */
EXCEL_METHOD(Font, strike)
{
	zval *object = ZEND_THIS;
	FontHandle font;
	bool strike = 0;
	bool strike_is_null = 1;

	ZEND_PARSE_PARAMETERS_START(0, 1)
		Z_PARAM_OPTIONAL
		Z_PARAM_BOOL_OR_NULL(strike, strike_is_null)
	ZEND_PARSE_PARAMETERS_END_EX(RETURN_FALSE);

	FONT_FROM_OBJECT(font, object);

	if (!strike_is_null) {
		xlFontSetStrikeOut(font, strike);
	}

	RETURN_BOOL(xlFontStrikeOut(font));
}
/* }}} */

/* {{{ proto bool ExcelFont::bold([bool bold])
	Get or set the font bold */
EXCEL_METHOD(Font, bold)
{
	zval *object = ZEND_THIS;
	FontHandle font;
	bool bold = 0;
	bool bold_is_null = 1;

	ZEND_PARSE_PARAMETERS_START(0, 1)
		Z_PARAM_OPTIONAL
		Z_PARAM_BOOL_OR_NULL(bold, bold_is_null)
	ZEND_PARSE_PARAMETERS_END_EX(RETURN_FALSE);

	FONT_FROM_OBJECT(font, object);

	if (!bold_is_null) {
		xlFontSetBold(font, bold);
	}

	RETURN_BOOL(xlFontBold(font));
}
/* }}} */

/* {{{ proto int ExcelFont::color([int color])
	Get or set the font color */
EXCEL_METHOD(Font, color)
{
	zval *object = ZEND_THIS;
	FontHandle font;
	zend_long color = 0;
	bool color_is_null = 1;

	ZEND_PARSE_PARAMETERS_START(0, 1)
		Z_PARAM_OPTIONAL
		Z_PARAM_LONG_OR_NULL(color, color_is_null)
	ZEND_PARSE_PARAMETERS_END_EX(RETURN_FALSE);

	FONT_FROM_OBJECT(font, object);

	if (!color_is_null) {
		if (color < 0 || color > INT_MAX) {
			php_error_docref(NULL, E_WARNING, "Argument out of int range");
			RETURN_FALSE;
		}
		xlFontSetColor(font, (int)color);
	}

	RETURN_LONG(xlFontColor(font));
}
/* }}} */

/* {{{ proto int ExcelFont::mode([int mode])
	Get or set the font mode */
EXCEL_METHOD(Font, mode)
{
	zval *object = ZEND_THIS;
	FontHandle font;
	zend_long mode = 0;
	bool mode_is_null = 1;

	ZEND_PARSE_PARAMETERS_START(0, 1)
		Z_PARAM_OPTIONAL
		Z_PARAM_LONG_OR_NULL(mode, mode_is_null)
	ZEND_PARSE_PARAMETERS_END_EX(RETURN_FALSE);

	FONT_FROM_OBJECT(font, object);

	if (!mode_is_null) {
		if (mode < 0 || mode > INT_MAX) {
			php_error_docref(NULL, E_WARNING, "Argument out of int range");
			RETURN_FALSE;
		}
		xlFontSetScript(font, (int)mode);
	}

	RETURN_LONG(xlFontScript(font));
}
/* }}} */

/* {{{ proto int ExcelFont::underline([int underline_style])
	Get or set the font underline style */
EXCEL_METHOD(Font, underline)
{
	zval *object = ZEND_THIS;
	FontHandle font;
	zend_long underline = 0;
	bool underline_is_null = 1;

	ZEND_PARSE_PARAMETERS_START(0, 1)
		Z_PARAM_OPTIONAL
		Z_PARAM_LONG_OR_NULL(underline, underline_is_null)
	ZEND_PARSE_PARAMETERS_END_EX(RETURN_FALSE);

	FONT_FROM_OBJECT(font, object);

	if (!underline_is_null) {
		if (underline < 0 || underline > INT_MAX) {
			php_error_docref(NULL, E_WARNING, "Argument out of int range");
			RETURN_FALSE;
		}
		xlFontSetUnderline(font, (int)underline);
	}

	RETURN_LONG(xlFontUnderline(font));
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

	RETURN_STRING((char *)xlFontName(font));
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

	sh = xlBookAddSheet(book, ZSTR_VAL(name_zs), 0);

	if (!sh) {
		zend_throw_exception(NULL, "Failed to add sheet", 0);
		RETURN_THROWS();
	}

	obj->sheet = sh;
	obj->book = book;
	EXCEL_INIT_PARENT(obj, zbook);
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

	xlSheetSetCellFormat(sheet, row, col, format);
}
/* }}} */

bool php_excel_read_cell(int row, int col, zval *val, SheetHandle sheet, BookHandle book, FormatHandle *format, bool read_formula)
{
	const char *s;
	if (read_formula && xlSheetIsFormula(sheet, row, col)) {
		s = xlSheetReadFormula(sheet, row, col, format);
		if (s) {
			ZVAL_STRING(val, (char *)s);
			return 1;
		} else {
			return 0;
		}
	}

	switch (xlSheetCellType(sheet, row, col)) {
		case CELLTYPE_EMPTY:
			*format = xlSheetCellFormat(sheet, row, col);
			ZVAL_EMPTY_STRING(val);
			return 1;

		case CELLTYPE_BLANK:
			if (!xlSheetReadBlank(sheet, row, col, format)) {
				return 0;
			} else {
				ZVAL_NULL(val);
				return 1;
			}

		case CELLTYPE_NUMBER: {
			double d = xlSheetReadNum(sheet, row, col, format);
			if (xlSheetIsDate(sheet, row, col)) {
				zend_long dt = _php_excel_date_unpack(book, d);
				if (dt == -1) {
					return 0;
				} else {
					ZVAL_LONG(val, dt);
					return 1;
				}
			} else {
				ZVAL_DOUBLE(val, d);
				return 1;
			}
		}

		case CELLTYPE_STRING: {
			s = xlSheetReadStr(sheet, row, col, format);
			if (s) {
				ZVAL_STRING(val, (char *)s);
				return 1;
			} else {
				return 0;
			}
		}

		case CELLTYPE_BOOLEAN:
			ZVAL_BOOL(val, xlSheetReadBool(sheet, row, col, format));
			return 1;

		case CELLTYPE_ERROR:
			ZVAL_LONG(val, xlSheetReadError(sheet, row, col));
			return 1;
	}

	return 0;
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

	if (col_end == -1) {
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

		if (!php_excel_read_cell(row, lc, &value, sheet, book, &format, read_formula)) {
			zval_ptr_dtor(&value);
			zval_ptr_dtor(return_value);
			php_error_docref(NULL, E_WARNING, "Failed to read cell in row " ZEND_LONG_FMT ", column %d with error '%s'", row, lc, xlBookErrorMessage(book));
			RETURN_FALSE;
		} else {
			add_next_index_zval(return_value, &value);
		}

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

	if (row_end == -1) {
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

		if (!php_excel_read_cell(lc, col, &value, sheet, book, &format, read_formula)) {
			zval_ptr_dtor(&value);
			zval_ptr_dtor(return_value);
			php_error_docref(NULL, E_WARNING, "Failed to read cell in row %d, column " ZEND_LONG_FMT " with error '%s'", lc, col, xlBookErrorMessage(book));
			RETURN_FALSE;
		} else {
			add_next_index_zval(return_value, &value);
		}

		lc++;
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
	CHECK_BOOK_GENERATION_PR(sheet_obj, book_obj);

	if (oformat) {
		ZVAL_DEREF(oformat);
		zval_ptr_dtor(oformat);
		ZVAL_NULL(oformat);
	}

	if (!php_excel_read_cell(row, col, return_value, sheet, book, &format, read_formula)) {
		php_error_docref(NULL, E_WARNING, "Failed to read cell in row " ZEND_LONG_FMT ", column " ZEND_LONG_FMT " with error '%s'", row, col, xlBookErrorMessage(book));
		RETURN_FALSE;
	}

	if (oformat) {
		excel_format_object *fo;

		ZVAL_OBJ(oformat, excel_object_new_format(excel_ce_format));
		fo = Z_EXCEL_FORMAT_OBJ_P(oformat);
		fo->format = format;
		fo->book = book;
		EXCEL_INIT_PARENT(fo, object);
	}
}
/* }}} */

bool php_excel_write_cell(SheetHandle sheet, excel_book_object *book_obj, int row, int col, zval *data, FormatHandle format, zend_long dtype)
{
	zend_string *data_zs;
	BookHandle book = book_obj->book;

	try_again:
	switch (Z_TYPE_P(data)) {
		case IS_NULL:
			if (EXCEL_G(ini_skip_empty) > 0) {
				return 1;
			}
			if (!format) {
				return xlSheetWriteBlank(sheet, row, col, NULL);
			} else {
				return xlSheetWriteBlank(sheet, row, col, format);
			}

		case IS_LONG:
			if (dtype == PHP_EXCEL_DATE) {
				double dt;
				if ((dt = _php_excel_date_pack(book, Z_LVAL_P(data))) == -1) {
					return 0;
				}
				if (!format) {
					if (!book_obj->default_date_format) {
						/* Lazily create one shared date format and cache it
						 * on the book so bulk date exports don't push the
						 * style-table count up by one per cell. */
						book_obj->default_date_format = xlBookAddFormat(book, NULL);
						if (book_obj->default_date_format) {
							xlFormatSetNumFormat(book_obj->default_date_format, NUMFORMAT_DATE);
						}
					}
					return xlSheetWriteNum(sheet, row, col, dt, book_obj->default_date_format);
				} else {
					return xlSheetWriteNum(sheet, row, col, dt, format);
				}
			} else {
				return xlSheetWriteNum(sheet, row, col, (double) Z_LVAL_P(data), format);
			}

		case IS_DOUBLE:
			return xlSheetWriteNum(sheet, row, col, Z_DVAL_P(data), format);

		case IS_STRING:
			data_zs = Z_STR_P(data);
			/* libxl writes use NUL-terminated C strings; reject embedded
			 * NULs so the caller's value isn't silently truncated. */
			if (ZSTR_LEN(data_zs) != strlen(ZSTR_VAL(data_zs))) {
				php_error_docref(NULL, E_WARNING, "Cell string must not contain NUL bytes");
				return 0;
			}
			if (Z_STRLEN_P(data) > 0 && '\'' == Z_STRVAL_P(data)[0]) {
				return xlSheetWriteStr(sheet, row, col, (const char*) ZSTR_VAL(data_zs) + 1, format);
			}
			if (Z_STRLEN_P(data) > 0 && '=' == Z_STRVAL_P(data)[0]) {
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
			php_error_docref(NULL, E_WARNING, "Type mismatch: %d not supported for atomic write operation in row %d, column %d", Z_TYPE_P(data), row, col);
			return 0;
	}

	return 0;
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
	FormatHandle format;
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
	CHECK_BOOK_GENERATION_PR(sheet_obj, book_obj);

	if (oformat) {
		FORMAT_FROM_OBJECT(format, oformat);
	}

	if (!php_excel_write_cell(sheet, book_obj, row, col, data, oformat ? format : 0, dtype)) {
		php_error_docref(NULL, E_WARNING, "Failed to write cell in row " ZEND_LONG_FMT ", column " ZEND_LONG_FMT " with error '%s'", row, col, xlBookErrorMessage(book));
		RETURN_FALSE;
	}

	RETURN_TRUE;
}
/* }}} */

/* {{{ proto bool ExcelSheet::writeRow(int row, array data [, int start_column [, ExcelFormat format]])
	Write an array of values into a row */
EXCEL_METHOD(Sheet, writeRow)
{
	zval *object = ZEND_THIS;
	SheetHandle sheet;
	BookHandle book;
	FormatHandle format;
	zend_long row, col = 0;
	zval *oformat = NULL;
	zval *data;
	zval *element;
	zend_long i;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "la|lO!", &row, &data, &col, &oformat, excel_ce_format) == FAILURE) {
		RETURN_FALSE;
	}

	/* Validate the start cell first so col is known to be in
	 * 0..EXCEL_MAX_COL_XLSX before we do any signed arithmetic on it.
	 * Then the (max - col) remaining-capacity computation is safe and
	 * count <= remaining is the equivalent of (col + count - 1) <= max
	 * without the signed-overflow risk on extreme starts. */
	EXCEL_VALIDATE_ROW_COL(row, col, object);
	{
		zend_ulong count = zend_array_count(Z_ARRVAL_P(data));
		excel_book_object *_vb = php_excel_resolve_book_obj(object);
		zend_long _maxc = (_vb && _vb->is_xlsx) ? EXCEL_MAX_COL_XLSX : EXCEL_MAX_COL_XLS;
		if (count > (zend_ulong)(_maxc - col + 1)) {
			php_error_docref(NULL, E_WARNING,
				"writeRow would overflow column range: start=" ZEND_LONG_FMT
				", count=" ZEND_ULONG_FMT, col, count);
			RETURN_FALSE;
		}
	}
	SHEET_AND_BOOK_FROM_OBJECT(sheet, book, object);
	if (oformat) {
		FORMAT_FROM_OBJECT(format, oformat);
	}

	i = col;

	{
		excel_book_object *book_obj = php_excel_resolve_book_obj(object);
		ZEND_HASH_FOREACH_VAL(Z_ARRVAL_P(data), element) {
			if (!php_excel_write_cell(sheet, book_obj, row, i++, element, (oformat ? format : 0), -1)) {
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
	FormatHandle format;
	zend_long row = 0, col;
	zval *oformat = NULL;
	zval *data;
	zval *element;
	zend_long i;
	zend_long dtype = -1;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "la|lO!l", &col, &data, &row, &oformat, excel_ce_format, &dtype) == FAILURE) {
		RETURN_FALSE;
	}

	EXCEL_VALIDATE_ROW_COL(row, col, object);
	{
		zend_ulong count = zend_array_count(Z_ARRVAL_P(data));
		excel_book_object *_vb = php_excel_resolve_book_obj(object);
		zend_long _maxr = (_vb && _vb->is_xlsx) ? EXCEL_MAX_ROW_XLSX : EXCEL_MAX_ROW_XLS;
		if (count > (zend_ulong)(_maxr - row + 1)) {
			php_error_docref(NULL, E_WARNING,
				"writeCol would overflow row range: start=" ZEND_LONG_FMT
				", count=" ZEND_ULONG_FMT, row, count);
			RETURN_FALSE;
		}
	}
	SHEET_AND_BOOK_FROM_OBJECT(sheet, book, object);
	if (oformat) {
		FORMAT_FROM_OBJECT(format, oformat);
	}

	i = row;

	{
		excel_book_object *book_obj = php_excel_resolve_book_obj(object);
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

	if (xlSheetCellType(sheet, r, c) != CELLTYPE_NUMBER) {
		RETURN_FALSE;
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

		SHEET_FROM_OBJECT(sheet, object);

		xlSheetWriteComment(sheet, r, c, ZSTR_VAL(val_zs), ZSTR_VAL(auth_zs), w, h);
}
/* }}} */

/* {{{ proto void ExcelSheet::setColWidth(int column_start, int column_end, double width [, bool hidden [, ExcelFormat format]])
	Set width of cells within column(s); Value -1 is used for autofit column widths in LibXL 3.6+ */
EXCEL_METHOD(Sheet, setColWidth)
{
		SheetHandle sheet;
		FormatHandle format;
		zval *object = ZEND_THIS;
		zend_long s, e;
		double width;
		zval *f = NULL;
		bool h = 0;

		if (zend_parse_parameters(ZEND_NUM_ARGS(), "lld|bO!", &s, &e, &width, &h, &f, excel_ce_format) == FAILURE) {
			RETURN_FALSE;
		}

		EXCEL_VALIDATE_COL_RANGE(s, e, object);
		SHEET_FROM_OBJECT(sheet, object);

		if (f) {
			FORMAT_FROM_OBJECT(format, f);
		}

		if (e < s) {
			php_error_docref(NULL, E_WARNING, "Start cell is greater then end cell");
			RETURN_FALSE;
		} else if (width < -1) {
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
		FormatHandle format;
		zval *object = ZEND_THIS;
		zend_long row;
		double height;
		zval *f = NULL;
		bool h = 0;

		if (zend_parse_parameters(ZEND_NUM_ARGS(), "ld|O!b", &row, &height, &f, excel_ce_format, &h) == FAILURE) {
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
		SHEET_FROM_OBJECT(sheet, object);

		if (f) {
			FORMAT_FROM_OBJECT(format, f);
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

#define PE_RETURN_IS_LONG RETURN_LONG
#define PE_RETURN_IS_BOOL RETURN_BOOL
#define PE_RETURN_IS_DOUBLE RETURN_DOUBLE
#define PE_RETURN_IS_STRING(data) if (data) { RETURN_STRING((char *)data); } else { RETURN_NULL(); }

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
	SheetHandle sheet;
	zval *object = ZEND_THIS;

	if (zend_parse_parameters_none() == FAILURE) {
		RETURN_FALSE;
	}

	SHEET_FROM_OBJECT(sheet, object);

	RETURN_BOOL(xlSheetHidden(sheet));
}
/* }}} */

/* {{{ proto array ExcelSheet::getTopLeftView()
	Extracts the first visible row and the leftmost visible column of the sheet. */
EXCEL_METHOD(Sheet, getTopLeftView)
{
	SheetHandle sheet;
	zval *object = ZEND_THIS;
	int r = 0, c = 0;

	if (zend_parse_parameters_none() == FAILURE) {
		RETURN_FALSE;
	}

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

/* {{{ proto array ExcelSheet::addrToRowCol(string cell_reference)
	Converts a cell reference to row and column. */
EXCEL_METHOD(Sheet, addrToRowCol)
{
	SheetHandle sheet;
	zval *object = ZEND_THIS;
	zend_string *cell_reference_zs = NULL;
	int row = 0, col = 0, rowRelative = 0, colRelative = 0;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "S", &cell_reference_zs) == FAILURE) {
		RETURN_FALSE;
	}

	if (!cell_reference_zs || ZSTR_LEN(cell_reference_zs) < 1) {
		php_error_docref(NULL, E_WARNING, "Cell reference cannot be empty");
		RETURN_FALSE;
	}
	EXCEL_NUL_SAFE_STRING(cell_reference_zs)

	SHEET_FROM_OBJECT(sheet, object);

	xlSheetAddrToRowCol(sheet, ZSTR_VAL(cell_reference_zs), &row, &col, &rowRelative, &colRelative);
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

#define PHP_EXCEL_SET_HF(func_name) \
	{ \
		SheetHandle sheet; \
		zval *object = ZEND_THIS; \
		zend_string *val_zs = NULL; \
		double margin; \
		if (zend_parse_parameters(ZEND_NUM_ARGS(), "Sd", &val_zs, &margin) == FAILURE) { \
			RETURN_FALSE; \
		} \
		if (!val_zs || ZSTR_LEN(val_zs) > 255) { \
			RETURN_FALSE; \
		} \
		EXCEL_NUL_SAFE_STRING(val_zs); \
		SHEET_FROM_OBJECT(sheet, object); \
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

/* {{{ proto bool ExcelSheet::setNamedRange(string name, int row, int col, int to_row, int to_col [, int scope_id])
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
	SheetHandle sheet;
	zval *object = ZEND_THIS;

	ZEND_PARSE_PARAMETERS_NONE();

	SHEET_FROM_OBJECT(sheet, object);
	RETURN_BOOL(xlSheetGroupSummaryBelow(sheet));
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
	SheetHandle sheet;
	zval *object = ZEND_THIS;

	ZEND_PARSE_PARAMETERS_NONE();

	SHEET_FROM_OBJECT(sheet, object);
	RETURN_BOOL(xlSheetGroupSummaryRight(sheet));
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
	if (xlSheetNamedRange(sheet, (int)index, &rf, &rl, &cf, &cl, &scope_out, &hidden)) {
		array_init(return_value);
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
	SheetHandle sheet;
	zval *object = ZEND_THIS;

	ZEND_PARSE_PARAMETERS_NONE();

	SHEET_FROM_OBJECT(sheet, object);
	RETURN_LONG(xlSheetNamedRangeSize(sheet));
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
	SheetHandle sheet;
	zval *object = ZEND_THIS;

	ZEND_PARSE_PARAMETERS_NONE();

	SHEET_FROM_OBJECT(sheet, object);
	RETURN_LONG(xlSheetGetVerPageBreakSize(sheet));
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
	SheetHandle sheet;
	zval *object = ZEND_THIS;

	ZEND_PARSE_PARAMETERS_NONE();

	SHEET_FROM_OBJECT(sheet, object);
	RETURN_LONG(xlSheetGetHorPageBreakSize(sheet));
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
	SheetHandle sheet;
	zval *object = ZEND_THIS;

	ZEND_PARSE_PARAMETERS_NONE();

	SHEET_FROM_OBJECT(sheet, object);
	RETURN_LONG(xlSheetPictureSize(sheet));
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
	BookHandle book;
	zval *object = ZEND_THIS;

	ZEND_PARSE_PARAMETERS_NONE();

	BOOK_FROM_OBJECT(book, object);
	RETURN_BOOL(xlBookRefR1C1A(book));
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

	array_init(return_value);
	add_assoc_stringl(return_value, "data", (char *)buf, buf_len);
	add_assoc_long(return_value, "type", type);
}
/* }}} */

/* {{{ proto long ExcelBook::getNumPictures()
	Returns a number of pictures in this workbook. */
EXCEL_METHOD(Book, getNumPictures)
{
	BookHandle book;
	zval *object = ZEND_THIS;

	ZEND_PARSE_PARAMETERS_NONE();

	BOOK_FROM_OBJECT(book, object);
	RETURN_LONG(xlBookPictureSize(book));
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
	EXCEL_INIT_PARENT(fo, object);
}
/* }}} */

/* {{{ proto bool ExcelBook::isTemplate()
	Returns whether the workbook is template. */
EXCEL_METHOD(Book, isTemplate)
{
	BookHandle book;
	zval *object = ZEND_THIS;

	ZEND_PARSE_PARAMETERS_NONE();

	BOOK_FROM_OBJECT(book, object);
	RETURN_BOOL(xlBookIsTemplate(book));
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
}
/* }}} */

/* {{{ proto long ExcelSheet::getRightToLeft()
	Returns whether the text is displayed in right-to-left mode: 1 - yes, 0 - no. */
EXCEL_METHOD(Sheet, getRightToLeft)
{
	SheetHandle sheet;
	zval *object = ZEND_THIS;

	ZEND_PARSE_PARAMETERS_NONE();

	SHEET_FROM_OBJECT(sheet, object);
	RETURN_LONG(xlSheetRightToLeft(sheet));
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
	zval *object = ZEND_THIS;
	SheetHandle sheet;

	ZEND_PARSE_PARAMETERS_NONE();

	SHEET_FROM_OBJECT(sheet, object);
	xlSheetClearPrintRepeats(sheet);

	RETURN_TRUE;
}
/* }}} */

/* {{{ proto bool ExcelSheet::clearPrintArea()
	Clears the print area. */
EXCEL_METHOD(Sheet, clearPrintArea)
{
	zval *object = ZEND_THIS;
	SheetHandle sheet;

	ZEND_PARSE_PARAMETERS_NONE();

	SHEET_FROM_OBJECT(sheet, object);
	xlSheetClearPrintArea(sheet);

	RETURN_TRUE;
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
	zval *object = ZEND_THIS;
	SheetHandle sheet;

	ZEND_PARSE_PARAMETERS_NONE();

	SHEET_FROM_OBJECT(sheet, object);
	RETURN_LONG(xlSheetHyperlinkSize(sheet));
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
	zval *object = ZEND_THIS;
	SheetHandle sheet;

	ZEND_PARSE_PARAMETERS_NONE();

	SHEET_FROM_OBJECT(sheet, object);
	RETURN_LONG(xlSheetMergeSize(sheet));
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
		    || colLast < -1 || colLast > _maxc) {
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

	SHEET_FROM_OBJECT(sheet, object);

	if (!xlSheetPrintArea(sheet, &rowFirst, &colFirst, &rowLast, &colLast)) {
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

	ZVAL_OBJ(return_value, excel_object_new_autofilter(excel_ce_autofilter));
	obj = Z_EXCEL_AUTOFILTER_OBJ_P(return_value);
	obj->autofilter = ah;
	obj->sheet = sheet;
	EXCEL_INIT_PARENT(obj, object);
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

	SHEET_FROM_OBJECT(sheet, object);

	if (oformat) {
		FORMAT_FROM_OBJECT(format, oformat);
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

	afh = xlSheetAutoFilter(sheet);

	if (!afh) {
		zend_throw_exception(NULL, "Failed to create autofilter", 0);
		RETURN_THROWS();
	}

	obj->sheet = sheet;
	obj->autofilter = afh;
	EXCEL_INIT_PARENT(obj, zsheet);
}
/* }}} */

/* {{{ proto long AutoFilter::getRef()
	Gets the cell range of AutoFilter with header. Returns 0 if error. */
EXCEL_METHOD(AutoFilter, getRef)
{
	zval *object = ZEND_THIS;
	AutoFilterHandle autofilter;
	int rowFirst=0, colFirst=0, rowLast=0, colLast=0;

	AUTOFILTER_FROM_OBJECT(autofilter, object);

	if (!xlAutoFilterGetRef(autofilter, &rowFirst, &colFirst, &rowLast, &colLast)) {
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

	ZVAL_OBJ(return_value, excel_object_new_filtercolumn(excel_ce_filtercolumn));
	obj = Z_EXCEL_FILTERCOLUMN_OBJ_P(return_value);
	obj->autofilter = autofilter;
	obj->filtercolumn = fch;
	EXCEL_INIT_PARENT(obj, object);
}
/* }}} */

/* {{{ proto long AutoFilter::columnSize()
	Returns the number of specified AutoFilter columns which have a filter information. */
EXCEL_METHOD(AutoFilter, columnSize)
{
	zval *object = ZEND_THIS;
	AutoFilterHandle autofilter;

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

	ZVAL_OBJ(return_value, excel_object_new_filtercolumn(excel_ce_filtercolumn));
	obj = Z_EXCEL_FILTERCOLUMN_OBJ_P(return_value);
	obj->autofilter = autofilter;
	obj->filtercolumn = fch;
	EXCEL_INIT_PARENT(obj, object);
}
/* }}} */

/* {{{ proto long AutoFilter::getSortRange()
	Gets the whole range of data to sort. Returns 0 if error. */
EXCEL_METHOD(AutoFilter, getSortRange)
{
	zval *object = ZEND_THIS;
	AutoFilterHandle autofilter;
	int rowFirst=0, rowLast=0, colFirst=0, colLast=0;

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

	AUTOFILTER_FROM_OBJECT(autofilter, object);

	if (!xlAutoFilterGetSort(autofilter, &columnIndex, &descending)) {
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

	fch = xlAutoFilterColumn(autofilter, colId);

	if (!fch) {
		zend_throw_exception(NULL, "Failed to get filter column", 0);
		RETURN_THROWS();
	}

	obj->filtercolumn = fch;
	obj->autofilter = autofilter;
	EXCEL_INIT_PARENT(obj, zautofilter);
}
/* }}} */

/* {{{ proto long FilterColumn::index()
	Returns the zero-based index of this AutoFilter column. */
EXCEL_METHOD(FilterColumn, index)
{
	zval *object = ZEND_THIS;
	FilterColumnHandle filtercolumn;

	FILTERCOLUMN_FROM_OBJECT(filtercolumn, object);

	RETURN_LONG(xlFilterColumnIndex(filtercolumn));
}
/* }}} */

/* {{{ proto long FilterColumn::filterType()
	Returns the filter type of this AutoFilter column. */
EXCEL_METHOD(FilterColumn, filterType)
{
	zval *object = ZEND_THIS;
	FilterColumnHandle filtercolumn;

	FILTERCOLUMN_FROM_OBJECT(filtercolumn, object);

	RETURN_LONG(xlFilterColumnFilterType(filtercolumn));
}
/* }}} */

/* {{{ proto long FilterColumn::filterSize()
	Returns the number of filter values. */
EXCEL_METHOD(FilterColumn, filterSize)
{
	zval *object = ZEND_THIS;
	FilterColumnHandle filtercolumn;

	FILTERCOLUMN_FROM_OBJECT(filtercolumn, object);

	RETURN_LONG(xlFilterColumnFilterSize(filtercolumn));
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

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "dbb", &value, &top, &percent) == FAILURE) {
		RETURN_FALSE;
	}

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

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "lS|lSb", &op1, &v1, &op2, &v2, &andOp) == FAILURE) {
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
	 * a different sheet. Bump generation to invalidate them. */
	php_excel_book_bump_generation(object);
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

	EXCEL_NUL_SAFE_STRING(prompt_title)
	EXCEL_NUL_SAFE_STRING(prompt)
	EXCEL_NUL_SAFE_STRING(error_title)
	EXCEL_NUL_SAFE_STRING(error)
	EXCEL_VALIDATE_ROW_RANGE(row_first, row_last, object);
	EXCEL_VALIDATE_COL_RANGE(col_first, col_last, object);

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
	zval *object = ZEND_THIS;
	SheetHandle sheet;

	SHEET_FROM_OBJECT(sheet, object);
	xlSheetRemoveDataValidations(sheet);

	RETURN_TRUE;
}
/* }}} */

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
	if (zfmt) {
		FORMAT_FROM_OBJECT(format, zfmt);
	}

	RETURN_BOOL(xlSheetWriteRichStr(sheet, row, col, rs, format));
}

EXCEL_METHOD(Sheet, formControlSize)
{
	SheetHandle sheet;
	zval *object = ZEND_THIS;

	ZEND_PARSE_PARAMETERS_NONE();

	SHEET_FROM_OBJECT(sheet, object);

	RETURN_LONG(xlSheetFormControlSize(sheet));
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
	EXCEL_INIT_PARENT(fco, object);
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
	SheetHandle sheet;
	zval *object = ZEND_THIS;

	ZEND_PARSE_PARAMETERS_NONE();

	SHEET_FROM_OBJECT(sheet, object);

	xlSheetRemoveSelection(sheet);
	RETURN_TRUE;
}

EXCEL_METHOD(Sheet, tabColor)
{
	SheetHandle sheet;
	zval *object = ZEND_THIS;

	ZEND_PARSE_PARAMETERS_NONE();

	SHEET_FROM_OBJECT(sheet, object);

	RETURN_LONG(xlSheetTabColor(sheet));
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
	SHEET_FROM_OBJECT(sheet, object);

	if (f) {
		FORMAT_FROM_OBJECT(format, f);
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
	SHEET_FROM_OBJECT(sheet, object);

	if (f) {
		FORMAT_FROM_OBJECT(format, f);
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

	SHEET_FROM_OBJECT(sheet, object);

	th = xlSheetAddTable(sheet, ZSTR_VAL(name), rowFirst, rowLast, colFirst, colLast, hasHeaders, style);
	if (!th) {
		RETURN_FALSE;
	}

	ZVAL_OBJ(return_value, excel_object_new_table(excel_ce_table));
	to = Z_EXCEL_TABLE_OBJ_P(return_value);
	to->table = th;
	to->sheet = sheet;
	EXCEL_INIT_PARENT(to, object);
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
	EXCEL_INIT_PARENT(to, object);
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
	EXCEL_INIT_PARENT(to, object);
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
	EXCEL_INIT_PARENT(cfo, object);
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
	EXCEL_INIT_PARENT(cfo, object);
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

	RETURN_BOOL(xlSheetRemoveConditionalFormatting(sheet, index));
}

EXCEL_METHOD(Sheet, conditionalFormattingSize)
{
	SheetHandle sheet;
	zval *object = ZEND_THIS;

	ZEND_PARSE_PARAMETERS_NONE();

	SHEET_FROM_OBJECT(sheet, object);

	RETURN_LONG(xlSheetConditionalFormattingSize(sheet));
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

	fc = xlSheetFormControl(sheet, index);
	if (!fc) {
		zend_throw_exception(NULL, "Failed to get form control", 0);
		RETURN_THROWS();
	}

	obj->formcontrol = fc;
	obj->sheet = sheet;
	EXCEL_INIT_PARENT(obj, zsheet);
}

EXCEL_METHOD(FormControl, objectType)
{
	zval *object = ZEND_THIS;
	FormControlHandle fc;
	FORMCONTROL_FROM_OBJECT(fc, object);
	RETURN_LONG(xlFormControlObjectType(fc));
}

EXCEL_METHOD(FormControl, checked)
{
	zval *object = ZEND_THIS;
	FormControlHandle fc;
	FORMCONTROL_FROM_OBJECT(fc, object);
	RETURN_LONG(xlFormControlChecked(fc));
}

EXCEL_METHOD(FormControl, setChecked)
{
	zval *object = ZEND_THIS;
	FormControlHandle fc;
	zend_long val;
	if (zend_parse_parameters(ZEND_NUM_ARGS(), "l", &val) == FAILURE) {
		RETURN_FALSE;
	}
	FORMCONTROL_FROM_OBJECT(fc, object);
	xlFormControlSetChecked(fc, val);
	RETURN_TRUE;
}

EXCEL_METHOD(FormControl, fmlaGroup)
{
	zval *object = ZEND_THIS;
	FormControlHandle fc;
	const char *result;
	FORMCONTROL_FROM_OBJECT(fc, object);
	result = xlFormControlFmlaGroup(fc);
	PE_RETURN_IS_STRING(result)
}

EXCEL_METHOD(FormControl, setFmlaGroup)
{
	zval *object = ZEND_THIS;
	FormControlHandle fc;
	zend_string *val;
	if (zend_parse_parameters(ZEND_NUM_ARGS(), "S", &val) == FAILURE) {
		RETURN_FALSE;
	}
	EXCEL_NUL_SAFE_STRING(val)
	FORMCONTROL_FROM_OBJECT(fc, object);
	xlFormControlSetFmlaGroup(fc, ZSTR_VAL(val));
	RETURN_TRUE;
}

EXCEL_METHOD(FormControl, fmlaLink)
{
	zval *object = ZEND_THIS;
	FormControlHandle fc;
	const char *result;
	FORMCONTROL_FROM_OBJECT(fc, object);
	result = xlFormControlFmlaLink(fc);
	PE_RETURN_IS_STRING(result)
}

EXCEL_METHOD(FormControl, setFmlaLink)
{
	zval *object = ZEND_THIS;
	FormControlHandle fc;
	zend_string *val;
	if (zend_parse_parameters(ZEND_NUM_ARGS(), "S", &val) == FAILURE) {
		RETURN_FALSE;
	}
	EXCEL_NUL_SAFE_STRING(val)
	FORMCONTROL_FROM_OBJECT(fc, object);
	xlFormControlSetFmlaLink(fc, ZSTR_VAL(val));
	RETURN_TRUE;
}

EXCEL_METHOD(FormControl, fmlaRange)
{
	zval *object = ZEND_THIS;
	FormControlHandle fc;
	const char *result;
	FORMCONTROL_FROM_OBJECT(fc, object);
	result = xlFormControlFmlaRange(fc);
	PE_RETURN_IS_STRING(result)
}

EXCEL_METHOD(FormControl, setFmlaRange)
{
	zval *object = ZEND_THIS;
	FormControlHandle fc;
	zend_string *val;
	if (zend_parse_parameters(ZEND_NUM_ARGS(), "S", &val) == FAILURE) {
		RETURN_FALSE;
	}
	EXCEL_NUL_SAFE_STRING(val)
	FORMCONTROL_FROM_OBJECT(fc, object);
	xlFormControlSetFmlaRange(fc, ZSTR_VAL(val));
	RETURN_TRUE;
}

EXCEL_METHOD(FormControl, fmlaTxbx)
{
	zval *object = ZEND_THIS;
	FormControlHandle fc;
	const char *result;
	FORMCONTROL_FROM_OBJECT(fc, object);
	result = xlFormControlFmlaTxbx(fc);
	PE_RETURN_IS_STRING(result)
}

EXCEL_METHOD(FormControl, setFmlaTxbx)
{
	zval *object = ZEND_THIS;
	FormControlHandle fc;
	zend_string *val;
	if (zend_parse_parameters(ZEND_NUM_ARGS(), "S", &val) == FAILURE) {
		RETURN_FALSE;
	}
	EXCEL_NUL_SAFE_STRING(val)
	FORMCONTROL_FROM_OBJECT(fc, object);
	xlFormControlSetFmlaTxbx(fc, ZSTR_VAL(val));
	RETURN_TRUE;
}

EXCEL_METHOD(FormControl, name)
{
	zval *object = ZEND_THIS;
	FormControlHandle fc;
	const char *result;
	FORMCONTROL_FROM_OBJECT(fc, object);
	result = xlFormControlName(fc);
	PE_RETURN_IS_STRING(result)
}

EXCEL_METHOD(FormControl, linkedCell)
{
	zval *object = ZEND_THIS;
	FormControlHandle fc;
	const char *result;
	FORMCONTROL_FROM_OBJECT(fc, object);
	result = xlFormControlLinkedCell(fc);
	PE_RETURN_IS_STRING(result)
}

EXCEL_METHOD(FormControl, listFillRange)
{
	zval *object = ZEND_THIS;
	FormControlHandle fc;
	const char *result;
	FORMCONTROL_FROM_OBJECT(fc, object);
	result = xlFormControlListFillRange(fc);
	PE_RETURN_IS_STRING(result)
}

EXCEL_METHOD(FormControl, macro)
{
	zval *object = ZEND_THIS;
	FormControlHandle fc;
	const char *result;
	FORMCONTROL_FROM_OBJECT(fc, object);
	result = xlFormControlMacro(fc);
	PE_RETURN_IS_STRING(result)
}

EXCEL_METHOD(FormControl, altText)
{
	zval *object = ZEND_THIS;
	FormControlHandle fc;
	const char *result;
	FORMCONTROL_FROM_OBJECT(fc, object);
	result = xlFormControlAltText(fc);
	PE_RETURN_IS_STRING(result)
}

EXCEL_METHOD(FormControl, locked)
{
	zval *object = ZEND_THIS;
	FormControlHandle fc;
	FORMCONTROL_FROM_OBJECT(fc, object);
	RETURN_BOOL(xlFormControlLocked(fc));
}

EXCEL_METHOD(FormControl, defaultSize)
{
	zval *object = ZEND_THIS;
	FormControlHandle fc;
	FORMCONTROL_FROM_OBJECT(fc, object);
	RETURN_BOOL(xlFormControlDefaultSize(fc));
}

EXCEL_METHOD(FormControl, print)
{
	zval *object = ZEND_THIS;
	FormControlHandle fc;
	FORMCONTROL_FROM_OBJECT(fc, object);
	RETURN_BOOL(xlFormControlPrint(fc));
}

EXCEL_METHOD(FormControl, disabled)
{
	zval *object = ZEND_THIS;
	FormControlHandle fc;
	FORMCONTROL_FROM_OBJECT(fc, object);
	RETURN_BOOL(xlFormControlDisabled(fc));
}

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

EXCEL_METHOD(FormControl, itemSize)
{
	zval *object = ZEND_THIS;
	FormControlHandle fc;
	FORMCONTROL_FROM_OBJECT(fc, object);
	RETURN_LONG(xlFormControlItemSize(fc));
}

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
	FORMCONTROL_FROM_OBJECT(fc, object);
	xlFormControlClearItems(fc);
	RETURN_TRUE;
}

EXCEL_METHOD(FormControl, dropLines)
{
	zval *object = ZEND_THIS;
	FormControlHandle fc;
	FORMCONTROL_FROM_OBJECT(fc, object);
	RETURN_LONG(xlFormControlDropLines(fc));
}

EXCEL_METHOD(FormControl, setDropLines)
{
	zval *object = ZEND_THIS;
	FormControlHandle fc;
	zend_long val;
	if (zend_parse_parameters(ZEND_NUM_ARGS(), "l", &val) == FAILURE) {
		RETURN_FALSE;
	}
	FORMCONTROL_FROM_OBJECT(fc, object);
	xlFormControlSetDropLines(fc, val);
	RETURN_TRUE;
}

EXCEL_METHOD(FormControl, dx)
{
	zval *object = ZEND_THIS;
	FormControlHandle fc;
	FORMCONTROL_FROM_OBJECT(fc, object);
	RETURN_LONG(xlFormControlDx(fc));
}

EXCEL_METHOD(FormControl, setDx)
{
	zval *object = ZEND_THIS;
	FormControlHandle fc;
	zend_long val;
	if (zend_parse_parameters(ZEND_NUM_ARGS(), "l", &val) == FAILURE) {
		RETURN_FALSE;
	}
	FORMCONTROL_FROM_OBJECT(fc, object);
	xlFormControlSetDx(fc, val);
	RETURN_TRUE;
}

EXCEL_METHOD(FormControl, firstButton)
{
	zval *object = ZEND_THIS;
	FormControlHandle fc;
	FORMCONTROL_FROM_OBJECT(fc, object);
	RETURN_BOOL(xlFormControlFirstButton(fc));
}

EXCEL_METHOD(FormControl, setFirstButton)
{
	zval *object = ZEND_THIS;
	FormControlHandle fc;
	bool val;
	if (zend_parse_parameters(ZEND_NUM_ARGS(), "b", &val) == FAILURE) {
		RETURN_FALSE;
	}
	FORMCONTROL_FROM_OBJECT(fc, object);
	xlFormControlSetFirstButton(fc, val);
	RETURN_TRUE;
}

EXCEL_METHOD(FormControl, horiz)
{
	zval *object = ZEND_THIS;
	FormControlHandle fc;
	FORMCONTROL_FROM_OBJECT(fc, object);
	RETURN_BOOL(xlFormControlHoriz(fc));
}

EXCEL_METHOD(FormControl, setHoriz)
{
	zval *object = ZEND_THIS;
	FormControlHandle fc;
	bool val;
	if (zend_parse_parameters(ZEND_NUM_ARGS(), "b", &val) == FAILURE) {
		RETURN_FALSE;
	}
	FORMCONTROL_FROM_OBJECT(fc, object);
	xlFormControlSetHoriz(fc, val);
	RETURN_TRUE;
}

EXCEL_METHOD(FormControl, inc)
{
	zval *object = ZEND_THIS;
	FormControlHandle fc;
	FORMCONTROL_FROM_OBJECT(fc, object);
	RETURN_LONG(xlFormControlInc(fc));
}

EXCEL_METHOD(FormControl, setInc)
{
	zval *object = ZEND_THIS;
	FormControlHandle fc;
	zend_long val;
	if (zend_parse_parameters(ZEND_NUM_ARGS(), "l", &val) == FAILURE) {
		RETURN_FALSE;
	}
	FORMCONTROL_FROM_OBJECT(fc, object);
	xlFormControlSetInc(fc, val);
	RETURN_TRUE;
}

EXCEL_METHOD(FormControl, getMax)
{
	zval *object = ZEND_THIS;
	FormControlHandle fc;
	FORMCONTROL_FROM_OBJECT(fc, object);
	RETURN_LONG(xlFormControlGetMax(fc));
}

EXCEL_METHOD(FormControl, setMax)
{
	zval *object = ZEND_THIS;
	FormControlHandle fc;
	zend_long val;
	if (zend_parse_parameters(ZEND_NUM_ARGS(), "l", &val) == FAILURE) {
		RETURN_FALSE;
	}
	FORMCONTROL_FROM_OBJECT(fc, object);
	xlFormControlSetMax(fc, val);
	RETURN_TRUE;
}

EXCEL_METHOD(FormControl, getMin)
{
	zval *object = ZEND_THIS;
	FormControlHandle fc;
	FORMCONTROL_FROM_OBJECT(fc, object);
	RETURN_LONG(xlFormControlGetMin(fc));
}

EXCEL_METHOD(FormControl, setMin)
{
	zval *object = ZEND_THIS;
	FormControlHandle fc;
	zend_long val;
	if (zend_parse_parameters(ZEND_NUM_ARGS(), "l", &val) == FAILURE) {
		RETURN_FALSE;
	}
	FORMCONTROL_FROM_OBJECT(fc, object);
	xlFormControlSetMin(fc, val);
	RETURN_TRUE;
}

EXCEL_METHOD(FormControl, multiSel)
{
	zval *object = ZEND_THIS;
	FormControlHandle fc;
	const char *result;
	FORMCONTROL_FROM_OBJECT(fc, object);
	result = xlFormControlMultiSel(fc);
	PE_RETURN_IS_STRING(result)
}

EXCEL_METHOD(FormControl, setMultiSel)
{
	zval *object = ZEND_THIS;
	FormControlHandle fc;
	zend_string *val;
	if (zend_parse_parameters(ZEND_NUM_ARGS(), "S", &val) == FAILURE) {
		RETURN_FALSE;
	}
	EXCEL_NUL_SAFE_STRING(val)
	FORMCONTROL_FROM_OBJECT(fc, object);
	xlFormControlSetMultiSel(fc, ZSTR_VAL(val));
	RETURN_TRUE;
}

EXCEL_METHOD(FormControl, sel)
{
	zval *object = ZEND_THIS;
	FormControlHandle fc;
	FORMCONTROL_FROM_OBJECT(fc, object);
	RETURN_LONG(xlFormControlSel(fc));
}

EXCEL_METHOD(FormControl, setSel)
{
	zval *object = ZEND_THIS;
	FormControlHandle fc;
	zend_long val;
	if (zend_parse_parameters(ZEND_NUM_ARGS(), "l", &val) == FAILURE) {
		RETURN_FALSE;
	}
	FORMCONTROL_FROM_OBJECT(fc, object);
	xlFormControlSetSel(fc, val);
	RETURN_TRUE;
}

EXCEL_METHOD(FormControl, fromAnchor)
{
	zval *object = ZEND_THIS;
	FormControlHandle fc;
	int col = 0, colOff = 0, row = 0, rowOff = 0;

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

EXCEL_METHOD(ConditionalFormat, numFormat)
{
	zval *object = ZEND_THIS;
	ConditionalFormatHandle cf;
	CONDITIONALFORMAT_FROM_OBJECT(cf, object);
	RETURN_LONG(xlConditionalFormatNumFormat(cf));
}

EXCEL_METHOD(ConditionalFormat, setNumFormat)
{
	zval *object = ZEND_THIS;
	ConditionalFormatHandle cf;
	zend_long val;
	if (zend_parse_parameters(ZEND_NUM_ARGS(), "l", &val) == FAILURE) {
		RETURN_FALSE;
	}
	CONDITIONALFORMAT_FROM_OBJECT(cf, object);
	xlConditionalFormatSetNumFormat(cf, val);
	RETURN_TRUE;
}

EXCEL_METHOD(ConditionalFormat, customNumFormat)
{
	zval *object = ZEND_THIS;
	ConditionalFormatHandle cf;
	const char *result;
	CONDITIONALFORMAT_FROM_OBJECT(cf, object);
	result = xlConditionalFormatCustomNumFormat(cf);
	PE_RETURN_IS_STRING(result)
}

EXCEL_METHOD(ConditionalFormat, setCustomNumFormat)
{
	zval *object = ZEND_THIS;
	ConditionalFormatHandle cf;
	zend_string *val;
	if (zend_parse_parameters(ZEND_NUM_ARGS(), "S", &val) == FAILURE) {
		RETURN_FALSE;
	}
	EXCEL_NUL_SAFE_STRING(val)
	CONDITIONALFORMAT_FROM_OBJECT(cf, object);
	xlConditionalFormatSetCustomNumFormat(cf, ZSTR_VAL(val));
	RETURN_TRUE;
}

EXCEL_METHOD(ConditionalFormat, setBorder)
{
	zval *object = ZEND_THIS;
	ConditionalFormatHandle cf;
	zend_long val;
	if (zend_parse_parameters(ZEND_NUM_ARGS(), "l", &val) == FAILURE) {
		RETURN_FALSE;
	}
	CONDITIONALFORMAT_FROM_OBJECT(cf, object);
	xlConditionalFormatSetBorder(cf, val);
	RETURN_TRUE;
}

EXCEL_METHOD(ConditionalFormat, setBorderColor)
{
	zval *object = ZEND_THIS;
	ConditionalFormatHandle cf;
	zend_long val;
	if (zend_parse_parameters(ZEND_NUM_ARGS(), "l", &val) == FAILURE) {
		RETURN_FALSE;
	}
	CONDITIONALFORMAT_FROM_OBJECT(cf, object);
	xlConditionalFormatSetBorderColor(cf, val);
	RETURN_TRUE;
}

EXCEL_METHOD(ConditionalFormat, borderLeft)
{
	zval *object = ZEND_THIS;
	ConditionalFormatHandle cf;
	CONDITIONALFORMAT_FROM_OBJECT(cf, object);
	RETURN_LONG(xlConditionalFormatBorderLeft(cf));
}

EXCEL_METHOD(ConditionalFormat, setBorderLeft)
{
	zval *object = ZEND_THIS;
	ConditionalFormatHandle cf;
	zend_long val;
	if (zend_parse_parameters(ZEND_NUM_ARGS(), "l", &val) == FAILURE) {
		RETURN_FALSE;
	}
	CONDITIONALFORMAT_FROM_OBJECT(cf, object);
	xlConditionalFormatSetBorderLeft(cf, val);
	RETURN_TRUE;
}

EXCEL_METHOD(ConditionalFormat, borderRight)
{
	zval *object = ZEND_THIS;
	ConditionalFormatHandle cf;
	CONDITIONALFORMAT_FROM_OBJECT(cf, object);
	RETURN_LONG(xlConditionalFormatBorderRight(cf));
}

EXCEL_METHOD(ConditionalFormat, setBorderRight)
{
	zval *object = ZEND_THIS;
	ConditionalFormatHandle cf;
	zend_long val;
	if (zend_parse_parameters(ZEND_NUM_ARGS(), "l", &val) == FAILURE) {
		RETURN_FALSE;
	}
	CONDITIONALFORMAT_FROM_OBJECT(cf, object);
	xlConditionalFormatSetBorderRight(cf, val);
	RETURN_TRUE;
}

EXCEL_METHOD(ConditionalFormat, borderTop)
{
	zval *object = ZEND_THIS;
	ConditionalFormatHandle cf;
	CONDITIONALFORMAT_FROM_OBJECT(cf, object);
	RETURN_LONG(xlConditionalFormatBorderTop(cf));
}

EXCEL_METHOD(ConditionalFormat, setBorderTop)
{
	zval *object = ZEND_THIS;
	ConditionalFormatHandle cf;
	zend_long val;
	if (zend_parse_parameters(ZEND_NUM_ARGS(), "l", &val) == FAILURE) {
		RETURN_FALSE;
	}
	CONDITIONALFORMAT_FROM_OBJECT(cf, object);
	xlConditionalFormatSetBorderTop(cf, val);
	RETURN_TRUE;
}

EXCEL_METHOD(ConditionalFormat, borderBottom)
{
	zval *object = ZEND_THIS;
	ConditionalFormatHandle cf;
	CONDITIONALFORMAT_FROM_OBJECT(cf, object);
	RETURN_LONG(xlConditionalFormatBorderBottom(cf));
}

EXCEL_METHOD(ConditionalFormat, setBorderBottom)
{
	zval *object = ZEND_THIS;
	ConditionalFormatHandle cf;
	zend_long val;
	if (zend_parse_parameters(ZEND_NUM_ARGS(), "l", &val) == FAILURE) {
		RETURN_FALSE;
	}
	CONDITIONALFORMAT_FROM_OBJECT(cf, object);
	xlConditionalFormatSetBorderBottom(cf, val);
	RETURN_TRUE;
}

EXCEL_METHOD(ConditionalFormat, borderLeftColor)
{
	zval *object = ZEND_THIS;
	ConditionalFormatHandle cf;
	CONDITIONALFORMAT_FROM_OBJECT(cf, object);
	RETURN_LONG(xlConditionalFormatBorderLeftColor(cf));
}

EXCEL_METHOD(ConditionalFormat, setBorderLeftColor)
{
	zval *object = ZEND_THIS;
	ConditionalFormatHandle cf;
	zend_long val;
	if (zend_parse_parameters(ZEND_NUM_ARGS(), "l", &val) == FAILURE) {
		RETURN_FALSE;
	}
	CONDITIONALFORMAT_FROM_OBJECT(cf, object);
	xlConditionalFormatSetBorderLeftColor(cf, val);
	RETURN_TRUE;
}

EXCEL_METHOD(ConditionalFormat, borderRightColor)
{
	zval *object = ZEND_THIS;
	ConditionalFormatHandle cf;
	CONDITIONALFORMAT_FROM_OBJECT(cf, object);
	RETURN_LONG(xlConditionalFormatBorderRightColor(cf));
}

EXCEL_METHOD(ConditionalFormat, setBorderRightColor)
{
	zval *object = ZEND_THIS;
	ConditionalFormatHandle cf;
	zend_long val;
	if (zend_parse_parameters(ZEND_NUM_ARGS(), "l", &val) == FAILURE) {
		RETURN_FALSE;
	}
	CONDITIONALFORMAT_FROM_OBJECT(cf, object);
	xlConditionalFormatSetBorderRightColor(cf, val);
	RETURN_TRUE;
}

EXCEL_METHOD(ConditionalFormat, borderTopColor)
{
	zval *object = ZEND_THIS;
	ConditionalFormatHandle cf;
	CONDITIONALFORMAT_FROM_OBJECT(cf, object);
	RETURN_LONG(xlConditionalFormatBorderTopColor(cf));
}

EXCEL_METHOD(ConditionalFormat, setBorderTopColor)
{
	zval *object = ZEND_THIS;
	ConditionalFormatHandle cf;
	zend_long val;
	if (zend_parse_parameters(ZEND_NUM_ARGS(), "l", &val) == FAILURE) {
		RETURN_FALSE;
	}
	CONDITIONALFORMAT_FROM_OBJECT(cf, object);
	xlConditionalFormatSetBorderTopColor(cf, val);
	RETURN_TRUE;
}

EXCEL_METHOD(ConditionalFormat, borderBottomColor)
{
	zval *object = ZEND_THIS;
	ConditionalFormatHandle cf;
	CONDITIONALFORMAT_FROM_OBJECT(cf, object);
	RETURN_LONG(xlConditionalFormatBorderBottomColor(cf));
}

EXCEL_METHOD(ConditionalFormat, setBorderBottomColor)
{
	zval *object = ZEND_THIS;
	ConditionalFormatHandle cf;
	zend_long val;
	if (zend_parse_parameters(ZEND_NUM_ARGS(), "l", &val) == FAILURE) {
		RETURN_FALSE;
	}
	CONDITIONALFORMAT_FROM_OBJECT(cf, object);
	xlConditionalFormatSetBorderBottomColor(cf, val);
	RETURN_TRUE;
}

EXCEL_METHOD(ConditionalFormat, fillPattern)
{
	zval *object = ZEND_THIS;
	ConditionalFormatHandle cf;
	CONDITIONALFORMAT_FROM_OBJECT(cf, object);
	RETURN_LONG(xlConditionalFormatFillPattern(cf));
}

EXCEL_METHOD(ConditionalFormat, setFillPattern)
{
	zval *object = ZEND_THIS;
	ConditionalFormatHandle cf;
	zend_long val;
	if (zend_parse_parameters(ZEND_NUM_ARGS(), "l", &val) == FAILURE) {
		RETURN_FALSE;
	}
	CONDITIONALFORMAT_FROM_OBJECT(cf, object);
	xlConditionalFormatSetFillPattern(cf, val);
	RETURN_TRUE;
}

EXCEL_METHOD(ConditionalFormat, patternForegroundColor)
{
	zval *object = ZEND_THIS;
	ConditionalFormatHandle cf;
	CONDITIONALFORMAT_FROM_OBJECT(cf, object);
	RETURN_LONG(xlConditionalFormatPatternForegroundColor(cf));
}

EXCEL_METHOD(ConditionalFormat, setPatternForegroundColor)
{
	zval *object = ZEND_THIS;
	ConditionalFormatHandle cf;
	zend_long val;
	if (zend_parse_parameters(ZEND_NUM_ARGS(), "l", &val) == FAILURE) {
		RETURN_FALSE;
	}
	CONDITIONALFORMAT_FROM_OBJECT(cf, object);
	xlConditionalFormatSetPatternForegroundColor(cf, val);
	RETURN_TRUE;
}

EXCEL_METHOD(ConditionalFormat, patternBackgroundColor)
{
	zval *object = ZEND_THIS;
	ConditionalFormatHandle cf;
	CONDITIONALFORMAT_FROM_OBJECT(cf, object);
	RETURN_LONG(xlConditionalFormatPatternBackgroundColor(cf));
}

EXCEL_METHOD(ConditionalFormat, setPatternBackgroundColor)
{
	zval *object = ZEND_THIS;
	ConditionalFormatHandle cf;
	zend_long val;
	if (zend_parse_parameters(ZEND_NUM_ARGS(), "l", &val) == FAILURE) {
		RETURN_FALSE;
	}
	CONDITIONALFORMAT_FROM_OBJECT(cf, object);
	xlConditionalFormatSetPatternBackgroundColor(cf, val);
	RETURN_TRUE;
}

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
		if (colFirst < 0 || colFirst > _maxc || colLast < 0 || colLast > _maxc) {
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
	EXCEL_INIT_PARENT(obj, zsheet);
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

	EXCEL_NUL_SAFE_STRING(value)

	CONDITIONALFORMATTING_FROM_OBJECT(cfing, object);
	CONDITIONALFORMAT_FROM_OBJECT(cf, zcf);

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

	CONDITIONALFORMATTING_FROM_OBJECT(cfing, object);
	CONDITIONALFORMAT_FROM_OBJECT(cf, zcf);

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

	CONDITIONALFORMATTING_FROM_OBJECT(cfing, object);
	CONDITIONALFORMAT_FROM_OBJECT(cf, zcf);

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

	EXCEL_NUL_SAFE_STRING(v1)
	EXCEL_NUL_SAFE_STRING(v2)

	CONDITIONALFORMATTING_FROM_OBJECT(cfing, object);
	CONDITIONALFORMAT_FROM_OBJECT(cf, zcf);

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

	CONDITIONALFORMATTING_FROM_OBJECT(cfing, object);
	CONDITIONALFORMAT_FROM_OBJECT(cf, zcf);

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

	CONDITIONALFORMATTING_FROM_OBJECT(cfing, object);
	CONDITIONALFORMAT_FROM_OBJECT(cf, zcf);

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
	COREPROPERTIES_FROM_OBJECT(cp, object);
	xlCorePropertiesSetCreatedAsDouble(cp, val);
	RETURN_TRUE;
}

EXCEL_METHOD(CoreProperties, modifiedAsDouble)
{
	zval *object = ZEND_THIS;
	CorePropertiesHandle cp;
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
	COREPROPERTIES_FROM_OBJECT(cp, object);
	xlCorePropertiesSetModifiedAsDouble(cp, val);
	RETURN_TRUE;
}

EXCEL_METHOD(CoreProperties, removeAll)
{
	zval *object = ZEND_THIS;
	CorePropertiesHandle cp;
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
	}

	SHEET_FROM_OBJECT_THROW(sheet, zsheet);

	obj = Z_EXCEL_TABLE_OBJ_P(object);

	th = xlSheetAddTable(sheet, ZSTR_VAL(name), rowFirst, rowLast, colFirst, colLast, hasHeaders, style);
	if (!th) {
		zend_throw_exception(NULL, "Failed to create table", 0);
		RETURN_THROWS();
	}

	obj->table = th;
	obj->sheet = sheet;
	EXCEL_INIT_PARENT(obj, zsheet);
}

EXCEL_METHOD(Table, name)
{
	zval *object = ZEND_THIS;
	TableHandle table;
	const char *result;
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
	EXCEL_INIT_PARENT(aobj, object);
}

EXCEL_METHOD(Table, style)
{
	zval *object = ZEND_THIS;
	TableHandle table;
	TABLE_FROM_OBJECT(table, object);
	RETURN_LONG(xlTableStyle(table));
}

EXCEL_METHOD(Table, setStyle)
{
	zval *object = ZEND_THIS;
	TableHandle table;
	zend_long val;
	if (zend_parse_parameters(ZEND_NUM_ARGS(), "l", &val) == FAILURE) {
		RETURN_FALSE;
	}
	TABLE_FROM_OBJECT(table, object);
	xlTableSetStyle(table, val);
	RETURN_TRUE;
}

EXCEL_METHOD(Table, showRowStripes)
{
	zval *object = ZEND_THIS;
	TableHandle table;
	TABLE_FROM_OBJECT(table, object);
	RETURN_BOOL(xlTableShowRowStripes(table));
}

EXCEL_METHOD(Table, setShowRowStripes)
{
	zval *object = ZEND_THIS;
	TableHandle table;
	bool val;
	if (zend_parse_parameters(ZEND_NUM_ARGS(), "b", &val) == FAILURE) {
		RETURN_FALSE;
	}
	TABLE_FROM_OBJECT(table, object);
	xlTableSetShowRowStripes(table, val);
	RETURN_TRUE;
}

EXCEL_METHOD(Table, showColumnStripes)
{
	zval *object = ZEND_THIS;
	TableHandle table;
	TABLE_FROM_OBJECT(table, object);
	RETURN_BOOL(xlTableShowColumnStripes(table));
}

EXCEL_METHOD(Table, setShowColumnStripes)
{
	zval *object = ZEND_THIS;
	TableHandle table;
	bool val;
	if (zend_parse_parameters(ZEND_NUM_ARGS(), "b", &val) == FAILURE) {
		RETURN_FALSE;
	}
	TABLE_FROM_OBJECT(table, object);
	xlTableSetShowColumnStripes(table, val);
	RETURN_TRUE;
}

EXCEL_METHOD(Table, showFirstColumn)
{
	zval *object = ZEND_THIS;
	TableHandle table;
	TABLE_FROM_OBJECT(table, object);
	RETURN_BOOL(xlTableShowFirstColumn(table));
}

EXCEL_METHOD(Table, setShowFirstColumn)
{
	zval *object = ZEND_THIS;
	TableHandle table;
	bool val;
	if (zend_parse_parameters(ZEND_NUM_ARGS(), "b", &val) == FAILURE) {
		RETURN_FALSE;
	}
	TABLE_FROM_OBJECT(table, object);
	xlTableSetShowFirstColumn(table, val);
	RETURN_TRUE;
}

EXCEL_METHOD(Table, showLastColumn)
{
	zval *object = ZEND_THIS;
	TableHandle table;
	TABLE_FROM_OBJECT(table, object);
	RETURN_BOOL(xlTableShowLastColumn(table));
}

EXCEL_METHOD(Table, setShowLastColumn)
{
	zval *object = ZEND_THIS;
	TableHandle table;
	bool val;
	if (zend_parse_parameters(ZEND_NUM_ARGS(), "b", &val) == FAILURE) {
		RETURN_FALSE;
	}
	TABLE_FROM_OBJECT(table, object);
	xlTableSetShowLastColumn(table, val);
	RETURN_TRUE;
}

EXCEL_METHOD(Table, columnSize)
{
	zval *object = ZEND_THIS;
	TableHandle table;
	TABLE_FROM_OBJECT(table, object);
	RETURN_LONG(xlTableColumnSize(table));
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
	NULL,
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
