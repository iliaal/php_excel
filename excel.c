/*
  +----------------------------------------------------------------------+
  | PHP Version 5                                                        |
  +----------------------------------------------------------------------+
  | Copyright (c) 1997-2014 The PHP Group                                |
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
#include <stdlib.h>

#include "php.h"
#include "php_ini.h"
#include "ext/standard/info.h"
#include "ext/date/php_date.h"

#if defined(HAVE_XML) && defined(EXCEL_WITH_LIBXML)
#include "ext/xml/php_xml.h"
#endif

#include "php_excel.h"
#include "zend_exceptions.h"

static long xlFormatBorder(FormatHandle f)
{
	return 1;
}
static long xlFormatBorderColor(FormatHandle f)
{
	return 1;
}

/* work-around for missing headers in LibXL */

#ifndef LIBXL_VERSION

#define xlSheetSetProtect xlSheetSetProtectA
#ifndef HAVE_LIBXL_243_PLUS
#define xlSheetProtect xlSheetProtectA
#endif

#endif

#if LIBXL_VERSION >= 0x03020000
#define xlBookSetRefR1C1 xlBookSetRefR1C1A
#define xlBookRefR1C1 xlBookRefR1C1A
#endif

#if LIBXL_VERSION >= 0x03020000 && LIBXL_VERSION < 0x03050401
enum libXLPictureType {PICTURETYPE_PNG, PICTURETYPE_JPEG, PICTURETYPE_WMF, PICTURETYPE_DIB, PICTURETYPE_EMF, PICTURETYPE_PICT, PICTURETYPE_TIFF, PICTURETYPE_ERROR = 0xFF};
#endif

#define PHP_EXCEL_DATE 1
#define PHP_EXCEL_FORMULA 2
#define PHP_EXCEL_NUMERIC_STRING 3

#define PHP_EXCEL_VERSION "1.0.1dev"

#ifdef COMPILE_DL_EXCEL
ZEND_GET_MODULE(excel)
#endif

ZEND_DECLARE_MODULE_GLOBALS(excel)

static PHP_GINIT_FUNCTION(excel);

PHP_INI_BEGIN()
	STD_PHP_INI_ENTRY("excel.license_name", NULL, PHP_INI_ALL, OnUpdateString, ini_license_name, zend_excel_globals, excel_globals)
	STD_PHP_INI_ENTRY("excel.license_key", NULL, PHP_INI_ALL, OnUpdateString, ini_license_key, zend_excel_globals, excel_globals)
	STD_PHP_INI_ENTRY("excel.skip_empty", "0", PHP_INI_ALL, OnUpdateLong, ini_skip_empty, zend_excel_globals, excel_globals)
PHP_INI_END()

/* {{{ OO init/structure stuff */
#define REGISTER_EXCEL_CLASS(name, c_name, clone) \
	{ \
		zend_class_entry ce; \
		INIT_CLASS_ENTRY(ce, "Excel" # name, excel_funcs_ ## c_name); \
		ce.create_object = excel_object_new_ ## c_name; \
		excel_ce_ ## c_name = zend_register_internal_class_ex(&ce, NULL, NULL TSRMLS_CC); \
		memcpy(&excel_object_handlers_ ## c_name, zend_get_std_object_handlers(), sizeof(zend_object_handlers)); \
		excel_object_handlers_ ## c_name.clone_obj = clone; \
	}

zend_class_entry *excel_ce_book, *excel_ce_sheet, *excel_ce_format, *excel_ce_font;

static zend_object_handlers excel_object_handlers_book;
static zend_object_handlers excel_object_handlers_sheet;
static zend_object_handlers excel_object_handlers_format;
static zend_object_handlers excel_object_handlers_font;

typedef struct _excel_book_object {
	zend_object	std;
	BookHandle book;
} excel_book_object;

#define BOOK_FROM_OBJECT(book, object) \
	{ \
		excel_book_object *obj = (excel_book_object*) zend_object_store_get_object(object TSRMLS_CC); \
		book = obj->book; \
		if (!book) { \
			php_error_docref(NULL TSRMLS_CC, E_WARNING, "The book wasn't initialized"); \
			RETURN_FALSE; \
		} \
	}

typedef struct _excel_sheet_object {
	zend_object	std;
	SheetHandle	sheet;
	BookHandle book;
} excel_sheet_object;

#define SHEET_FROM_OBJECT(sheet, object) \
	{ \
		excel_sheet_object *obj = (excel_sheet_object*) zend_object_store_get_object(object TSRMLS_CC); \
		sheet = obj->sheet; \
		if (!sheet) { \
			php_error_docref(NULL TSRMLS_CC, E_WARNING, "The sheet wasn't initialized"); \
			RETURN_FALSE; \
		} \
	}

#define SHEET_AND_BOOK_FROM_OBJECT(sheet, book, object) \
	{ \
		excel_sheet_object *obj = (excel_sheet_object*) zend_object_store_get_object(object TSRMLS_CC); \
		sheet = obj->sheet; \
		book = obj->book; \
		if (!sheet) { \
			php_error_docref(NULL TSRMLS_CC, E_WARNING, "The sheet wasn't initialized"); \
			RETURN_FALSE; \
		} \
	}

#define FONT_FROM_OBJECT(font, object) \
	{ \
		excel_font_object *obj = (excel_font_object*) zend_object_store_get_object(object TSRMLS_CC); \
		font = obj->font; \
		if (!font) { \
			php_error_docref(NULL TSRMLS_CC, E_WARNING, "The font wasn't initialized"); \
			RETURN_FALSE; \
		} \
	}

typedef struct _excel_font_object {
	zend_object	std;
	FontHandle font;
	BookHandle book;
} excel_font_object;

#define FORMAT_FROM_OBJECT(format, object) \
	{ \
		excel_format_object *obj = (excel_format_object*) zend_object_store_get_object(object TSRMLS_CC); \
		format = obj->format; \
		if (!format) { \
			php_error_docref(NULL TSRMLS_CC, E_WARNING, "The format wasn't initialized"); \
			RETURN_FALSE; \
		} \
	}

typedef struct _excel_format_object {
	zend_object	std;
	FormatHandle format;
	BookHandle book;
} excel_format_object;

static void excel_book_object_free_storage(void *object TSRMLS_DC)
{
	excel_book_object *intern = (excel_book_object *)object;

	zend_object_std_dtor(&intern->std TSRMLS_CC);

	if (intern->book) {
		xlBookRelease(intern->book);
		intern->book = NULL;
	}

	efree(object);
}

static zend_object_value excel_object_new_book(zend_class_entry *class_type TSRMLS_DC)
{
	excel_book_object *intern;
	zend_object_value retval;

	intern = emalloc(sizeof(excel_book_object));
	memset(intern, 0, sizeof(excel_book_object));

	zend_object_std_init(&intern->std, class_type TSRMLS_CC);
#ifdef ZEND_ENGINE_2_4
	object_properties_init(&intern->std, class_type);
#else
{
	zval *tmp;
	zend_hash_copy(intern->std.properties, &class_type->default_properties, (copy_ctor_func_t) zval_add_ref, (void *) &tmp, sizeof(zval *));
}
#endif
	intern->book = xlCreateBook();

	(&retval)->handle = zend_objects_store_put(intern, (zend_objects_store_dtor_t)zend_objects_destroy_object, (zend_objects_free_object_storage_t) excel_book_object_free_storage, NULL TSRMLS_CC);
	(&retval)->handlers = &excel_object_handlers_book;

	return retval;
}

static void excel_sheet_object_free_storage(void *object TSRMLS_DC)
{
	excel_sheet_object *intern = (excel_sheet_object *)object;

	zend_object_std_dtor(&intern->std TSRMLS_CC);

	efree(object);
}

static zend_object_value excel_object_new_sheet(zend_class_entry *class_type TSRMLS_DC)
{
	excel_sheet_object *intern;
	zend_object_value retval;
	intern = emalloc(sizeof(excel_sheet_object));
	memset(intern, 0, sizeof(excel_sheet_object));

	zend_object_std_init(&intern->std, class_type TSRMLS_CC);
#ifdef ZEND_ENGINE_2_4
	object_properties_init(&intern->std, class_type);
#else
	{
		zval *tmp;
		zend_hash_copy(intern->std.properties, &class_type->default_properties, (copy_ctor_func_t) zval_add_ref, (void *) &tmp, sizeof(zval *));
	}
#endif
	(&retval)->handle = zend_objects_store_put(intern, (zend_objects_store_dtor_t)zend_objects_destroy_object, (zend_objects_free_object_storage_t) excel_sheet_object_free_storage, NULL TSRMLS_CC);
	(&retval)->handlers = &excel_object_handlers_sheet;

	return retval;
}

static void excel_font_object_free_storage(void *object TSRMLS_DC)
{
	excel_font_object *intern = (excel_font_object *)object;

	zend_object_std_dtor(&intern->std TSRMLS_CC);

	efree(object);
}

#define REGISTER_EXCEL_CLASS_CONST_LONG(class_name, const_name, value) \
	zend_declare_class_constant_long(excel_ce_ ## class_name, const_name, sizeof(const_name)-1, (long)value TSRMLS_CC);

static zend_object_value excel_object_new_font_ex(zend_class_entry *class_type, excel_font_object **ptr TSRMLS_DC)
{
	excel_font_object *intern;
	zend_object_value retval;

	intern = emalloc(sizeof(excel_font_object));
	memset(intern, 0, sizeof(excel_font_object));
	if (ptr) {
		*ptr = intern;
	}

	zend_object_std_init(&intern->std, class_type TSRMLS_CC);
#ifdef ZEND_ENGINE_2_4
	object_properties_init(&intern->std, class_type);
#else
	{
		zval *tmp;
		zend_hash_copy(intern->std.properties, &class_type->default_properties, (copy_ctor_func_t) zval_add_ref, (void *) &tmp, sizeof(zval *));
	}
#endif
	(&retval)->handle = zend_objects_store_put(intern, (zend_objects_store_dtor_t)zend_objects_destroy_object, (zend_objects_free_object_storage_t) excel_font_object_free_storage, NULL TSRMLS_CC);
	(&retval)->handlers = &excel_object_handlers_font;

	return retval;
}

static zend_object_value excel_object_new_font(zend_class_entry *class_type TSRMLS_DC)
{
	return excel_object_new_font_ex(class_type, NULL TSRMLS_CC);
}

static zend_object_value excel_font_object_clone(zval *this_ptr TSRMLS_DC)
{
	excel_font_object *new_obj = NULL;
	excel_font_object *old_obj = (excel_font_object *) zend_object_store_get_object(this_ptr TSRMLS_CC);
	zend_object_value new_ov = excel_object_new_font_ex(old_obj->std.ce, &new_obj TSRMLS_CC);
	FontHandle font;

	font = xlBookAddFont(old_obj->book, old_obj->font);
	if (!font) {
		zend_throw_exception(NULL, "Failed to copy font", 0 TSRMLS_CC);
	} else {
		new_obj->book = old_obj->book;
		new_obj->font = font;
	}

	zend_objects_clone_members(&new_obj->std, new_ov, &old_obj->std, Z_OBJ_HANDLE_P(this_ptr) TSRMLS_CC);

	return new_ov;
}

static void excel_format_object_free_storage(void *object TSRMLS_DC)
{
	excel_format_object *intern = (excel_format_object *)object;

	zend_object_std_dtor(&intern->std TSRMLS_CC);

	efree(object);
}

static zend_object_value excel_object_new_format_ex(zend_class_entry *class_type, excel_format_object **ptr TSRMLS_DC)
{
	excel_format_object *intern;
	zend_object_value retval;

	intern = emalloc(sizeof(excel_format_object));
	memset(intern, 0, sizeof(excel_format_object));
	if (ptr) {
		*ptr = intern;
	}

	zend_object_std_init(&intern->std, class_type TSRMLS_CC);
#ifdef ZEND_ENGINE_2_4
	object_properties_init(&intern->std, class_type);
#else
	{
		zval *tmp;
		zend_hash_copy(intern->std.properties, &class_type->default_properties, (copy_ctor_func_t) zval_add_ref, (void *) &tmp, sizeof(zval *));
	}
#endif
	(&retval)->handle = zend_objects_store_put(intern, (zend_objects_store_dtor_t)zend_objects_destroy_object, (zend_objects_free_object_storage_t) excel_format_object_free_storage, NULL TSRMLS_CC);
	(&retval)->handlers = &excel_object_handlers_format;

	return retval;
}

static zend_object_value excel_object_new_format(zend_class_entry *class_type TSRMLS_DC)
{
	return excel_object_new_format_ex(class_type, NULL TSRMLS_CC);
}

static zend_object_value excel_format_object_clone(zval *this_ptr TSRMLS_DC)
{
	excel_format_object *new_obj = NULL;
	excel_format_object *old_obj = (excel_format_object *) zend_object_store_get_object(this_ptr TSRMLS_CC);
	zend_object_value new_ov = excel_object_new_format_ex(old_obj->std.ce, &new_obj TSRMLS_CC);
	FormatHandle format;

	format = xlBookAddFormat(old_obj->book, old_obj->format);
	if (!format) {
		zend_throw_exception(NULL, "Failed to copy format", 0 TSRMLS_CC);
	} else {
		new_obj->book = old_obj->book;
		new_obj->format = format;
	}

	zend_objects_clone_members(&new_obj->std, new_ov, &old_obj->std, Z_OBJ_HANDLE_P(this_ptr) TSRMLS_CC);

	return new_ov;
}

#if LIBXL_VERSION <= 0x03010000
static wchar_t * _php_excel_to_wide(const char *string, size_t len, size_t *out_len)
{
	wchar_t *buf = safe_emalloc(len, sizeof(wchar_t), 0);

	*out_len = mbstowcs(buf, string, len);
	if (*out_len == (size_t) -1) {
		efree(buf);
		return NULL;
	}

	return erealloc(buf, (*out_len + 1) * sizeof(wchar_t));
}
#endif

#define EXCEL_METHOD(class_name, function_name) \
	PHP_METHOD(Excel ## class_name, function_name)

/* {{{ proto bool ExcelBook::load(string data)
	Load Excel data string. */
EXCEL_METHOD(Book, load)
{
	BookHandle book;
	zval *object = getThis();
	char *data;
	int data_len;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &data, &data_len) == FAILURE) {
		RETURN_FALSE;
	}

	if (!data_len) {
		RETURN_FALSE;
	}

	BOOK_FROM_OBJECT(book, object);

	RETURN_BOOL(xlBookLoadRaw(book, data, data_len));
}
/* }}} */

/* {{{ proto bool ExcelBook::loadFile(string filename)
	Load Excel from file. */
EXCEL_METHOD(Book, loadFile)
{
	BookHandle book;
	zval *object = getThis();
	char *filename;
	int filename_len;
	php_stream *stream;
	int len;
	char *contents;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &filename, &filename_len) == FAILURE) {
		RETURN_FALSE;
	}

	if (!filename_len) {
		RETURN_FALSE;
	}

	BOOK_FROM_OBJECT(book, object);

	stream = php_stream_open_wrapper(filename, "rb", ENFORCE_SAFE_MODE | REPORT_ERRORS, NULL);
	if (!stream) {
		RETURN_FALSE;
	}

	len = php_stream_copy_to_mem(stream, &contents, PHP_STREAM_COPY_ALL, 0);
	php_stream_close(stream);

	if (len < 1) {
		RETURN_FALSE;
	}

	RETVAL_BOOL(xlBookLoadRaw(book, contents, len));
	efree(contents);
}
/* }}} */

/* {{{ proto mixed ExcelBook::save([string filename])
	Save Excel file. */
EXCEL_METHOD(Book, save)
{
	BookHandle book;
	zval *object = getThis();
	char *filename = NULL;
	int filename_len;
	unsigned int len = 0;
	char *contents = NULL;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "|s", &filename, &filename_len) == FAILURE) {
		RETURN_FALSE;
	}

	BOOK_FROM_OBJECT(book, object);

	if (!xlBookSaveRaw(book, (const char **)&contents, &len)) {
		RETURN_FALSE;
	}

	if (filename) {
		int numbytes;
		php_stream *stream = php_stream_open_wrapper(filename, "wb", ENFORCE_SAFE_MODE | REPORT_ERRORS, NULL);

		if (!stream) {
			RETURN_FALSE;
		}

		if ((numbytes = php_stream_write(stream, contents, len)) != len) {
			php_error_docref(NULL TSRMLS_CC, E_WARNING, "Only %d of %d bytes written, possibly out of free disk space", numbytes, len);
			RETURN_FALSE;
		}

		php_stream_close(stream);
		RETURN_TRUE;
	} else {
		RETURN_STRINGL(contents, len, 1);
	}

}
/* }}} */

/* {{{ proto ExcelSheet ExcelBook::getSheet([int sheet])
	Get an excel sheet. */
EXCEL_METHOD(Book, getSheet)
{
	BookHandle book;
	zval *object = getThis();
	long sheet = 0;
	SheetHandle sh;
	excel_sheet_object *fo;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "|l", &sheet) == FAILURE) {
		RETURN_FALSE;
	}

	if (sheet < 0) {
		RETURN_FALSE;
	}

	BOOK_FROM_OBJECT(book, object);

	if (!(sh = xlBookGetSheet(book, sheet))) {
		RETURN_FALSE;
	}

	Z_TYPE_P(return_value) = IS_OBJECT;
	object_init_ex(return_value, excel_ce_sheet);
	Z_SET_REFCOUNT_P(return_value, 1);
	Z_SET_ISREF_P(return_value);
	fo = (excel_sheet_object *) zend_object_store_get_object(return_value TSRMLS_CC);
	fo->sheet = sh;
	fo->book = book;
}
/* }}} */

/* {{{ proto ExcelSheet ExcelBook::getSheetByName(string name [, bool case_insensitive])
	Get an excel sheet by name. */
EXCEL_METHOD(Book, getSheetByName)
{
	BookHandle book;
	zval *object = getThis();
	char *sheet_name;
	int sheet_name_len;
	long sheet;
	excel_sheet_object *fo;
	long sheet_count;
	zend_bool case_s = 0;
	const char *s;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s|b", &sheet_name, &sheet_name_len, &case_s) == FAILURE) {
		RETURN_FALSE;
	}

	if (sheet_name_len == 0) {
		RETURN_FALSE;
	}

	BOOK_FROM_OBJECT(book, object);

	sheet_count = xlBookSheetCount(book);
	for(sheet = 0; sheet < sheet_count; sheet++) {
		SheetHandle sh = xlBookGetSheet(book, sheet);
		if (sh) {
			s = xlSheetName(sh);
			if (s) {
				if ((case_s && !strcasecmp(s, sheet_name)) || (!case_s && !strcmp(s, sheet_name))) {
					Z_TYPE_P(return_value) = IS_OBJECT;
					object_init_ex(return_value, excel_ce_sheet);
					Z_SET_REFCOUNT_P(return_value, 1);
					Z_SET_ISREF_P(return_value);
					fo = (excel_sheet_object *) zend_object_store_get_object(return_value TSRMLS_CC);
					fo->sheet = sh;
					fo->book = book;

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
	zval *object = getThis();
	long sheet;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l", &sheet) == FAILURE) {
		RETURN_FALSE;
	}

	if (sheet < 0) {
		RETURN_FALSE;
	}

	BOOK_FROM_OBJECT(book, object);

	RETURN_BOOL(xlBookDelSheet(book, sheet));
}
/* }}} */

/* {{{ proto int ExcelBook::activeSheet([int sheet])
	Get or set an active excel sheet. */
EXCEL_METHOD(Book, activeSheet)
{
	BookHandle book;
	zval *object = getThis();
	long sheet = -1;
	long res;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "|l", &sheet) == FAILURE) {
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
	zval *object = getThis();
	SheetHandle sh;
	excel_sheet_object *fo;
	char *name;
	int name_len;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &name, &name_len) == FAILURE) {
		RETURN_FALSE;
	}

	BOOK_FROM_OBJECT(book, object);

#ifdef LIBXL_VERSION
	sh = xlBookAddSheet(book, name, 0);
#else
	sh = xlBookAddSheet(book, name);
#endif

	if (!sh) {
		RETURN_FALSE;
	}

	Z_TYPE_P(return_value) = IS_OBJECT;
	object_init_ex(return_value, excel_ce_sheet);
	Z_SET_REFCOUNT_P(return_value, 1);
	Z_SET_ISREF_P(return_value);
	fo = (excel_sheet_object *) zend_object_store_get_object(return_value TSRMLS_CC);
	fo->sheet = sh;
	fo->book = book;
}
/* }}} */

/* {{{ proto ExcelSheet ExcelBook::copySheet(string name, int sheet_number)
	Copy an excel sheet. */
EXCEL_METHOD(Book, copySheet)
{
	BookHandle book;
	zval *object = getThis();
	SheetHandle sh;
	excel_sheet_object *fo;
	char *name;
	int name_len;
	long num;
#ifdef LIBXL_VERSION
	SheetHandle osh;
#endif

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "sl", &name, &name_len, &num) == FAILURE) {
		RETURN_FALSE;
	}

	if (num < 0) {
		RETURN_FALSE;
	}

	BOOK_FROM_OBJECT(book, object);

#ifdef LIBXL_VERSION
	if (!(osh = xlBookGetSheet(book, num))) {
		RETURN_FALSE;
	}
	sh = xlBookAddSheet(book, name, osh);
#else
	sh = xlBookCopySheet(book, name, num);
#endif

	if (!sh) {
		RETURN_FALSE;
	}

	Z_TYPE_P(return_value) = IS_OBJECT;
	object_init_ex(return_value, excel_ce_sheet);
	Z_SET_REFCOUNT_P(return_value, 1);
	Z_SET_ISREF_P(return_value);
	fo = (excel_sheet_object *) zend_object_store_get_object(return_value TSRMLS_CC);
	fo->sheet = sh;
	fo->book = book;
}
/* }}} */

/* {{{ proto int ExcelBook::sheetCount()
	Get the number of sheets inside a file. */
EXCEL_METHOD(Book, sheetCount)
{
	BookHandle book;
	zval *object = getThis();

	if (ZEND_NUM_ARGS()) {
		RETURN_FALSE;
	}

	BOOK_FROM_OBJECT(book, object);

	RETURN_LONG(xlBookSheetCount(book));
}
/* }}} */

/* {{{ proto string ExcelBook::getError()
	Get Excel error string. */
EXCEL_METHOD(Book, getError)
{
	BookHandle book;
	zval *object = getThis();
	char *err;

	if (ZEND_NUM_ARGS()) {
		RETURN_FALSE;
	}

	BOOK_FROM_OBJECT(book, object);

	err = (char *)xlBookErrorMessage(book);
	if (err) {
		if (!strcmp(err, "ok")) {
			RETURN_FALSE;
		} else {
			RETURN_STRING(err, 1);
		}
	} else {
		RETURN_STRING("Unknown Error", 1);
	}
}
/* }}} */

/* {{{ proto ExcelFont ExcelBook::addFont([ExcelFont font])
	Add or Copy ExcelFont object. */
EXCEL_METHOD(Book, addFont)
{
	BookHandle book;
	zval *object = getThis();
	FontHandle nfont;
	FontHandle font = NULL;
	excel_font_object *fo;
	zval *fob = NULL;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "|O", &fob, excel_ce_font) == FAILURE) {
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

	Z_TYPE_P(return_value) = IS_OBJECT;
	object_init_ex(return_value, excel_ce_font);
	Z_SET_REFCOUNT_P(return_value, 1);
	Z_SET_ISREF_P(return_value);
	fo = (excel_font_object *) zend_object_store_get_object(return_value TSRMLS_CC);
	fo->font = nfont;
	fo->book = book;
}
/* }}} */

/* {{{ proto ExcelFormat ExcelBook::addFormat([ExcelFormat format])
	Add or Copy ExcelFormat object. */
EXCEL_METHOD(Book, addFormat)
{
	BookHandle book;
	zval *object = getThis();
	FormatHandle nformat;
	FormatHandle format = NULL;
	excel_format_object *fo;
	zval *fob = NULL;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "|O", &fob, excel_ce_format) == FAILURE) {
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

	Z_TYPE_P(return_value) = IS_OBJECT;
	object_init_ex(return_value, excel_ce_format);
	Z_SET_REFCOUNT_P(return_value, 1);
	Z_SET_ISREF_P(return_value);
	fo = (excel_format_object *) zend_object_store_get_object(return_value TSRMLS_CC);
	fo->format = nformat;
	fo->book = book;
}
/* }}} */

#ifdef HAVE_LIBXL_243_PLUS
/* {{{ proto array ExcelBook::getAllFormats()
	Get an array of all ExcelFormat objects used inside a document. */
EXCEL_METHOD(Book, getAllFormats)
{
	BookHandle book;
	zval *object = getThis();
	unsigned short fc;
	unsigned short c;

	if (ZEND_NUM_ARGS()) {
		RETURN_FALSE;
	}

	BOOK_FROM_OBJECT(book, object);

	array_init(return_value);

	fc = xlBookFormatSize(book);
	if (!fc) {
		return;
	}

	for (c = 0; c < fc; c++) {
		FormatHandle format;

		if ((format = xlBookFormat(book, c))) {
			excel_format_object *fo;
			zval *value;

			MAKE_STD_ZVAL(value);
			Z_TYPE_P(value) = IS_OBJECT;
			object_init_ex(value, excel_ce_format);
			Z_SET_REFCOUNT_P(value, 1);
			Z_SET_ISREF_P(value);
			fo = (excel_format_object *) zend_object_store_get_object(value TSRMLS_CC);
			fo->format = format;
			fo->book = book;

			add_next_index_zval(return_value, value);
		}
	}
}
/* }}} */
#endif

/* {{{ proto int ExcelBook::addCustomFormat(string format)
	Create a custom cell format */
EXCEL_METHOD(Book, addCustomFormat)
{
	BookHandle book;
	zval *object = getThis();
	char *format;
	int format_len;
	int id;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &format, &format_len) == FAILURE) {
		RETURN_FALSE;
	}

	if (!format_len) {
		RETURN_FALSE;
	}

	BOOK_FROM_OBJECT(book, object);

	if (!(id = xlBookAddCustomNumFormat(book, format))) {
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
	zval *object = getThis();
	long id;
	char *data;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l", &id) == FAILURE) {
		RETURN_FALSE;
	}

	if (id < 1) {
		RETURN_FALSE;
	}

	BOOK_FROM_OBJECT(book, object);

	if (!(data = (char *)xlBookCustomNumFormat(book, id))) {
		RETURN_FALSE;
	}
	RETURN_STRING(data, 1);
}
/* }}} */

static double _php_excel_date_pack(BookHandle book, long ts)
{
	struct tm tm;

	if (!php_localtime_r(&ts, &tm)) {
		return -1;
	}

	tm.tm_year += 1900;
	tm.tm_mon += 1;

	return xlBookDatePack(book, tm.tm_year, tm.tm_mon, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec
#ifdef HAVE_LIBXL_243_PLUS
							, 0
#endif
	);
}

/* {{{ proto float ExcelBook::packDate(int timestamp)
	Pack a unix timestamp into an Excel Double */
EXCEL_METHOD(Book, packDate)
{
	BookHandle book;
	zval *object = getThis();
	long ts;
	double dt;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l", &ts) == FAILURE) {
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

static double _php_excel_date_pack_values(BookHandle book, int year, int month, int day, int hour, int min, int sec)
{
	return xlBookDatePack(book, year, month, day, hour, min, sec
#ifdef HAVE_LIBXL_243_PLUS
							, 0
#endif
	);
}

/* {{{ proto float ExcelBook::packDateValues(int year, int month, int day, int hour, int minute, int second)
	Pack a date by single values into an Excel Double */
EXCEL_METHOD(Book, packDateValues)
{
	BookHandle book;
	zval *object = getThis();
	long year, month, day, hour, min, sec;
	double dt;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "llllll", &year, &month, &day, &hour, &min, &sec) == FAILURE) {
		RETURN_FALSE;
	}

	if (year < 1) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Invalid '%ld' value for year", year);
		RETURN_FALSE;
	} else if (month < 1 || month > 12) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Invalid '%ld' value for month", month);
		RETURN_FALSE;
	} else if (day < 1 || day > 31) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Invalid '%ld' value for day", day);
		RETURN_FALSE;
	} else if (hour < 0 || hour > 23) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Invalid '%ld' value for hour", hour);
		RETURN_FALSE;
	} else if (min < 0 || min > 59) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Invalid '%ld' value for minute", min);
		RETURN_FALSE;
	} else if (sec < 0 || sec > 59) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Invalid '%ld' value for second", sec);
		RETURN_FALSE;
	}

	BOOK_FROM_OBJECT(book, object);

	if ((dt = _php_excel_date_pack_values(book, year, month, day, hour, min, sec)) == -1) {
		RETURN_FALSE;
	}
	RETURN_DOUBLE(dt);
}
/* }}} */

static long _php_excel_date_unpack(BookHandle book, double dt)
{
	struct tm tm = {0};
#ifdef HAVE_LIBXL_243_PLUS
#if LIBXL_VERSION >= 0x03010000
	int msec;
#else
	unsigned short msec;
#endif
#endif

#if LIBXL_VERSION >= 0x03010000
	if (!xlBookDateUnpack(book, dt, (int *) &(tm.tm_year), (int *) &(tm.tm_mon), (int *) &(tm.tm_mday), (int *) &(tm.tm_hour), (int *) &(tm.tm_min), (int *) &(tm.tm_sec)
#else
	if (!xlBookDateUnpack(book, dt, (short unsigned int *) &(tm.tm_year), (short unsigned int *) &(tm.tm_mon), (short unsigned int *) &(tm.tm_mday),
									(short unsigned int *) &(tm.tm_hour), (short unsigned int *) &(tm.tm_min), (short unsigned int *) &(tm.tm_sec)
#endif
#ifdef HAVE_LIBXL_243_PLUS
									, &msec
#endif
	)) {
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
	zval *object = getThis();
	double dt;
	time_t t;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "d", &dt) == FAILURE) {
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

#if LIBXL_VERSION >= 0x03050300
/* {{{ proto bool ExcelBook::isDate1904()
	Returns whether the 1904 date system is active: true - 1904 date system, false - 1900 date system */
EXCEL_METHOD(Book, isDate1904)
{
	BookHandle book;
	zval *object = getThis();

	if (ZEND_NUM_ARGS()) {
		RETURN_FALSE;
	}

	BOOK_FROM_OBJECT(book, object);

	RETURN_BOOL(xlBookIsDate1904(book));
}
/* }}} */

/* {{{ proto bool ExcelBook::setDate1904(bool date_type)
	Sets the date system mode: true - 1904 date system, false - 1900 date system (default) */
EXCEL_METHOD(Book, setDate1904)
{
	BookHandle book;
	zval *object = getThis();
	zend_bool date_type;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "b", &date_type) == FAILURE) {
		RETURN_FALSE;
	}

	BOOK_FROM_OBJECT(book, object);

	xlBookSetDate1904(book, (int)date_type);

	RETURN_TRUE;
}
/* }}} */
#endif

/* {{{ proto int ExcelBook::getActiveSheet()
	Get the active sheet inside a file. */
EXCEL_METHOD(Book, getActiveSheet)
{
	BookHandle book;
	zval *object = getThis();

	if (ZEND_NUM_ARGS()) {
		RETURN_FALSE;
	}

	BOOK_FROM_OBJECT(book, object);

	RETURN_LONG(xlBookActiveSheet(book));
}
/* }}} */

/* {{{ proto array ExcelBook::getDefaultFont()
	Get the default font. */
EXCEL_METHOD(Book, getDefaultFont)
{
	BookHandle book;
	zval *object = getThis();
	const char *font;
#if LIBXL_VERSION >= 0x03010000
	int font_size;
#else
	unsigned short font_size;
#endif

	if (ZEND_NUM_ARGS()) {
		RETURN_FALSE;
	}

	BOOK_FROM_OBJECT(book, object);

	if (!(font = xlBookDefaultFont(book, &font_size))) {
		RETURN_FALSE;
	}

	array_init(return_value);
	add_assoc_string(return_value, "font", (char *)font, 1);
	add_assoc_long(return_value, "font_size", font_size);
}
/* }}} */

/* {{{ proto void ExcelBook::setDefaultFont(string font, int font_size)
	Set the default font, and size. */
EXCEL_METHOD(Book, setDefaultFont)
{
	BookHandle book;
	zval *object = getThis();
	char *font;
	int font_len;
	long font_size;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "sl", &font, &font_len, &font_size) == FAILURE || font_size < 1) {
		RETURN_FALSE;
	}

	BOOK_FROM_OBJECT(book, object);

	xlBookSetDefaultFont(book, font, (int)font_size);
}
/* }}} */

/* {{{ proto void ExcelBook::setLocale(string locale)
	Set the locale. */
EXCEL_METHOD(Book, setLocale)
{
	BookHandle book;
	zval *object = getThis();
	char *locale;
	int locale_len;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &locale, &locale_len) == FAILURE || locale_len < 1) {
		RETURN_FALSE;
	}

	BOOK_FROM_OBJECT(book, object);

	xlBookSetLocale(book, locale);
}
/* }}} */

/* {{{ proto ExcelBook ExcelBook::__construct([string license_name, string license_key [, bool excel_2007 = false]])
	Book Constructor. */
EXCEL_METHOD(Book, __construct)
{
	BookHandle book;
	zval *object = getThis();
	char *name = NULL, *key;
	int name_len = 0, key_len = 0;
#if LIBXL_VERSION <= 0x03010000
	wchar_t *nw, *kw;
	size_t nw_l, kw_l;
#endif
#if defined(HAVE_XML) && defined(EXCEL_WITH_LIBXML)
	char *namep, *keyp;
	int plen;
#endif

#ifdef LIBXL_VERSION
	zend_bool new_excel = 0;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "|ssb", &name, &name_len, &key, &key_len, &new_excel) == FAILURE) {
		RETURN_FALSE;
	}
#else
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "|ss", &name, &name_len, &key, &key_len) == FAILURE) {
		RETURN_FALSE;
	}
#endif
	if (!name_len) {
		if (INI_STR("excel.license_name") && INI_STR("excel.license_key")) {
			name = INI_STR("excel.license_name");
			name_len = strlen(name);
			key = INI_STR("excel.license_key");
			key_len = strlen(key);
		} else {
#ifndef LIBXL_VERSION
			return;
#endif
		}
	}

	BOOK_FROM_OBJECT(book, object);
#ifdef LIBXL_VERSION
	if (new_excel) {
		excel_book_object *obj = (excel_book_object*) zend_object_store_get_object(object TSRMLS_CC);
		if ((book = xlCreateXMLBook())) {
			xlBookRelease(obj->book);
			obj->book = book;
		} else {
			RETURN_FALSE;
		}
		if (!name_len && !key_len) {
			return;
		}
	}
#endif
	if (!name_len || !key_len) {
		RETURN_FALSE;
	}
#if LIBXL_VERSION <= 0x03010000
	if (!(nw = _php_excel_to_wide(name, name_len + 1, &nw_l))) {
		RETURN_FALSE;
	}
	if (!(kw = _php_excel_to_wide(key, key_len + 1, &kw_l))) {
		efree(nw);
		RETURN_FALSE;
	}

	xlBookSetKey(book, nw, kw);
	efree(nw);
	efree(kw);
#else

#if defined(HAVE_XML) && defined(EXCEL_WITH_LIBXML)
	namep = xml_utf8_decode((const XML_Char *) name, name_len, &plen, (const XML_Char *)"ISO-8859-1");
	keyp = xml_utf8_decode((const XML_Char *) key, key_len, &plen, (const XML_Char *)"ISO-8859-1");
	xlBookSetKey(book, namep, keyp);
	efree(namep);
	efree(keyp);
#else
	xlBookSetKey(book, name, key);
#endif
#endif
}
/* }}} */

/* {{{ proto bool ExcelBook::setActiveSheet(int sheet)
	Set the sheet active. */
EXCEL_METHOD(Book, setActiveSheet)
{
	BookHandle book;
	zval *object = getThis();
	long id;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l", &id) == FAILURE || id < 0) {
		RETURN_FALSE;
	}

	BOOK_FROM_OBJECT(book, object);

	xlBookSetActiveSheet(book, id);
	RETURN_BOOL(id == xlBookActiveSheet(book));
}
/* }}} */

static void php_excel_add_picture(INTERNAL_FUNCTION_PARAMETERS, int mode) /* {{{ */
{
	char *data;
	int data_len;
	BookHandle book;
	zval *object = getThis();
	int ret;

	BOOK_FROM_OBJECT(book, object);

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &data, &data_len) == FAILURE) {
		RETURN_FALSE;
	}

	if (mode == 1) {
		ret = xlBookAddPicture2(book, data, data_len);
	} else {
		php_stream *stream = php_stream_open_wrapper(data, "rb", ENFORCE_SAFE_MODE | REPORT_ERRORS, NULL);
		int len;
		char *contents;

		if (!stream) {
			RETURN_FALSE;
		}

		len = php_stream_copy_to_mem(stream, &contents, PHP_STREAM_COPY_ALL, 0);
		php_stream_close(stream);

		if (len < 1) {
			RETURN_FALSE;
		}
		ret = xlBookAddPicture2(book, contents, len);
		efree(contents);
	}

	if (ret == -1) {
		RETURN_FALSE;
	} else {
#if LIBXL_VERSION >= 0x03020200 && LIBXL_VERSION < 0x03020300
		/* work-around for a bug inside libxl 3.2.2 */
		ret -= 1;
#endif
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

#ifdef LIBXL_VERSION
/* {{{ proto bool ExcelBook::rgbMode()
	Returns whether the RGB mode is active. */
EXCEL_METHOD(Book, rgbMode)
{
	BookHandle book;
	zval *object = getThis();

	if (ZEND_NUM_ARGS()) {
		RETURN_FALSE;
	}

	BOOK_FROM_OBJECT(book, object);

	RETURN_BOOL(xlBookRgbMode(book));
}
/* }}} */

/* {{{ proto void ExcelBook::setRGBMode(bool mode)
	Sets a RGB mode on or off. */
EXCEL_METHOD(Book, setRGBMode)
{
	BookHandle book;
	zval *object = getThis();
	zend_bool val;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "b", &val) == FAILURE) {
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
	zval *object = getThis();
	long r, g, b;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "lll", &r, &g, &b) == FAILURE) {
		RETURN_FALSE;
	}

	if (r < 0 || r > 255) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Invalid '%ld' value for color red", r);
		RETURN_FALSE;
	} else if (g < 0 || g > 255) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Invalid '%ld' value for color green", g);
		RETURN_FALSE;
	} else if (b < 0 || b > 255) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Invalid '%ld' value for color blue", b);
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
	zval *object = getThis();
#if LIBXL_VERSION >= 0x03010000
	int r, g, b;
#else
	unsigned short r, g, b;
#endif
	long color;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l", &color) == FAILURE) {
		RETURN_FALSE;
	}

	if (color <= 0) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Invalid '%ld' value for color code", color);
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
#endif

/* {{{ proto int ExcelFont::size([int size])
	Get or set the font size */
EXCEL_METHOD(Font, size)
{
	zval *object = getThis();
	FontHandle font;
	long size = -1;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "|l", &size) == FAILURE) {
		RETURN_FALSE;
	}

	FONT_FROM_OBJECT(font, object);

	if (size > 0) {
		xlFontSetSize(font, size);
	}

	RETURN_LONG(xlFontSize(font));
}
/* }}} */

/* {{{ proto bool ExcelFont::italics([bool italics])
	Get or set the if italics are enabled */
EXCEL_METHOD(Font, italics)
{
	zval *object = getThis();
	FontHandle font;
	zend_bool italics;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "|b", &italics) == FAILURE) {
		RETURN_FALSE;
	}

	FONT_FROM_OBJECT(font, object);

	if (ZEND_NUM_ARGS()) {
		xlFontSetItalic(font, italics);
	}

	RETURN_BOOL(xlFontItalic(font));
}
/* }}} */

/* {{{ proto bool ExcelFont::strike([bool strike])
	Get or set the font strike-through */
EXCEL_METHOD(Font, strike)
{
	zval *object = getThis();
	FontHandle font;
	zend_bool strike;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "|b", &strike) == FAILURE) {
		RETURN_FALSE;
	}

	FONT_FROM_OBJECT(font, object);

	if (ZEND_NUM_ARGS()) {
		xlFontSetStrikeOut(font, strike);
	}

	RETURN_BOOL(xlFontStrikeOut(font));
}
/* }}} */

/* {{{ proto bool ExcelFont::bold([bool bold])
	Get or set the font bold */
EXCEL_METHOD(Font, bold)
{
	zval *object = getThis();
	FontHandle font;
	zend_bool bold;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "|b", &bold) == FAILURE) {
		RETURN_FALSE;
	}

	FONT_FROM_OBJECT(font, object);

	if (ZEND_NUM_ARGS()) {
		xlFontSetBold(font, bold);
	}

	RETURN_BOOL(xlFontBold(font));
}
/* }}} */

/* {{{ proto int ExcelFont::color([int color])
	Get or set the font color */
EXCEL_METHOD(Font, color)
{
	zval *object = getThis();
	FontHandle font;
	long color;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "|l", &color) == FAILURE) {
		RETURN_FALSE;
	}

	FONT_FROM_OBJECT(font, object);

	if (ZEND_NUM_ARGS()) {
		xlFontSetColor(font, color);
	}

	RETURN_LONG(xlFontColor(font));
}
/* }}} */

/* {{{ proto int ExcelFont::mode([int mode])
	Get or set the font mode */
EXCEL_METHOD(Font, mode)
{
	zval *object = getThis();
	FontHandle font;
	long mode;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "|l", &mode) == FAILURE) {
		RETURN_FALSE;
	}

	FONT_FROM_OBJECT(font, object);

	if (ZEND_NUM_ARGS()) {
		xlFontSetScript(font, mode);
	}

	RETURN_LONG(xlFontScript(font));
}
/* }}} */

/* {{{ proto int ExcelFont::underline([int underline_style])
	Get or set the font underline style */
EXCEL_METHOD(Font, underline)
{
	zval *object = getThis();
	FontHandle font;
	long underline;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "|l", &underline) == FAILURE) {
		RETURN_FALSE;
	}

	FONT_FROM_OBJECT(font, object);

	if (ZEND_NUM_ARGS()) {
		xlFontSetUnderline(font, underline);
	}

	RETURN_LONG(xlFontUnderline(font));
}
/* }}} */

/* {{{ proto string ExcelFont::name([string name])
	Get or set the font name */
EXCEL_METHOD(Font, name)
{
	zval *object = getThis();
	FontHandle font;
	char *name = NULL;
	int name_len;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "|s", &name, &name_len) == FAILURE) {
		RETURN_FALSE;
	}

	FONT_FROM_OBJECT(font, object);

	if (name) {
		xlFontSetName(font, name);
	}

	RETURN_STRING((char *)xlFontName(font), 1);
}
/* }}} */

/* {{{ proto ExcelFormat ExcelFormat::__construct(ExcelBook book)
	Format Constructor. */
EXCEL_METHOD(Format, __construct)
{
	BookHandle book;
	FormatHandle format;
	zval *object = getThis();
	excel_format_object *obj;
	zval *zbook;

	PHP_EXCEL_ERROR_HANDLING();
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "O", &zbook, excel_ce_book) == FAILURE) {
		PHP_EXCEL_RESTORE_ERRORS();
		return;
	}
	PHP_EXCEL_RESTORE_ERRORS();

	BOOK_FROM_OBJECT(book, zbook);

	obj = (excel_format_object*) zend_object_store_get_object(object TSRMLS_CC);

	format = xlBookAddFormat(book, NULL);
	if (!format) {
		RETURN_FALSE;
	}

	obj->format = format;
	obj->book = book;
}
/* }}} */

/* {{{ proto ExcelFont ExcelFont::__construct(ExcelBook book)
	Font Constructor. */
EXCEL_METHOD(Font, __construct)
{
	BookHandle book;
	FontHandle font;
	zval *object = getThis();
	excel_font_object *obj;
	zval *zbook;

	PHP_EXCEL_ERROR_HANDLING();
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "O", &zbook, excel_ce_book) == FAILURE) {
		PHP_EXCEL_RESTORE_ERRORS();
		return;
	}
	PHP_EXCEL_RESTORE_ERRORS();

	BOOK_FROM_OBJECT(book, zbook);

	obj = (excel_font_object*) zend_object_store_get_object(object TSRMLS_CC);

	font = xlBookAddFont(book, NULL);
	if (!font) {
		RETURN_FALSE;
	}

	obj->font = font;
	obj->book = book;
}
/* }}} */

/* {{{ proto bool ExcelFormat::setFont(ExcelFont font)
	Set the font for a format. */
EXCEL_METHOD(Format, setFont)
{
	FormatHandle format;
	zval *object = getThis();
	FontHandle font;
	zval *zfont;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "O", &zfont, excel_ce_font) == FAILURE) {
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
	zval *object = getThis();
	FontHandle font;
	excel_font_object *fo;
	excel_format_object *obj = (excel_format_object*) zend_object_store_get_object(object TSRMLS_CC);

	format = obj->format;
	if (!format) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "The format wasn't initialized");
		RETURN_FALSE;
	}

	if (ZEND_NUM_ARGS()) {
		RETURN_FALSE;
	}

	FORMAT_FROM_OBJECT(format, object);

	font = xlFormatFont(format);
	if (!font) {
		RETURN_FALSE;
	}

	Z_TYPE_P(return_value) = IS_OBJECT;
	object_init_ex(return_value, excel_ce_font);
	Z_SET_REFCOUNT_P(return_value, 1);
	Z_SET_ISREF_P(return_value);
	fo = (excel_font_object *) zend_object_store_get_object(return_value TSRMLS_CC);
	fo->font = font;
	fo->book = obj->book;
}
/* }}} */

#define PHP_EXCEL_LONG_FORMAT_OPTION(func_name, write_only) \
	{ \
		FormatHandle format; \
		zval *object = getThis(); \
		long data; \
		if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "|l", &data) == FAILURE) { \
			RETURN_FALSE; \
		} \
		FORMAT_FROM_OBJECT(format, object); \
		if (ZEND_NUM_ARGS()) { \
			xlFormatSet ## func_name (format, data); \
		} \
		if (!write_only) { \
			RETURN_LONG(xlFormat ## func_name (format)); \
		} else { \
			RETURN_TRUE; \
		} \
	}

#define PHP_EXCEL_BOOL_FORMAT_OPTION(func_name) \
	{ \
		FormatHandle format; \
		zval *object = getThis(); \
		zend_bool data; \
		if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "|b", &data) == FAILURE) { \
			RETURN_FALSE; \
		} \
		FORMAT_FROM_OBJECT(format, object); \
		if (ZEND_NUM_ARGS()) { \
			xlFormatSet ## func_name (format, data); \
		} \
		RETURN_BOOL(xlFormat ## func_name (format)); \
	}

/* {{{ proto int ExcelFormat::numberFormat([int format])
	Get or set the cell number format */
EXCEL_METHOD(Format, numberFormat)
{
	PHP_EXCEL_LONG_FORMAT_OPTION(NumFormat, 0);
}
/* }}} */

/* {{{ proto int ExcelFormat::horizontalAlign([int align_mode])
	Get or set the cell horizontal alignment */
EXCEL_METHOD(Format, horizontalAlign)
{
	PHP_EXCEL_LONG_FORMAT_OPTION(AlignH, 0);
}
/* }}} */

/* {{{ proto int ExcelFormat::verticalAlign([int align_mode])
	Get or set the cell vertical alignment */
EXCEL_METHOD(Format, verticalAlign)
{
	PHP_EXCEL_LONG_FORMAT_OPTION(AlignV, 0);
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
	zval *object = getThis();
	long angle;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "|l", &angle) == FAILURE) {
		RETURN_FALSE;
	}

	FORMAT_FROM_OBJECT(format, object);

	if (ZEND_NUM_ARGS()) {
		if (angle < 0 || (angle > 180 && angle != 255)) {
			php_error_docref(NULL TSRMLS_CC, E_WARNING, "Rotation can be a number between 0 and 180 or 255");
			RETURN_FALSE;
		}
		xlFormatSetRotation(format, angle);
	}

	RETURN_LONG(xlFormatRotation(format));
}
/* }}} */

/* {{{ proto int ExcelFormat::indent([int indent])
	Get or set the cell text indentation level */
EXCEL_METHOD(Format, indent)
{
	FormatHandle format;
	zval *object = getThis();
	long indent;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "|l", &indent) == FAILURE) {
		RETURN_FALSE;
	}

	FORMAT_FROM_OBJECT(format, object);

	if (ZEND_NUM_ARGS()) {
		if (indent < 0 || indent > 15) {
			php_error_docref(NULL TSRMLS_CC, E_WARNING, "Text indentation level must be less than or equal to 15");
			RETURN_FALSE;
		}
		xlFormatSetIndent(format, indent);
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
	PHP_EXCEL_LONG_FORMAT_OPTION(Border, 1);
}
/* }}} */

/* {{{ proto int ExcelFormat::borderColor([int color])
	Get or set the cell color */
EXCEL_METHOD(Format, borderColor)
{
	PHP_EXCEL_LONG_FORMAT_OPTION(BorderColor, 1);
}
/* }}} */

/* {{{ proto int ExcelFormat::borderLeftStyle([int style])
	Get or set the cell left border */
EXCEL_METHOD(Format, borderLeftStyle)
{
	PHP_EXCEL_LONG_FORMAT_OPTION(BorderLeft, 0);
}
/* }}} */

/* {{{ proto int ExcelFormat::borderLeftColor([int color])
	Get or set the cell left color */
EXCEL_METHOD(Format, borderLeftColor)
{
	PHP_EXCEL_LONG_FORMAT_OPTION(BorderLeftColor, 0);
}
/* }}} */

/* {{{ proto int ExcelFormat::borderRightStyle([int style])
	Get or set the cell right border */
EXCEL_METHOD(Format, borderRightStyle)
{
	PHP_EXCEL_LONG_FORMAT_OPTION(BorderRight, 0);
}
/* }}} */

/* {{{ proto int ExcelFormat::borderRightColor([int color])
	Get or set the cell right color */
EXCEL_METHOD(Format, borderRightColor)
{
	PHP_EXCEL_LONG_FORMAT_OPTION(BorderRightColor, 0);
}
/* }}} */

/* {{{ proto int ExcelFormat::borderTopStyle([int style])
	Get or set the cell top border */
EXCEL_METHOD(Format, borderTopStyle)
{
	PHP_EXCEL_LONG_FORMAT_OPTION(BorderTop, 0);
}
/* }}} */

/* {{{ proto int ExcelFormat::borderTopColor([int color])
	Get or set the cell top color */
EXCEL_METHOD(Format, borderTopColor)
{
	PHP_EXCEL_LONG_FORMAT_OPTION(BorderTopColor, 0);
}
/* }}} */

/* {{{ proto int ExcelFormat::borderBottomStyle([int style])
	Get or set the cell bottom border */
EXCEL_METHOD(Format, borderBottomStyle)
{
	PHP_EXCEL_LONG_FORMAT_OPTION(BorderBottom, 0);
}
/* }}} */

/* {{{ proto int ExcelFormat::borderBottomColor([int color])
	Get or set the cell bottom color */
EXCEL_METHOD(Format, borderBottomColor)
{
	PHP_EXCEL_LONG_FORMAT_OPTION(BorderBottomColor, 0);
}
/* }}} */

/* {{{ proto int ExcelFormat::borderDiagonalStyle([int style])
	Get or set the cell diagonal border */
EXCEL_METHOD(Format, borderDiagonalStyle)
{
	PHP_EXCEL_LONG_FORMAT_OPTION(BorderDiagonal, 0);
}
/* }}} */

/* {{{ proto int ExcelFormat::borderDiagonalColor([int color])
	Get or set the cell diagonal color */
EXCEL_METHOD(Format, borderDiagonalColor)
{
	PHP_EXCEL_LONG_FORMAT_OPTION(BorderDiagonalColor, 0);
}
/* }}} */

/* {{{ proto int ExcelFormat::fillPattern([int patern])
	Get or set the cell fill pattern */
EXCEL_METHOD(Format, fillPattern)
{
	PHP_EXCEL_LONG_FORMAT_OPTION(FillPattern, 0);
}
/* }}} */

/* {{{ proto int ExcelFormat::patternForegroundColor([int color])
	Get or set the cell pattern foreground color */
EXCEL_METHOD(Format, patternForegroundColor)
{
	PHP_EXCEL_LONG_FORMAT_OPTION(PatternForegroundColor, 0);
}
/* }}} */

/* {{{ proto int ExcelFormat::patternBackgroundColor([int color])
	Get or set the cell pattern background color */
EXCEL_METHOD(Format, patternBackgroundColor)
{
	PHP_EXCEL_LONG_FORMAT_OPTION(PatternBackgroundColor, 0);
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
	zval *object = getThis();
	excel_sheet_object *obj;
	zval *zbook;
	char *name;
	int name_len;

	PHP_EXCEL_ERROR_HANDLING();
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "Os", &zbook, excel_ce_book, &name, &name_len) == FAILURE) {
		PHP_EXCEL_RESTORE_ERRORS();
		return;
	}
	PHP_EXCEL_RESTORE_ERRORS();

	BOOK_FROM_OBJECT(book, zbook);

	obj = (excel_sheet_object*) zend_object_store_get_object(object TSRMLS_CC);

#ifdef LIBXL_VERSION
	sh = xlBookAddSheet(book, name, 0);
#else
	sh = xlBookAddSheet(book, name);
#endif

	if (!sh) {
		RETURN_FALSE;
	}

	obj->sheet = sh;
	obj->book = book;
}
/* }}} */

/* {{{ proto int ExcelSheet::cellType(int row, int column)
	Get cell type */
EXCEL_METHOD(Sheet, cellType)
{
	zval *object = getThis();
	SheetHandle sheet;
	long row, col;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ll", &row, &col) == FAILURE) {
		RETURN_FALSE;
	}

	SHEET_FROM_OBJECT(sheet, object);

	RETURN_LONG(xlSheetCellType(sheet, row, col));
}
/* }}} */

/* {{{ proto ExcelFormat ExcelSheet::cellFormat(int row, int column)
	Get cell format */
EXCEL_METHOD(Sheet, cellFormat)
{
	zval *object = getThis();
	SheetHandle sheet;
	FormatHandle format;
	long row, col;
	excel_format_object *fo;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ll", &row, &col) == FAILURE) {
		RETURN_FALSE;
	}

	SHEET_FROM_OBJECT(sheet, object);

	format = xlSheetCellFormat(sheet, row, col);

	Z_TYPE_P(return_value) = IS_OBJECT;
	object_init_ex(return_value, excel_ce_format);
	Z_SET_REFCOUNT_P(return_value, 1);
	Z_SET_ISREF_P(return_value);
	fo = (excel_format_object *) zend_object_store_get_object(return_value TSRMLS_CC);
	fo->format = format;
}
/* }}} */

#ifdef HAVE_LIBXL_243_PLUS
/* {{{ proto void ExcelFormat ExcelSheet::setCellFormat(int row, int column, ExcelFormat format)
	Set cell format */
EXCEL_METHOD(Sheet, setCellFormat)
{
	zval *object = getThis();
	SheetHandle sheet;
	FormatHandle format;
	zval *oformat;
	long row, col;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "llo", &row, &col, &oformat, excel_ce_format) == FAILURE) {
		RETURN_FALSE;
	}

	SHEET_FROM_OBJECT(sheet, object);
	FORMAT_FROM_OBJECT(format, oformat);

	xlSheetSetCellFormat(sheet, row, col, format);
}
/* }}} */
#endif

static zend_bool php_excel_read_cell(int row, int col, zval *val, SheetHandle sheet, BookHandle book, FormatHandle *format, zend_bool read_formula)
{
	const char *s;
	if (read_formula && xlSheetIsFormula(sheet, row, col)) {
		s = xlSheetReadFormula(sheet, row, col, format);
		if (s) {
			ZVAL_STRING(val, (char *)s, 1);
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
#if LIBXL_VERSION <= 0x03010000
			if (xlSheetIsDate(sheet, row, col) && xlFormatNumFormat(*format) < 100) {
#else
			if (xlSheetIsDate(sheet, row, col)) {
#endif
				long dt = _php_excel_date_unpack(book, d);
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
				ZVAL_STRING(val, (char *)s, 1);
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
	zval *object = getThis();
	long row;
	long col_start = 0;
	long col_end = -1;
	int lc;
	SheetHandle sheet;
	BookHandle book;
	zend_bool read_formula = 1;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l|llb", &row, &col_start, &col_end, &read_formula) == FAILURE) {
		RETURN_FALSE;
	}

	SHEET_AND_BOOK_FROM_OBJECT(sheet, book, object);

	if (row < 0 || row > xlSheetLastRow(sheet)) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Invalid row number '%ld'", row);
		RETURN_FALSE;
	}

	lc = xlSheetLastCol(sheet);
	if (col_start < 0 || col_start > lc) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Invalid starting column number '%ld'", col_start);
		RETURN_FALSE;
	}

	if (col_end == -1) {
		col_end = lc - 1;
	}

	if (col_end < col_start || col_end > lc) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Invalid ending column number '%ld'", col_end);
		RETURN_FALSE;
	}

	lc = col_start;

	array_init(return_value);
	while (lc < (col_end + 1)) {
		zval *value;
		FormatHandle format = NULL;

		MAKE_STD_ZVAL(value);
		if (!php_excel_read_cell(row, lc, value, sheet, book, &format, read_formula)) {
			zval_ptr_dtor(&value);
			zval_dtor(return_value);
			php_error_docref(NULL TSRMLS_CC, E_WARNING, "Failed to read cell in row %d, column %d with error '%s'", row, lc, xlBookErrorMessage(book));
			RETURN_FALSE;
		} else {
			add_next_index_zval(return_value, value);
		}

		lc++;
	}
}
/* }}} */

/* {{{ proto array ExcelSheet::readCol(int column [, int start_row [, int end_row [, bool read_formula]]])
	Read an entire column worth of data */
EXCEL_METHOD(Sheet, readCol)
{
	zval *object = getThis();
	long col;
	long row_start = 0;
	long row_end = -1;
	int lc;
	SheetHandle sheet;
	BookHandle book;
	zend_bool read_formula = 1;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l|ll", &col, &row_start, &row_end, &read_formula) == FAILURE) {
		RETURN_FALSE;
	}

	SHEET_AND_BOOK_FROM_OBJECT(sheet, book, object);

	if (col < 0 || col > xlSheetLastCol(sheet)) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Invalid column number '%ld'", col);
		RETURN_FALSE;
	}

	lc = xlSheetLastRow(sheet);
	if (row_start < 0 || row_start > lc) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Invalid starting row number '%ld'", row_start);
		RETURN_FALSE;
	}

	if (row_end == -1) {
		row_end = lc - 1;
	}

	if (row_end < row_start || row_end > lc) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Invalid ending row number '%ld'", row_end);
		RETURN_FALSE;
	}

	lc = row_start;

	array_init(return_value);
	while (lc < (row_end + 1)) {
		zval *value;
		FormatHandle format = NULL;

		MAKE_STD_ZVAL(value);
		if (!php_excel_read_cell(lc, col, value, sheet, book, &format, read_formula)) {
			zval_ptr_dtor(&value);
			zval_dtor(return_value);
			php_error_docref(NULL TSRMLS_CC, E_WARNING, "Failed to read cell in row %d, column %d with error '%s'", lc, col, xlBookErrorMessage(book));
			RETURN_FALSE;
		} else {
			add_next_index_zval(return_value, value);
		}

		lc++;
	}
}
/* }}} */

/* {{{ proto mixed ExcelSheet::read(int row, int column [, mixed &format [, bool read_formula]])
	Read data stored inside a cell */
EXCEL_METHOD(Sheet, read)
{
	zval *object = getThis();
	SheetHandle sheet;
	BookHandle book;
	long row, col;
	zval *oformat = NULL;
	FormatHandle format = NULL;
	zend_bool read_formula = 1;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ll|z/b", &row, &col, &oformat, &read_formula) == FAILURE) {
		RETURN_FALSE;
	}

	SHEET_AND_BOOK_FROM_OBJECT(sheet, book, object);

	if (oformat) {
		zval_dtor(oformat);
		ZVAL_NULL(oformat);
	}

	if (!php_excel_read_cell(row, col, return_value, sheet, book, &format, read_formula)) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Failed to read cell in row %d, column %d with error '%s'", row, col, xlBookErrorMessage(book));
		RETURN_FALSE;
	}

	if (oformat) {
		excel_format_object *fo;

		Z_TYPE_P(oformat) = IS_OBJECT;
		object_init_ex(oformat, excel_ce_format);
		fo = (excel_format_object *) zend_object_store_get_object(oformat TSRMLS_CC);
		fo->format = format;
	}
}
/* }}} */

static zend_bool php_excel_write_cell(SheetHandle sheet, BookHandle book, int row, int col, zval *data, FormatHandle format, long dtype TSRMLS_DC)
{
	switch (Z_TYPE_P(data)) {
		case IS_NULL:
			if (INI_INT("excel.skip_empty") > 0) {
				return 1;
			}
			if (!format) {
				FormatHandle fmt = xlBookAddFormat(book, NULL);
				return xlSheetWriteBlank(sheet, row, col, fmt);
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
					FormatHandle fmt = xlBookAddFormat(book, NULL);
					xlFormatSetNumFormat(fmt, NUMFORMAT_DATE);
					return xlSheetWriteNum(sheet, row, col, dt, fmt);
				} else {
					return xlSheetWriteNum(sheet, row, col, dt, format);
				}
			} else {
				return xlSheetWriteNum(sheet, row, col, (double) Z_LVAL_P(data), format);
			}

		case IS_DOUBLE:
			return xlSheetWriteNum(sheet, row, col, Z_DVAL_P(data), format);

		case IS_STRING:
			if (Z_STRLEN_P(data) > 0 && '=' == Z_STRVAL_P(data)[0]) {
				dtype = PHP_EXCEL_FORMULA;
			}
			if (dtype == PHP_EXCEL_FORMULA) {
				return xlSheetWriteFormula(sheet, row, col, Z_STRVAL_P(data), format);
			} else {
				if (dtype == PHP_EXCEL_NUMERIC_STRING) {
					long lval;
					double dval;

					switch (is_numeric_string(Z_STRVAL_P(data), Z_STRLEN_P(data), &lval, &dval, 0)) {
						case IS_LONG:
							return xlSheetWriteNum(sheet, row, col, (double) lval, format);

						case IS_DOUBLE:
							return xlSheetWriteNum(sheet, row, col, dval, format);
					}
				}
				if (Z_STRLEN_P(data) == 0 && INI_INT("excel.skip_empty") == 2) {
					return 1;
				}
				return xlSheetWriteStr(sheet, row, col, Z_STRVAL_P(data), format);
			}

		case IS_BOOL:
			return xlSheetWriteBool(sheet, row, col, Z_BVAL_P(data), format);
	}

	return 0;
}

/* {{{ proto bool ExcelSheet::write(int row, int column, mixed data [, ExcelFormat format [, int datatype]])
	Write data into a cell */
EXCEL_METHOD(Sheet, write)
{
	zval *object = getThis();
	SheetHandle sheet;
	BookHandle book;
	FormatHandle format;
	long row, col;
	zval *oformat = NULL;
	long dtype = -1;
	zval *data;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "llz|O!l", &row, &col, &data, &oformat, excel_ce_format, &dtype) == FAILURE) {
		RETURN_FALSE;
	}

	SHEET_AND_BOOK_FROM_OBJECT(sheet, book, object);
	if (oformat) {
		FORMAT_FROM_OBJECT(format, oformat);
	}

	if (!php_excel_write_cell(sheet, book, row, col, data, oformat ? format : 0, dtype TSRMLS_CC)) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Failed to write cell in row %d, column %d with error '%s'", row, col, xlBookErrorMessage(book));
		RETURN_FALSE;
	}

	RETURN_TRUE;
}
/* }}} */

/* {{{ proto bool ExcelSheet::writeRow(int row, array data [, int start_column [, ExcelFormat format]])
	Write an array of values into a row */
EXCEL_METHOD(Sheet, writeRow)
{
	zval *object = getThis();
	SheetHandle sheet;
	BookHandle book;
	FormatHandle format;
	long row, col = 0;
	zval *oformat = NULL;
	zval *data;
	HashPosition pos;
	zval **element;
	long i;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "la|lO", &row, &data, &col, &oformat, excel_ce_format) == FAILURE) {
		RETURN_FALSE;
	}

	SHEET_AND_BOOK_FROM_OBJECT(sheet, book, object);
	if (oformat) {
		FORMAT_FROM_OBJECT(format, oformat);
	}

	if (row < 0) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Invalid row number '%ld'", row);
		RETURN_FALSE;
	}

	if (col < 0) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Invalid starting column number '%ld'", col);
		RETURN_FALSE;
	}

	i = col;

	for (zend_hash_internal_pointer_reset_ex(Z_ARRVAL_P(data), &pos);
		zend_hash_get_current_data_ex(Z_ARRVAL_P(data), (void **) &element, &pos) == SUCCESS;
		zend_hash_move_forward_ex(Z_ARRVAL_P(data), &pos)) {

		if (!php_excel_write_cell(sheet, book, row, i++, *element, oformat ? format : 0, -1 TSRMLS_CC)) {
			php_error_docref(NULL TSRMLS_CC, E_WARNING, "Failed to write cell in row %d, column %d with error '%s'", row, i-1, xlBookErrorMessage(book));
			RETURN_FALSE;
		}
	}

	RETURN_TRUE;
}
/* }}} */

/* {{{ proto bool ExcelSheet::writeCol(int row, array data [, int start_row [, ExcelFormat format [, int datatype]]])
	Write an array of values into a column */
EXCEL_METHOD(Sheet, writeCol)
{
	zval *object = getThis();
	SheetHandle sheet;
	BookHandle book;
	FormatHandle format;
	long row = 0, col;
	zval *oformat = NULL;
	zval *data;
	HashPosition pos;
	zval **element;
	long i;
	long dtype = -1;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "la|lO!l", &col, &data, &row, &oformat, excel_ce_format, &dtype) == FAILURE) {
		RETURN_FALSE;
	}

	SHEET_AND_BOOK_FROM_OBJECT(sheet, book, object);
	if (oformat) {
		FORMAT_FROM_OBJECT(format, oformat);
	}

	if (col < 0) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Invalid column number '%ld'", col);
		RETURN_FALSE;
	}

	if (row < 0) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Invalid starting row number '%ld'", row);
		RETURN_FALSE;
	}

	i = row;

	for (zend_hash_internal_pointer_reset_ex(Z_ARRVAL_P(data), &pos);
		zend_hash_get_current_data_ex(Z_ARRVAL_P(data), (void **) &element, &pos) == SUCCESS;
		zend_hash_move_forward_ex(Z_ARRVAL_P(data), &pos)) {

		if (!php_excel_write_cell(sheet, book, i++, col, *element, oformat ? format : 0, dtype TSRMLS_CC)) {
			php_error_docref(NULL TSRMLS_CC, E_WARNING, "Failed to write cell in row %d, column %d with error '%s'", i-1, col, xlBookErrorMessage(book));
			RETURN_FALSE;
		}
	}

	RETURN_TRUE;
}
/* }}} */

#define PHP_EXCEL_SHEET_GET_BOOL_STATE(func_name) \
	{ \
		SheetHandle sheet; \
		zval *object = getThis(); \
		long r, c; \
		if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ll", &r, &c) == FAILURE) { \
			RETURN_FALSE; \
		} \
		SHEET_FROM_OBJECT(sheet, object); \
		RETURN_BOOL(xlSheet ## func_name (sheet, r, c)); \
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
	zval *object = getThis();
	long r, c;
#if LIBXL_VERSION <= 0x03010000
	double d;
	FormatHandle format = NULL;
#endif
	SheetHandle sheet;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ll", &r, &c) == FAILURE) {
		RETURN_FALSE;
	}

	SHEET_FROM_OBJECT(sheet, object);

	if (xlSheetCellType(sheet, r, c) != CELLTYPE_NUMBER) {
		RETURN_FALSE;
	}
#if LIBXL_VERSION <= 0x03010000
	d = xlSheetReadNum(sheet, r, c, &format);
	RETURN_BOOL(xlSheetIsDate(sheet, r, c) && (!format || (xlFormatNumFormat(format) < 100)));
#else
	RETURN_BOOL(xlSheetIsDate(sheet, r, c));
#endif
}
/* }}} */

/* {{{ proto bool ExcelSheet::insertRow(int row_first, int row_last)
	Inserts rows from rowFirst to rowLast */
EXCEL_METHOD(Sheet, insertRow)
{
	PHP_EXCEL_SHEET_GET_BOOL_STATE(InsertRow)
}
/* }}} */

/* {{{ proto bool ExcelSheet::insertCol(int col_first, int col_last)
	Inserts columns from colFirst to colLast */
EXCEL_METHOD(Sheet, insertCol)
{
	PHP_EXCEL_SHEET_GET_BOOL_STATE(InsertCol)
}
/* }}} */

/* {{{ proto bool ExcelSheet::removeRow(int row_first, int row_last)
	Removes rows from rowFirst to rowLast */
EXCEL_METHOD(Sheet, removeRow)
{
	PHP_EXCEL_SHEET_GET_BOOL_STATE(RemoveRow)
}
/* }}} */

/* {{{ proto bool ExcelSheet::removeCol(int col_first, int col_last)
	Removes columns from colFirst to colLast */
EXCEL_METHOD(Sheet, removeCol)
{
	PHP_EXCEL_SHEET_GET_BOOL_STATE(RemoveCol)
}
/* }}} */

#define PHP_EXCEL_SHEET_GET_DOUBLE_STATE(func_name) \
	{ \
		SheetHandle sheet; \
		zval *object = getThis(); \
		long val; \
		if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l", &val) == FAILURE) { \
			RETURN_FALSE; \
		} \
		SHEET_FROM_OBJECT(sheet, object); \
		RETURN_DOUBLE(xlSheet ## func_name (sheet, val)); \
	}

/* {{{ proto double ExcelSheet::colWidth(int column)
	Returns the cell width */
EXCEL_METHOD(Sheet, colWidth)
{
	PHP_EXCEL_SHEET_GET_DOUBLE_STATE(ColWidth)
}
/* }}} */

/* {{{ proto double ExcelSheet::rowHeight(int row)
	Returns the cell height */
EXCEL_METHOD(Sheet, rowHeight)
{
	PHP_EXCEL_SHEET_GET_DOUBLE_STATE(RowHeight)
}
/* }}} */

/* {{{ proto string ExcelSheet::readComment(int row, int column)
	Read comment from a cell */
EXCEL_METHOD(Sheet, readComment)
{
		SheetHandle sheet;
		zval *object = getThis();
		const char *s;
		long r, c;

		if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ll", &r, &c) == FAILURE) {
			RETURN_FALSE;
		}

		SHEET_FROM_OBJECT(sheet, object);

		s = xlSheetReadComment(sheet, r, c);
		if (!s) {
			RETURN_FALSE;
		}
		RETURN_STRING((char *)s, 1);
}
/* }}} */

/* {{{ proto void ExcelSheet::writeComment(int row, int column, string value, string author, int width, int height)
	Write comment to a cell */
EXCEL_METHOD(Sheet, writeComment)
{
		SheetHandle sheet;
		zval *object = getThis();
		char *val, *auth;
		int val_len, auth_len;
		long r, c, w, h;

		if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "llssll", &r, &c, &val, &val_len, &auth, &auth_len, &w, &h) == FAILURE) {
			RETURN_FALSE;
		}

		SHEET_FROM_OBJECT(sheet, object);

		xlSheetWriteComment(sheet, r, c, val, auth, w, h);
}
/* }}} */

/* {{{ proto void ExcelSheet::setColWidth(int column_start, int column_end, double width [, bool hidden [, ExcelFormat format]])
	Set width of cells within column(s); Value -1 is used for autofit column widths in LibXL 3.6+ */
EXCEL_METHOD(Sheet, setColWidth)
{
		SheetHandle sheet;
		FormatHandle format;
		zval *object = getThis();
		long s, e;
		double width;
		zval *f = NULL;
		zend_bool h = 0;

		if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "lld|bz/", &s, &e, &width, &h, &f) == FAILURE) {
			RETURN_FALSE;
		}

		SHEET_FROM_OBJECT(sheet, object);

		if (f) {
			FORMAT_FROM_OBJECT(format, f);
		}

		if (e < s) {
			php_error_docref(NULL TSRMLS_CC, E_WARNING, "Start cell is greater then end cell");
			RETURN_FALSE;
		} else if (s < 0) {
			php_error_docref(NULL TSRMLS_CC, E_WARNING, "Start cell cannot be less then 0");
			RETURN_FALSE;
		} else if (width < -1) {
			php_error_docref(NULL TSRMLS_CC, E_WARNING, "Width cannot be less then -1");
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
		zval *object = getThis();
		long row;
		double height;
		zval *f = NULL;
		zend_bool h = 0;

		if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ld|z/b", &row, &height, &f, &h) == FAILURE) {
			RETURN_FALSE;
		}

		SHEET_FROM_OBJECT(sheet, object);

		if (f) {
			FORMAT_FROM_OBJECT(format, f);
		}

		if (row < 0) {
			php_error_docref(NULL TSRMLS_CC, E_WARNING, "Row number cannot be less then 0");
			RETURN_FALSE;
		} else if (height < 0) {
			php_error_docref(NULL TSRMLS_CC, E_WARNING, "Height cannot be less then 0");
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
		zval *object = getThis();
		long row, col;
#if LIBXL_VERSION >= 0x03010000
		int rowFirst, rowLast, colFirst, colLast;
#else
		unsigned short rowFirst, rowLast, colFirst, colLast;
#endif

		if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ll", &row, &col) == FAILURE) {
			RETURN_FALSE;
		}

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
	zval *object = getThis();
	long row_s, col_s, row_e, col_e;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "llll", &row_s, &row_e, &col_s, &col_e) == FAILURE) {
		RETURN_FALSE;
	}

	SHEET_FROM_OBJECT(sheet, object);

	RETURN_BOOL(xlSheetSetMerge(sheet, row_s, row_e, col_s, col_e));
}
/* }}} */

/* {{{ proto bool ExcelSheet::deleteMerge(int row, int column)
	Delete cell merge */
EXCEL_METHOD(Sheet, deleteMerge)
{
	SheetHandle sheet;
	zval *object = getThis();
	long row, col;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ll", &row, &col) == FAILURE) {
		RETURN_FALSE;
	}

	SHEET_FROM_OBJECT(sheet, object);

	RETURN_BOOL(xlSheetDelMerge(sheet, row, col));
}
/* }}} */

#if LIBXL_VERSION >= 0x03040000
/* {{{ proto void ExcelSheet::addPictureScaled(int row, int column, int pic_id, double scale [, int x_offset [, int y_offset]])
	Insert picture into a cell with a set scale */
EXCEL_METHOD(Sheet, addPictureScaled)
{
	SheetHandle sheet;
	zval *object = getThis();
	long row, col, pic_id;
	long x_offset = 0, y_offset = 0;
	double scale;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "llld|ll", &row, &col, &pic_id, &scale, &x_offset, &y_offset) == FAILURE) {
		RETURN_FALSE;
	}

	SHEET_FROM_OBJECT(sheet, object);

	xlSheetSetPicture(sheet, row, col, pic_id, scale, x_offset, y_offset);
}
/* }}} */

/* {{{ proto void ExcelSheet::addPictureDim(int row, int column, int pic_id, int width, int height [, int x_offset [, int y_offset]])
	Insert picture into a cell with a given dimensions */
EXCEL_METHOD(Sheet, addPictureDim)
{
	SheetHandle sheet;
	zval *object = getThis();
	long row, col, pic_id, w, h;
	long x_offset = 0, y_offset = 0;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "lllll", &row, &col, &pic_id, &w, &h, &x_offset, &y_offset) == FAILURE) {
		RETURN_FALSE;
	}

	SHEET_FROM_OBJECT(sheet, object);

	xlSheetSetPicture2(sheet, row, col, pic_id, w, h, x_offset, y_offset);
}
/* }}} */
#else
/* {{{ proto void ExcelSheet::addPictureScaled(int row, int column, int pic_id, double scale)
	Insert picture into a cell with a set scale */
EXCEL_METHOD(Sheet, addPictureScaled)
{
	SheetHandle sheet;
	zval *object = getThis();
	long row, col, pic_id;
	double scale;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "llld", &row, &col, &pic_id, &scale) == FAILURE) {
		RETURN_FALSE;
	}

	SHEET_FROM_OBJECT(sheet, object);

	xlSheetSetPicture(sheet, row, col, pic_id, scale);
}
/* }}} */

/* {{{ proto void ExcelSheet::addPictureDim(int row, int column, int pic_id, int width, int height)
	Insert picture into a cell with a given dimensions */
EXCEL_METHOD(Sheet, addPictureDim)
{
	SheetHandle sheet;
	zval *object = getThis();
	long row, col, pic_id, w, h;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "lllll", &row, &col, &pic_id, &w, &h) == FAILURE) {
		RETURN_FALSE;
	}

	SHEET_FROM_OBJECT(sheet, object);

	xlSheetSetPicture2(sheet, row, col, pic_id, w, h);
}
/* }}} */
#endif

#define PHP_EXCEL_SHEET_SET_BREAK(func_name) \
	{ \
		SheetHandle sheet; \
		zval *object = getThis(); \
		long val; \
		zend_bool brk; \
		if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "lb", &val, &brk) == FAILURE) { \
			RETURN_FALSE; \
		} \
		SHEET_FROM_OBJECT(sheet, object); \
		RETURN_BOOL(xlSheet ## func_name (sheet, val, brk)); \
	}

/* {{{ proto bool ExcelSheet::horPageBreak(int row, bool break)
	Set/Remove horizontal page break */
EXCEL_METHOD(Sheet, horPageBreak)
{
	PHP_EXCEL_SHEET_SET_BREAK(SetHorPageBreak)
}
/* }}} */

/* {{{ proto bool ExcelSheet::verPageBreak(int col, bool break)
	Set/Remove vertical page break */
EXCEL_METHOD(Sheet, verPageBreak)
{
	PHP_EXCEL_SHEET_SET_BREAK(SetVerPageBreak)
}
/* }}} */

/* {{{ proto void ExcelSheet::splitSheet(int row, int column)
	Split sheet at indicated position */
EXCEL_METHOD(Sheet, splitSheet)
{
	SheetHandle sheet;
	zval *object = getThis();
	long row, col;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ll", &row, &col) == FAILURE) {
		RETURN_FALSE;
	}

	SHEET_FROM_OBJECT(sheet, object);

	xlSheetSplit(sheet, row, col);
}
/* }}} */

#define PHP_EXCEL_SHEET_GROUP(func_name) \
	{ \
		SheetHandle sheet; \
		zval *object = getThis(); \
		long s, e; \
		zend_bool brk = 0; \
		if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ll|b", &s, &e, &brk) == FAILURE) { \
			RETURN_FALSE; \
		} \
		SHEET_FROM_OBJECT(sheet, object); \
		RETURN_BOOL(xlSheet ## func_name (sheet, s, e, brk)); \
	}

/* {{{ proto bool ExcelSheet::groupRows(int start_row, int end_row [, bool collapse])
	Group rows from rowFirst to rowLast */
EXCEL_METHOD(Sheet, groupRows)
{
	PHP_EXCEL_SHEET_GROUP(GroupRows)
}
/* }}} */

/* {{{ proto bool ExcelSheet::groupCols(int start_column, int end_column [, bool collapse])
	Group columns from colFirst to colLast */
EXCEL_METHOD(Sheet, groupCols)
{
	PHP_EXCEL_SHEET_GROUP(GroupCols)
}
/* }}} */

/* {{{ proto void ExcelSheet::clear(int row_s, int row_e, int col_s, int col_e)
	Clear cells in specified area. */
EXCEL_METHOD(Sheet, clear)
{
	SheetHandle sheet;
	zval *object = getThis();
	long row_s, col_s, col_e, row_e;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "llll", &row_s, &row_e, &col_s, &col_e) == FAILURE) {
		RETURN_FALSE;
	}

	SHEET_FROM_OBJECT(sheet, object);

	xlSheetClear(sheet, row_s, row_e, col_s, col_e);
}
/* }}} */

/* {{{ proto void ExcelSheet::copy(int row, int col, int to_row, int to_col)
	Copy a cell */
EXCEL_METHOD(Sheet, copy)
{
	SheetHandle sheet;
	zval *object = getThis();
	long row, col, to_row, to_col;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "llll", &row, &col, &to_row, &to_col) == FAILURE) {
		RETURN_FALSE;
	}

	SHEET_FROM_OBJECT(sheet, object);

	RETURN_BOOL(xlSheetCopyCell(sheet, row, col, to_row, to_col));
}
/* }}} */

#define PE_RETURN_IS_LONG RETURN_LONG
#define PE_RETURN_IS_BOOL RETURN_BOOL
#define PE_RETURN_IS_DOUBLE RETURN_DOUBLE
#define PE_RETURN_IS_STRING(data) if (data) { RETURN_STRING((char *)data, 1) } else { RETURN_NULL(); }

#define PHP_EXCEL_INFO(func_name, type) \
{ \
	SheetHandle sheet; \
	zval *object = getThis(); \
	if (ZEND_NUM_ARGS()) { \
		RETURN_FALSE; \
	} \
	SHEET_FROM_OBJECT(sheet, object); \
	PE_RETURN_ ## type (xlSheet ## func_name (sheet)); \
}

#define PHP_EXCEL_SET_BOOL_VAL(func_name) \
	{ \
		SheetHandle sheet; \
		zval *object = getThis(); \
		zend_bool val; \
		if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "b", &val) == FAILURE) { \
			RETURN_FALSE; \
		} \
		SHEET_FROM_OBJECT(sheet, object); \
		xlSheet ## func_name (sheet, val); \
	}

#define PHP_EXCEL_SET_LONG_VAL(func_name) \
	{ \
		SheetHandle sheet; \
		zval *object = getThis(); \
		long val; \
		if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l", &val) == FAILURE) { \
			RETURN_FALSE; \
		} \
		SHEET_FROM_OBJECT(sheet, object); \
		xlSheet ## func_name (sheet, val); \
	}

#define PHP_EXCEL_SET_DOUBLE_VAL(func_name) \
	{ \
		SheetHandle sheet; \
		zval *object = getThis(); \
		double val; \
		if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "d", &val) == FAILURE) { \
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

#if LIBXL_VERSION >= 0x03020300
/* {{{ proto bool ExcelSheet::setHidden(bool value)
	Hides/unhides the sheet. */
EXCEL_METHOD(Sheet, setHidden)
{
	SheetHandle sheet;
	zval *object = getThis();
	zend_bool val;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "b", &val) == FAILURE) {
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
	zval *object = getThis();

	if (zend_parse_parameters_none() == FAILURE) {
		RETURN_FALSE;
	}

	SHEET_FROM_OBJECT(sheet, object);

	RETURN_BOOL(xlSheetHidden(sheet));
}
/* }}} */
#endif

#if LIBXL_VERSION >= 0x03020400
/* {{{ proto array ExcelSheet::getTopLeftView()
	Extracts the first visible row and the leftmost visible column of the sheet. */
EXCEL_METHOD(Sheet, getTopLeftView)
{
	SheetHandle sheet;
	zval *object = getThis();
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
	zval *object = getThis();
	long r,c;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ll", &r, &c) == FAILURE) {
		RETURN_FALSE;
	}

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
	zval *object = getThis();
	zend_bool row_relative = 1, col_relative = 1;
	long row, col;
	const char *cel_ref;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ll|bb", &row, &col, &row_relative, &col_relative) == FAILURE) {
		RETURN_FALSE;
	}

	SHEET_FROM_OBJECT(sheet, object);

	cel_ref = xlSheetRowColToAddr(sheet, row, col, row_relative, col_relative);
	if (!cel_ref) {
		RETURN_FALSE;
	}
	RETURN_STRING(cel_ref, 1);
}
/* }}} */

/* {{{ proto array ExcelSheet::addrToRowCol(string cell_reference)
	Converts a cell reference to row and column. */
EXCEL_METHOD(Sheet, addrToRowCol)
{
	SheetHandle sheet;
	zval *object = getThis();
	char *cell_reference;
	int cell_reference_len;
	int row = 0, col = 0, rowRelative = 0, colRelative = 0;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &cell_reference, &cell_reference_len) == FAILURE) {
		RETURN_FALSE;
	}

	if (!cell_reference_len) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Cell reference cannot be empty");
		RETURN_FALSE;
	}

	SHEET_FROM_OBJECT(sheet, object);

	xlSheetAddrToRowCol(sheet, cell_reference, &row, &col, &rowRelative, &colRelative);
	array_init(return_value);
	add_assoc_long(return_value, "row", row);
	add_assoc_long(return_value, "column", col);
	add_assoc_bool(return_value, "col_relative", colRelative);
	add_assoc_bool(return_value, "row_relative", rowRelative);
}
/* }}} */
#endif

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
		zval *object = getThis(); \
		char *val; \
		int val_len; \
		double margin; \
		if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "sd", &val, &val_len, &margin) == FAILURE || val_len > 255) { \
			RETURN_FALSE; \
		} \
		SHEET_FROM_OBJECT(sheet, object); \
		RETURN_BOOL(xlSheet ## func_name (sheet, val, margin)); \
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
	zval *object = getThis();
	char *val;
	int val_len;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &val, &val_len) == FAILURE) {
		RETURN_FALSE;
	}

	SHEET_FROM_OBJECT(sheet, object);

	xlSheetSetName(sheet, val);
}
/* }}} */

#if LIBXL_VERSION >= 0x03010000
/* {{{ proto bool ExcelSheet::setNamedRange(string name, int row, int col, int to_row, int to_col [, int scope_id])
	Create a named range */
EXCEL_METHOD(Sheet, setNamedRange)
{
	SheetHandle sheet;
	zval *object = getThis();
	long row, to_row, col, to_col;
	char *name;
	int name_len;
#if LIBXL_VERSION >= 0x03050401
	long scope_id = SCOPE_WORKBOOK;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "sllll|l", &name, &name_len, &row, &to_row, &col, &to_col, &scope_id) == FAILURE) {
#else
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "sllll", &name, &name_len, &row, &to_row, &col, &to_col) == FAILURE) {
#endif
		RETURN_FALSE;
	}

	if (!name_len) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "The range name cannot be empty.");
		RETURN_FALSE;
	}
	if (row > to_row) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "The range row start cannot be greater than row end.");
		RETURN_FALSE;
	} else if (col > to_col) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "The range column start cannot be greater than column end.");
		RETURN_FALSE;
	}

	SHEET_FROM_OBJECT(sheet, object);

#if LIBXL_VERSION >= 0x03050401
	RETURN_BOOL(xlSheetSetNamedRange(sheet, name, row, to_row, col, to_col, scope_id));
#else
	RETURN_BOOL(xlSheetSetNamedRange(sheet, name, row, to_row, col, to_col));
#endif
}
/* }}} */

/* {{{ proto bool ExcelSheet::delNamedRange(string name)
	Delete a named range. */
EXCEL_METHOD(Sheet, delNamedRange)
{
	SheetHandle sheet;
	zval *object = getThis();
	char *val;
	int val_len;
#if LIBXL_VERSION >= 0x03050401
	long scope_id = SCOPE_WORKBOOK;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s|l", &val, &val_len, &scope_id) == FAILURE) {
#else
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &val, &val_len) == FAILURE) {
#endif
		RETURN_FALSE;
	}

	if (!val_len) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "The range name cannot be empty.");
		RETURN_FALSE;
	}

	SHEET_FROM_OBJECT(sheet, object);

#if LIBXL_VERSION >= 0x03050401
	RETURN_BOOL(xlSheetDelNamedRange(sheet, val, scope_id));
#else
	RETURN_BOOL(xlSheetDelNamedRange(sheet, val));
#endif
}
/* }}} */

#define PHP_EXCEL_SHEET_PRINT_AREA(func_name) \
	{ \
		SheetHandle sheet; \
		zval *object = getThis(); \
		long s, e; \
		if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ll", &s, &e) == FAILURE) { \
			RETURN_FALSE; \
		} \
		if (s > e) { \
			php_error_docref(NULL TSRMLS_CC, E_WARNING, "The range start is greater than the end."); \
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
	PHP_EXCEL_SHEET_PRINT_AREA(SetPrintRepeatRows)
}
/* }}} */

/* {{{ proto bool ExcelSheet::setPrintRepeatCols(int colFirst, int colLast)
	Sets repeated columns on each page from colFirst to colLast. */
EXCEL_METHOD(Sheet, setPrintRepeatCols)
{
	PHP_EXCEL_SHEET_PRINT_AREA(SetPrintRepeatCols)
}
/* }}} */

/* {{{ proto bool ExcelSheet::getGroupSummaryBelow()
	Returns whether grouping rows summary is below. Returns 1 if summary is below and 0 if isn't. */
EXCEL_METHOD(Sheet, getGroupSummaryBelow)
{
	SheetHandle sheet;
	zval *object = getThis();

	if (ZEND_NUM_ARGS()) {
		RETURN_FALSE;
	}

	SHEET_FROM_OBJECT(sheet, object);
	RETURN_BOOL(xlSheetGroupSummaryBelow(sheet));
}
/* }}} */

/* {{{ proto bool ExcelSheet::setGroupSummaryBelow(bool direction)
	Sets a flag of grouping rows summary: 1 - below, 0 - above. */
EXCEL_METHOD(Sheet, setGroupSummaryBelow)
{
	SheetHandle sheet;
	zval *object = getThis();
	zend_bool val;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "b", &val) == FAILURE) {
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
	zval *object = getThis();

	if (ZEND_NUM_ARGS()) {
		RETURN_FALSE;
	}

	SHEET_FROM_OBJECT(sheet, object);
	RETURN_BOOL(xlSheetGroupSummaryRight(sheet));
}
/* }}} */

/* {{{ proto bool ExcelSheet::setGroupSummaryRight(bool direction)
	Sets a flag of grouping columns summary: 1 - right, 0 - left. */
EXCEL_METHOD(Sheet, setGroupSummaryRight)
{
	SheetHandle sheet;
	zval *object = getThis();
	zend_bool val;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "b", &val) == FAILURE) {
		RETURN_FALSE;
	}

	SHEET_FROM_OBJECT(sheet, object);
	xlSheetSetGroupSummaryRight(sheet, val);

	RETURN_TRUE;
}
/* }}} */

#if LIBXL_VERSION >= 0x03020000
/* {{{ proto bool ExcelSheet::setPrintFit(int wPages, int hPages)
	Fits sheet width and sheet height to wPages and hPages respectively. */
EXCEL_METHOD(Sheet, setPrintFit)
{
	SheetHandle sheet;
	zval *object = getThis();
	long wPages, hPages;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ll", &wPages, &hPages) == FAILURE) {
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
	zval *object = getThis();
	int wPages, hPages;

	if (ZEND_NUM_ARGS()) {
		RETURN_FALSE;
	}

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
	zval *object = getThis();
	char *name;
	int name_len;
	int rf, rl, cf, cl;
#if LIBXL_VERSION >= 0x03050401
	long scope_id = SCOPE_WORKBOOK;
	int hidden = 0;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s|l", &name, &name_len, &scope_id) == FAILURE) {
#else
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &name, &name_len) == FAILURE) {
#endif
		RETURN_FALSE;
	}

	SHEET_FROM_OBJECT(sheet, object);
#if LIBXL_VERSION >= 0x03050401
	if (xlSheetGetNamedRange(sheet, name, &rf, &rl, &cf, &cl, scope_id, &hidden)) {
#else
	if (xlSheetGetNamedRange(sheet, name, &rf, &rl, &cf, &cl)) {
#endif
		array_init(return_value);
		add_assoc_long(return_value, "row_first", rf);
		add_assoc_long(return_value, "row_last", rl);
		add_assoc_long(return_value, "col_first", cf);
		add_assoc_long(return_value, "col_last", cl);
#if LIBXL_VERSION >= 0x03050401
		add_assoc_bool(return_value, "hidden", hidden);
#endif
	} else {
		RETURN_FALSE;
	}
}

/* {{{ proto array ExcelSheet::getIndexRange(int index [, int scope_id])
	Gets the named range coordinates by index, returns false if range is not found. */
EXCEL_METHOD(Sheet, getIndexRange)
{
	SheetHandle sheet;
	zval *object = getThis();
	long index;
	int rf, rl, cf, cl;
#if LIBXL_VERSION >= 0x03050401
	int hidden;
	long scope_id = SCOPE_WORKBOOK;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l|l", &index, &scope_id) == FAILURE) {
		RETURN_FALSE;
	}
#else
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l", &index) == FAILURE) {
		RETURN_FALSE;
	}
#endif

	SHEET_FROM_OBJECT(sheet, object);
#if LIBXL_VERSION >= 0x03050401
	if (xlSheetNamedRange(sheet, (int)index, &rf, &rl, &cf, &cl, (int *)&scope_id, &hidden)) {
#else
	if (xlSheetNamedRange(sheet, (int)index, &rf, &rl, &cf, &cl)) {
#endif
		array_init(return_value);
		add_assoc_long(return_value, "row_first", rf);
		add_assoc_long(return_value, "row_last", rl);
		add_assoc_long(return_value, "col_first", cf);
		add_assoc_long(return_value, "col_last", cl);
#if LIBXL_VERSION >= 0x03050401
		add_assoc_bool(return_value, "hidden", hidden);
		add_assoc_long(return_value, "scope", scope_id);
#endif
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
	zval *object = getThis();

	if (ZEND_NUM_ARGS()) {
		RETURN_FALSE;
	}

	SHEET_FROM_OBJECT(sheet, object);
	RETURN_LONG(xlSheetNamedRangeSize(sheet));
}
/* }}} */

/* {{{ proto long ExcelSheet::getVerPageBreak(int index)
	Returns column with vertical page break at position index. */
EXCEL_METHOD(Sheet, getVerPageBreak)
{
	SheetHandle sheet;
	zval *object = getThis();
	long index;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l", &index) == FAILURE) {
		RETURN_FALSE;
	}

	SHEET_FROM_OBJECT(sheet, object);
	RETURN_LONG(xlSheetGetVerPageBreak(sheet, index));
}
/* }}} */

/* {{{ proto long ExcelSheet::getVerPageBreakSize()
	Returns a number of vertical page breaks in the sheet. */
EXCEL_METHOD(Sheet, getVerPageBreakSize)
{
	SheetHandle sheet;
	zval *object = getThis();

	if (ZEND_NUM_ARGS()) {
		RETURN_FALSE;
	}

	SHEET_FROM_OBJECT(sheet, object);
	RETURN_LONG(xlSheetGetVerPageBreakSize(sheet));
}
/* }}} */

/* {{{ proto long ExcelSheet::getHorPageBreak(int index)
	Returns column with horizontal page break at position index. */
EXCEL_METHOD(Sheet, getHorPageBreak)
{
	SheetHandle sheet;
	zval *object = getThis();
	long index;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l", &index) == FAILURE) {
		RETURN_FALSE;
	}

	SHEET_FROM_OBJECT(sheet, object);
	RETURN_LONG(xlSheetGetHorPageBreak(sheet, index));
}
/* }}} */

/* {{{ proto long ExcelSheet::getHorPageBreakSize()
	Returns a number of horizontal page breaks in the sheet. */
EXCEL_METHOD(Sheet, getHorPageBreakSize)
{
	SheetHandle sheet;
	zval *object = getThis();

	if (ZEND_NUM_ARGS()) {
		RETURN_FALSE;
	}

	SHEET_FROM_OBJECT(sheet, object);
	RETURN_LONG(xlSheetGetHorPageBreakSize(sheet));
}
/* }}} */

/* {{{ proto array ExcelSheet::getPictureInfo(int index)
	Returns a information about a workbook picture at position index in worksheet. */
EXCEL_METHOD(Sheet, getPictureInfo)
{
	SheetHandle sheet;
	zval *object = getThis();
	long index;
	int rowTop, colLeft, rowBottom, colRight, width, height, offset_x, offset_y;
	int pic_index;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l", &index) == FAILURE) {
		RETURN_FALSE;
	}

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
	zval *object = getThis();

	if (ZEND_NUM_ARGS()) {
		RETURN_FALSE;
	}

	SHEET_FROM_OBJECT(sheet, object);
	RETURN_LONG(xlSheetPictureSize(sheet));
}
/* }}} */

/* {{{ proto long ExcelBook::biffVersion()
	Returns BIFF version of binary file. Used for xls format only. */
EXCEL_METHOD(Book, biffVersion)
{
	BookHandle book;
	zval *object = getThis();
	int version;

	if (ZEND_NUM_ARGS()) {
		RETURN_FALSE;
	}

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
	zval *object = getThis();

	if (ZEND_NUM_ARGS()) {
		RETURN_FALSE;
	}

	BOOK_FROM_OBJECT(book, object);
	RETURN_BOOL(xlBookRefR1C1(book));
}
/* }}} */

/* {{{ proto void ExcelBook::setRefR1C1(bool active)
	Sets the R1C1 reference mode. */
EXCEL_METHOD(Book, setRefR1C1)
{
	BookHandle book;
	zval *object = getThis();
	zend_bool active;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "b", &active) == FAILURE) {
		RETURN_FALSE;
	}

	BOOK_FROM_OBJECT(book, object);
	xlBookSetRefR1C1(book, (int)active);
}
/* }}} */

/* {{{ proto array ExcelBook::getPicture(int picture_index)
	Returns a picture at position index. */
EXCEL_METHOD(Book, getPicture)
{
	BookHandle book;
	zval *object = getThis();
	long index;
	int type;
	const char *buf;
	unsigned int buf_len;
	enum PictureType {PICTURETYPE_PNG, PICTURETYPE_JPEG, PICTURETYPE_WMF, PICTURETYPE_DIB, PICTURETYPE_EMF, PICTURETYPE_PICT, PICTURETYPE_TIFF, PICTURETYPE_ERROR = 0xFF};

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l", &index) == FAILURE) {
		RETURN_FALSE;
	}

	BOOK_FROM_OBJECT(book, object);

	if ((type = xlBookGetPicture(book, (int)index, &buf, &buf_len)) == PICTURETYPE_ERROR) {
		RETURN_FALSE;
	}

	array_init(return_value);
	add_assoc_stringl(return_value, "data", (char *)buf, buf_len, 1);
	add_assoc_long(return_value, "type", type);
}
/* }}} */

/* {{{ proto long ExcelBook::getNumPictures()
	Returns a number of pictures in this workbook. */
EXCEL_METHOD(Book, getNumPictures)
{
	BookHandle book;
	zval *object = getThis();

	if (ZEND_NUM_ARGS()) {
		RETURN_FALSE;
	}

	BOOK_FROM_OBJECT(book, object);
	RETURN_LONG(xlBookPictureSize(book));
}
/* }}} */

/* {{{ proto ExcelSheet ExcelBook::insertSheet(int index, string name [, ExcelSheet sh])
	Inserts a new sheet to this book at position index, returns the sheet handle. Set initSheet to 0 if you wish to add a new empty sheet or use existing sheet's handle for copying. */
EXCEL_METHOD(Book, insertSheet)
{
	BookHandle book;
	zval *object = getThis();
	zval *shz = NULL;
	SheetHandle sh, sheet;
	excel_sheet_object *fo;
	char *name;
	int name_len;
	long index;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ls|o", &index, &name, &name_len, &shz) == FAILURE) {
		RETURN_FALSE;
	}

	BOOK_FROM_OBJECT(book, object);
	if (shz) {
		SHEET_FROM_OBJECT(sheet, shz);
		if (!(sh = xlBookInsertSheet(book, index, name, sheet))) {
			RETURN_FALSE;
		}
	} else {
		if (!(sh = xlBookInsertSheet(book, index, name, 0))) {
			RETURN_FALSE;
		}
	}

	Z_TYPE_P(return_value) = IS_OBJECT;
	object_init_ex(return_value, excel_ce_sheet);
	Z_SET_REFCOUNT_P(return_value, 1);
	Z_SET_ISREF_P(return_value);
	fo = (excel_sheet_object *) zend_object_store_get_object(return_value TSRMLS_CC);
	fo->sheet = sh;
	fo->book = book;
}
/* }}} */

#endif

#if LIBXL_VERSION >= 0x03050401
/* {{{ proto bool ExcelBook::isTemplate()
	Returns whether the workbook is template. */
EXCEL_METHOD(Book, isTemplate)
{
	BookHandle book;
	zval *object = getThis();

	if (ZEND_NUM_ARGS()) {
		RETURN_FALSE;
	}

	BOOK_FROM_OBJECT(book, object);
	RETURN_BOOL(xlBookIsTemplate(book));
}
/* }}} */

/* {{{ proto void ExcelBook::setTemplate(bool mode)
	Sets the template flag, if the workbook is template. */
EXCEL_METHOD(Book, setTemplate)
{
	BookHandle book;
	zval *object = getThis();
	zend_bool mode;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "b", &mode) == FAILURE) {
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
	zval *object = getThis();

	if (ZEND_NUM_ARGS()) {
		RETURN_FALSE;
	}

	SHEET_FROM_OBJECT(sheet, object);
	RETURN_LONG(xlSheetRightToLeft(sheet));
}
/* }}} */

/* {{{ proto void ExcelBook::setRightToLeft(bool mode)
	Sets the right-to-left mode: 1 - the text is displayed in right-to-left mode, 0 - the text is displayed in left-to-right mode. */
EXCEL_METHOD(Sheet, setRightToLeft)
{
	SheetHandle sheet;
	zval *object = getThis();
	long mode;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l", &mode) == FAILURE) {
		RETURN_FALSE;
	}

	SHEET_FROM_OBJECT(sheet, object);
	xlSheetSetRightToLeft(sheet, (int)mode);
}
/* }}} */
#endif

/* {{{ proto bool ExcelSheet::setPrintArea()
	Sets the print area. */
EXCEL_METHOD(Sheet, setPrintArea)
{
	zval *object = getThis();
	SheetHandle sheet;
	long row, col, to_row, to_col;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "llll", &row, &to_row, &col, &to_col) == FAILURE) {
		RETURN_FALSE;
	}

	if (row > to_row) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "The range row start cannot be greater than row end.");
		RETURN_FALSE;
	} else if (col > to_col) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "The range column start cannot be greater than column end.");
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
	zval *object = getThis();
	SheetHandle sheet;

	if (ZEND_NUM_ARGS()) {
		RETURN_FALSE;
	}

	SHEET_FROM_OBJECT(sheet, object);
	xlSheetClearPrintRepeats(sheet);

	RETURN_TRUE;
}
/* }}} */

/* {{{ proto bool ExcelSheet::clearPrintArea()
	Clears the print area. */
EXCEL_METHOD(Sheet, clearPrintArea)
{
	zval *object = getThis();
	SheetHandle sheet;

	if (ZEND_NUM_ARGS()) {
		RETURN_FALSE;
	}

	SHEET_FROM_OBJECT(sheet, object);
	xlSheetClearPrintArea(sheet);

	RETURN_TRUE;
}
/* }}} */
#endif

/* {{{ proto bool ExcelSheet::protect()
	Returns whether sheet is protected: 1 - yes, 0 - no. */
EXCEL_METHOD(Sheet, protect)
{
	PHP_EXCEL_INFO(Protect, IS_BOOL)
}
/* }}} */

/* {{{ proto void ExcelSheet::setProtect(bool value)
	Protects (protect = 1) or unprotects (protect = 0) the sheet. */
EXCEL_METHOD(Sheet, setProtect)
{
	PHP_EXCEL_SET_BOOL_VAL(SetProtect)
}
/* }}} */

#if LIBXL_VERSION >= 0x03060000
/* {{{ proto long ExcelSheet::hyperlinkSize()
	Returns the number of hyperlinks in the sheet. */
EXCEL_METHOD(Sheet, hyperlinkSize)
{
	zval *object = getThis();
	SheetHandle sheet;

	if (ZEND_NUM_ARGS()) {
		RETURN_FALSE;
	}

	SHEET_FROM_OBJECT(sheet, object);
	RETURN_LONG(xlSheetHyperlinkSize(sheet));
}
/* }}} */

/* {{{ proto array ExcelSheet::hyperlink(int index)
	Gets the hyperlink and its coordinates by index. */
EXCEL_METHOD(Sheet, hyperlink)
{
	SheetHandle sheet;
	zval *object = getThis();
	long index;
	int rowFirst, rowLast, colFirst, colLast;
	const char *s;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l", &index) == FAILURE) {
		RETURN_FALSE;
	}

	SHEET_FROM_OBJECT(sheet, object);

	s = xlSheetHyperlink(sheet, index, &rowFirst, &rowLast, &colFirst, &colLast);

	if (!s) {
		RETURN_FALSE;
	}

	array_init(return_value);
	add_assoc_string(return_value, "hyperlink", (char *)s, 1);
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
	zval *object = getThis();
	SheetHandle sheet;
	long index;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l", &index) == FAILURE) {
		RETURN_FALSE;
	}

	if (index < 0) {
		RETURN_FALSE;
	}

	SHEET_FROM_OBJECT(sheet, object);
	RETURN_BOOL(xlSheetDelHyperlink(sheet, index));
}
/* }}} */

/* {{{ proto void ExcelSheet::addHyperlink(string hyperlink, int row_first, int row_last, int col_first, int col_last)
	Adds the new hyperlink. */
EXCEL_METHOD(Sheet, addHyperlink)
{
	SheetHandle sheet;
	zval *object = getThis();
	char *val;
	int val_len;
	long row_first, row_last, col_first, col_last;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "sllll", &val, &val_len, &row_first, &row_last, &col_first, &col_last) == FAILURE) {
		RETURN_FALSE;
	}

	SHEET_FROM_OBJECT(sheet, object);

	xlSheetAddHyperlink(sheet, val, row_first, row_last, col_first, col_last);
	RETURN_TRUE;
}
/* }}} */

/* {{{ proto long ExcelSheet::mergeSize()
	Returns a number of merged cells in this worksheet. */
EXCEL_METHOD(Sheet, mergeSize)
{
	zval *object = getThis();
	SheetHandle sheet;

	if (ZEND_NUM_ARGS()) {
		RETURN_FALSE;
	}

	SHEET_FROM_OBJECT(sheet, object);
	RETURN_LONG(xlSheetMergeSize(sheet));
}
/* }}} */

/* {{{ proto array ExcelSheet::merge(int index)
	Gets the merged cells by index. */
EXCEL_METHOD(Sheet, merge)
{
	SheetHandle sheet;
	zval *object = getThis();
	long index;
	int rowFirst, rowLast, colFirst, colLast;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l", &index) == FAILURE) {
		RETURN_FALSE;
	}

	if (index < 0) {
		RETURN_FALSE;
	}

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
	zval *object = getThis();
	SheetHandle sheet;
	long index;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l", &index) == FAILURE) {
		RETURN_FALSE;
	}

	if (index < 0) {
		RETURN_FALSE;
	}

	SHEET_FROM_OBJECT(sheet, object);
	RETURN_BOOL(xlSheetDelMergeByIndex(sheet, index));
}
/* }}} */

/* {{{ proto bool ExcelSheet::splitInfo()
	Gets the split information (position of frozen pane) in the sheet: row - vertical position of the split; col - horizontal position of the split. */
EXCEL_METHOD(Sheet, splitInfo)
{
	SheetHandle sheet;
	zval *object = getThis();
	int row, col;

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
	zval *object = getThis();
	SheetHandle sheet;
	long row;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l", &row) == FAILURE) {
		RETURN_FALSE;
	}

	if (row < 0) {
		RETURN_FALSE;
	}

	SHEET_FROM_OBJECT(sheet, object);
	RETURN_BOOL(xlSheetRowHidden(sheet, row));
}
/* }}} */

/* {{{ proto bool ExcelSheet::setRowHidden(int row, bool hidden)
	Hides row. */
EXCEL_METHOD(Sheet, setRowHidden)
{
	zval *object = getThis();
	SheetHandle sheet;
	long row;
	zend_bool hidden;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "lb", &row, &hidden) == FAILURE) {
		RETURN_FALSE;
	}

	if (row < 0) {
		RETURN_FALSE;
	}

	SHEET_FROM_OBJECT(sheet, object);
	RETURN_BOOL(xlSheetSetRowHidden(sheet, row, hidden));
}
/* }}} */

/* {{{ proto bool ExcelSheet::colHidden(int col)
	Returns whether column is hidden. */
EXCEL_METHOD(Sheet, colHidden)
{
	zval *object = getThis();
	SheetHandle sheet;
	long col;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l", &col) == FAILURE) {
		RETURN_FALSE;
	}

	if (col < 0) {
		RETURN_FALSE;
	}

	SHEET_FROM_OBJECT(sheet, object);
	RETURN_BOOL(xlSheetColHidden(sheet, col));
}
/* }}} */

/* {{{ proto bool ExcelSheet::setColHidden(int col, bool hidden)
	Hides column. */
EXCEL_METHOD(Sheet, setColHidden)
{
	zval *object = getThis();
	SheetHandle sheet;
	long col;
	zend_bool hidden;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "lb", &col, &hidden) == FAILURE) {
		RETURN_FALSE;
	}

	if (col < 0) {
		RETURN_FALSE;
	}

	SHEET_FROM_OBJECT(sheet, object);
	RETURN_BOOL(xlSheetSetColHidden(sheet, col, hidden));
}
/* }}} */

/* {{{ proto bool ExcelSheet::isLicensed()
	Get license status */
EXCEL_METHOD(Sheet, isLicensed)
{
	char *err;
	zval *object = getThis();
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

/* {{{ proto long ExcelBook::sheetType(int sheet)
	Returns type of sheet with specified index. */
EXCEL_METHOD(Book, sheetType)
{
	zval *object = getThis();
	BookHandle book;
	long index;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l", &index) == FAILURE) {
		RETURN_FALSE;
	}

	if (index < 0) {
		RETURN_FALSE;
	}

	BOOK_FROM_OBJECT(book, object);
	RETURN_LONG(xlBookSheetType(book, index));
}
/* }}} */
#endif

#if PHP_MAJOR_VERSION > 5 || (PHP_MAJOR_VERSION == 5 && PHP_MINOR_VERSION >= 3)
# define PHP_EXCEL_ARGINFO
# else
# define PHP_EXCEL_ARGINFO static
#endif
PHP_EXCEL_ARGINFO
ZEND_BEGIN_ARG_INFO_EX(arginfo_Book_load, 0, 0, 1)
	ZEND_ARG_INFO(0, data)
ZEND_END_ARG_INFO()

PHP_EXCEL_ARGINFO
ZEND_BEGIN_ARG_INFO_EX(arginfo_Book_loadFile, 0, 0, 1)
	ZEND_ARG_INFO(0, filename)
ZEND_END_ARG_INFO()

PHP_EXCEL_ARGINFO
ZEND_BEGIN_ARG_INFO_EX(arginfo_Book_save, 0, 0, 0)
	ZEND_ARG_INFO(0, filename)
ZEND_END_ARG_INFO()

PHP_EXCEL_ARGINFO
ZEND_BEGIN_ARG_INFO_EX(arginfo_Book_getSheet, 0, 0, 0)
	ZEND_ARG_INFO(0, sheet)
ZEND_END_ARG_INFO()

PHP_EXCEL_ARGINFO
ZEND_BEGIN_ARG_INFO_EX(arginfo_Book_getSheetByName, 0, 0, 1)
	ZEND_ARG_INFO(0, name)
	ZEND_ARG_INFO(0, case_insensitive)
ZEND_END_ARG_INFO()

PHP_EXCEL_ARGINFO
ZEND_BEGIN_ARG_INFO_EX(arginfo_Book_deleteSheet, 0, 0, 1)
	ZEND_ARG_INFO(0, sheet)
ZEND_END_ARG_INFO()

PHP_EXCEL_ARGINFO
ZEND_BEGIN_ARG_INFO_EX(arginfo_Book_activeSheet, 0, 0, 0)
	ZEND_ARG_INFO(0, sheet)
ZEND_END_ARG_INFO()

PHP_EXCEL_ARGINFO
ZEND_BEGIN_ARG_INFO_EX(arginfo_Book_addSheet, 0, 0, 1)
	ZEND_ARG_INFO(0, name)
ZEND_END_ARG_INFO()

PHP_EXCEL_ARGINFO
ZEND_BEGIN_ARG_INFO_EX(arginfo_Book_copySheet, 0, 0, 2)
	ZEND_ARG_INFO(0, name)
	ZEND_ARG_INFO(0, sheet_number)
ZEND_END_ARG_INFO()

PHP_EXCEL_ARGINFO
ZEND_BEGIN_ARG_INFO_EX(arginfo_Book_sheetCount, 0, 0, 0)
ZEND_END_ARG_INFO()

PHP_EXCEL_ARGINFO
ZEND_BEGIN_ARG_INFO_EX(arginfo_Book_getError, 0, 0, 0)
ZEND_END_ARG_INFO()

PHP_EXCEL_ARGINFO
ZEND_BEGIN_ARG_INFO_EX(arginfo_Book_addFont, 0, 0, 0)
	ZEND_ARG_OBJ_INFO(0, font, ExcelFont, 0)
ZEND_END_ARG_INFO()

PHP_EXCEL_ARGINFO
ZEND_BEGIN_ARG_INFO_EX(arginfo_Book_addFormat, 0, 0, 0)
	ZEND_ARG_OBJ_INFO(0, format, ExcelFormat, 1)
ZEND_END_ARG_INFO()

#ifdef HAVE_LIBXL_243_PLUS
PHP_EXCEL_ARGINFO
ZEND_BEGIN_ARG_INFO_EX(arginfo_Book_getAllFormats, 0, 0, 0)
ZEND_END_ARG_INFO()
#endif

PHP_EXCEL_ARGINFO
ZEND_BEGIN_ARG_INFO_EX(arginfo_Book_addCustomFormat, 0, 0, 1)
	ZEND_ARG_INFO(0, format)
ZEND_END_ARG_INFO()

PHP_EXCEL_ARGINFO
ZEND_BEGIN_ARG_INFO_EX(arginfo_Book_getCustomFormat, 0, 0, 1)
	ZEND_ARG_INFO(0, id)
ZEND_END_ARG_INFO()

PHP_EXCEL_ARGINFO
ZEND_BEGIN_ARG_INFO_EX(arginfo_Book_packDate, 0, 0, 1)
	ZEND_ARG_INFO(0, timestamp)
ZEND_END_ARG_INFO()

PHP_EXCEL_ARGINFO
ZEND_BEGIN_ARG_INFO_EX(arginfo_Book_packDateValues, 0, 0, 6)
	ZEND_ARG_INFO(0, year)
	ZEND_ARG_INFO(0, month)
	ZEND_ARG_INFO(0, day)
	ZEND_ARG_INFO(0, hour)
	ZEND_ARG_INFO(0, min)
	ZEND_ARG_INFO(0, sec)
ZEND_END_ARG_INFO()

PHP_EXCEL_ARGINFO
ZEND_BEGIN_ARG_INFO_EX(arginfo_Book_unpackDate, 0, 0, 1)
	ZEND_ARG_INFO(0, date)
ZEND_END_ARG_INFO()

PHP_EXCEL_ARGINFO
ZEND_BEGIN_ARG_INFO_EX(arginfo_Book_getActiveSheet, 0, 0, 0)
ZEND_END_ARG_INFO()

PHP_EXCEL_ARGINFO
ZEND_BEGIN_ARG_INFO_EX(arginfo_Book_getDefaultFont, 0, 0, 0)
ZEND_END_ARG_INFO()

#if LIBXL_VERSION >= 0x03050300
PHP_EXCEL_ARGINFO
ZEND_BEGIN_ARG_INFO_EX(arginfo_Book_isDate1904, 0, 0, 0)
ZEND_END_ARG_INFO()

PHP_EXCEL_ARGINFO
ZEND_BEGIN_ARG_INFO_EX(arginfo_Book_setDate1904, 0, 0, 1)
	ZEND_ARG_INFO(0, date_type)
ZEND_END_ARG_INFO()
#endif

PHP_EXCEL_ARGINFO
ZEND_BEGIN_ARG_INFO_EX(arginfo_Book_setDefaultFont, 0, 0, 2)
	ZEND_ARG_INFO(0, font)
	ZEND_ARG_INFO(0, font_size)
ZEND_END_ARG_INFO()

PHP_EXCEL_ARGINFO
ZEND_BEGIN_ARG_INFO_EX(arginfo_Book_setLocale, 0, 0, 1)
	ZEND_ARG_INFO(0, locale)
ZEND_END_ARG_INFO()

PHP_EXCEL_ARGINFO
ZEND_BEGIN_ARG_INFO_EX(arginfo_Book___construct, 0, 0, 0)
	ZEND_ARG_INFO(0, license_name)
	ZEND_ARG_INFO(0, license_key)
	ZEND_ARG_INFO(0, excel_2007)
ZEND_END_ARG_INFO()

PHP_EXCEL_ARGINFO
ZEND_BEGIN_ARG_INFO_EX(arginfo_Book_setActiveSheet, 0, 0, 1)
	ZEND_ARG_INFO(0, sheet)
ZEND_END_ARG_INFO()

PHP_EXCEL_ARGINFO
ZEND_BEGIN_ARG_INFO_EX(arginfo_Book_addPictureFromFile, 0, 0, 1)
	ZEND_ARG_INFO(0, filename)
ZEND_END_ARG_INFO()

PHP_EXCEL_ARGINFO
ZEND_BEGIN_ARG_INFO_EX(arginfo_Book_addPictureFromString, 0, 0, 1)
	ZEND_ARG_INFO(0, data)
ZEND_END_ARG_INFO()

#ifdef LIBXL_VERSION
PHP_EXCEL_ARGINFO
ZEND_BEGIN_ARG_INFO_EX(arginfo_Book_rgbMode, 0, 0, 0)
ZEND_END_ARG_INFO()

PHP_EXCEL_ARGINFO
ZEND_BEGIN_ARG_INFO_EX(arginfo_Book_setRGBMode, 0, 0, 1)
	ZEND_ARG_INFO(0, mode)
ZEND_END_ARG_INFO()

PHP_EXCEL_ARGINFO
ZEND_BEGIN_ARG_INFO_EX(arginfo_Book_colorPack, 0, 0, 3)
	ZEND_ARG_INFO(0, r)
	ZEND_ARG_INFO(0, g)
	ZEND_ARG_INFO(0, b)
ZEND_END_ARG_INFO()

PHP_EXCEL_ARGINFO
ZEND_BEGIN_ARG_INFO_EX(arginfo_Book_colorUnpack, 0, 0, 1)
	ZEND_ARG_INFO(0, color)
ZEND_END_ARG_INFO()
#endif

#if LIBXL_VERSION >= 0x03020000
PHP_EXCEL_ARGINFO
ZEND_BEGIN_ARG_INFO_EX(arginfo_Book_setRefR1C1, 0, 0, 1)
	ZEND_ARG_INFO(0, active)
ZEND_END_ARG_INFO()

PHP_EXCEL_ARGINFO
ZEND_BEGIN_ARG_INFO_EX(arginfo_Book_biffVersion, 0, 0, 0)
ZEND_END_ARG_INFO()

PHP_EXCEL_ARGINFO
ZEND_BEGIN_ARG_INFO_EX(arginfo_Book_getRefR1C1, 0, 0, 0)
ZEND_END_ARG_INFO()

PHP_EXCEL_ARGINFO
ZEND_BEGIN_ARG_INFO_EX(arginfo_Book_getPicture, 0, 0, 1)
	ZEND_ARG_INFO(0, index)
ZEND_END_ARG_INFO()

PHP_EXCEL_ARGINFO
ZEND_BEGIN_ARG_INFO_EX(arginfo_Book_getNumPictures, 0, 0, 0)
ZEND_END_ARG_INFO()

PHP_EXCEL_ARGINFO
ZEND_BEGIN_ARG_INFO_EX(arginfo_Book_insertSheet, 0, 0, 2)
	ZEND_ARG_INFO(0, index)
	ZEND_ARG_INFO(0, name)
	ZEND_ARG_OBJ_INFO(0, sheet, ExcelSheet, 0)
ZEND_END_ARG_INFO()
#endif

#if LIBXL_VERSION >= 0x03050401
PHP_EXCEL_ARGINFO
ZEND_BEGIN_ARG_INFO_EX(arginfo_Book_isTemplate, 0, 0, 0)
ZEND_END_ARG_INFO()

PHP_EXCEL_ARGINFO
ZEND_BEGIN_ARG_INFO_EX(arginfo_Book_setTemplate, 0, 0, 1)
	ZEND_ARG_INFO(0, mode)
ZEND_END_ARG_INFO()

PHP_EXCEL_ARGINFO
ZEND_BEGIN_ARG_INFO_EX(arginfo_Sheet_getRightToLeft, 0, 0, 0)
ZEND_END_ARG_INFO()

PHP_EXCEL_ARGINFO
ZEND_BEGIN_ARG_INFO_EX(arginfo_Sheet_setRightToLeft, 0, 0, 1)
	ZEND_ARG_INFO(0, mode)
ZEND_END_ARG_INFO()
#endif

#if LIBXL_VERSION >= 0x03060000
PHP_EXCEL_ARGINFO
ZEND_BEGIN_ARG_INFO_EX(arginfo_Book_sheetType, 0, 0, 1)
	ZEND_ARG_INFO(0, sheet)
ZEND_END_ARG_INFO()
#endif

PHP_EXCEL_ARGINFO
ZEND_BEGIN_ARG_INFO_EX(arginfo_Font_size, 0, 0, 0)
	ZEND_ARG_INFO(0, size)
ZEND_END_ARG_INFO()

PHP_EXCEL_ARGINFO
ZEND_BEGIN_ARG_INFO_EX(arginfo_Font_italics, 0, 0, 0)
	ZEND_ARG_INFO(0, size)
ZEND_END_ARG_INFO()

PHP_EXCEL_ARGINFO
ZEND_BEGIN_ARG_INFO_EX(arginfo_Font_strike, 0, 0, 0)
	ZEND_ARG_INFO(0, strike)
ZEND_END_ARG_INFO()

PHP_EXCEL_ARGINFO
ZEND_BEGIN_ARG_INFO_EX(arginfo_Font_bold, 0, 0, 0)
	ZEND_ARG_INFO(0, bold)
ZEND_END_ARG_INFO()

PHP_EXCEL_ARGINFO
ZEND_BEGIN_ARG_INFO_EX(arginfo_Font_color, 0, 0, 0)
	ZEND_ARG_INFO(0, color)
ZEND_END_ARG_INFO()

PHP_EXCEL_ARGINFO
ZEND_BEGIN_ARG_INFO_EX(arginfo_Font_mode, 0, 0, 0)
	ZEND_ARG_INFO(0, mode)
ZEND_END_ARG_INFO()

PHP_EXCEL_ARGINFO
ZEND_BEGIN_ARG_INFO_EX(arginfo_Font_underline, 0, 0, 0)
	ZEND_ARG_INFO(0, underline_style)
ZEND_END_ARG_INFO()

PHP_EXCEL_ARGINFO
ZEND_BEGIN_ARG_INFO_EX(arginfo_Font_name, 0, 0, 0)
	ZEND_ARG_INFO(0, name)
ZEND_END_ARG_INFO()

PHP_EXCEL_ARGINFO
ZEND_BEGIN_ARG_INFO_EX(arginfo_Format___construct, 0, 0, 1)
	ZEND_ARG_OBJ_INFO(0, book, ExcelBook, 0)
ZEND_END_ARG_INFO()

PHP_EXCEL_ARGINFO
ZEND_BEGIN_ARG_INFO_EX(arginfo_Font___construct, 0, 0, 1)
	ZEND_ARG_OBJ_INFO(0, book, ExcelBook, 0)
ZEND_END_ARG_INFO()

PHP_EXCEL_ARGINFO
ZEND_BEGIN_ARG_INFO_EX(arginfo_Format_setFont, 0, 0, 1)
	ZEND_ARG_OBJ_INFO(0, font, ExcelFont, 0)
ZEND_END_ARG_INFO()

PHP_EXCEL_ARGINFO
ZEND_BEGIN_ARG_INFO_EX(arginfo_Format_getFont, 0, 0, 0)
ZEND_END_ARG_INFO()

PHP_EXCEL_ARGINFO
ZEND_BEGIN_ARG_INFO_EX(arginfo_Format_numberFormat, 0, 0, 0)
	ZEND_ARG_INFO(0, format)
ZEND_END_ARG_INFO()

PHP_EXCEL_ARGINFO
ZEND_BEGIN_ARG_INFO_EX(arginfo_Format_horizontalAlign, 0, 0, 0)
	ZEND_ARG_INFO(0, align_mode)
ZEND_END_ARG_INFO()

PHP_EXCEL_ARGINFO
ZEND_BEGIN_ARG_INFO_EX(arginfo_Format_verticalAlign, 0, 0, 0)
	ZEND_ARG_INFO(0, align_mode)
ZEND_END_ARG_INFO()

PHP_EXCEL_ARGINFO
ZEND_BEGIN_ARG_INFO_EX(arginfo_Format_wrap, 0, 0, 0)
	ZEND_ARG_INFO(0, wrap)
ZEND_END_ARG_INFO()

PHP_EXCEL_ARGINFO
ZEND_BEGIN_ARG_INFO_EX(arginfo_Format_rotate, 0, 0, 0)
	ZEND_ARG_INFO(0, angle)
ZEND_END_ARG_INFO()

PHP_EXCEL_ARGINFO
ZEND_BEGIN_ARG_INFO_EX(arginfo_Format_indent, 0, 0, 0)
	ZEND_ARG_INFO(0, indent)
ZEND_END_ARG_INFO()

PHP_EXCEL_ARGINFO
ZEND_BEGIN_ARG_INFO_EX(arginfo_Format_shrinkToFit, 0, 0, 0)
	ZEND_ARG_INFO(0, shrink)
ZEND_END_ARG_INFO()

PHP_EXCEL_ARGINFO
ZEND_BEGIN_ARG_INFO_EX(arginfo_Format_borderStyle, 0, 0, 0)
	ZEND_ARG_INFO(0, style)
ZEND_END_ARG_INFO()

PHP_EXCEL_ARGINFO
ZEND_BEGIN_ARG_INFO_EX(arginfo_Format_borderColor, 0, 0, 0)
	ZEND_ARG_INFO(0, color)
ZEND_END_ARG_INFO()

PHP_EXCEL_ARGINFO
ZEND_BEGIN_ARG_INFO_EX(arginfo_Format_borderLeftStyle, 0, 0, 0)
	ZEND_ARG_INFO(0, style)
ZEND_END_ARG_INFO()

PHP_EXCEL_ARGINFO
ZEND_BEGIN_ARG_INFO_EX(arginfo_Format_borderLeftColor, 0, 0, 0)
	ZEND_ARG_INFO(0, color)
ZEND_END_ARG_INFO()

PHP_EXCEL_ARGINFO
ZEND_BEGIN_ARG_INFO_EX(arginfo_Format_borderRightStyle, 0, 0, 0)
	ZEND_ARG_INFO(0, style)
ZEND_END_ARG_INFO()

PHP_EXCEL_ARGINFO
ZEND_BEGIN_ARG_INFO_EX(arginfo_Format_borderRightColor, 0, 0, 0)
	ZEND_ARG_INFO(0, color)
ZEND_END_ARG_INFO()

PHP_EXCEL_ARGINFO
ZEND_BEGIN_ARG_INFO_EX(arginfo_Format_borderTopStyle, 0, 0, 0)
	ZEND_ARG_INFO(0, style)
ZEND_END_ARG_INFO()

PHP_EXCEL_ARGINFO
ZEND_BEGIN_ARG_INFO_EX(arginfo_Format_borderTopColor, 0, 0, 0)
	ZEND_ARG_INFO(0, color)
ZEND_END_ARG_INFO()

PHP_EXCEL_ARGINFO
ZEND_BEGIN_ARG_INFO_EX(arginfo_Format_borderBottomStyle, 0, 0, 0)
	ZEND_ARG_INFO(0, style)
ZEND_END_ARG_INFO()

PHP_EXCEL_ARGINFO
ZEND_BEGIN_ARG_INFO_EX(arginfo_Format_borderBottomColor, 0, 0, 0)
	ZEND_ARG_INFO(0, color)
ZEND_END_ARG_INFO()

PHP_EXCEL_ARGINFO
ZEND_BEGIN_ARG_INFO_EX(arginfo_Format_borderDiagonalStyle, 0, 0, 0)
	ZEND_ARG_INFO(0, style)
ZEND_END_ARG_INFO()

PHP_EXCEL_ARGINFO
ZEND_BEGIN_ARG_INFO_EX(arginfo_Format_borderDiagonalColor, 0, 0, 0)
	ZEND_ARG_INFO(0, color)
ZEND_END_ARG_INFO()

PHP_EXCEL_ARGINFO
ZEND_BEGIN_ARG_INFO_EX(arginfo_Format_fillPattern, 0, 0, 0)
	ZEND_ARG_INFO(0, patern)
ZEND_END_ARG_INFO()

PHP_EXCEL_ARGINFO
ZEND_BEGIN_ARG_INFO_EX(arginfo_Format_patternForegroundColor, 0, 0, 0)
	ZEND_ARG_INFO(0, color)
ZEND_END_ARG_INFO()

PHP_EXCEL_ARGINFO
ZEND_BEGIN_ARG_INFO_EX(arginfo_Format_patternBackgroundColor, 0, 0, 0)
	ZEND_ARG_INFO(0, color)
ZEND_END_ARG_INFO()

PHP_EXCEL_ARGINFO
ZEND_BEGIN_ARG_INFO_EX(arginfo_Format_locked, 0, 0, 0)
	ZEND_ARG_INFO(0, locked)
ZEND_END_ARG_INFO()

PHP_EXCEL_ARGINFO
ZEND_BEGIN_ARG_INFO_EX(arginfo_Format_hidden, 0, 0, 0)
	ZEND_ARG_INFO(0, hidden)
ZEND_END_ARG_INFO()

PHP_EXCEL_ARGINFO
ZEND_BEGIN_ARG_INFO_EX(arginfo_Sheet___construct, 0, 0, 2)
	ZEND_ARG_OBJ_INFO(0, book, ExcelBook, 0)
	ZEND_ARG_INFO(0, name)
ZEND_END_ARG_INFO()

PHP_EXCEL_ARGINFO
ZEND_BEGIN_ARG_INFO_EX(arginfo_Sheet_cellType, 0, 0, 2)
	ZEND_ARG_INFO(0, row)
	ZEND_ARG_INFO(0, column)
ZEND_END_ARG_INFO()

PHP_EXCEL_ARGINFO
ZEND_BEGIN_ARG_INFO_EX(arginfo_Sheet_cellFormat, 0, 0, 2)
	ZEND_ARG_INFO(0, row)
	ZEND_ARG_INFO(0, column)
ZEND_END_ARG_INFO()

#ifdef HAVE_LIBXL_243_PLUS
PHP_EXCEL_ARGINFO
ZEND_BEGIN_ARG_INFO_EX(arginfo_Sheet_setCellFormat, 0, 0, 3)
	ZEND_ARG_INFO(0, row)
	ZEND_ARG_INFO(0, column)
	ZEND_ARG_OBJ_INFO(0, format, ExcelFormat, 0)
ZEND_END_ARG_INFO()
#endif

PHP_EXCEL_ARGINFO
ZEND_BEGIN_ARG_INFO_EX(arginfo_Sheet_readRow, 0, 0, 1)
	ZEND_ARG_INFO(0, row)
	ZEND_ARG_INFO(0, start_col)
	ZEND_ARG_INFO(0, end_column)
	ZEND_ARG_INFO(0, read_formula)
ZEND_END_ARG_INFO()

PHP_EXCEL_ARGINFO
ZEND_BEGIN_ARG_INFO_EX(arginfo_Sheet_readCol, 0, 0, 1)
	ZEND_ARG_INFO(0, column)
	ZEND_ARG_INFO(0, start_row)
	ZEND_ARG_INFO(0, end_row)
	ZEND_ARG_INFO(0, read_formula)
ZEND_END_ARG_INFO()

PHP_EXCEL_ARGINFO
ZEND_BEGIN_ARG_INFO_EX(arginfo_Sheet_read, 0, 0, 2)
	ZEND_ARG_INFO(0, row)
	ZEND_ARG_INFO(0, column)
	ZEND_ARG_INFO(1, format)
	ZEND_ARG_INFO(0, read_formula)
ZEND_END_ARG_INFO()

PHP_EXCEL_ARGINFO
ZEND_BEGIN_ARG_INFO_EX(arginfo_Sheet_write, 0, 0, 3)
	ZEND_ARG_INFO(0, row)
	ZEND_ARG_INFO(0, column)
	ZEND_ARG_INFO(0, data)
	ZEND_ARG_OBJ_INFO(0, format, ExcelFormat, 1)
	ZEND_ARG_INFO(0, datatype)
ZEND_END_ARG_INFO()

PHP_EXCEL_ARGINFO
ZEND_BEGIN_ARG_INFO_EX(arginfo_Sheet_writeRow, 0, 0, 2)
	ZEND_ARG_INFO(0, row)
	ZEND_ARG_INFO(0, data)
	ZEND_ARG_INFO(0, start_column)
	ZEND_ARG_OBJ_INFO(0, format, ExcelFormat, 1)
ZEND_END_ARG_INFO()

PHP_EXCEL_ARGINFO
ZEND_BEGIN_ARG_INFO_EX(arginfo_Sheet_writeCol, 0, 0, 2)
	ZEND_ARG_INFO(0, row)
	ZEND_ARG_INFO(0, data)
	ZEND_ARG_INFO(0, start_row)
	ZEND_ARG_OBJ_INFO(0, format, ExcelFormat, 1)
	ZEND_ARG_INFO(0, data_type)
ZEND_END_ARG_INFO()

PHP_EXCEL_ARGINFO
ZEND_BEGIN_ARG_INFO_EX(arginfo_Sheet_isFormula, 0, 0, 2)
	ZEND_ARG_INFO(0, row)
	ZEND_ARG_INFO(0, column)
ZEND_END_ARG_INFO()

PHP_EXCEL_ARGINFO
ZEND_BEGIN_ARG_INFO_EX(arginfo_Sheet_isDate, 0, 0, 2)
	ZEND_ARG_INFO(0, row)
	ZEND_ARG_INFO(0, column)
ZEND_END_ARG_INFO()

PHP_EXCEL_ARGINFO
ZEND_BEGIN_ARG_INFO_EX(arginfo_Sheet_insertRow, 0, 0, 2)
	ZEND_ARG_INFO(0, row_first)
	ZEND_ARG_INFO(0, row_last)
ZEND_END_ARG_INFO()

PHP_EXCEL_ARGINFO
ZEND_BEGIN_ARG_INFO_EX(arginfo_Sheet_insertCol, 0, 0, 2)
	ZEND_ARG_INFO(0, col_first)
	ZEND_ARG_INFO(0, col_last)
ZEND_END_ARG_INFO()

PHP_EXCEL_ARGINFO
ZEND_BEGIN_ARG_INFO_EX(arginfo_Sheet_removeRow, 0, 0, 2)
	ZEND_ARG_INFO(0, row_first)
	ZEND_ARG_INFO(0, row_last)
ZEND_END_ARG_INFO()

PHP_EXCEL_ARGINFO
ZEND_BEGIN_ARG_INFO_EX(arginfo_Sheet_removeCol, 0, 0, 2)
	ZEND_ARG_INFO(0, col_first)
	ZEND_ARG_INFO(0, col_last)
ZEND_END_ARG_INFO()

PHP_EXCEL_ARGINFO
ZEND_BEGIN_ARG_INFO_EX(arginfo_Sheet_colWidth, 0, 0, 1)
	ZEND_ARG_INFO(0, column)
ZEND_END_ARG_INFO()

PHP_EXCEL_ARGINFO
ZEND_BEGIN_ARG_INFO_EX(arginfo_Sheet_rowHeight, 0, 0, 1)
	ZEND_ARG_INFO(0, row)
ZEND_END_ARG_INFO()

PHP_EXCEL_ARGINFO
ZEND_BEGIN_ARG_INFO_EX(arginfo_Sheet_readComment, 0, 0, 2)
	ZEND_ARG_INFO(0, row)
	ZEND_ARG_INFO(0, column)
ZEND_END_ARG_INFO()

PHP_EXCEL_ARGINFO
ZEND_BEGIN_ARG_INFO_EX(arginfo_Sheet_writeComment, 0, 0, 6)
	ZEND_ARG_INFO(0, row)
	ZEND_ARG_INFO(0, column)
	ZEND_ARG_INFO(0, value)
	ZEND_ARG_INFO(0, author)
	ZEND_ARG_INFO(0, width)
	ZEND_ARG_INFO(0, height)
ZEND_END_ARG_INFO()

PHP_EXCEL_ARGINFO
ZEND_BEGIN_ARG_INFO_EX(arginfo_Sheet_setColWidth, 0, 0, 3)
	ZEND_ARG_INFO(0, column_start)
	ZEND_ARG_INFO(0, column_end)
	ZEND_ARG_INFO(0, width)
	ZEND_ARG_INFO(0, hidden)
	ZEND_ARG_OBJ_INFO(0, format, ExcelFormat, 1)
ZEND_END_ARG_INFO()

PHP_EXCEL_ARGINFO
ZEND_BEGIN_ARG_INFO_EX(arginfo_Sheet_setRowHeight, 0, 0, 2)
	ZEND_ARG_INFO(0, row)
	ZEND_ARG_INFO(0, height)
	ZEND_ARG_OBJ_INFO(0, format, ExcelFormat, 1)
	ZEND_ARG_INFO(0, hidden)
ZEND_END_ARG_INFO()

PHP_EXCEL_ARGINFO
ZEND_BEGIN_ARG_INFO_EX(arginfo_Sheet_getMerge, 0, 0, 2)
	ZEND_ARG_INFO(0, row)
	ZEND_ARG_INFO(0, column)
ZEND_END_ARG_INFO()

PHP_EXCEL_ARGINFO
ZEND_BEGIN_ARG_INFO_EX(arginfo_Sheet_setMerge, 0, 0, 4)
	ZEND_ARG_INFO(0, row_start)
	ZEND_ARG_INFO(0, row_end)
	ZEND_ARG_INFO(0, col_start)
	ZEND_ARG_INFO(0, col_end)
ZEND_END_ARG_INFO()

PHP_EXCEL_ARGINFO
ZEND_BEGIN_ARG_INFO_EX(arginfo_Sheet_deleteMerge, 0, 0, 2)
	ZEND_ARG_INFO(0, row)
	ZEND_ARG_INFO(0, column)
ZEND_END_ARG_INFO()

PHP_EXCEL_ARGINFO
ZEND_BEGIN_ARG_INFO_EX(arginfo_Sheet_addPictureScaled, 0, 0, 4)
	ZEND_ARG_INFO(0, row)
	ZEND_ARG_INFO(0, column)
	ZEND_ARG_INFO(0, pic_id)
	ZEND_ARG_INFO(0, scale)
#if LIBXL_VERSION >= 0x03040000
	ZEND_ARG_INFO(0, x_offset)
	ZEND_ARG_INFO(0, y_offset)
#endif
ZEND_END_ARG_INFO()

PHP_EXCEL_ARGINFO
ZEND_BEGIN_ARG_INFO_EX(arginfo_Sheet_addPictureDim, 0, 0, 5)
	ZEND_ARG_INFO(0, row)
	ZEND_ARG_INFO(0, column)
	ZEND_ARG_INFO(0, pic_id)
	ZEND_ARG_INFO(0, width)
	ZEND_ARG_INFO(0, height)
#if LIBXL_VERSION >= 0x03040000
	ZEND_ARG_INFO(0, x_offset)
	ZEND_ARG_INFO(0, y_offset)
#endif
ZEND_END_ARG_INFO()

PHP_EXCEL_ARGINFO
ZEND_BEGIN_ARG_INFO_EX(arginfo_Sheet_horPageBreak, 0, 0, 2)
	ZEND_ARG_INFO(0, row)
	ZEND_ARG_INFO(0, break)
ZEND_END_ARG_INFO()

PHP_EXCEL_ARGINFO
ZEND_BEGIN_ARG_INFO_EX(arginfo_Sheet_verPageBreak, 0, 0, 2)
	ZEND_ARG_INFO(0, col)
	ZEND_ARG_INFO(0, break)
ZEND_END_ARG_INFO()

PHP_EXCEL_ARGINFO
ZEND_BEGIN_ARG_INFO_EX(arginfo_Sheet_splitSheet, 0, 0, 2)
	ZEND_ARG_INFO(0, row)
	ZEND_ARG_INFO(0, column)
ZEND_END_ARG_INFO()

PHP_EXCEL_ARGINFO
ZEND_BEGIN_ARG_INFO_EX(arginfo_Sheet_groupRows, 0, 0, 2)
	ZEND_ARG_INFO(0, start_row)
	ZEND_ARG_INFO(0, end_row)
	ZEND_ARG_INFO(0, collapse)
ZEND_END_ARG_INFO()

PHP_EXCEL_ARGINFO
ZEND_BEGIN_ARG_INFO_EX(arginfo_Sheet_groupCols, 0, 0, 2)
	ZEND_ARG_INFO(0, start_column)
	ZEND_ARG_INFO(0, end_column)
	ZEND_ARG_INFO(0, collapse)
ZEND_END_ARG_INFO()

PHP_EXCEL_ARGINFO
ZEND_BEGIN_ARG_INFO_EX(arginfo_Sheet_clear, 0, 0, 4)
	ZEND_ARG_INFO(0, row_s)
	ZEND_ARG_INFO(0, row_e)
	ZEND_ARG_INFO(0, col_s)
	ZEND_ARG_INFO(0, col_s)
ZEND_END_ARG_INFO()

PHP_EXCEL_ARGINFO
ZEND_BEGIN_ARG_INFO_EX(arginfo_Sheet_copy, 0, 0, 4)
	ZEND_ARG_INFO(0, row)
	ZEND_ARG_INFO(0, col)
	ZEND_ARG_INFO(0, to_row)
	ZEND_ARG_INFO(0, to_col)
ZEND_END_ARG_INFO()

PHP_EXCEL_ARGINFO
ZEND_BEGIN_ARG_INFO_EX(arginfo_Sheet_firstRow, 0, 0, 0)
ZEND_END_ARG_INFO()

PHP_EXCEL_ARGINFO
ZEND_BEGIN_ARG_INFO_EX(arginfo_Sheet_lastRow, 0, 0, 0)
ZEND_END_ARG_INFO()

PHP_EXCEL_ARGINFO
ZEND_BEGIN_ARG_INFO_EX(arginfo_Sheet_firstCol, 0, 0, 0)
ZEND_END_ARG_INFO()

PHP_EXCEL_ARGINFO
ZEND_BEGIN_ARG_INFO_EX(arginfo_Sheet_lastCol, 0, 0, 0)
ZEND_END_ARG_INFO()

PHP_EXCEL_ARGINFO
ZEND_BEGIN_ARG_INFO_EX(arginfo_Sheet_displayGridlines, 0, 0, 0)
ZEND_END_ARG_INFO()

PHP_EXCEL_ARGINFO
ZEND_BEGIN_ARG_INFO_EX(arginfo_Sheet_printGridlines, 0, 0, 0)
ZEND_END_ARG_INFO()

PHP_EXCEL_ARGINFO
ZEND_BEGIN_ARG_INFO_EX(arginfo_Sheet_setDisplayGridlines, 0, 0, 1)
	ZEND_ARG_INFO(0, value)
ZEND_END_ARG_INFO()

PHP_EXCEL_ARGINFO
ZEND_BEGIN_ARG_INFO_EX(arginfo_Sheet_setPrintGridlines, 0, 0, 1)
	ZEND_ARG_INFO(0, value)
ZEND_END_ARG_INFO()

PHP_EXCEL_ARGINFO
ZEND_BEGIN_ARG_INFO_EX(arginfo_Sheet_zoom, 0, 0, 0)
ZEND_END_ARG_INFO()

#if LIBXL_VERSION >= 0x03020300
PHP_EXCEL_ARGINFO
ZEND_BEGIN_ARG_INFO_EX(arginfo_Sheet_isHidden, 0, 0, 0)
ZEND_END_ARG_INFO()

PHP_EXCEL_ARGINFO
ZEND_BEGIN_ARG_INFO_EX(arginfo_Sheet_setHidden, 0, 0, 1)
	ZEND_ARG_INFO(0, value)
ZEND_END_ARG_INFO()
#endif

#if LIBXL_VERSION >= 0x03020400
PHP_EXCEL_ARGINFO
ZEND_BEGIN_ARG_INFO_EX(arginfo_Sheet_getTopLeftView, 0, 0, 0)
ZEND_END_ARG_INFO()

PHP_EXCEL_ARGINFO
ZEND_BEGIN_ARG_INFO_EX(arginfo_Sheet_setTopLeftView, 0, 0, 2)
	ZEND_ARG_INFO(0, row)
	ZEND_ARG_INFO(0, column)
ZEND_END_ARG_INFO()

PHP_EXCEL_ARGINFO
ZEND_BEGIN_ARG_INFO_EX(arginfo_Sheet_rowColToAddr, 0, 0, 2)
	ZEND_ARG_INFO(0, row)
	ZEND_ARG_INFO(0, column)
	ZEND_ARG_INFO(0, row_relative)
	ZEND_ARG_INFO(0, col_relative)
ZEND_END_ARG_INFO()

PHP_EXCEL_ARGINFO
ZEND_BEGIN_ARG_INFO_EX(arginfo_Sheet_addrToRowCol, 0, 0, 1)
	ZEND_ARG_INFO(0, cell_reference)
ZEND_END_ARG_INFO()
#endif

PHP_EXCEL_ARGINFO
ZEND_BEGIN_ARG_INFO_EX(arginfo_Sheet_zoomPrint, 0, 0, 0)
ZEND_END_ARG_INFO()

PHP_EXCEL_ARGINFO
ZEND_BEGIN_ARG_INFO_EX(arginfo_Sheet_setZoom, 0, 0, 1)
	ZEND_ARG_INFO(0, value)
ZEND_END_ARG_INFO()

PHP_EXCEL_ARGINFO
ZEND_BEGIN_ARG_INFO_EX(arginfo_Sheet_setZoomPrint, 0, 0, 1)
	ZEND_ARG_INFO(0, value)
ZEND_END_ARG_INFO()

PHP_EXCEL_ARGINFO
ZEND_BEGIN_ARG_INFO_EX(arginfo_Sheet_setLandscape, 0, 0, 1)
	ZEND_ARG_INFO(0, value)
ZEND_END_ARG_INFO()

PHP_EXCEL_ARGINFO
ZEND_BEGIN_ARG_INFO_EX(arginfo_Sheet_landscape, 0, 0, 0)
ZEND_END_ARG_INFO()

PHP_EXCEL_ARGINFO
ZEND_BEGIN_ARG_INFO_EX(arginfo_Sheet_paper, 0, 0, 0)
ZEND_END_ARG_INFO()

PHP_EXCEL_ARGINFO
ZEND_BEGIN_ARG_INFO_EX(arginfo_Sheet_setPaper, 0, 0, 1)
	ZEND_ARG_INFO(0, value)
ZEND_END_ARG_INFO()

PHP_EXCEL_ARGINFO
ZEND_BEGIN_ARG_INFO_EX(arginfo_Sheet_header, 0, 0, 0)
ZEND_END_ARG_INFO()

PHP_EXCEL_ARGINFO
ZEND_BEGIN_ARG_INFO_EX(arginfo_Sheet_footer, 0, 0, 0)
ZEND_END_ARG_INFO()

PHP_EXCEL_ARGINFO
ZEND_BEGIN_ARG_INFO_EX(arginfo_Sheet_setHeader, 0, 0, 2)
	ZEND_ARG_INFO(0, header)
	ZEND_ARG_INFO(0, margin)
ZEND_END_ARG_INFO()

PHP_EXCEL_ARGINFO
ZEND_BEGIN_ARG_INFO_EX(arginfo_Sheet_setFooter, 0, 0, 2)
	ZEND_ARG_INFO(0, footer)
	ZEND_ARG_INFO(0, margin)
ZEND_END_ARG_INFO()

PHP_EXCEL_ARGINFO
ZEND_BEGIN_ARG_INFO_EX(arginfo_Sheet_headerMargin, 0, 0, 0)
ZEND_END_ARG_INFO()

PHP_EXCEL_ARGINFO
ZEND_BEGIN_ARG_INFO_EX(arginfo_Sheet_footerMargin, 0, 0, 0)
ZEND_END_ARG_INFO()

PHP_EXCEL_ARGINFO
ZEND_BEGIN_ARG_INFO_EX(arginfo_Sheet_hcenter, 0, 0, 0)
ZEND_END_ARG_INFO()

PHP_EXCEL_ARGINFO
ZEND_BEGIN_ARG_INFO_EX(arginfo_Sheet_vcenter, 0, 0, 0)
ZEND_END_ARG_INFO()

PHP_EXCEL_ARGINFO
ZEND_BEGIN_ARG_INFO_EX(arginfo_Sheet_setHCenter, 0, 0, 1)
	ZEND_ARG_INFO(0, value)
ZEND_END_ARG_INFO()

PHP_EXCEL_ARGINFO
ZEND_BEGIN_ARG_INFO_EX(arginfo_Sheet_setVCenter, 0, 0, 1)
	ZEND_ARG_INFO(0, value)
ZEND_END_ARG_INFO()

PHP_EXCEL_ARGINFO
ZEND_BEGIN_ARG_INFO_EX(arginfo_Sheet_marginLeft, 0, 0, 0)
ZEND_END_ARG_INFO()

PHP_EXCEL_ARGINFO
ZEND_BEGIN_ARG_INFO_EX(arginfo_Sheet_marginRight, 0, 0, 0)
ZEND_END_ARG_INFO()

PHP_EXCEL_ARGINFO
ZEND_BEGIN_ARG_INFO_EX(arginfo_Sheet_marginTop, 0, 0, 0)
ZEND_END_ARG_INFO()

PHP_EXCEL_ARGINFO
ZEND_BEGIN_ARG_INFO_EX(arginfo_Sheet_marginBottom, 0, 0, 0)
ZEND_END_ARG_INFO()

PHP_EXCEL_ARGINFO
ZEND_BEGIN_ARG_INFO_EX(arginfo_Sheet_setMarginLeft, 0, 0, 1)
	ZEND_ARG_INFO(0, value)
ZEND_END_ARG_INFO()

PHP_EXCEL_ARGINFO
ZEND_BEGIN_ARG_INFO_EX(arginfo_Sheet_setMarginRight, 0, 0, 1)
	ZEND_ARG_INFO(0, value)
ZEND_END_ARG_INFO()

PHP_EXCEL_ARGINFO
ZEND_BEGIN_ARG_INFO_EX(arginfo_Sheet_setMarginTop, 0, 0, 1)
	ZEND_ARG_INFO(0, value)
ZEND_END_ARG_INFO()

PHP_EXCEL_ARGINFO
ZEND_BEGIN_ARG_INFO_EX(arginfo_Sheet_setMarginBottom, 0, 0, 1)
	ZEND_ARG_INFO(0, value)
ZEND_END_ARG_INFO()

PHP_EXCEL_ARGINFO
ZEND_BEGIN_ARG_INFO_EX(arginfo_Sheet_printHeaders, 0, 0, 0)
ZEND_END_ARG_INFO()

PHP_EXCEL_ARGINFO
ZEND_BEGIN_ARG_INFO_EX(arginfo_Sheet_setPrintHeaders, 0, 0, 1)
	ZEND_ARG_INFO(0, value)
ZEND_END_ARG_INFO()

PHP_EXCEL_ARGINFO
ZEND_BEGIN_ARG_INFO_EX(arginfo_Sheet_name, 0, 0, 0)
ZEND_END_ARG_INFO()

PHP_EXCEL_ARGINFO
ZEND_BEGIN_ARG_INFO_EX(arginfo_Sheet_setName, 0, 0, 1)
	ZEND_ARG_INFO(0, name)
ZEND_END_ARG_INFO()

PHP_EXCEL_ARGINFO
ZEND_BEGIN_ARG_INFO_EX(arginfo_Sheet_protect, 0, 0, 0)
ZEND_END_ARG_INFO()

PHP_EXCEL_ARGINFO
ZEND_BEGIN_ARG_INFO_EX(arginfo_Sheet_setProtect, 0, 0, 1)
	ZEND_ARG_INFO(0, value)
ZEND_END_ARG_INFO()

#if LIBXL_VERSION >= 0x03010000
PHP_EXCEL_ARGINFO
ZEND_BEGIN_ARG_INFO_EX(arginfo_Sheet_setNamedRange, 0, 0, 5)
	ZEND_ARG_INFO(0, name)
	ZEND_ARG_INFO(0, row)
	ZEND_ARG_INFO(0, col)
	ZEND_ARG_INFO(0, to_row)
	ZEND_ARG_INFO(0, to_col)
#if LIBXL_VERSION >= 0x03050401
	ZEND_ARG_INFO(0, scope_id)
#endif
ZEND_END_ARG_INFO()

PHP_EXCEL_ARGINFO
ZEND_BEGIN_ARG_INFO_EX(arginfo_Sheet_delNamedRange, 0, 0, 1)
	ZEND_ARG_INFO(0, name)
#if LIBXL_VERSION >= 0x03050401
	ZEND_ARG_INFO(0, scope_id)
#endif
ZEND_END_ARG_INFO()

PHP_EXCEL_ARGINFO
ZEND_BEGIN_ARG_INFO_EX(arginfo_Sheet_setPrintRepeatRows, 0, 0, 2)
	ZEND_ARG_INFO(0, row_start)
	ZEND_ARG_INFO(0, row_end)
ZEND_END_ARG_INFO()

PHP_EXCEL_ARGINFO
ZEND_BEGIN_ARG_INFO_EX(arginfo_Sheet_setPrintRepeatCols, 0, 0, 2)
	ZEND_ARG_INFO(0, col_start)
	ZEND_ARG_INFO(0, col_end)
ZEND_END_ARG_INFO()

PHP_EXCEL_ARGINFO
ZEND_BEGIN_ARG_INFO_EX(arginfo_Sheet_setPrintArea, 0, 0, 4)
	ZEND_ARG_INFO(0, row_first)
	ZEND_ARG_INFO(0, row_last)
	ZEND_ARG_INFO(0, col_first)
	ZEND_ARG_INFO(0, col_last)
ZEND_END_ARG_INFO()

PHP_EXCEL_ARGINFO
ZEND_BEGIN_ARG_INFO_EX(arginfo_Sheet_clearPrintRepeats, 0, 0, 0)
ZEND_END_ARG_INFO()

PHP_EXCEL_ARGINFO
ZEND_BEGIN_ARG_INFO_EX(arginfo_Sheet_clearPrintArea, 0, 0, 0)
ZEND_END_ARG_INFO()

PHP_EXCEL_ARGINFO
ZEND_BEGIN_ARG_INFO_EX(arginfo_Sheet_getGroupSummaryRight, 0, 0, 0)
ZEND_END_ARG_INFO()

PHP_EXCEL_ARGINFO
ZEND_BEGIN_ARG_INFO_EX(arginfo_Sheet_getGroupSummaryBelow, 0, 0, 0)
ZEND_END_ARG_INFO()

PHP_EXCEL_ARGINFO
ZEND_BEGIN_ARG_INFO_EX(arginfo_Sheet_setGroupSummaryBelow, 0, 0, 1)
	ZEND_ARG_INFO(0, direction)
ZEND_END_ARG_INFO()

PHP_EXCEL_ARGINFO
ZEND_BEGIN_ARG_INFO_EX(arginfo_Sheet_setGroupSummaryRight, 0, 0, 1)
	ZEND_ARG_INFO(0, direction)
ZEND_END_ARG_INFO()
#endif

#if LIBXL_VERSION >= 0x03020000
PHP_EXCEL_ARGINFO
ZEND_BEGIN_ARG_INFO_EX(arginfo_Sheet_setPrintFit, 0, 0, 2)
	ZEND_ARG_INFO(0, wPages)
	ZEND_ARG_INFO(0, hPages)
ZEND_END_ARG_INFO()

PHP_EXCEL_ARGINFO
ZEND_BEGIN_ARG_INFO_EX(arginfo_Sheet_getPrintFit, 0, 0, 0)
ZEND_END_ARG_INFO()

PHP_EXCEL_ARGINFO
ZEND_BEGIN_ARG_INFO_EX(arginfo_Sheet_getNamedRange, 0, 0, 1)
	ZEND_ARG_INFO(0, name)
#if LIBXL_VERSION >= 0x03050401
	ZEND_ARG_INFO(0, scope_id)
#endif
ZEND_END_ARG_INFO()

PHP_EXCEL_ARGINFO
ZEND_BEGIN_ARG_INFO_EX(arginfo_Sheet_getIndexRange, 0, 0, 1)
	ZEND_ARG_INFO(0, index)
#if LIBXL_VERSION >= 0x03050401
	ZEND_ARG_INFO(0, scope_id)
#endif
ZEND_END_ARG_INFO()

PHP_EXCEL_ARGINFO
ZEND_BEGIN_ARG_INFO_EX(arginfo_Sheet_namedRangeSize, 0, 0, 0)
ZEND_END_ARG_INFO()

PHP_EXCEL_ARGINFO
ZEND_BEGIN_ARG_INFO_EX(arginfo_Sheet_getVerPageBreak, 0, 0, 1)
	ZEND_ARG_INFO(0, index)
ZEND_END_ARG_INFO()

PHP_EXCEL_ARGINFO
ZEND_BEGIN_ARG_INFO_EX(arginfo_Sheet_getVerPageBreakSize, 0, 0, 0)
ZEND_END_ARG_INFO()

PHP_EXCEL_ARGINFO
ZEND_BEGIN_ARG_INFO_EX(arginfo_Sheet_getHorPageBreak, 0, 0, 1)
	ZEND_ARG_INFO(0, index)
ZEND_END_ARG_INFO()

PHP_EXCEL_ARGINFO
ZEND_BEGIN_ARG_INFO_EX(arginfo_Sheet_getHorPageBreakSize, 0, 0, 0)
ZEND_END_ARG_INFO()

PHP_EXCEL_ARGINFO
ZEND_BEGIN_ARG_INFO_EX(arginfo_Sheet_getPictureInfo, 0, 0, 1)
	ZEND_ARG_INFO(0, index)
ZEND_END_ARG_INFO()

PHP_EXCEL_ARGINFO
ZEND_BEGIN_ARG_INFO_EX(arginfo_Sheet_getNumPictures, 0, 0, 0)
ZEND_END_ARG_INFO()
#endif

#if LIBXL_VERSION >= 0x03060000
PHP_EXCEL_ARGINFO
ZEND_BEGIN_ARG_INFO_EX(arginfo_Sheet_hyperlinkSize, 0, 0, 0)
ZEND_END_ARG_INFO()

PHP_EXCEL_ARGINFO
ZEND_BEGIN_ARG_INFO_EX(arginfo_Sheet_hyperlink, 0, 0, 1)
	ZEND_ARG_INFO(0, index)
ZEND_END_ARG_INFO()

PHP_EXCEL_ARGINFO
ZEND_BEGIN_ARG_INFO_EX(arginfo_Sheet_delHyperlink, 0, 0, 1)
	ZEND_ARG_INFO(0, index)
ZEND_END_ARG_INFO()

PHP_EXCEL_ARGINFO
ZEND_BEGIN_ARG_INFO_EX(arginfo_Sheet_addHyperlink, 0, 0, 5)
	ZEND_ARG_INFO(0, hyperlink)
	ZEND_ARG_INFO(0, row_first)
	ZEND_ARG_INFO(0, row_last)
	ZEND_ARG_INFO(0, col_first)
	ZEND_ARG_INFO(0, col_last)
ZEND_END_ARG_INFO()

PHP_EXCEL_ARGINFO
ZEND_BEGIN_ARG_INFO_EX(arginfo_Sheet_mergeSize, 0, 0, 0)
ZEND_END_ARG_INFO()

PHP_EXCEL_ARGINFO
ZEND_BEGIN_ARG_INFO_EX(arginfo_Sheet_merge, 0, 0, 1)
	ZEND_ARG_INFO(0, index)
ZEND_END_ARG_INFO()

PHP_EXCEL_ARGINFO
ZEND_BEGIN_ARG_INFO_EX(arginfo_Sheet_delMergeByIndex, 0, 0, 1)
	ZEND_ARG_INFO(0, index)
ZEND_END_ARG_INFO()

PHP_EXCEL_ARGINFO
ZEND_BEGIN_ARG_INFO_EX(arginfo_Sheet_splitInfo, 0, 0, 0)
ZEND_END_ARG_INFO()

PHP_EXCEL_ARGINFO
ZEND_BEGIN_ARG_INFO_EX(arginfo_Sheet_colHidden, 0, 0, 1)
	ZEND_ARG_INFO(0, col)
ZEND_END_ARG_INFO()

PHP_EXCEL_ARGINFO
ZEND_BEGIN_ARG_INFO_EX(arginfo_Sheet_rowHidden, 0, 0, 1)
	ZEND_ARG_INFO(0, row)
ZEND_END_ARG_INFO()

PHP_EXCEL_ARGINFO
ZEND_BEGIN_ARG_INFO_EX(arginfo_Sheet_setColHidden, 0, 0, 2)
	ZEND_ARG_INFO(0, col)
	ZEND_ARG_INFO(0, hidden)
ZEND_END_ARG_INFO()

PHP_EXCEL_ARGINFO
ZEND_BEGIN_ARG_INFO_EX(arginfo_Sheet_setRowHidden, 0, 0, 2)
	ZEND_ARG_INFO(0, row)
	ZEND_ARG_INFO(0, hidden)
ZEND_END_ARG_INFO()
#endif

PHP_EXCEL_ARGINFO
ZEND_BEGIN_ARG_INFO_EX(arginfo_Sheet_isLicensed, 0, 0, 0)
ZEND_END_ARG_INFO()

#define EXCEL_ME(class_name, function_name, arg_info, flags) \
	PHP_ME( Excel ## class_name, function_name, arg_info, flags)

zend_function_entry excel_funcs_book[] = {
	EXCEL_ME(Book, addFont, arginfo_Book_addFont, 0)
	EXCEL_ME(Book, addFormat, arginfo_Book_addFormat, 0)
#ifdef HAVE_LIBXL_243_PLUS
	EXCEL_ME(Book, getAllFormats, arginfo_Book_getAllFormats, 0)
#endif
	EXCEL_ME(Book, getError, arginfo_Book_getError, 0)
	EXCEL_ME(Book, loadFile, arginfo_Book_loadFile, 0)
	EXCEL_ME(Book, load, arginfo_Book_load, 0)
	EXCEL_ME(Book, save, arginfo_Book_save, 0)
	EXCEL_ME(Book, getSheet, arginfo_Book_getSheet, 0)
	EXCEL_ME(Book, getSheetByName, arginfo_Book_getSheetByName, 0)
	EXCEL_ME(Book, addSheet, arginfo_Book_addSheet, 0)
	EXCEL_ME(Book, copySheet, arginfo_Book_copySheet, 0)
	EXCEL_ME(Book, deleteSheet, arginfo_Book_deleteSheet, 0)
	EXCEL_ME(Book, sheetCount, arginfo_Book_sheetCount, 0)
	EXCEL_ME(Book, activeSheet, arginfo_Book_activeSheet, 0)
	EXCEL_ME(Book, getCustomFormat, arginfo_Book_getCustomFormat, 0)
	EXCEL_ME(Book, addCustomFormat, arginfo_Book_addCustomFormat, 0)
	EXCEL_ME(Book, packDate, arginfo_Book_packDate, 0)
	EXCEL_ME(Book, packDateValues, arginfo_Book_packDateValues, 0)
	EXCEL_ME(Book, unpackDate, arginfo_Book_unpackDate, 0)
	EXCEL_ME(Book, getActiveSheet, arginfo_Book_getActiveSheet, 0)
	EXCEL_ME(Book, setActiveSheet, arginfo_Book_setActiveSheet, 0)
	EXCEL_ME(Book, getDefaultFont, arginfo_Book_getDefaultFont, 0)
	EXCEL_ME(Book, setDefaultFont, arginfo_Book_setDefaultFont, 0)
	EXCEL_ME(Book, setLocale, arginfo_Book_setLocale, 0)
	EXCEL_ME(Book, addPictureFromFile, arginfo_Book_addPictureFromFile, 0)
	EXCEL_ME(Book, addPictureFromString, arginfo_Book_addPictureFromString, 0)
#ifdef LIBXL_VERSION
	EXCEL_ME(Book, rgbMode, arginfo_Book_rgbMode, 0)
	EXCEL_ME(Book, setRGBMode, arginfo_Book_setRGBMode, 0)
	EXCEL_ME(Book, colorPack, arginfo_Book_colorPack, 0)
	EXCEL_ME(Book, colorUnpack, arginfo_Book_colorUnpack, 0)
#endif
#if LIBXL_VERSION >= 0x03050300
	EXCEL_ME(Book, isDate1904, arginfo_Book_isDate1904, 0)
	EXCEL_ME(Book, setDate1904, arginfo_Book_setDate1904, 0)
#endif
	EXCEL_ME(Book, __construct, arginfo_Book___construct, 0)
#if LIBXL_VERSION >= 0x03020000
	EXCEL_ME(Book, biffVersion, arginfo_Book_biffVersion, 0)
	EXCEL_ME(Book, setRefR1C1, arginfo_Book_setRefR1C1, 0)
	EXCEL_ME(Book, getRefR1C1, arginfo_Book_getRefR1C1, 0)
	EXCEL_ME(Book, getPicture, arginfo_Book_getPicture, 0)
	EXCEL_ME(Book, getNumPictures, arginfo_Book_getNumPictures, 0)
	EXCEL_ME(Book, insertSheet, arginfo_Book_insertSheet, 0)
#endif
#if LIBXL_VERSION >= 0x03050401
	EXCEL_ME(Book, isTemplate, arginfo_Book_isTemplate, 0)
	EXCEL_ME(Book, setTemplate, arginfo_Book_setTemplate, 0)
#endif
#if LIBXL_VERSION >= 0x03060000
	EXCEL_ME(Book, sheetType, arginfo_Book_sheetType, 0)
#endif
	{NULL, NULL, NULL}
};

zend_function_entry excel_funcs_sheet[] = {
	EXCEL_ME(Sheet, __construct, arginfo_Sheet___construct, 0)
	EXCEL_ME(Sheet, cellType, arginfo_Sheet_cellType, 0)
	EXCEL_ME(Sheet, cellFormat, arginfo_Sheet_cellFormat, 0)
#ifdef HAVE_LIBXL_243_PLUS
	EXCEL_ME(Sheet, setCellFormat, arginfo_Sheet_setCellFormat, 0)
#endif
	EXCEL_ME(Sheet, read, arginfo_Sheet_read, 0)
	EXCEL_ME(Sheet, readRow, arginfo_Sheet_readRow, 0)
	EXCEL_ME(Sheet, readCol, arginfo_Sheet_readCol, 0)
	EXCEL_ME(Sheet, write, arginfo_Sheet_write, 0)
	EXCEL_ME(Sheet, writeRow, arginfo_Sheet_writeRow, 0)
	EXCEL_ME(Sheet, writeCol, arginfo_Sheet_writeCol, 0)
	EXCEL_ME(Sheet, isFormula, arginfo_Sheet_isFormula, 0)
	EXCEL_ME(Sheet, isDate, arginfo_Sheet_isDate, 0)
	EXCEL_ME(Sheet, insertRow, arginfo_Sheet_insertRow, 0)
	EXCEL_ME(Sheet, insertCol, arginfo_Sheet_insertCol, 0)
	EXCEL_ME(Sheet, removeRow, arginfo_Sheet_removeRow, 0)
	EXCEL_ME(Sheet, removeCol, arginfo_Sheet_removeCol, 0)
	EXCEL_ME(Sheet, colWidth, arginfo_Sheet_colWidth, 0)
	EXCEL_ME(Sheet, rowHeight, arginfo_Sheet_rowHeight, 0)
	EXCEL_ME(Sheet, readComment, arginfo_Sheet_readComment, 0)
	EXCEL_ME(Sheet, writeComment, arginfo_Sheet_writeComment, 0)
	EXCEL_ME(Sheet, setColWidth, arginfo_Sheet_setColWidth, 0)
	EXCEL_ME(Sheet, setRowHeight, arginfo_Sheet_setRowHeight, 0)
	EXCEL_ME(Sheet, getMerge, arginfo_Sheet_getMerge, 0)
	EXCEL_ME(Sheet, setMerge, arginfo_Sheet_setMerge, 0)
	EXCEL_ME(Sheet, deleteMerge, arginfo_Sheet_deleteMerge, 0)
	EXCEL_ME(Sheet, addPictureScaled, arginfo_Sheet_addPictureScaled, 0)
	EXCEL_ME(Sheet, addPictureDim, arginfo_Sheet_addPictureDim, 0)
	EXCEL_ME(Sheet, horPageBreak, arginfo_Sheet_horPageBreak, 0)
	EXCEL_ME(Sheet, verPageBreak, arginfo_Sheet_verPageBreak, 0)
	EXCEL_ME(Sheet, splitSheet, arginfo_Sheet_splitSheet, 0)
	EXCEL_ME(Sheet, groupRows, arginfo_Sheet_groupRows, 0)
	EXCEL_ME(Sheet, groupCols, arginfo_Sheet_groupCols, 0)
	EXCEL_ME(Sheet, clear, arginfo_Sheet_clear, 0)
	EXCEL_ME(Sheet, copy, arginfo_Sheet_copy, 0)
	EXCEL_ME(Sheet, firstRow, arginfo_Sheet_firstRow, 0)
	EXCEL_ME(Sheet, lastRow, arginfo_Sheet_lastRow, 0)
	EXCEL_ME(Sheet, firstCol, arginfo_Sheet_firstCol, 0)
	EXCEL_ME(Sheet, lastCol, arginfo_Sheet_lastCol, 0)
	EXCEL_ME(Sheet, displayGridlines, arginfo_Sheet_displayGridlines, 0)
	EXCEL_ME(Sheet, printGridlines, arginfo_Sheet_printGridlines, 0)
	EXCEL_ME(Sheet, setDisplayGridlines, arginfo_Sheet_setDisplayGridlines, 0)
	EXCEL_ME(Sheet, setPrintGridlines, arginfo_Sheet_setPrintGridlines, 0)
	EXCEL_ME(Sheet, zoom, arginfo_Sheet_zoom, 0)
	EXCEL_ME(Sheet, zoomPrint, arginfo_Sheet_zoomPrint, 0)
	EXCEL_ME(Sheet, setZoom, arginfo_Sheet_setZoom, 0)
	EXCEL_ME(Sheet, setZoomPrint, arginfo_Sheet_setZoomPrint, 0)
	EXCEL_ME(Sheet, setLandscape, arginfo_Sheet_setLandscape, 0)
	EXCEL_ME(Sheet, landscape, arginfo_Sheet_landscape, 0)
	EXCEL_ME(Sheet, paper, arginfo_Sheet_paper, 0)
	EXCEL_ME(Sheet, setPaper, arginfo_Sheet_setPaper, 0)
	EXCEL_ME(Sheet, header, arginfo_Sheet_header, 0)
	EXCEL_ME(Sheet, footer, arginfo_Sheet_footer, 0)
	EXCEL_ME(Sheet, setHeader, arginfo_Sheet_setHeader, 0)
	EXCEL_ME(Sheet, setFooter, arginfo_Sheet_setFooter, 0)
	EXCEL_ME(Sheet, headerMargin, arginfo_Sheet_headerMargin, 0)
	EXCEL_ME(Sheet, footerMargin, arginfo_Sheet_footerMargin, 0)
	EXCEL_ME(Sheet, hcenter, arginfo_Sheet_hcenter, 0)
	EXCEL_ME(Sheet, vcenter, arginfo_Sheet_vcenter, 0)
	EXCEL_ME(Sheet, setHCenter, arginfo_Sheet_setHCenter, 0)
	EXCEL_ME(Sheet, setVCenter, arginfo_Sheet_setVCenter, 0)
	EXCEL_ME(Sheet, marginLeft, arginfo_Sheet_marginLeft, 0)
	EXCEL_ME(Sheet, marginRight, arginfo_Sheet_marginRight, 0)
	EXCEL_ME(Sheet, marginTop, arginfo_Sheet_marginTop, 0)
	EXCEL_ME(Sheet, marginBottom, arginfo_Sheet_marginBottom, 0)
	EXCEL_ME(Sheet, setMarginLeft, arginfo_Sheet_setMarginLeft, 0)
	EXCEL_ME(Sheet, setMarginRight, arginfo_Sheet_setMarginRight, 0)
	EXCEL_ME(Sheet, setMarginTop, arginfo_Sheet_setMarginTop, 0)
	EXCEL_ME(Sheet, setMarginBottom, arginfo_Sheet_setMarginBottom, 0)
	EXCEL_ME(Sheet, printHeaders, arginfo_Sheet_printHeaders, 0)
	EXCEL_ME(Sheet, setPrintHeaders, arginfo_Sheet_setPrintHeaders, 0)
	EXCEL_ME(Sheet, name, arginfo_Sheet_name, 0)
	EXCEL_ME(Sheet, setName, arginfo_Sheet_setName, 0)
	EXCEL_ME(Sheet, protect, arginfo_Sheet_protect, 0)
	EXCEL_ME(Sheet, setProtect, arginfo_Sheet_setProtect, 0)
#if LIBXL_VERSION >= 0x03010000
	EXCEL_ME(Sheet, setNamedRange, arginfo_Sheet_setNamedRange, 0)
	EXCEL_ME(Sheet, delNamedRange, arginfo_Sheet_delNamedRange, 0)
	EXCEL_ME(Sheet, setPrintRepeatRows, arginfo_Sheet_setPrintRepeatRows, 0)
	EXCEL_ME(Sheet, setPrintRepeatCols, arginfo_Sheet_setPrintRepeatCols, 0)
	EXCEL_ME(Sheet, setPrintArea, arginfo_Sheet_setPrintArea, 0)
	EXCEL_ME(Sheet, clearPrintRepeats, arginfo_Sheet_clearPrintRepeats, 0)
	EXCEL_ME(Sheet, clearPrintArea, arginfo_Sheet_clearPrintArea, 0)
	EXCEL_ME(Sheet, getGroupSummaryRight, arginfo_Sheet_getGroupSummaryRight, 0)
	EXCEL_ME(Sheet, setGroupSummaryBelow, arginfo_Sheet_setGroupSummaryBelow, 0)
	EXCEL_ME(Sheet, getGroupSummaryBelow, arginfo_Sheet_getGroupSummaryBelow, 0)
	EXCEL_ME(Sheet, setGroupSummaryRight, arginfo_Sheet_setGroupSummaryRight, 0)
#endif
#if LIBXL_VERSION >= 0x03020000
	EXCEL_ME(Sheet, setPrintFit, arginfo_Sheet_setPrintFit, 0)
	EXCEL_ME(Sheet, getPrintFit, arginfo_Sheet_getPrintFit, 0)
	EXCEL_ME(Sheet, getNamedRange, arginfo_Sheet_getNamedRange, 0)
	EXCEL_ME(Sheet, getIndexRange, arginfo_Sheet_getIndexRange, 0)
	EXCEL_ME(Sheet, namedRangeSize, arginfo_Sheet_namedRangeSize, 0)
	EXCEL_ME(Sheet, getVerPageBreakSize, arginfo_Sheet_getVerPageBreakSize, 0)
	EXCEL_ME(Sheet, getVerPageBreak, arginfo_Sheet_getVerPageBreak, 0)
	EXCEL_ME(Sheet, getHorPageBreak, arginfo_Sheet_getHorPageBreak, 0)
	EXCEL_ME(Sheet, getHorPageBreakSize, arginfo_Sheet_getHorPageBreakSize, 0)
	EXCEL_ME(Sheet, getNumPictures, arginfo_Sheet_getNumPictures, 0)
	EXCEL_ME(Sheet, getPictureInfo, arginfo_Sheet_getPictureInfo, 0)
#endif
#if LIBXL_VERSION >= 0x03020300
	EXCEL_ME(Sheet, setHidden, arginfo_Sheet_setHidden, 0)
	EXCEL_ME(Sheet, isHidden, arginfo_Sheet_isHidden, 0)
#endif
#if LIBXL_VERSION >= 0x03020400
	EXCEL_ME(Sheet, setTopLeftView, arginfo_Sheet_setTopLeftView, 0)
	EXCEL_ME(Sheet, getTopLeftView, arginfo_Sheet_getTopLeftView, 0)
	EXCEL_ME(Sheet, rowColToAddr, arginfo_Sheet_rowColToAddr, 0)
	EXCEL_ME(Sheet, addrToRowCol, arginfo_Sheet_addrToRowCol, 0)
#endif
#if LIBXL_VERSION >= 0x03050401
	EXCEL_ME(Sheet, getRightToLeft, arginfo_Sheet_getRightToLeft, 0)
	EXCEL_ME(Sheet, setRightToLeft, arginfo_Sheet_setRightToLeft, 0)
#endif
#if LIBXL_VERSION >= 0x03060000
	EXCEL_ME(Sheet, hyperlinkSize, arginfo_Sheet_hyperlinkSize, 0)
	EXCEL_ME(Sheet, hyperlink, arginfo_Sheet_hyperlink, 0)
	EXCEL_ME(Sheet, delHyperlink, arginfo_Sheet_delHyperlink, 0)
	EXCEL_ME(Sheet, addHyperlink, arginfo_Sheet_addHyperlink, 0)
	EXCEL_ME(Sheet, mergeSize, arginfo_Sheet_mergeSize, 0)
	EXCEL_ME(Sheet, merge, arginfo_Sheet_merge, 0)
	EXCEL_ME(Sheet, delMergeByIndex, arginfo_Sheet_delMergeByIndex, 0)
	EXCEL_ME(Sheet, splitInfo, arginfo_Sheet_splitInfo, 0)

	EXCEL_ME(Sheet, colHidden, arginfo_Sheet_colHidden, 0)
	EXCEL_ME(Sheet, rowHidden, arginfo_Sheet_rowHidden, 0)
	EXCEL_ME(Sheet, setColHidden, arginfo_Sheet_setColHidden, 0)
	EXCEL_ME(Sheet, setRowHidden, arginfo_Sheet_setRowHidden, 0)
#endif
	EXCEL_ME(Sheet, isLicensed, arginfo_Sheet_isLicensed, 0)
	{NULL, NULL, NULL}
};

zend_function_entry excel_funcs_font[] = {
	EXCEL_ME(Font, size, arginfo_Font_size, 0)
	EXCEL_ME(Font, name, arginfo_Font_name, 0)
	EXCEL_ME(Font, underline, arginfo_Font_underline, 0)
	EXCEL_ME(Font, mode, arginfo_Font_mode, 0)
	EXCEL_ME(Font, color, arginfo_Font_color, 0)
	EXCEL_ME(Font, bold, arginfo_Font_bold, 0)
	EXCEL_ME(Font, strike, arginfo_Font_strike, 0)
	EXCEL_ME(Font, italics, arginfo_Font_italics, 0)
	EXCEL_ME(Font, __construct, arginfo_Font___construct, ZEND_ACC_PUBLIC|ZEND_ACC_FINAL)
	{NULL, NULL, NULL}
};

zend_function_entry excel_funcs_format[] = {
	EXCEL_ME(Format, getFont, arginfo_Format_getFont, 0)
	EXCEL_ME(Format, setFont, arginfo_Format_setFont, 0)
	EXCEL_ME(Format, numberFormat, arginfo_Format_numberFormat, 0)
	EXCEL_ME(Format, horizontalAlign, arginfo_Format_horizontalAlign, 0)
	EXCEL_ME(Format, verticalAlign, arginfo_Format_verticalAlign, 0)
	EXCEL_ME(Format, wrap, arginfo_Format_wrap, 0)
	EXCEL_ME(Format, rotate, arginfo_Format_rotate, 0)
	EXCEL_ME(Format, indent, arginfo_Format_indent, 0)
	EXCEL_ME(Format, shrinkToFit, arginfo_Format_shrinkToFit, 0)
	EXCEL_ME(Format, borderStyle, arginfo_Format_borderStyle, 0)
	EXCEL_ME(Format, borderColor, arginfo_Format_borderColor, 0)
	EXCEL_ME(Format, borderLeftStyle, arginfo_Format_borderLeftStyle, 0)
	EXCEL_ME(Format, borderLeftColor, arginfo_Format_borderLeftColor, 0)
	EXCEL_ME(Format, borderRightStyle, arginfo_Format_borderRightStyle, 0)
	EXCEL_ME(Format, borderRightColor, arginfo_Format_borderRightColor, 0)
	EXCEL_ME(Format, borderTopStyle, arginfo_Format_borderTopStyle, 0)
	EXCEL_ME(Format, borderTopColor, arginfo_Format_borderTopColor, 0)
	EXCEL_ME(Format, borderBottomStyle, arginfo_Format_borderBottomStyle, 0)
	EXCEL_ME(Format, borderBottomColor, arginfo_Format_borderBottomColor, 0)
	EXCEL_ME(Format, borderDiagonalStyle, arginfo_Format_borderDiagonalStyle, 0)
	EXCEL_ME(Format, borderDiagonalColor, arginfo_Format_borderDiagonalColor, 0)
	EXCEL_ME(Format, fillPattern, arginfo_Format_fillPattern, 0)
	EXCEL_ME(Format, patternForegroundColor, arginfo_Format_patternForegroundColor, 0)
	EXCEL_ME(Format, patternBackgroundColor, arginfo_Format_patternBackgroundColor, 0)
	EXCEL_ME(Format, locked, arginfo_Format_locked, 0)
	EXCEL_ME(Format, hidden, arginfo_Format_hidden, 0)
	EXCEL_ME(Format, __construct, arginfo_Format___construct, 0)
	{NULL, NULL, NULL}
};

/* {{{ PHP_MINIT_FUNCTION
 */
PHP_MINIT_FUNCTION(excel)
{
	REGISTER_INI_ENTRIES();

	REGISTER_EXCEL_CLASS(Book,		book,	NULL);
	REGISTER_EXCEL_CLASS(Sheet,		sheet,	NULL);
	REGISTER_EXCEL_CLASS(Format,	format,	excel_format_object_clone);
	REGISTER_EXCEL_CLASS(Font,		font,	excel_font_object_clone);

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
#ifdef HAVE_LIBXL_243_PLUS
	REGISTER_EXCEL_CLASS_CONST_LONG(format, "FILLPATTERN_HORSTRIPE", FILLPATTERN_HORSTRIPE);
#else
	REGISTER_EXCEL_CLASS_CONST_LONG(format, "FILLPATTERN_HORSTRIPE", FILLPATTEN_HORSTRIPE);
#endif
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

	REGISTER_EXCEL_CLASS_CONST_LONG(sheet, "ERRORTYPE_NA", ERRORTYPE_NA);
	REGISTER_EXCEL_CLASS_CONST_LONG(sheet, "ERRORTYPE_NUM", ERRORTYPE_NUM);
	REGISTER_EXCEL_CLASS_CONST_LONG(sheet, "ERRORTYPE_NAME", ERRORTYPE_NAME);
	REGISTER_EXCEL_CLASS_CONST_LONG(sheet, "ERRORTYPE_REF", ERRORTYPE_REF);
	REGISTER_EXCEL_CLASS_CONST_LONG(sheet, "ERRORTYPE_VALUE", ERRORTYPE_VALUE);
	REGISTER_EXCEL_CLASS_CONST_LONG(sheet, "ERRORTYPE_DIV_0", ERRORTYPE_DIV_0);
	REGISTER_EXCEL_CLASS_CONST_LONG(sheet, "ERRORTYPE_NULL", ERRORTYPE_NULL);

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
#if LIBXL_VERSION >= 0x03020000
	REGISTER_EXCEL_CLASS_CONST_LONG(book, "PICTURETYPE_PNG", PICTURETYPE_PNG);
	REGISTER_EXCEL_CLASS_CONST_LONG(book, "PICTURETYPE_JPEG", PICTURETYPE_JPEG);
	REGISTER_EXCEL_CLASS_CONST_LONG(book, "PICTURETYPE_WMF", PICTURETYPE_WMF);
	REGISTER_EXCEL_CLASS_CONST_LONG(book, "PICTURETYPE_DIB", PICTURETYPE_DIB);
	REGISTER_EXCEL_CLASS_CONST_LONG(book, "PICTURETYPE_EMF", PICTURETYPE_EMF);
	REGISTER_EXCEL_CLASS_CONST_LONG(book, "PICTURETYPE_PICT", PICTURETYPE_PICT);
	REGISTER_EXCEL_CLASS_CONST_LONG(book, "PICTURETYPE_TIFF", PICTURETYPE_TIFF);
#endif
#if LIBXL_VERSION >= 0x03050401
	REGISTER_EXCEL_CLASS_CONST_LONG(book, "SCOPE_UNDEFINED", SCOPE_UNDEFINED);
	REGISTER_EXCEL_CLASS_CONST_LONG(book, "SCOPE_WORKBOOK", SCOPE_WORKBOOK);
	REGISTER_EXCEL_CLASS_CONST_LONG(sheet, "RIGHT_TO_LEFT", 1);
	REGISTER_EXCEL_CLASS_CONST_LONG(sheet, "LEFT_TO_RIGHT", 0);
#endif
#if LIBXL_VERSION >= 0x03060000
	REGISTER_EXCEL_CLASS_CONST_LONG(book, "SHEETTYPE_SHEET", SHEETTYPE_SHEET);
	REGISTER_EXCEL_CLASS_CONST_LONG(book, "SHEETTYPE_CHART", SHEETTYPE_CHART);
	REGISTER_EXCEL_CLASS_CONST_LONG(book, "SHEETTYPE_UNKNOWN", SHEETTYPE_UNKNOWN);
#endif
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
	php_info_print_table_header(2, "Excel Version", PHP_EXCEL_VERSION);
	snprintf(temp_api, sizeof(temp_api), "%x", LIBXL_VERSION);
	php_info_print_table_header(2, "LibXL Version", temp_api);
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
zend_function_entry excel_functions[] = {
	{NULL, NULL, NULL}
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
