/* This is a generated file, edit excel.stub.php instead.
 * Stub hash: d91e87df7471590cc01c96b533fb7b82b93ed4ec */

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_ExcelBook_requiresKey, 0, 0, _IS_BOOL, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_ExcelBook_load, 0, 1, _IS_BOOL, 0)
	ZEND_ARG_TYPE_INFO(0, data, IS_STRING, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_ExcelBook_loadFile, 0, 1, _IS_BOOL, 0)
	ZEND_ARG_TYPE_INFO(0, filename, IS_STRING, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_class_ExcelBook_save, 0, 0, MAY_BE_STRING|MAY_BE_TRUE|MAY_BE_FALSE)
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, filename, IS_STRING, 0, "\"\"")
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_OBJ_TYPE_MASK_EX(arginfo_class_ExcelBook_getSheet, 0, 0, ExcelSheet, MAY_BE_FALSE)
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, sheet, IS_LONG, 0, "0")
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_OBJ_TYPE_MASK_EX(arginfo_class_ExcelBook_getSheetByName, 0, 1, ExcelSheet, MAY_BE_FALSE)
	ZEND_ARG_TYPE_INFO(0, name, IS_STRING, 0)
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, case_insensitive, _IS_BOOL, 0, "false")
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_ExcelBook_deleteSheet, 0, 1, _IS_BOOL, 0)
	ZEND_ARG_TYPE_INFO(0, sheet, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_class_ExcelBook_activeSheet, 0, 0, MAY_BE_LONG|MAY_BE_FALSE)
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, sheet, IS_LONG, 0, "-1")
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_OBJ_TYPE_MASK_EX(arginfo_class_ExcelBook_addSheet, 0, 1, ExcelSheet, MAY_BE_FALSE)
	ZEND_ARG_TYPE_INFO(0, name, IS_STRING, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_OBJ_TYPE_MASK_EX(arginfo_class_ExcelBook_copySheet, 0, 2, ExcelSheet, MAY_BE_FALSE)
	ZEND_ARG_TYPE_INFO(0, name, IS_STRING, 0)
	ZEND_ARG_TYPE_INFO(0, sheet_number, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_class_ExcelBook_sheetCount, 0, 0, MAY_BE_LONG|MAY_BE_FALSE)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_class_ExcelBook_getError, 0, 0, MAY_BE_STRING|MAY_BE_FALSE)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_OBJ_TYPE_MASK_EX(arginfo_class_ExcelBook_addFont, 0, 0, ExcelFont, MAY_BE_FALSE)
	ZEND_ARG_OBJ_INFO_WITH_DEFAULT_VALUE(0, font, ExcelFont, 1, "null")
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_OBJ_TYPE_MASK_EX(arginfo_class_ExcelBook_addFormat, 0, 0, ExcelFormat, MAY_BE_FALSE)
	ZEND_ARG_OBJ_INFO_WITH_DEFAULT_VALUE(0, format, ExcelFormat, 1, "null")
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_class_ExcelBook_getAllFormats, 0, 0, MAY_BE_ARRAY|MAY_BE_FALSE)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_class_ExcelBook_addCustomFormat, 0, 1, MAY_BE_LONG|MAY_BE_FALSE)
	ZEND_ARG_TYPE_INFO(0, format, IS_STRING, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_class_ExcelBook_getCustomFormat, 0, 1, MAY_BE_STRING|MAY_BE_FALSE)
	ZEND_ARG_TYPE_INFO(0, id, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_class_ExcelBook_packDate, 0, 1, MAY_BE_DOUBLE|MAY_BE_FALSE)
	ZEND_ARG_TYPE_INFO(0, timestamp, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_class_ExcelBook_packDateValues, 0, 6, MAY_BE_DOUBLE|MAY_BE_FALSE)
	ZEND_ARG_TYPE_INFO(0, year, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, month, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, day, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, hour, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, min, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, sec, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_class_ExcelBook_unpackDate, 0, 1, MAY_BE_LONG|MAY_BE_FALSE)
	ZEND_ARG_TYPE_INFO(0, date, IS_DOUBLE, 0)
ZEND_END_ARG_INFO()

#define arginfo_class_ExcelBook_isDate1904 arginfo_class_ExcelBook_requiresKey

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_ExcelBook_setDate1904, 0, 1, _IS_BOOL, 0)
	ZEND_ARG_TYPE_INFO(0, date_type, _IS_BOOL, 0)
ZEND_END_ARG_INFO()

#define arginfo_class_ExcelBook_getActiveSheet arginfo_class_ExcelBook_sheetCount

#define arginfo_class_ExcelBook_getDefaultFont arginfo_class_ExcelBook_getAllFormats

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_ExcelBook_setDefaultFont, 0, 2, IS_MIXED, 0)
	ZEND_ARG_TYPE_INFO(0, font, IS_STRING, 0)
	ZEND_ARG_TYPE_INFO(0, font_size, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_ExcelBook_setLocale, 0, 1, IS_MIXED, 0)
	ZEND_ARG_TYPE_INFO(0, locale, IS_STRING, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_class_ExcelBook___construct, 0, 0, 0)
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, license_name, IS_STRING, 1, "null")
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, license_key, IS_STRING, 1, "null")
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, excel_2007, _IS_BOOL, 0, "false")
ZEND_END_ARG_INFO()

#define arginfo_class_ExcelBook_setActiveSheet arginfo_class_ExcelBook_deleteSheet

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_class_ExcelBook_addPictureFromFile, 0, 1, MAY_BE_LONG|MAY_BE_FALSE)
	ZEND_ARG_TYPE_INFO(0, filename, IS_STRING, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_class_ExcelBook_addPictureFromString, 0, 1, MAY_BE_LONG|MAY_BE_FALSE)
	ZEND_ARG_TYPE_INFO(0, data, IS_STRING, 0)
ZEND_END_ARG_INFO()

#define arginfo_class_ExcelBook_rgbMode arginfo_class_ExcelBook_requiresKey

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_ExcelBook_setRGBMode, 0, 1, _IS_BOOL, 0)
	ZEND_ARG_TYPE_INFO(0, mode, _IS_BOOL, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_class_ExcelBook_colorPack, 0, 3, MAY_BE_LONG|MAY_BE_FALSE)
	ZEND_ARG_TYPE_INFO(0, r, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, g, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, b, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_class_ExcelBook_colorUnpack, 0, 1, MAY_BE_ARRAY|MAY_BE_FALSE)
	ZEND_ARG_TYPE_INFO(0, color, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_ExcelBook_getLibXlVersion, 0, 0, IS_STRING, 0)
ZEND_END_ARG_INFO()

#define arginfo_class_ExcelBook_getPhpExcelVersion arginfo_class_ExcelBook_getLibXlVersion

#define arginfo_class_ExcelBook_loadInfo arginfo_class_ExcelBook_loadFile

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_class_ExcelBook_getSheetName, 0, 1, MAY_BE_STRING|MAY_BE_FALSE)
	ZEND_ARG_TYPE_INFO(0, index, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_OBJ_TYPE_MASK_EX(arginfo_class_ExcelBook_addRichString, 0, 0, ExcelRichString, MAY_BE_FALSE)
ZEND_END_ARG_INFO()

#define arginfo_class_ExcelBook_calcMode arginfo_class_ExcelBook_sheetCount

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_ExcelBook_setCalcMode, 0, 1, _IS_BOOL, 0)
	ZEND_ARG_TYPE_INFO(0, mode, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_OBJ_TYPE_MASK_EX(arginfo_class_ExcelBook_addConditionalFormat, 0, 0, ExcelConditionalFormat, MAY_BE_FALSE)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_OBJ_TYPE_MASK_EX(arginfo_class_ExcelBook_addFormatFromStyle, 0, 1, ExcelFormat, MAY_BE_FALSE)
	ZEND_ARG_TYPE_INFO(0, style, IS_LONG, 0)
ZEND_END_ARG_INFO()

#define arginfo_class_ExcelBook_removeVBA arginfo_class_ExcelBook_requiresKey

#define arginfo_class_ExcelBook_removePrinterSettings arginfo_class_ExcelBook_requiresKey

#if LIBXL_VERSION >= 0x05000000
ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_ExcelBook_setPassword, 0, 1, _IS_BOOL, 0)
	ZEND_ARG_TYPE_INFO(0, password, IS_STRING, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_class_ExcelBook_dpiAwareness, 0, 0, MAY_BE_LONG|MAY_BE_FALSE)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_ExcelBook_setDpiAwareness, 0, 1, _IS_BOOL, 0)
	ZEND_ARG_TYPE_INFO(0, value, IS_LONG, 0)
ZEND_END_ARG_INFO()
#endif

#if LIBXL_VERSION >= 0x05000100
ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_ExcelBook_loadInfoRaw, 0, 1, _IS_BOOL, 0)
	ZEND_ARG_TYPE_INFO(0, data, IS_STRING, 0)
ZEND_END_ARG_INFO()
#endif

#if LIBXL_VERSION >= 0x05010000
ZEND_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_class_ExcelBook_errorCode, 0, 0, MAY_BE_LONG|MAY_BE_FALSE)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_OBJ_TYPE_MASK_EX(arginfo_class_ExcelBook_conditionalFormat, 0, 1, ExcelConditionalFormat, MAY_BE_FALSE)
	ZEND_ARG_TYPE_INFO(0, index, IS_LONG, 0)
ZEND_END_ARG_INFO()

#define arginfo_class_ExcelBook_conditionalFormatSize arginfo_class_ExcelBook_errorCode

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_ExcelBook_clear, 0, 0, _IS_BOOL, 0)
ZEND_END_ARG_INFO()
#endif

ZEND_BEGIN_ARG_WITH_RETURN_OBJ_TYPE_MASK_EX(arginfo_class_ExcelBook_coreProperties, 0, 0, ExcelCoreProperties, MAY_BE_FALSE)
ZEND_END_ARG_INFO()

#define arginfo_class_ExcelBook_removeAllPhonetics arginfo_class_ExcelBook_requiresKey

#define arginfo_class_ExcelBook_biffVersion arginfo_class_ExcelBook_sheetCount

#define arginfo_class_ExcelBook_getRefR1C1 arginfo_class_ExcelBook_requiresKey

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_ExcelBook_setRefR1C1, 0, 1, _IS_BOOL, 0)
	ZEND_ARG_TYPE_INFO(0, active, _IS_BOOL, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_class_ExcelBook_getPicture, 0, 1, MAY_BE_ARRAY|MAY_BE_FALSE)
	ZEND_ARG_TYPE_INFO(0, index, IS_LONG, 0)
ZEND_END_ARG_INFO()

#define arginfo_class_ExcelBook_getNumPictures arginfo_class_ExcelBook_sheetCount

ZEND_BEGIN_ARG_WITH_RETURN_OBJ_TYPE_MASK_EX(arginfo_class_ExcelBook_insertSheet, 0, 2, ExcelSheet, MAY_BE_FALSE)
	ZEND_ARG_TYPE_INFO(0, index, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, name, IS_STRING, 0)
	ZEND_ARG_OBJ_INFO_WITH_DEFAULT_VALUE(0, sheet, ExcelSheet, 1, "null")
ZEND_END_ARG_INFO()

#define arginfo_class_ExcelBook_isTemplate arginfo_class_ExcelBook_requiresKey

#define arginfo_class_ExcelBook_setTemplate arginfo_class_ExcelBook_setRGBMode

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_class_ExcelBook_sheetType, 0, 1, MAY_BE_LONG|MAY_BE_FALSE)
	ZEND_ARG_TYPE_INFO(0, sheet, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_class_ExcelBook_addPictureAsLink, 0, 1, MAY_BE_LONG|MAY_BE_FALSE)
	ZEND_ARG_TYPE_INFO(0, filename, IS_STRING, 0)
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, insert, _IS_BOOL, 0, "false")
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_ExcelBook_moveSheet, 0, 2, _IS_BOOL, 0)
	ZEND_ARG_TYPE_INFO(0, src_index, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, dest_index, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_class_ExcelSheet___construct, 0, 0, 2)
	ZEND_ARG_OBJ_INFO(0, book, ExcelBook, 0)
	ZEND_ARG_TYPE_INFO(0, name, IS_STRING, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_class_ExcelSheet_cellType, 0, 2, MAY_BE_LONG|MAY_BE_FALSE)
	ZEND_ARG_TYPE_INFO(0, row, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, column, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_OBJ_TYPE_MASK_EX(arginfo_class_ExcelSheet_cellFormat, 0, 2, ExcelFormat, MAY_BE_FALSE)
	ZEND_ARG_TYPE_INFO(0, row, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, column, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_ExcelSheet_setCellFormat, 0, 3, IS_MIXED, 0)
	ZEND_ARG_TYPE_INFO(0, row, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, column, IS_LONG, 0)
	ZEND_ARG_OBJ_INFO(0, format, ExcelFormat, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_class_ExcelSheet_readRow, 0, 1, MAY_BE_ARRAY|MAY_BE_FALSE)
	ZEND_ARG_TYPE_INFO(0, row, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, start_col, IS_LONG, 0, "0")
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, end_column, IS_LONG, 0, "-1")
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, read_formula, _IS_BOOL, 0, "true")
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_class_ExcelSheet_readCol, 0, 1, MAY_BE_ARRAY|MAY_BE_FALSE)
	ZEND_ARG_TYPE_INFO(0, column, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, start_row, IS_LONG, 0, "0")
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, end_row, IS_LONG, 0, "-1")
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, read_formula, _IS_BOOL, 0, "true")
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_ExcelSheet_read, 0, 2, IS_MIXED, 0)
	ZEND_ARG_TYPE_INFO(0, row, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, column, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(1, format, IS_MIXED, 0, "null")
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, read_formula, _IS_BOOL, 0, "true")
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_ExcelSheet_write, 0, 3, _IS_BOOL, 0)
	ZEND_ARG_TYPE_INFO(0, row, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, column, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, data, IS_MIXED, 0)
	ZEND_ARG_OBJ_INFO_WITH_DEFAULT_VALUE(0, format, ExcelFormat, 1, "null")
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, datatype, IS_LONG, 0, "0")
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_ExcelSheet_writeRow, 0, 2, _IS_BOOL, 0)
	ZEND_ARG_TYPE_INFO(0, row, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, data, IS_ARRAY, 0)
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, start_column, IS_LONG, 0, "0")
	ZEND_ARG_OBJ_INFO_WITH_DEFAULT_VALUE(0, format, ExcelFormat, 1, "null")
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_ExcelSheet_writeCol, 0, 2, _IS_BOOL, 0)
	ZEND_ARG_TYPE_INFO(0, column, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, data, IS_ARRAY, 0)
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, start_row, IS_LONG, 0, "0")
	ZEND_ARG_OBJ_INFO_WITH_DEFAULT_VALUE(0, format, ExcelFormat, 1, "null")
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, data_type, IS_LONG, 0, "0")
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_ExcelSheet_isFormula, 0, 2, _IS_BOOL, 0)
	ZEND_ARG_TYPE_INFO(0, row, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, column, IS_LONG, 0)
ZEND_END_ARG_INFO()

#define arginfo_class_ExcelSheet_isDate arginfo_class_ExcelSheet_isFormula

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_ExcelSheet_insertRow, 0, 2, _IS_BOOL, 0)
	ZEND_ARG_TYPE_INFO(0, row_first, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, row_last, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_ExcelSheet_insertCol, 0, 2, _IS_BOOL, 0)
	ZEND_ARG_TYPE_INFO(0, col_first, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, col_last, IS_LONG, 0)
ZEND_END_ARG_INFO()

#define arginfo_class_ExcelSheet_removeRow arginfo_class_ExcelSheet_insertRow

#define arginfo_class_ExcelSheet_removeCol arginfo_class_ExcelSheet_insertCol

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_class_ExcelSheet_colWidth, 0, 1, MAY_BE_DOUBLE|MAY_BE_FALSE)
	ZEND_ARG_TYPE_INFO(0, column, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_class_ExcelSheet_rowHeight, 0, 1, MAY_BE_DOUBLE|MAY_BE_FALSE)
	ZEND_ARG_TYPE_INFO(0, row, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_class_ExcelSheet_readComment, 0, 2, MAY_BE_STRING|MAY_BE_FALSE)
	ZEND_ARG_TYPE_INFO(0, row, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, column, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_ExcelSheet_writeComment, 0, 6, IS_MIXED, 0)
	ZEND_ARG_TYPE_INFO(0, row, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, column, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, value, IS_STRING, 0)
	ZEND_ARG_TYPE_INFO(0, author, IS_STRING, 0)
	ZEND_ARG_TYPE_INFO(0, width, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, height, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_ExcelSheet_setColWidth, 0, 3, _IS_BOOL, 0)
	ZEND_ARG_TYPE_INFO(0, column_start, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, column_end, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, width, IS_DOUBLE, 0)
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, hidden, _IS_BOOL, 0, "false")
	ZEND_ARG_OBJ_INFO_WITH_DEFAULT_VALUE(0, format, ExcelFormat, 1, "null")
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_ExcelSheet_setRowHeight, 0, 2, _IS_BOOL, 0)
	ZEND_ARG_TYPE_INFO(0, row, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, height, IS_DOUBLE, 0)
	ZEND_ARG_OBJ_INFO_WITH_DEFAULT_VALUE(0, format, ExcelFormat, 1, "null")
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, hidden, _IS_BOOL, 0, "false")
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_class_ExcelSheet_getMerge, 0, 2, MAY_BE_ARRAY|MAY_BE_FALSE)
	ZEND_ARG_TYPE_INFO(0, row, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, column, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_ExcelSheet_setMerge, 0, 4, _IS_BOOL, 0)
	ZEND_ARG_TYPE_INFO(0, row_start, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, row_end, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, col_start, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, col_end, IS_LONG, 0)
ZEND_END_ARG_INFO()

#define arginfo_class_ExcelSheet_deleteMerge arginfo_class_ExcelSheet_isFormula

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_ExcelSheet_addPictureScaled, 0, 4, IS_MIXED, 0)
	ZEND_ARG_TYPE_INFO(0, row, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, column, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, pic_id, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, scale, IS_DOUBLE, 0)
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, x_offset, IS_LONG, 0, "0")
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, y_offset, IS_LONG, 0, "0")
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, pos, IS_LONG, 0, "0")
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_ExcelSheet_addPictureDim, 0, 5, IS_MIXED, 0)
	ZEND_ARG_TYPE_INFO(0, row, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, column, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, pic_id, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, width, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, height, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, x_offset, IS_LONG, 0, "0")
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, y_offset, IS_LONG, 0, "0")
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, pos, IS_LONG, 0, "0")
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_ExcelSheet_horPageBreak, 0, 2, _IS_BOOL, 0)
	ZEND_ARG_TYPE_INFO(0, row, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, break, _IS_BOOL, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_ExcelSheet_verPageBreak, 0, 2, _IS_BOOL, 0)
	ZEND_ARG_TYPE_INFO(0, col, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, break, _IS_BOOL, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_ExcelSheet_splitSheet, 0, 2, IS_MIXED, 0)
	ZEND_ARG_TYPE_INFO(0, row, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, column, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_ExcelSheet_groupRows, 0, 2, _IS_BOOL, 0)
	ZEND_ARG_TYPE_INFO(0, start_row, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, end_row, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, collapse, _IS_BOOL, 0, "false")
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_ExcelSheet_groupCols, 0, 2, _IS_BOOL, 0)
	ZEND_ARG_TYPE_INFO(0, start_column, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, end_column, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, collapse, _IS_BOOL, 0, "false")
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_ExcelSheet_clear, 0, 4, IS_MIXED, 0)
	ZEND_ARG_TYPE_INFO(0, row_s, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, row_e, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, col_s, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, col_e, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_ExcelSheet_copy, 0, 4, _IS_BOOL, 0)
	ZEND_ARG_TYPE_INFO(0, row, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, col, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, to_row, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, to_col, IS_LONG, 0)
ZEND_END_ARG_INFO()

#define arginfo_class_ExcelSheet_firstRow arginfo_class_ExcelBook_sheetCount

#define arginfo_class_ExcelSheet_lastRow arginfo_class_ExcelBook_sheetCount

#define arginfo_class_ExcelSheet_firstCol arginfo_class_ExcelBook_sheetCount

#define arginfo_class_ExcelSheet_lastCol arginfo_class_ExcelBook_sheetCount

#define arginfo_class_ExcelSheet_displayGridlines arginfo_class_ExcelBook_requiresKey

#define arginfo_class_ExcelSheet_printGridlines arginfo_class_ExcelBook_requiresKey

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_ExcelSheet_setDisplayGridlines, 0, 1, IS_MIXED, 0)
	ZEND_ARG_TYPE_INFO(0, value, _IS_BOOL, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_ExcelSheet_setHidden, 0, 1, _IS_BOOL, 0)
	ZEND_ARG_TYPE_INFO(0, value, _IS_BOOL, 0)
ZEND_END_ARG_INFO()

#define arginfo_class_ExcelSheet_isHidden arginfo_class_ExcelBook_requiresKey

#define arginfo_class_ExcelSheet_getTopLeftView arginfo_class_ExcelBook_getAllFormats

#define arginfo_class_ExcelSheet_setTopLeftView arginfo_class_ExcelSheet_isFormula

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_class_ExcelSheet_rowColToAddr, 0, 2, MAY_BE_STRING|MAY_BE_FALSE)
	ZEND_ARG_TYPE_INFO(0, row, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, column, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, row_relative, _IS_BOOL, 0, "true")
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, col_relative, _IS_BOOL, 0, "true")
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_class_ExcelSheet_addrToRowCol, 0, 1, MAY_BE_ARRAY|MAY_BE_FALSE)
	ZEND_ARG_TYPE_INFO(0, cell_reference, IS_STRING, 0)
ZEND_END_ARG_INFO()

#define arginfo_class_ExcelSheet_setPrintGridlines arginfo_class_ExcelSheet_setDisplayGridlines

#define arginfo_class_ExcelSheet_zoom arginfo_class_ExcelBook_sheetCount

#define arginfo_class_ExcelSheet_zoomPrint arginfo_class_ExcelBook_sheetCount

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_ExcelSheet_setZoom, 0, 1, IS_MIXED, 0)
	ZEND_ARG_TYPE_INFO(0, value, IS_LONG, 0)
ZEND_END_ARG_INFO()

#define arginfo_class_ExcelSheet_setZoomPrint arginfo_class_ExcelSheet_setZoom

#define arginfo_class_ExcelSheet_setLandscape arginfo_class_ExcelSheet_setDisplayGridlines

#define arginfo_class_ExcelSheet_landscape arginfo_class_ExcelBook_requiresKey

#define arginfo_class_ExcelSheet_paper arginfo_class_ExcelBook_sheetCount

#define arginfo_class_ExcelSheet_setPaper arginfo_class_ExcelSheet_setZoom

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_class_ExcelSheet_header, 0, 0, MAY_BE_STRING|MAY_BE_NULL|MAY_BE_FALSE)
ZEND_END_ARG_INFO()

#define arginfo_class_ExcelSheet_footer arginfo_class_ExcelSheet_header

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_ExcelSheet_setHeader, 0, 2, _IS_BOOL, 0)
	ZEND_ARG_TYPE_INFO(0, header, IS_STRING, 0)
	ZEND_ARG_TYPE_INFO(0, margin, IS_DOUBLE, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_ExcelSheet_setFooter, 0, 2, _IS_BOOL, 0)
	ZEND_ARG_TYPE_INFO(0, footer, IS_STRING, 0)
	ZEND_ARG_TYPE_INFO(0, margin, IS_DOUBLE, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_class_ExcelSheet_headerMargin, 0, 0, MAY_BE_DOUBLE|MAY_BE_FALSE)
ZEND_END_ARG_INFO()

#define arginfo_class_ExcelSheet_footerMargin arginfo_class_ExcelSheet_headerMargin

#define arginfo_class_ExcelSheet_hcenter arginfo_class_ExcelBook_requiresKey

#define arginfo_class_ExcelSheet_vcenter arginfo_class_ExcelBook_requiresKey

#define arginfo_class_ExcelSheet_setHCenter arginfo_class_ExcelSheet_setDisplayGridlines

#define arginfo_class_ExcelSheet_setVCenter arginfo_class_ExcelSheet_setDisplayGridlines

#define arginfo_class_ExcelSheet_marginLeft arginfo_class_ExcelSheet_headerMargin

#define arginfo_class_ExcelSheet_marginRight arginfo_class_ExcelSheet_headerMargin

#define arginfo_class_ExcelSheet_marginTop arginfo_class_ExcelSheet_headerMargin

#define arginfo_class_ExcelSheet_marginBottom arginfo_class_ExcelSheet_headerMargin

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_ExcelSheet_setMarginLeft, 0, 1, IS_MIXED, 0)
	ZEND_ARG_TYPE_INFO(0, value, IS_DOUBLE, 0)
ZEND_END_ARG_INFO()

#define arginfo_class_ExcelSheet_setMarginRight arginfo_class_ExcelSheet_setMarginLeft

#define arginfo_class_ExcelSheet_setMarginTop arginfo_class_ExcelSheet_setMarginLeft

#define arginfo_class_ExcelSheet_setMarginBottom arginfo_class_ExcelSheet_setMarginLeft

#define arginfo_class_ExcelSheet_printHeaders arginfo_class_ExcelBook_requiresKey

#define arginfo_class_ExcelSheet_setPrintHeaders arginfo_class_ExcelSheet_setDisplayGridlines

#define arginfo_class_ExcelSheet_name arginfo_class_ExcelSheet_header

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_ExcelSheet_setName, 0, 1, IS_MIXED, 0)
	ZEND_ARG_TYPE_INFO(0, name, IS_STRING, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_ExcelSheet_setNamedRange, 0, 5, _IS_BOOL, 0)
	ZEND_ARG_TYPE_INFO(0, name, IS_STRING, 0)
	ZEND_ARG_TYPE_INFO(0, row_first, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, row_last, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, col_first, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, col_last, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, scope_id, IS_LONG, 0, "-1")
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_ExcelSheet_delNamedRange, 0, 1, _IS_BOOL, 0)
	ZEND_ARG_TYPE_INFO(0, name, IS_STRING, 0)
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, scope_id, IS_LONG, 0, "-1")
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_ExcelSheet_setPrintRepeatRows, 0, 2, _IS_BOOL, 0)
	ZEND_ARG_TYPE_INFO(0, row_start, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, row_end, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_ExcelSheet_setPrintRepeatCols, 0, 2, _IS_BOOL, 0)
	ZEND_ARG_TYPE_INFO(0, col_start, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, col_end, IS_LONG, 0)
ZEND_END_ARG_INFO()

#define arginfo_class_ExcelSheet_getGroupSummaryBelow arginfo_class_ExcelBook_requiresKey

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_ExcelSheet_setGroupSummaryBelow, 0, 1, _IS_BOOL, 0)
	ZEND_ARG_TYPE_INFO(0, direction, _IS_BOOL, 0)
ZEND_END_ARG_INFO()

#define arginfo_class_ExcelSheet_getGroupSummaryRight arginfo_class_ExcelBook_requiresKey

#define arginfo_class_ExcelSheet_setGroupSummaryRight arginfo_class_ExcelSheet_setGroupSummaryBelow

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_ExcelSheet_setPrintFit, 0, 2, _IS_BOOL, 0)
	ZEND_ARG_TYPE_INFO(0, wPages, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, hPages, IS_LONG, 0)
ZEND_END_ARG_INFO()

#define arginfo_class_ExcelSheet_getPrintFit arginfo_class_ExcelBook_getAllFormats

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_class_ExcelSheet_getNamedRange, 0, 1, MAY_BE_ARRAY|MAY_BE_FALSE)
	ZEND_ARG_TYPE_INFO(0, name, IS_STRING, 0)
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, scope_id, IS_LONG, 0, "-1")
ZEND_END_ARG_INFO()

#define arginfo_class_ExcelSheet_getIndexRange arginfo_class_ExcelBook_getPicture

#define arginfo_class_ExcelSheet_namedRangeSize arginfo_class_ExcelBook_sheetCount

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_class_ExcelSheet_getVerPageBreak, 0, 1, MAY_BE_LONG|MAY_BE_FALSE)
	ZEND_ARG_TYPE_INFO(0, index, IS_LONG, 0)
ZEND_END_ARG_INFO()

#define arginfo_class_ExcelSheet_getVerPageBreakSize arginfo_class_ExcelBook_sheetCount

#define arginfo_class_ExcelSheet_getHorPageBreak arginfo_class_ExcelSheet_getVerPageBreak

#define arginfo_class_ExcelSheet_getHorPageBreakSize arginfo_class_ExcelBook_sheetCount

#define arginfo_class_ExcelSheet_getPictureInfo arginfo_class_ExcelBook_getPicture

#define arginfo_class_ExcelSheet_getNumPictures arginfo_class_ExcelBook_sheetCount

#define arginfo_class_ExcelSheet_getRightToLeft arginfo_class_ExcelBook_sheetCount

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_ExcelSheet_setRightToLeft, 0, 1, IS_MIXED, 0)
	ZEND_ARG_TYPE_INFO(0, mode, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_ExcelSheet_setPrintArea, 0, 4, _IS_BOOL, 0)
	ZEND_ARG_TYPE_INFO(0, row_first, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, row_last, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, col_first, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, col_last, IS_LONG, 0)
ZEND_END_ARG_INFO()

#define arginfo_class_ExcelSheet_clearPrintRepeats arginfo_class_ExcelBook_requiresKey

#define arginfo_class_ExcelSheet_clearPrintArea arginfo_class_ExcelBook_requiresKey

#define arginfo_class_ExcelSheet_protect arginfo_class_ExcelBook_requiresKey

#define arginfo_class_ExcelSheet_hyperlinkSize arginfo_class_ExcelBook_sheetCount

#define arginfo_class_ExcelSheet_hyperlink arginfo_class_ExcelBook_getPicture

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_ExcelSheet_delHyperlink, 0, 1, _IS_BOOL, 0)
	ZEND_ARG_TYPE_INFO(0, index, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_ExcelSheet_addHyperlink, 0, 5, _IS_BOOL, 0)
	ZEND_ARG_TYPE_INFO(0, hyperlink, IS_STRING, 0)
	ZEND_ARG_TYPE_INFO(0, row_first, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, row_last, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, col_first, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, col_last, IS_LONG, 0)
ZEND_END_ARG_INFO()

#define arginfo_class_ExcelSheet_mergeSize arginfo_class_ExcelBook_sheetCount

#define arginfo_class_ExcelSheet_merge arginfo_class_ExcelBook_getPicture

#define arginfo_class_ExcelSheet_delMergeByIndex arginfo_class_ExcelSheet_delHyperlink

#define arginfo_class_ExcelSheet_splitInfo arginfo_class_ExcelBook_getAllFormats

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_ExcelSheet_rowHidden, 0, 1, _IS_BOOL, 0)
	ZEND_ARG_TYPE_INFO(0, row, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_ExcelSheet_setRowHidden, 0, 2, _IS_BOOL, 0)
	ZEND_ARG_TYPE_INFO(0, row, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, hidden, _IS_BOOL, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_ExcelSheet_colHidden, 0, 1, _IS_BOOL, 0)
	ZEND_ARG_TYPE_INFO(0, col, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_ExcelSheet_setColHidden, 0, 2, _IS_BOOL, 0)
	ZEND_ARG_TYPE_INFO(0, col, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, hidden, _IS_BOOL, 0)
ZEND_END_ARG_INFO()

#define arginfo_class_ExcelSheet_isLicensed arginfo_class_ExcelBook_requiresKey

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_ExcelSheet_setAutoFitArea, 0, 0, _IS_BOOL, 0)
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, row_start, IS_LONG, 0, "0")
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, row_end, IS_LONG, 0, "-1")
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, col_start, IS_LONG, 0, "0")
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, col_end, IS_LONG, 0, "-1")
ZEND_END_ARG_INFO()

#define arginfo_class_ExcelSheet_printRepeatRows arginfo_class_ExcelBook_getAllFormats

#define arginfo_class_ExcelSheet_printRepeatCols arginfo_class_ExcelBook_getAllFormats

#define arginfo_class_ExcelSheet_printArea arginfo_class_ExcelBook_getAllFormats

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_ExcelSheet_setProtect, 0, 1, _IS_BOOL, 0)
	ZEND_ARG_TYPE_INFO(0, value, _IS_BOOL, 0)
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, password, IS_STRING, 0, "\"\"")
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, enhancedProtection, IS_LONG, 0, "-1")
ZEND_END_ARG_INFO()

#define arginfo_class_ExcelSheet_table arginfo_class_ExcelBook_getPicture

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_ExcelSheet_setTabColor, 0, 0, _IS_BOOL, 0)
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, color, IS_LONG, 0, "0")
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_OBJ_TYPE_MASK_EX(arginfo_class_ExcelSheet_autoFilter, 0, 0, ExcelAutoFilter, MAY_BE_FALSE)
ZEND_END_ARG_INFO()

#define arginfo_class_ExcelSheet_applyFilter arginfo_class_ExcelBook_requiresKey

#define arginfo_class_ExcelSheet_removeFilter arginfo_class_ExcelBook_requiresKey

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_ExcelSheet_addIgnoredError, 0, 1, _IS_BOOL, 0)
	ZEND_ARG_TYPE_INFO(0, iError, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, rowFirst, IS_LONG, 0, "0")
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, colFirst, IS_LONG, 0, "0")
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, rowLast, IS_LONG, 0, "0")
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, colLast, IS_LONG, 0, "0")
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_ExcelSheet_writeError, 0, 3, IS_MIXED, 0)
	ZEND_ARG_TYPE_INFO(0, row, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, col, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, iError, IS_LONG, 0)
	ZEND_ARG_OBJ_INFO_WITH_DEFAULT_VALUE(0, format, ExcelFormat, 1, "null")
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_ExcelSheet_removeComment, 0, 2, IS_MIXED, 0)
	ZEND_ARG_TYPE_INFO(0, row, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, col, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_ExcelSheet_addDataValidation, 0, 7, _IS_BOOL, 0)
	ZEND_ARG_TYPE_INFO(0, type, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, op, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, row_first, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, row_last, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, col_first, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, col_last, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, val_1, IS_STRING, 0)
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, val_2, IS_STRING, 1, "null")
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, allow_blank, _IS_BOOL, 0, "true")
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, hide_dropdown, _IS_BOOL, 0, "false")
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, show_inputmessage, _IS_BOOL, 0, "true")
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, show_errormessage, _IS_BOOL, 0, "true")
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, prompt_title, IS_STRING, 0, "\"\"")
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, prompt, IS_STRING, 0, "\"\"")
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, error_title, IS_STRING, 0, "\"\"")
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, error, IS_STRING, 0, "\"\"")
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, error_style, IS_LONG, 0, "1")
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_ExcelSheet_addDataValidationDouble, 0, 7, _IS_BOOL, 0)
	ZEND_ARG_TYPE_INFO(0, type, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, op, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, row_first, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, row_last, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, col_first, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, col_last, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, val_1, IS_DOUBLE, 0)
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, val_2, IS_DOUBLE, 1, "null")
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, allow_blank, _IS_BOOL, 0, "true")
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, hide_dropdown, _IS_BOOL, 0, "false")
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, show_inputmessage, _IS_BOOL, 0, "true")
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, show_errormessage, _IS_BOOL, 0, "true")
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, prompt_title, IS_STRING, 0, "\"\"")
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, prompt, IS_STRING, 0, "\"\"")
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, error_title, IS_STRING, 0, "\"\"")
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, error, IS_STRING, 0, "\"\"")
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, error_style, IS_LONG, 0, "1")
ZEND_END_ARG_INFO()

#define arginfo_class_ExcelSheet_removeDataValidations arginfo_class_ExcelBook_requiresKey

#if LIBXL_VERSION >= 0x05020000
ZEND_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_class_ExcelSheet_dataValidationSize, 0, 0, MAY_BE_LONG|MAY_BE_FALSE)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_class_ExcelSheet_dataValidation, 0, 1, MAY_BE_ARRAY|MAY_BE_FALSE)
	ZEND_ARG_TYPE_INFO(0, index, IS_LONG, 0)
ZEND_END_ARG_INFO()
#endif

#define arginfo_class_ExcelSheet_firstFilledRow arginfo_class_ExcelBook_sheetCount

#define arginfo_class_ExcelSheet_lastFilledRow arginfo_class_ExcelBook_sheetCount

#define arginfo_class_ExcelSheet_firstFilledCol arginfo_class_ExcelBook_sheetCount

#define arginfo_class_ExcelSheet_lastFilledCol arginfo_class_ExcelBook_sheetCount

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_ExcelSheet_removePicture, 0, 2, _IS_BOOL, 0)
	ZEND_ARG_TYPE_INFO(0, row, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, col, IS_LONG, 0)
ZEND_END_ARG_INFO()

#define arginfo_class_ExcelSheet_removePictureByIndex arginfo_class_ExcelSheet_delHyperlink

#define arginfo_class_ExcelSheet_isRichStr arginfo_class_ExcelSheet_removePicture

ZEND_BEGIN_ARG_WITH_RETURN_OBJ_TYPE_MASK_EX(arginfo_class_ExcelSheet_readRichStr, 0, 2, ExcelRichString, MAY_BE_FALSE)
	ZEND_ARG_TYPE_INFO(0, row, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, col, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_ExcelSheet_writeRichStr, 0, 3, _IS_BOOL, 0)
	ZEND_ARG_TYPE_INFO(0, row, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, col, IS_LONG, 0)
	ZEND_ARG_OBJ_INFO(0, richString, ExcelRichString, 0)
	ZEND_ARG_OBJ_INFO_WITH_DEFAULT_VALUE(0, format, ExcelFormat, 1, "null")
ZEND_END_ARG_INFO()

#define arginfo_class_ExcelSheet_formControlSize arginfo_class_ExcelBook_sheetCount

ZEND_BEGIN_ARG_WITH_RETURN_OBJ_TYPE_MASK_EX(arginfo_class_ExcelSheet_formControl, 0, 1, ExcelFormControl, MAY_BE_FALSE)
	ZEND_ARG_TYPE_INFO(0, index, IS_LONG, 0)
ZEND_END_ARG_INFO()

#define arginfo_class_ExcelSheet_getActiveCell arginfo_class_ExcelBook_getAllFormats

#define arginfo_class_ExcelSheet_setActiveCell arginfo_class_ExcelSheet_removePicture

#define arginfo_class_ExcelSheet_selectionRange arginfo_class_ExcelSheet_header

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_ExcelSheet_addSelectionRange, 0, 1, _IS_BOOL, 0)
	ZEND_ARG_TYPE_INFO(0, sqref, IS_STRING, 0)
ZEND_END_ARG_INFO()

#define arginfo_class_ExcelSheet_removeSelection arginfo_class_ExcelBook_requiresKey

#define arginfo_class_ExcelSheet_tabColor arginfo_class_ExcelBook_sheetCount

#define arginfo_class_ExcelSheet_getTabRgbColor arginfo_class_ExcelBook_getAllFormats

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_ExcelSheet_setTabRgbColor, 0, 3, _IS_BOOL, 0)
	ZEND_ARG_TYPE_INFO(0, red, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, green, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, blue, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_class_ExcelSheet_hyperlinkIndex, 0, 2, MAY_BE_LONG|MAY_BE_FALSE)
	ZEND_ARG_TYPE_INFO(0, row, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, col, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_class_ExcelSheet_colWidthPx, 0, 1, MAY_BE_LONG|MAY_BE_FALSE)
	ZEND_ARG_TYPE_INFO(0, col, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_class_ExcelSheet_rowHeightPx, 0, 1, MAY_BE_LONG|MAY_BE_FALSE)
	ZEND_ARG_TYPE_INFO(0, row, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_OBJ_TYPE_MASK_EX(arginfo_class_ExcelSheet_colFormat, 0, 1, ExcelFormat, MAY_BE_FALSE)
	ZEND_ARG_TYPE_INFO(0, col, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_OBJ_TYPE_MASK_EX(arginfo_class_ExcelSheet_rowFormat, 0, 1, ExcelFormat, MAY_BE_FALSE)
	ZEND_ARG_TYPE_INFO(0, row, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_ExcelSheet_setColPx, 0, 3, _IS_BOOL, 0)
	ZEND_ARG_TYPE_INFO(0, colFirst, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, colLast, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, widthPx, IS_LONG, 0)
	ZEND_ARG_OBJ_INFO_WITH_DEFAULT_VALUE(0, format, ExcelFormat, 1, "null")
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, hidden, _IS_BOOL, 0, "false")
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_ExcelSheet_setRowPx, 0, 2, _IS_BOOL, 0)
	ZEND_ARG_TYPE_INFO(0, row, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, heightPx, IS_LONG, 0)
	ZEND_ARG_OBJ_INFO_WITH_DEFAULT_VALUE(0, format, ExcelFormat, 1, "null")
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, hidden, _IS_BOOL, 0, "false")
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_ExcelSheet_setBorder, 0, 6, _IS_BOOL, 0)
	ZEND_ARG_TYPE_INFO(0, rowFirst, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, rowLast, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, colFirst, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, colLast, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, borderStyle, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, borderColor, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_OBJ_TYPE_MASK_EX(arginfo_class_ExcelSheet_addTable, 0, 5, ExcelTable, MAY_BE_FALSE)
	ZEND_ARG_TYPE_INFO(0, name, IS_STRING, 0)
	ZEND_ARG_TYPE_INFO(0, rowFirst, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, rowLast, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, colFirst, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, colLast, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, hasHeaders, _IS_BOOL, 0, "true")
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, style, IS_LONG, 0, "0")
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_OBJ_TYPE_MASK_EX(arginfo_class_ExcelSheet_getTableByName, 0, 1, ExcelTable, MAY_BE_FALSE)
	ZEND_ARG_TYPE_INFO(0, name, IS_STRING, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_OBJ_TYPE_MASK_EX(arginfo_class_ExcelSheet_getTableByIndex, 0, 1, ExcelTable, MAY_BE_FALSE)
	ZEND_ARG_TYPE_INFO(0, index, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_ExcelSheet_applyFilter2, 0, 1, _IS_BOOL, 0)
	ZEND_ARG_OBJ_INFO(0, autoFilter, ExcelAutoFilter, 0)
ZEND_END_ARG_INFO()

#if LIBXL_VERSION >= 0x05010000
ZEND_BEGIN_ARG_WITH_RETURN_OBJ_TYPE_MASK_EX(arginfo_class_ExcelSheet_addConditionalFormatting, 0, 4, ExcelConditionalFormatting, MAY_BE_FALSE)
	ZEND_ARG_TYPE_INFO(0, rowFirst, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, rowLast, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, colFirst, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, colLast, IS_LONG, 0)
ZEND_END_ARG_INFO()
#endif

#if !(LIBXL_VERSION >= 0x05010000)
ZEND_BEGIN_ARG_WITH_RETURN_OBJ_TYPE_MASK_EX(arginfo_class_ExcelSheet_addConditionalFormatting, 0, 0, ExcelConditionalFormatting, MAY_BE_FALSE)
ZEND_END_ARG_INFO()
#endif

#if LIBXL_VERSION >= 0x05010000
ZEND_BEGIN_ARG_WITH_RETURN_OBJ_TYPE_MASK_EX(arginfo_class_ExcelSheet_conditionalFormatting, 0, 1, ExcelConditionalFormatting, MAY_BE_FALSE)
	ZEND_ARG_TYPE_INFO(0, index, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_ExcelSheet_removeConditionalFormatting, 0, 1, _IS_BOOL, 0)
	ZEND_ARG_TYPE_INFO(0, index, IS_LONG, 0)
ZEND_END_ARG_INFO()

#define arginfo_class_ExcelSheet_conditionalFormattingSize arginfo_class_ExcelBook_errorCode
#endif

ZEND_BEGIN_ARG_INFO_EX(arginfo_class_ExcelFormat___construct, 0, 0, 1)
	ZEND_ARG_OBJ_INFO(0, book, ExcelBook, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_ExcelFormat_setFont, 0, 1, _IS_BOOL, 0)
	ZEND_ARG_OBJ_INFO(0, font, ExcelFont, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_OBJ_TYPE_MASK_EX(arginfo_class_ExcelFormat_getFont, 0, 0, ExcelFont, MAY_BE_FALSE)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_class_ExcelFormat_numberFormat, 0, 0, MAY_BE_LONG|MAY_BE_FALSE)
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, format, IS_LONG, 1, "null")
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_class_ExcelFormat_horizontalAlign, 0, 0, MAY_BE_LONG|MAY_BE_FALSE)
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, align_mode, IS_LONG, 1, "null")
ZEND_END_ARG_INFO()

#define arginfo_class_ExcelFormat_verticalAlign arginfo_class_ExcelFormat_horizontalAlign

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_ExcelFormat_wrap, 0, 0, _IS_BOOL, 0)
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, wrap, _IS_BOOL, 1, "null")
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_class_ExcelFormat_rotate, 0, 0, MAY_BE_LONG|MAY_BE_FALSE)
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, angle, IS_LONG, 1, "null")
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_class_ExcelFormat_indent, 0, 0, MAY_BE_LONG|MAY_BE_FALSE)
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, indent, IS_LONG, 1, "null")
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_ExcelFormat_shrinkToFit, 0, 0, _IS_BOOL, 0)
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, shrink, _IS_BOOL, 1, "null")
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_ExcelFormat_borderStyle, 0, 0, _IS_BOOL, 0)
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, style, IS_LONG, 1, "null")
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_ExcelFormat_borderColor, 0, 0, _IS_BOOL, 0)
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, color, IS_LONG, 1, "null")
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_class_ExcelFormat_borderLeftStyle, 0, 0, MAY_BE_LONG|MAY_BE_FALSE)
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, style, IS_LONG, 1, "null")
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_class_ExcelFormat_borderLeftColor, 0, 0, MAY_BE_LONG|MAY_BE_FALSE)
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, color, IS_LONG, 1, "null")
ZEND_END_ARG_INFO()

#define arginfo_class_ExcelFormat_borderRightStyle arginfo_class_ExcelFormat_borderLeftStyle

#define arginfo_class_ExcelFormat_borderRightColor arginfo_class_ExcelFormat_borderLeftColor

#define arginfo_class_ExcelFormat_borderTopStyle arginfo_class_ExcelFormat_borderLeftStyle

#define arginfo_class_ExcelFormat_borderTopColor arginfo_class_ExcelFormat_borderLeftColor

#define arginfo_class_ExcelFormat_borderBottomStyle arginfo_class_ExcelFormat_borderLeftStyle

#define arginfo_class_ExcelFormat_borderBottomColor arginfo_class_ExcelFormat_borderLeftColor

#define arginfo_class_ExcelFormat_borderDiagonalStyle arginfo_class_ExcelFormat_borderLeftStyle

#define arginfo_class_ExcelFormat_borderDiagonalColor arginfo_class_ExcelFormat_borderLeftColor

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_class_ExcelFormat_fillPattern, 0, 0, MAY_BE_LONG|MAY_BE_FALSE)
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, patern, IS_LONG, 1, "null")
ZEND_END_ARG_INFO()

#define arginfo_class_ExcelFormat_patternForegroundColor arginfo_class_ExcelFormat_borderLeftColor

#define arginfo_class_ExcelFormat_patternBackgroundColor arginfo_class_ExcelFormat_borderLeftColor

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_ExcelFormat_locked, 0, 0, _IS_BOOL, 0)
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, locked, _IS_BOOL, 1, "null")
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_ExcelFormat_hidden, 0, 0, _IS_BOOL, 0)
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, hidden, _IS_BOOL, 1, "null")
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_class_ExcelFont_size, 0, 0, MAY_BE_LONG|MAY_BE_FALSE)
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, size, IS_LONG, 1, "null")
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_ExcelFont_italics, 0, 0, _IS_BOOL, 0)
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, italics, _IS_BOOL, 1, "null")
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_ExcelFont_strike, 0, 0, _IS_BOOL, 0)
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, strike, _IS_BOOL, 1, "null")
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_ExcelFont_bold, 0, 0, _IS_BOOL, 0)
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, bold, _IS_BOOL, 1, "null")
ZEND_END_ARG_INFO()

#define arginfo_class_ExcelFont_color arginfo_class_ExcelFormat_borderLeftColor

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_class_ExcelFont_mode, 0, 0, MAY_BE_LONG|MAY_BE_FALSE)
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, mode, IS_LONG, 1, "null")
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_class_ExcelFont_underline, 0, 0, MAY_BE_LONG|MAY_BE_FALSE)
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, underline_style, IS_LONG, 1, "null")
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_class_ExcelFont_name, 0, 0, MAY_BE_STRING|MAY_BE_FALSE)
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, name, IS_STRING, 1, "null")
ZEND_END_ARG_INFO()

#define arginfo_class_ExcelFont___construct arginfo_class_ExcelFormat___construct

ZEND_BEGIN_ARG_INFO_EX(arginfo_class_ExcelAutoFilter___construct, 0, 0, 1)
	ZEND_ARG_OBJ_INFO(0, sheet, ExcelSheet, 0)
ZEND_END_ARG_INFO()

#define arginfo_class_ExcelAutoFilter_getRef arginfo_class_ExcelBook_getAllFormats

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_ExcelAutoFilter_setRef, 0, 4, IS_MIXED, 0)
	ZEND_ARG_TYPE_INFO(0, row_first, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, row_last, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, col_first, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, col_last, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_OBJ_TYPE_MASK_EX(arginfo_class_ExcelAutoFilter_column, 0, 1, ExcelFilterColumn, MAY_BE_FALSE)
	ZEND_ARG_TYPE_INFO(0, colId, IS_LONG, 0)
ZEND_END_ARG_INFO()

#define arginfo_class_ExcelAutoFilter_columnSize arginfo_class_ExcelBook_sheetCount

ZEND_BEGIN_ARG_WITH_RETURN_OBJ_TYPE_MASK_EX(arginfo_class_ExcelAutoFilter_columnByIndex, 0, 1, ExcelFilterColumn, MAY_BE_FALSE)
	ZEND_ARG_TYPE_INFO(0, index, IS_LONG, 0)
ZEND_END_ARG_INFO()

#define arginfo_class_ExcelAutoFilter_getSortRange arginfo_class_ExcelBook_getAllFormats

#define arginfo_class_ExcelAutoFilter_getSort arginfo_class_ExcelBook_getAllFormats

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_ExcelAutoFilter_setSort, 0, 2, _IS_BOOL, 0)
	ZEND_ARG_TYPE_INFO(0, columnIndex, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, descending, _IS_BOOL, 0)
ZEND_END_ARG_INFO()

#define arginfo_class_ExcelAutoFilter_addSort arginfo_class_ExcelAutoFilter_setSort

ZEND_BEGIN_ARG_INFO_EX(arginfo_class_ExcelFilterColumn___construct, 0, 0, 2)
	ZEND_ARG_OBJ_INFO(0, autoFilter, ExcelAutoFilter, 0)
	ZEND_ARG_TYPE_INFO(0, colId, IS_LONG, 0)
ZEND_END_ARG_INFO()

#define arginfo_class_ExcelFilterColumn_index arginfo_class_ExcelBook_sheetCount

#define arginfo_class_ExcelFilterColumn_filterType arginfo_class_ExcelBook_sheetCount

#define arginfo_class_ExcelFilterColumn_filterSize arginfo_class_ExcelBook_sheetCount

#define arginfo_class_ExcelFilterColumn_filter arginfo_class_ExcelBook_getSheetName

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_ExcelFilterColumn_addFilter, 0, 1, _IS_BOOL, 0)
	ZEND_ARG_TYPE_INFO(0, filterValue, IS_STRING, 0)
ZEND_END_ARG_INFO()

#define arginfo_class_ExcelFilterColumn_getTop10 arginfo_class_ExcelBook_getAllFormats

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_ExcelFilterColumn_setTop10, 0, 1, _IS_BOOL, 0)
	ZEND_ARG_TYPE_INFO(0, value, IS_DOUBLE, 0)
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, top, _IS_BOOL, 0, "true")
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, percent, _IS_BOOL, 0, "false")
ZEND_END_ARG_INFO()

#define arginfo_class_ExcelFilterColumn_getCustomFilter arginfo_class_ExcelBook_getAllFormats

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_ExcelFilterColumn_setCustomFilter, 0, 2, _IS_BOOL, 0)
	ZEND_ARG_TYPE_INFO(0, operator_1, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, value_1, IS_STRING, 0)
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, operator_2, IS_LONG, 0, "-1")
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, value_2, IS_STRING, 1, "null")
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, andOp, _IS_BOOL, 0, "false")
ZEND_END_ARG_INFO()

#define arginfo_class_ExcelFilterColumn_clear arginfo_class_ExcelBook_requiresKey

#define arginfo_class_ExcelRichString___construct arginfo_class_ExcelFormat___construct

#define arginfo_class_ExcelRichString_addFont arginfo_class_ExcelBook_addFont

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_ExcelRichString_addText, 0, 1, _IS_BOOL, 0)
	ZEND_ARG_TYPE_INFO(0, text, IS_STRING, 0)
	ZEND_ARG_OBJ_INFO_WITH_DEFAULT_VALUE(0, font, ExcelFont, 1, "null")
ZEND_END_ARG_INFO()

#define arginfo_class_ExcelRichString_getText arginfo_class_ExcelBook_getPicture

#define arginfo_class_ExcelRichString_textSize arginfo_class_ExcelBook_sheetCount

ZEND_BEGIN_ARG_INFO_EX(arginfo_class_ExcelFormControl___construct, 0, 0, 2)
	ZEND_ARG_OBJ_INFO(0, sheet, ExcelSheet, 0)
	ZEND_ARG_TYPE_INFO(0, index, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_ExcelFormControl_objectType, 0, 0, IS_MIXED, 0)
ZEND_END_ARG_INFO()

#define arginfo_class_ExcelFormControl_checked arginfo_class_ExcelFormControl_objectType

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_ExcelFormControl_setChecked, 0, 1, _IS_BOOL, 0)
	ZEND_ARG_TYPE_INFO(0, value, IS_LONG, 0)
ZEND_END_ARG_INFO()

#define arginfo_class_ExcelFormControl_fmlaGroup arginfo_class_ExcelFormControl_objectType

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_ExcelFormControl_setFmlaGroup, 0, 1, _IS_BOOL, 0)
	ZEND_ARG_TYPE_INFO(0, value, IS_STRING, 0)
ZEND_END_ARG_INFO()

#define arginfo_class_ExcelFormControl_fmlaLink arginfo_class_ExcelFormControl_objectType

#define arginfo_class_ExcelFormControl_setFmlaLink arginfo_class_ExcelFormControl_setFmlaGroup

#define arginfo_class_ExcelFormControl_fmlaRange arginfo_class_ExcelFormControl_objectType

#define arginfo_class_ExcelFormControl_setFmlaRange arginfo_class_ExcelFormControl_setFmlaGroup

#define arginfo_class_ExcelFormControl_fmlaTxbx arginfo_class_ExcelFormControl_objectType

#define arginfo_class_ExcelFormControl_setFmlaTxbx arginfo_class_ExcelFormControl_setFmlaGroup

#define arginfo_class_ExcelFormControl_name arginfo_class_ExcelFormControl_objectType

#define arginfo_class_ExcelFormControl_linkedCell arginfo_class_ExcelFormControl_objectType

#define arginfo_class_ExcelFormControl_listFillRange arginfo_class_ExcelFormControl_objectType

#define arginfo_class_ExcelFormControl_macro arginfo_class_ExcelFormControl_objectType

#define arginfo_class_ExcelFormControl_altText arginfo_class_ExcelFormControl_objectType

#define arginfo_class_ExcelFormControl_locked arginfo_class_ExcelFormControl_objectType

#define arginfo_class_ExcelFormControl_defaultSize arginfo_class_ExcelFormControl_objectType

#define arginfo_class_ExcelFormControl_print arginfo_class_ExcelFormControl_objectType

#define arginfo_class_ExcelFormControl_disabled arginfo_class_ExcelFormControl_objectType

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_class_ExcelFormControl_item, 0, 1, MAY_BE_STRING|MAY_BE_NULL|MAY_BE_FALSE)
	ZEND_ARG_TYPE_INFO(0, index, IS_LONG, 0)
ZEND_END_ARG_INFO()

#define arginfo_class_ExcelFormControl_itemSize arginfo_class_ExcelFormControl_objectType

#define arginfo_class_ExcelFormControl_addItem arginfo_class_ExcelFormControl_setFmlaGroup

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_ExcelFormControl_insertItem, 0, 2, _IS_BOOL, 0)
	ZEND_ARG_TYPE_INFO(0, index, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, value, IS_STRING, 0)
ZEND_END_ARG_INFO()

#define arginfo_class_ExcelFormControl_clearItems arginfo_class_ExcelFormControl_objectType

#define arginfo_class_ExcelFormControl_dropLines arginfo_class_ExcelFormControl_objectType

#define arginfo_class_ExcelFormControl_setDropLines arginfo_class_ExcelFormControl_setChecked

#define arginfo_class_ExcelFormControl_dx arginfo_class_ExcelFormControl_objectType

#define arginfo_class_ExcelFormControl_setDx arginfo_class_ExcelFormControl_setChecked

#define arginfo_class_ExcelFormControl_firstButton arginfo_class_ExcelFormControl_objectType

#define arginfo_class_ExcelFormControl_setFirstButton arginfo_class_ExcelSheet_setHidden

#define arginfo_class_ExcelFormControl_horiz arginfo_class_ExcelFormControl_objectType

#define arginfo_class_ExcelFormControl_setHoriz arginfo_class_ExcelSheet_setHidden

#define arginfo_class_ExcelFormControl_inc arginfo_class_ExcelFormControl_objectType

#define arginfo_class_ExcelFormControl_setInc arginfo_class_ExcelFormControl_setChecked

#define arginfo_class_ExcelFormControl_getMax arginfo_class_ExcelFormControl_objectType

#define arginfo_class_ExcelFormControl_setMax arginfo_class_ExcelFormControl_setChecked

#define arginfo_class_ExcelFormControl_getMin arginfo_class_ExcelFormControl_objectType

#define arginfo_class_ExcelFormControl_setMin arginfo_class_ExcelFormControl_setChecked

#define arginfo_class_ExcelFormControl_multiSel arginfo_class_ExcelFormControl_objectType

#define arginfo_class_ExcelFormControl_setMultiSel arginfo_class_ExcelFormControl_setFmlaGroup

#define arginfo_class_ExcelFormControl_sel arginfo_class_ExcelFormControl_objectType

#define arginfo_class_ExcelFormControl_setSel arginfo_class_ExcelFormControl_setChecked

#define arginfo_class_ExcelFormControl_fromAnchor arginfo_class_ExcelFormControl_objectType

#define arginfo_class_ExcelFormControl_toAnchor arginfo_class_ExcelFormControl_objectType

#define arginfo_class_ExcelConditionalFormat___construct arginfo_class_ExcelFormat___construct

#define arginfo_class_ExcelConditionalFormat_font arginfo_class_ExcelFormControl_objectType

#define arginfo_class_ExcelConditionalFormat_numFormat arginfo_class_ExcelFormControl_objectType

#define arginfo_class_ExcelConditionalFormat_setNumFormat arginfo_class_ExcelFormControl_setChecked

#define arginfo_class_ExcelConditionalFormat_customNumFormat arginfo_class_ExcelFormControl_objectType

#define arginfo_class_ExcelConditionalFormat_setCustomNumFormat arginfo_class_ExcelFormControl_setFmlaGroup

#define arginfo_class_ExcelConditionalFormat_setBorder arginfo_class_ExcelFormControl_setChecked

#define arginfo_class_ExcelConditionalFormat_setBorderColor arginfo_class_ExcelFormControl_setChecked

#define arginfo_class_ExcelConditionalFormat_borderLeft arginfo_class_ExcelFormControl_objectType

#define arginfo_class_ExcelConditionalFormat_setBorderLeft arginfo_class_ExcelFormControl_setChecked

#define arginfo_class_ExcelConditionalFormat_borderRight arginfo_class_ExcelFormControl_objectType

#define arginfo_class_ExcelConditionalFormat_setBorderRight arginfo_class_ExcelFormControl_setChecked

#define arginfo_class_ExcelConditionalFormat_borderTop arginfo_class_ExcelFormControl_objectType

#define arginfo_class_ExcelConditionalFormat_setBorderTop arginfo_class_ExcelFormControl_setChecked

#define arginfo_class_ExcelConditionalFormat_borderBottom arginfo_class_ExcelFormControl_objectType

#define arginfo_class_ExcelConditionalFormat_setBorderBottom arginfo_class_ExcelFormControl_setChecked

#define arginfo_class_ExcelConditionalFormat_borderLeftColor arginfo_class_ExcelFormControl_objectType

#define arginfo_class_ExcelConditionalFormat_setBorderLeftColor arginfo_class_ExcelFormControl_setChecked

#define arginfo_class_ExcelConditionalFormat_borderRightColor arginfo_class_ExcelFormControl_objectType

#define arginfo_class_ExcelConditionalFormat_setBorderRightColor arginfo_class_ExcelFormControl_setChecked

#define arginfo_class_ExcelConditionalFormat_borderTopColor arginfo_class_ExcelFormControl_objectType

#define arginfo_class_ExcelConditionalFormat_setBorderTopColor arginfo_class_ExcelFormControl_setChecked

#define arginfo_class_ExcelConditionalFormat_borderBottomColor arginfo_class_ExcelFormControl_objectType

#define arginfo_class_ExcelConditionalFormat_setBorderBottomColor arginfo_class_ExcelFormControl_setChecked

#define arginfo_class_ExcelConditionalFormat_fillPattern arginfo_class_ExcelFormControl_objectType

#define arginfo_class_ExcelConditionalFormat_setFillPattern arginfo_class_ExcelFormControl_setChecked

#define arginfo_class_ExcelConditionalFormat_patternForegroundColor arginfo_class_ExcelFormControl_objectType

#define arginfo_class_ExcelConditionalFormat_setPatternForegroundColor arginfo_class_ExcelFormControl_setChecked

#define arginfo_class_ExcelConditionalFormat_patternBackgroundColor arginfo_class_ExcelFormControl_objectType

#define arginfo_class_ExcelConditionalFormat_setPatternBackgroundColor arginfo_class_ExcelFormControl_setChecked

#if LIBXL_VERSION >= 0x05010000
ZEND_BEGIN_ARG_INFO_EX(arginfo_class_ExcelConditionalFormatting___construct, 0, 0, 5)
	ZEND_ARG_OBJ_INFO(0, sheet, ExcelSheet, 0)
	ZEND_ARG_TYPE_INFO(0, rowFirst, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, rowLast, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, colFirst, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, colLast, IS_LONG, 0)
ZEND_END_ARG_INFO()
#endif

#if !(LIBXL_VERSION >= 0x05010000)
ZEND_BEGIN_ARG_INFO_EX(arginfo_class_ExcelConditionalFormatting___construct, 0, 0, 1)
	ZEND_ARG_OBJ_INFO(0, sheet, ExcelSheet, 0)
ZEND_END_ARG_INFO()
#endif

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_ExcelConditionalFormatting_addRange, 0, 4, _IS_BOOL, 0)
	ZEND_ARG_TYPE_INFO(0, rowFirst, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, rowLast, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, colFirst, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, colLast, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_ExcelConditionalFormatting_addRule, 0, 3, _IS_BOOL, 0)
	ZEND_ARG_TYPE_INFO(0, type, IS_LONG, 0)
	ZEND_ARG_OBJ_INFO(0, cf, ExcelConditionalFormat, 0)
	ZEND_ARG_TYPE_INFO(0, value, IS_STRING, 0)
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, stopIfTrue, _IS_BOOL, 0, "false")
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_ExcelConditionalFormatting_addTopRule, 0, 4, _IS_BOOL, 0)
	ZEND_ARG_OBJ_INFO(0, cf, ExcelConditionalFormat, 0)
	ZEND_ARG_TYPE_INFO(0, value, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, bottom, _IS_BOOL, 0)
	ZEND_ARG_TYPE_INFO(0, percent, _IS_BOOL, 0)
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, stopIfTrue, _IS_BOOL, 0, "false")
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_ExcelConditionalFormatting_addOpNumRule, 0, 4, _IS_BOOL, 0)
	ZEND_ARG_TYPE_INFO(0, op, IS_LONG, 0)
	ZEND_ARG_OBJ_INFO(0, cf, ExcelConditionalFormat, 0)
	ZEND_ARG_TYPE_INFO(0, value1, IS_DOUBLE, 0)
	ZEND_ARG_TYPE_INFO(0, value2, IS_DOUBLE, 0)
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, stopIfTrue, _IS_BOOL, 0, "false")
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_ExcelConditionalFormatting_addOpStrRule, 0, 4, _IS_BOOL, 0)
	ZEND_ARG_TYPE_INFO(0, op, IS_LONG, 0)
	ZEND_ARG_OBJ_INFO(0, cf, ExcelConditionalFormat, 0)
	ZEND_ARG_TYPE_INFO(0, value1, IS_STRING, 0)
	ZEND_ARG_TYPE_INFO(0, value2, IS_STRING, 0)
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, stopIfTrue, _IS_BOOL, 0, "false")
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_ExcelConditionalFormatting_addAboveAverageRule, 0, 4, _IS_BOOL, 0)
	ZEND_ARG_OBJ_INFO(0, cf, ExcelConditionalFormat, 0)
	ZEND_ARG_TYPE_INFO(0, above, _IS_BOOL, 0)
	ZEND_ARG_TYPE_INFO(0, equal, _IS_BOOL, 0)
	ZEND_ARG_TYPE_INFO(0, stdDev, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, stopIfTrue, _IS_BOOL, 0, "false")
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_ExcelConditionalFormatting_addTimePeriodRule, 0, 2, _IS_BOOL, 0)
	ZEND_ARG_OBJ_INFO(0, cf, ExcelConditionalFormat, 0)
	ZEND_ARG_TYPE_INFO(0, timePeriod, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, stopIfTrue, _IS_BOOL, 0, "false")
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_ExcelConditionalFormatting_add2ColorScaleRule, 0, 6, _IS_BOOL, 0)
	ZEND_ARG_TYPE_INFO(0, minColor, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, maxColor, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, minType, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, minValue, IS_DOUBLE, 0)
	ZEND_ARG_TYPE_INFO(0, maxType, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, maxValue, IS_DOUBLE, 0)
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, stopIfTrue, _IS_BOOL, 0, "false")
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_ExcelConditionalFormatting_add2ColorScaleFormulaRule, 0, 6, _IS_BOOL, 0)
	ZEND_ARG_TYPE_INFO(0, minColor, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, maxColor, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, minType, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, minValue, IS_STRING, 0)
	ZEND_ARG_TYPE_INFO(0, maxType, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, maxValue, IS_STRING, 0)
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, stopIfTrue, _IS_BOOL, 0, "false")
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_ExcelConditionalFormatting_add3ColorScaleRule, 0, 9, _IS_BOOL, 0)
	ZEND_ARG_TYPE_INFO(0, minColor, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, midColor, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, maxColor, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, minType, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, minValue, IS_DOUBLE, 0)
	ZEND_ARG_TYPE_INFO(0, midType, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, midValue, IS_DOUBLE, 0)
	ZEND_ARG_TYPE_INFO(0, maxType, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, maxValue, IS_DOUBLE, 0)
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, stopIfTrue, _IS_BOOL, 0, "false")
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_ExcelConditionalFormatting_add3ColorScaleFormulaRule, 0, 9, _IS_BOOL, 0)
	ZEND_ARG_TYPE_INFO(0, minColor, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, midColor, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, maxColor, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, minType, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, minValue, IS_STRING, 0)
	ZEND_ARG_TYPE_INFO(0, midType, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, midValue, IS_STRING, 0)
	ZEND_ARG_TYPE_INFO(0, maxType, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, maxValue, IS_STRING, 0)
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, stopIfTrue, _IS_BOOL, 0, "false")
ZEND_END_ARG_INFO()

#define arginfo_class_ExcelCoreProperties___construct arginfo_class_ExcelFormat___construct

#define arginfo_class_ExcelCoreProperties_title arginfo_class_ExcelSheet_header

#define arginfo_class_ExcelCoreProperties_setTitle arginfo_class_ExcelFormControl_setFmlaGroup

#define arginfo_class_ExcelCoreProperties_subject arginfo_class_ExcelSheet_header

#define arginfo_class_ExcelCoreProperties_setSubject arginfo_class_ExcelFormControl_setFmlaGroup

#define arginfo_class_ExcelCoreProperties_creator arginfo_class_ExcelSheet_header

#define arginfo_class_ExcelCoreProperties_setCreator arginfo_class_ExcelFormControl_setFmlaGroup

#define arginfo_class_ExcelCoreProperties_lastModifiedBy arginfo_class_ExcelSheet_header

#define arginfo_class_ExcelCoreProperties_setLastModifiedBy arginfo_class_ExcelFormControl_setFmlaGroup

#define arginfo_class_ExcelCoreProperties_created arginfo_class_ExcelSheet_header

#define arginfo_class_ExcelCoreProperties_setCreated arginfo_class_ExcelFormControl_setFmlaGroup

#define arginfo_class_ExcelCoreProperties_modified arginfo_class_ExcelSheet_header

#define arginfo_class_ExcelCoreProperties_setModified arginfo_class_ExcelFormControl_setFmlaGroup

#define arginfo_class_ExcelCoreProperties_tags arginfo_class_ExcelSheet_header

#define arginfo_class_ExcelCoreProperties_setTags arginfo_class_ExcelFormControl_setFmlaGroup

#define arginfo_class_ExcelCoreProperties_categories arginfo_class_ExcelSheet_header

#define arginfo_class_ExcelCoreProperties_setCategories arginfo_class_ExcelFormControl_setFmlaGroup

#define arginfo_class_ExcelCoreProperties_comments arginfo_class_ExcelSheet_header

#define arginfo_class_ExcelCoreProperties_setComments arginfo_class_ExcelFormControl_setFmlaGroup

#define arginfo_class_ExcelCoreProperties_createdAsDouble arginfo_class_ExcelFormControl_objectType

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_ExcelCoreProperties_setCreatedAsDouble, 0, 1, _IS_BOOL, 0)
	ZEND_ARG_TYPE_INFO(0, value, IS_DOUBLE, 0)
ZEND_END_ARG_INFO()

#define arginfo_class_ExcelCoreProperties_modifiedAsDouble arginfo_class_ExcelFormControl_objectType

#define arginfo_class_ExcelCoreProperties_setModifiedAsDouble arginfo_class_ExcelCoreProperties_setCreatedAsDouble

#define arginfo_class_ExcelCoreProperties_removeAll arginfo_class_ExcelFormControl_objectType

ZEND_BEGIN_ARG_INFO_EX(arginfo_class_ExcelTable___construct, 0, 0, 6)
	ZEND_ARG_OBJ_INFO(0, sheet, ExcelSheet, 0)
	ZEND_ARG_TYPE_INFO(0, name, IS_STRING, 0)
	ZEND_ARG_TYPE_INFO(0, rowFirst, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, rowLast, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, colFirst, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, colLast, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, hasHeaders, _IS_BOOL, 0, "true")
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, style, IS_LONG, 0, "0")
ZEND_END_ARG_INFO()

#define arginfo_class_ExcelTable_name arginfo_class_ExcelFormControl_objectType

#define arginfo_class_ExcelTable_setName arginfo_class_ExcelFormControl_setFmlaGroup

#define arginfo_class_ExcelTable_ref arginfo_class_ExcelFormControl_objectType

#define arginfo_class_ExcelTable_setRef arginfo_class_ExcelFormControl_setFmlaGroup

#define arginfo_class_ExcelTable_autoFilter arginfo_class_ExcelFormControl_objectType

#if LIBXL_VERSION >= 0x05020000
ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_ExcelTable_isAutoFilter, 0, 0, _IS_BOOL, 0)
ZEND_END_ARG_INFO()

#define arginfo_class_ExcelTable_removeFilter arginfo_class_ExcelTable_isAutoFilter
#endif

#define arginfo_class_ExcelTable_style arginfo_class_ExcelFormControl_objectType

#define arginfo_class_ExcelTable_setStyle arginfo_class_ExcelFormControl_setChecked

#define arginfo_class_ExcelTable_showRowStripes arginfo_class_ExcelFormControl_objectType

#define arginfo_class_ExcelTable_setShowRowStripes arginfo_class_ExcelSheet_setHidden

#define arginfo_class_ExcelTable_showColumnStripes arginfo_class_ExcelFormControl_objectType

#define arginfo_class_ExcelTable_setShowColumnStripes arginfo_class_ExcelSheet_setHidden

#define arginfo_class_ExcelTable_showFirstColumn arginfo_class_ExcelFormControl_objectType

#define arginfo_class_ExcelTable_setShowFirstColumn arginfo_class_ExcelSheet_setHidden

#define arginfo_class_ExcelTable_showLastColumn arginfo_class_ExcelFormControl_objectType

#define arginfo_class_ExcelTable_setShowLastColumn arginfo_class_ExcelSheet_setHidden

#define arginfo_class_ExcelTable_columnSize arginfo_class_ExcelFormControl_objectType

#define arginfo_class_ExcelTable_columnName arginfo_class_ExcelFormControl_item

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_ExcelTable_setColumnName, 0, 2, _IS_BOOL, 0)
	ZEND_ARG_TYPE_INFO(0, index, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, name, IS_STRING, 0)
ZEND_END_ARG_INFO()

ZEND_METHOD(ExcelBook, requiresKey);
ZEND_METHOD(ExcelBook, load);
ZEND_METHOD(ExcelBook, loadFile);
ZEND_METHOD(ExcelBook, save);
ZEND_METHOD(ExcelBook, getSheet);
ZEND_METHOD(ExcelBook, getSheetByName);
ZEND_METHOD(ExcelBook, deleteSheet);
ZEND_METHOD(ExcelBook, activeSheet);
ZEND_METHOD(ExcelBook, addSheet);
ZEND_METHOD(ExcelBook, copySheet);
ZEND_METHOD(ExcelBook, sheetCount);
ZEND_METHOD(ExcelBook, getError);
ZEND_METHOD(ExcelBook, addFont);
ZEND_METHOD(ExcelBook, addFormat);
ZEND_METHOD(ExcelBook, getAllFormats);
ZEND_METHOD(ExcelBook, addCustomFormat);
ZEND_METHOD(ExcelBook, getCustomFormat);
ZEND_METHOD(ExcelBook, packDate);
ZEND_METHOD(ExcelBook, packDateValues);
ZEND_METHOD(ExcelBook, unpackDate);
ZEND_METHOD(ExcelBook, isDate1904);
ZEND_METHOD(ExcelBook, setDate1904);
ZEND_METHOD(ExcelBook, getActiveSheet);
ZEND_METHOD(ExcelBook, getDefaultFont);
ZEND_METHOD(ExcelBook, setDefaultFont);
ZEND_METHOD(ExcelBook, setLocale);
ZEND_METHOD(ExcelBook, __construct);
ZEND_METHOD(ExcelBook, setActiveSheet);
ZEND_METHOD(ExcelBook, addPictureFromFile);
ZEND_METHOD(ExcelBook, addPictureFromString);
ZEND_METHOD(ExcelBook, rgbMode);
ZEND_METHOD(ExcelBook, setRGBMode);
ZEND_METHOD(ExcelBook, colorPack);
ZEND_METHOD(ExcelBook, colorUnpack);
ZEND_METHOD(ExcelBook, getLibXlVersion);
ZEND_METHOD(ExcelBook, getPhpExcelVersion);
ZEND_METHOD(ExcelBook, loadInfo);
ZEND_METHOD(ExcelBook, getSheetName);
ZEND_METHOD(ExcelBook, addRichString);
ZEND_METHOD(ExcelBook, calcMode);
ZEND_METHOD(ExcelBook, setCalcMode);
ZEND_METHOD(ExcelBook, addConditionalFormat);
ZEND_METHOD(ExcelBook, addFormatFromStyle);
ZEND_METHOD(ExcelBook, removeVBA);
ZEND_METHOD(ExcelBook, removePrinterSettings);
#if LIBXL_VERSION >= 0x05000000
ZEND_METHOD(ExcelBook, setPassword);
ZEND_METHOD(ExcelBook, dpiAwareness);
ZEND_METHOD(ExcelBook, setDpiAwareness);
#endif
#if LIBXL_VERSION >= 0x05000100
ZEND_METHOD(ExcelBook, loadInfoRaw);
#endif
#if LIBXL_VERSION >= 0x05010000
ZEND_METHOD(ExcelBook, errorCode);
ZEND_METHOD(ExcelBook, conditionalFormat);
ZEND_METHOD(ExcelBook, conditionalFormatSize);
ZEND_METHOD(ExcelBook, clear);
#endif
ZEND_METHOD(ExcelBook, coreProperties);
ZEND_METHOD(ExcelBook, removeAllPhonetics);
ZEND_METHOD(ExcelBook, biffVersion);
ZEND_METHOD(ExcelBook, getRefR1C1);
ZEND_METHOD(ExcelBook, setRefR1C1);
ZEND_METHOD(ExcelBook, getPicture);
ZEND_METHOD(ExcelBook, getNumPictures);
ZEND_METHOD(ExcelBook, insertSheet);
ZEND_METHOD(ExcelBook, isTemplate);
ZEND_METHOD(ExcelBook, setTemplate);
ZEND_METHOD(ExcelBook, sheetType);
ZEND_METHOD(ExcelBook, addPictureAsLink);
ZEND_METHOD(ExcelBook, moveSheet);
ZEND_METHOD(ExcelSheet, __construct);
ZEND_METHOD(ExcelSheet, cellType);
ZEND_METHOD(ExcelSheet, cellFormat);
ZEND_METHOD(ExcelSheet, setCellFormat);
ZEND_METHOD(ExcelSheet, readRow);
ZEND_METHOD(ExcelSheet, readCol);
ZEND_METHOD(ExcelSheet, read);
ZEND_METHOD(ExcelSheet, write);
ZEND_METHOD(ExcelSheet, writeRow);
ZEND_METHOD(ExcelSheet, writeCol);
ZEND_METHOD(ExcelSheet, isFormula);
ZEND_METHOD(ExcelSheet, isDate);
ZEND_METHOD(ExcelSheet, insertRow);
ZEND_METHOD(ExcelSheet, insertCol);
ZEND_METHOD(ExcelSheet, removeRow);
ZEND_METHOD(ExcelSheet, removeCol);
ZEND_METHOD(ExcelSheet, colWidth);
ZEND_METHOD(ExcelSheet, rowHeight);
ZEND_METHOD(ExcelSheet, readComment);
ZEND_METHOD(ExcelSheet, writeComment);
ZEND_METHOD(ExcelSheet, setColWidth);
ZEND_METHOD(ExcelSheet, setRowHeight);
ZEND_METHOD(ExcelSheet, getMerge);
ZEND_METHOD(ExcelSheet, setMerge);
ZEND_METHOD(ExcelSheet, deleteMerge);
ZEND_METHOD(ExcelSheet, addPictureScaled);
ZEND_METHOD(ExcelSheet, addPictureDim);
ZEND_METHOD(ExcelSheet, horPageBreak);
ZEND_METHOD(ExcelSheet, verPageBreak);
ZEND_METHOD(ExcelSheet, splitSheet);
ZEND_METHOD(ExcelSheet, groupRows);
ZEND_METHOD(ExcelSheet, groupCols);
ZEND_METHOD(ExcelSheet, clear);
ZEND_METHOD(ExcelSheet, copy);
ZEND_METHOD(ExcelSheet, firstRow);
ZEND_METHOD(ExcelSheet, lastRow);
ZEND_METHOD(ExcelSheet, firstCol);
ZEND_METHOD(ExcelSheet, lastCol);
ZEND_METHOD(ExcelSheet, displayGridlines);
ZEND_METHOD(ExcelSheet, printGridlines);
ZEND_METHOD(ExcelSheet, setDisplayGridlines);
ZEND_METHOD(ExcelSheet, setHidden);
ZEND_METHOD(ExcelSheet, isHidden);
ZEND_METHOD(ExcelSheet, getTopLeftView);
ZEND_METHOD(ExcelSheet, setTopLeftView);
ZEND_METHOD(ExcelSheet, rowColToAddr);
ZEND_METHOD(ExcelSheet, addrToRowCol);
ZEND_METHOD(ExcelSheet, setPrintGridlines);
ZEND_METHOD(ExcelSheet, zoom);
ZEND_METHOD(ExcelSheet, zoomPrint);
ZEND_METHOD(ExcelSheet, setZoom);
ZEND_METHOD(ExcelSheet, setZoomPrint);
ZEND_METHOD(ExcelSheet, setLandscape);
ZEND_METHOD(ExcelSheet, landscape);
ZEND_METHOD(ExcelSheet, paper);
ZEND_METHOD(ExcelSheet, setPaper);
ZEND_METHOD(ExcelSheet, header);
ZEND_METHOD(ExcelSheet, footer);
ZEND_METHOD(ExcelSheet, setHeader);
ZEND_METHOD(ExcelSheet, setFooter);
ZEND_METHOD(ExcelSheet, headerMargin);
ZEND_METHOD(ExcelSheet, footerMargin);
ZEND_METHOD(ExcelSheet, hcenter);
ZEND_METHOD(ExcelSheet, vcenter);
ZEND_METHOD(ExcelSheet, setHCenter);
ZEND_METHOD(ExcelSheet, setVCenter);
ZEND_METHOD(ExcelSheet, marginLeft);
ZEND_METHOD(ExcelSheet, marginRight);
ZEND_METHOD(ExcelSheet, marginTop);
ZEND_METHOD(ExcelSheet, marginBottom);
ZEND_METHOD(ExcelSheet, setMarginLeft);
ZEND_METHOD(ExcelSheet, setMarginRight);
ZEND_METHOD(ExcelSheet, setMarginTop);
ZEND_METHOD(ExcelSheet, setMarginBottom);
ZEND_METHOD(ExcelSheet, printHeaders);
ZEND_METHOD(ExcelSheet, setPrintHeaders);
ZEND_METHOD(ExcelSheet, name);
ZEND_METHOD(ExcelSheet, setName);
ZEND_METHOD(ExcelSheet, setNamedRange);
ZEND_METHOD(ExcelSheet, delNamedRange);
ZEND_METHOD(ExcelSheet, setPrintRepeatRows);
ZEND_METHOD(ExcelSheet, setPrintRepeatCols);
ZEND_METHOD(ExcelSheet, getGroupSummaryBelow);
ZEND_METHOD(ExcelSheet, setGroupSummaryBelow);
ZEND_METHOD(ExcelSheet, getGroupSummaryRight);
ZEND_METHOD(ExcelSheet, setGroupSummaryRight);
ZEND_METHOD(ExcelSheet, setPrintFit);
ZEND_METHOD(ExcelSheet, getPrintFit);
ZEND_METHOD(ExcelSheet, getNamedRange);
ZEND_METHOD(ExcelSheet, getIndexRange);
ZEND_METHOD(ExcelSheet, namedRangeSize);
ZEND_METHOD(ExcelSheet, getVerPageBreak);
ZEND_METHOD(ExcelSheet, getVerPageBreakSize);
ZEND_METHOD(ExcelSheet, getHorPageBreak);
ZEND_METHOD(ExcelSheet, getHorPageBreakSize);
ZEND_METHOD(ExcelSheet, getPictureInfo);
ZEND_METHOD(ExcelSheet, getNumPictures);
ZEND_METHOD(ExcelSheet, getRightToLeft);
ZEND_METHOD(ExcelSheet, setRightToLeft);
ZEND_METHOD(ExcelSheet, setPrintArea);
ZEND_METHOD(ExcelSheet, clearPrintRepeats);
ZEND_METHOD(ExcelSheet, clearPrintArea);
ZEND_METHOD(ExcelSheet, protect);
ZEND_METHOD(ExcelSheet, hyperlinkSize);
ZEND_METHOD(ExcelSheet, hyperlink);
ZEND_METHOD(ExcelSheet, delHyperlink);
ZEND_METHOD(ExcelSheet, addHyperlink);
ZEND_METHOD(ExcelSheet, mergeSize);
ZEND_METHOD(ExcelSheet, merge);
ZEND_METHOD(ExcelSheet, delMergeByIndex);
ZEND_METHOD(ExcelSheet, splitInfo);
ZEND_METHOD(ExcelSheet, rowHidden);
ZEND_METHOD(ExcelSheet, setRowHidden);
ZEND_METHOD(ExcelSheet, colHidden);
ZEND_METHOD(ExcelSheet, setColHidden);
ZEND_METHOD(ExcelSheet, isLicensed);
ZEND_METHOD(ExcelSheet, setAutoFitArea);
ZEND_METHOD(ExcelSheet, printRepeatRows);
ZEND_METHOD(ExcelSheet, printRepeatCols);
ZEND_METHOD(ExcelSheet, printArea);
ZEND_METHOD(ExcelSheet, setProtect);
ZEND_METHOD(ExcelSheet, table);
ZEND_METHOD(ExcelSheet, setTabColor);
ZEND_METHOD(ExcelSheet, autoFilter);
ZEND_METHOD(ExcelSheet, applyFilter);
ZEND_METHOD(ExcelSheet, removeFilter);
ZEND_METHOD(ExcelSheet, addIgnoredError);
ZEND_METHOD(ExcelSheet, writeError);
ZEND_METHOD(ExcelSheet, removeComment);
ZEND_METHOD(ExcelSheet, addDataValidation);
ZEND_METHOD(ExcelSheet, addDataValidationDouble);
ZEND_METHOD(ExcelSheet, removeDataValidations);
#if LIBXL_VERSION >= 0x05020000
ZEND_METHOD(ExcelSheet, dataValidationSize);
ZEND_METHOD(ExcelSheet, dataValidation);
#endif
ZEND_METHOD(ExcelSheet, firstFilledRow);
ZEND_METHOD(ExcelSheet, lastFilledRow);
ZEND_METHOD(ExcelSheet, firstFilledCol);
ZEND_METHOD(ExcelSheet, lastFilledCol);
ZEND_METHOD(ExcelSheet, removePicture);
ZEND_METHOD(ExcelSheet, removePictureByIndex);
ZEND_METHOD(ExcelSheet, isRichStr);
ZEND_METHOD(ExcelSheet, readRichStr);
ZEND_METHOD(ExcelSheet, writeRichStr);
ZEND_METHOD(ExcelSheet, formControlSize);
ZEND_METHOD(ExcelSheet, formControl);
ZEND_METHOD(ExcelSheet, getActiveCell);
ZEND_METHOD(ExcelSheet, setActiveCell);
ZEND_METHOD(ExcelSheet, selectionRange);
ZEND_METHOD(ExcelSheet, addSelectionRange);
ZEND_METHOD(ExcelSheet, removeSelection);
ZEND_METHOD(ExcelSheet, tabColor);
ZEND_METHOD(ExcelSheet, getTabRgbColor);
ZEND_METHOD(ExcelSheet, setTabRgbColor);
ZEND_METHOD(ExcelSheet, hyperlinkIndex);
ZEND_METHOD(ExcelSheet, colWidthPx);
ZEND_METHOD(ExcelSheet, rowHeightPx);
ZEND_METHOD(ExcelSheet, colFormat);
ZEND_METHOD(ExcelSheet, rowFormat);
ZEND_METHOD(ExcelSheet, setColPx);
ZEND_METHOD(ExcelSheet, setRowPx);
ZEND_METHOD(ExcelSheet, setBorder);
ZEND_METHOD(ExcelSheet, addTable);
ZEND_METHOD(ExcelSheet, getTableByName);
ZEND_METHOD(ExcelSheet, getTableByIndex);
ZEND_METHOD(ExcelSheet, applyFilter2);
#if LIBXL_VERSION >= 0x05010000
ZEND_METHOD(ExcelSheet, addConditionalFormatting);
#endif
#if !(LIBXL_VERSION >= 0x05010000)
ZEND_METHOD(ExcelSheet, addConditionalFormatting);
#endif
#if LIBXL_VERSION >= 0x05010000
ZEND_METHOD(ExcelSheet, conditionalFormatting);
ZEND_METHOD(ExcelSheet, removeConditionalFormatting);
ZEND_METHOD(ExcelSheet, conditionalFormattingSize);
#endif
ZEND_METHOD(ExcelFormat, __construct);
ZEND_METHOD(ExcelFormat, setFont);
ZEND_METHOD(ExcelFormat, getFont);
ZEND_METHOD(ExcelFormat, numberFormat);
ZEND_METHOD(ExcelFormat, horizontalAlign);
ZEND_METHOD(ExcelFormat, verticalAlign);
ZEND_METHOD(ExcelFormat, wrap);
ZEND_METHOD(ExcelFormat, rotate);
ZEND_METHOD(ExcelFormat, indent);
ZEND_METHOD(ExcelFormat, shrinkToFit);
ZEND_METHOD(ExcelFormat, borderStyle);
ZEND_METHOD(ExcelFormat, borderColor);
ZEND_METHOD(ExcelFormat, borderLeftStyle);
ZEND_METHOD(ExcelFormat, borderLeftColor);
ZEND_METHOD(ExcelFormat, borderRightStyle);
ZEND_METHOD(ExcelFormat, borderRightColor);
ZEND_METHOD(ExcelFormat, borderTopStyle);
ZEND_METHOD(ExcelFormat, borderTopColor);
ZEND_METHOD(ExcelFormat, borderBottomStyle);
ZEND_METHOD(ExcelFormat, borderBottomColor);
ZEND_METHOD(ExcelFormat, borderDiagonalStyle);
ZEND_METHOD(ExcelFormat, borderDiagonalColor);
ZEND_METHOD(ExcelFormat, fillPattern);
ZEND_METHOD(ExcelFormat, patternForegroundColor);
ZEND_METHOD(ExcelFormat, patternBackgroundColor);
ZEND_METHOD(ExcelFormat, locked);
ZEND_METHOD(ExcelFormat, hidden);
ZEND_METHOD(ExcelFont, size);
ZEND_METHOD(ExcelFont, italics);
ZEND_METHOD(ExcelFont, strike);
ZEND_METHOD(ExcelFont, bold);
ZEND_METHOD(ExcelFont, color);
ZEND_METHOD(ExcelFont, mode);
ZEND_METHOD(ExcelFont, underline);
ZEND_METHOD(ExcelFont, name);
ZEND_METHOD(ExcelFont, __construct);
ZEND_METHOD(ExcelAutoFilter, __construct);
ZEND_METHOD(ExcelAutoFilter, getRef);
ZEND_METHOD(ExcelAutoFilter, setRef);
ZEND_METHOD(ExcelAutoFilter, column);
ZEND_METHOD(ExcelAutoFilter, columnSize);
ZEND_METHOD(ExcelAutoFilter, columnByIndex);
ZEND_METHOD(ExcelAutoFilter, getSortRange);
ZEND_METHOD(ExcelAutoFilter, getSort);
ZEND_METHOD(ExcelAutoFilter, setSort);
ZEND_METHOD(ExcelAutoFilter, addSort);
ZEND_METHOD(ExcelFilterColumn, __construct);
ZEND_METHOD(ExcelFilterColumn, index);
ZEND_METHOD(ExcelFilterColumn, filterType);
ZEND_METHOD(ExcelFilterColumn, filterSize);
ZEND_METHOD(ExcelFilterColumn, filter);
ZEND_METHOD(ExcelFilterColumn, addFilter);
ZEND_METHOD(ExcelFilterColumn, getTop10);
ZEND_METHOD(ExcelFilterColumn, setTop10);
ZEND_METHOD(ExcelFilterColumn, getCustomFilter);
ZEND_METHOD(ExcelFilterColumn, setCustomFilter);
ZEND_METHOD(ExcelFilterColumn, clear);
ZEND_METHOD(ExcelRichString, __construct);
ZEND_METHOD(ExcelRichString, addFont);
ZEND_METHOD(ExcelRichString, addText);
ZEND_METHOD(ExcelRichString, getText);
ZEND_METHOD(ExcelRichString, textSize);
ZEND_METHOD(ExcelFormControl, __construct);
ZEND_METHOD(ExcelFormControl, objectType);
ZEND_METHOD(ExcelFormControl, checked);
ZEND_METHOD(ExcelFormControl, setChecked);
ZEND_METHOD(ExcelFormControl, fmlaGroup);
ZEND_METHOD(ExcelFormControl, setFmlaGroup);
ZEND_METHOD(ExcelFormControl, fmlaLink);
ZEND_METHOD(ExcelFormControl, setFmlaLink);
ZEND_METHOD(ExcelFormControl, fmlaRange);
ZEND_METHOD(ExcelFormControl, setFmlaRange);
ZEND_METHOD(ExcelFormControl, fmlaTxbx);
ZEND_METHOD(ExcelFormControl, setFmlaTxbx);
ZEND_METHOD(ExcelFormControl, name);
ZEND_METHOD(ExcelFormControl, linkedCell);
ZEND_METHOD(ExcelFormControl, listFillRange);
ZEND_METHOD(ExcelFormControl, macro);
ZEND_METHOD(ExcelFormControl, altText);
ZEND_METHOD(ExcelFormControl, locked);
ZEND_METHOD(ExcelFormControl, defaultSize);
ZEND_METHOD(ExcelFormControl, print);
ZEND_METHOD(ExcelFormControl, disabled);
ZEND_METHOD(ExcelFormControl, item);
ZEND_METHOD(ExcelFormControl, itemSize);
ZEND_METHOD(ExcelFormControl, addItem);
ZEND_METHOD(ExcelFormControl, insertItem);
ZEND_METHOD(ExcelFormControl, clearItems);
ZEND_METHOD(ExcelFormControl, dropLines);
ZEND_METHOD(ExcelFormControl, setDropLines);
ZEND_METHOD(ExcelFormControl, dx);
ZEND_METHOD(ExcelFormControl, setDx);
ZEND_METHOD(ExcelFormControl, firstButton);
ZEND_METHOD(ExcelFormControl, setFirstButton);
ZEND_METHOD(ExcelFormControl, horiz);
ZEND_METHOD(ExcelFormControl, setHoriz);
ZEND_METHOD(ExcelFormControl, inc);
ZEND_METHOD(ExcelFormControl, setInc);
ZEND_METHOD(ExcelFormControl, getMax);
ZEND_METHOD(ExcelFormControl, setMax);
ZEND_METHOD(ExcelFormControl, getMin);
ZEND_METHOD(ExcelFormControl, setMin);
ZEND_METHOD(ExcelFormControl, multiSel);
ZEND_METHOD(ExcelFormControl, setMultiSel);
ZEND_METHOD(ExcelFormControl, sel);
ZEND_METHOD(ExcelFormControl, setSel);
ZEND_METHOD(ExcelFormControl, fromAnchor);
ZEND_METHOD(ExcelFormControl, toAnchor);
ZEND_METHOD(ExcelConditionalFormat, __construct);
ZEND_METHOD(ExcelConditionalFormat, font);
ZEND_METHOD(ExcelConditionalFormat, numFormat);
ZEND_METHOD(ExcelConditionalFormat, setNumFormat);
ZEND_METHOD(ExcelConditionalFormat, customNumFormat);
ZEND_METHOD(ExcelConditionalFormat, setCustomNumFormat);
ZEND_METHOD(ExcelConditionalFormat, setBorder);
ZEND_METHOD(ExcelConditionalFormat, setBorderColor);
ZEND_METHOD(ExcelConditionalFormat, borderLeft);
ZEND_METHOD(ExcelConditionalFormat, setBorderLeft);
ZEND_METHOD(ExcelConditionalFormat, borderRight);
ZEND_METHOD(ExcelConditionalFormat, setBorderRight);
ZEND_METHOD(ExcelConditionalFormat, borderTop);
ZEND_METHOD(ExcelConditionalFormat, setBorderTop);
ZEND_METHOD(ExcelConditionalFormat, borderBottom);
ZEND_METHOD(ExcelConditionalFormat, setBorderBottom);
ZEND_METHOD(ExcelConditionalFormat, borderLeftColor);
ZEND_METHOD(ExcelConditionalFormat, setBorderLeftColor);
ZEND_METHOD(ExcelConditionalFormat, borderRightColor);
ZEND_METHOD(ExcelConditionalFormat, setBorderRightColor);
ZEND_METHOD(ExcelConditionalFormat, borderTopColor);
ZEND_METHOD(ExcelConditionalFormat, setBorderTopColor);
ZEND_METHOD(ExcelConditionalFormat, borderBottomColor);
ZEND_METHOD(ExcelConditionalFormat, setBorderBottomColor);
ZEND_METHOD(ExcelConditionalFormat, fillPattern);
ZEND_METHOD(ExcelConditionalFormat, setFillPattern);
ZEND_METHOD(ExcelConditionalFormat, patternForegroundColor);
ZEND_METHOD(ExcelConditionalFormat, setPatternForegroundColor);
ZEND_METHOD(ExcelConditionalFormat, patternBackgroundColor);
ZEND_METHOD(ExcelConditionalFormat, setPatternBackgroundColor);
#if LIBXL_VERSION >= 0x05010000
ZEND_METHOD(ExcelConditionalFormatting, __construct);
#endif
#if !(LIBXL_VERSION >= 0x05010000)
ZEND_METHOD(ExcelConditionalFormatting, __construct);
#endif
ZEND_METHOD(ExcelConditionalFormatting, addRange);
ZEND_METHOD(ExcelConditionalFormatting, addRule);
ZEND_METHOD(ExcelConditionalFormatting, addTopRule);
ZEND_METHOD(ExcelConditionalFormatting, addOpNumRule);
ZEND_METHOD(ExcelConditionalFormatting, addOpStrRule);
ZEND_METHOD(ExcelConditionalFormatting, addAboveAverageRule);
ZEND_METHOD(ExcelConditionalFormatting, addTimePeriodRule);
ZEND_METHOD(ExcelConditionalFormatting, add2ColorScaleRule);
ZEND_METHOD(ExcelConditionalFormatting, add2ColorScaleFormulaRule);
ZEND_METHOD(ExcelConditionalFormatting, add3ColorScaleRule);
ZEND_METHOD(ExcelConditionalFormatting, add3ColorScaleFormulaRule);
ZEND_METHOD(ExcelCoreProperties, __construct);
ZEND_METHOD(ExcelCoreProperties, title);
ZEND_METHOD(ExcelCoreProperties, setTitle);
ZEND_METHOD(ExcelCoreProperties, subject);
ZEND_METHOD(ExcelCoreProperties, setSubject);
ZEND_METHOD(ExcelCoreProperties, creator);
ZEND_METHOD(ExcelCoreProperties, setCreator);
ZEND_METHOD(ExcelCoreProperties, lastModifiedBy);
ZEND_METHOD(ExcelCoreProperties, setLastModifiedBy);
ZEND_METHOD(ExcelCoreProperties, created);
ZEND_METHOD(ExcelCoreProperties, setCreated);
ZEND_METHOD(ExcelCoreProperties, modified);
ZEND_METHOD(ExcelCoreProperties, setModified);
ZEND_METHOD(ExcelCoreProperties, tags);
ZEND_METHOD(ExcelCoreProperties, setTags);
ZEND_METHOD(ExcelCoreProperties, categories);
ZEND_METHOD(ExcelCoreProperties, setCategories);
ZEND_METHOD(ExcelCoreProperties, comments);
ZEND_METHOD(ExcelCoreProperties, setComments);
ZEND_METHOD(ExcelCoreProperties, createdAsDouble);
ZEND_METHOD(ExcelCoreProperties, setCreatedAsDouble);
ZEND_METHOD(ExcelCoreProperties, modifiedAsDouble);
ZEND_METHOD(ExcelCoreProperties, setModifiedAsDouble);
ZEND_METHOD(ExcelCoreProperties, removeAll);
ZEND_METHOD(ExcelTable, __construct);
ZEND_METHOD(ExcelTable, name);
ZEND_METHOD(ExcelTable, setName);
ZEND_METHOD(ExcelTable, ref);
ZEND_METHOD(ExcelTable, setRef);
ZEND_METHOD(ExcelTable, autoFilter);
#if LIBXL_VERSION >= 0x05020000
ZEND_METHOD(ExcelTable, isAutoFilter);
ZEND_METHOD(ExcelTable, removeFilter);
#endif
ZEND_METHOD(ExcelTable, style);
ZEND_METHOD(ExcelTable, setStyle);
ZEND_METHOD(ExcelTable, showRowStripes);
ZEND_METHOD(ExcelTable, setShowRowStripes);
ZEND_METHOD(ExcelTable, showColumnStripes);
ZEND_METHOD(ExcelTable, setShowColumnStripes);
ZEND_METHOD(ExcelTable, showFirstColumn);
ZEND_METHOD(ExcelTable, setShowFirstColumn);
ZEND_METHOD(ExcelTable, showLastColumn);
ZEND_METHOD(ExcelTable, setShowLastColumn);
ZEND_METHOD(ExcelTable, columnSize);
ZEND_METHOD(ExcelTable, columnName);
ZEND_METHOD(ExcelTable, setColumnName);

static const zend_function_entry class_ExcelBook_methods[] = {
	ZEND_ME(ExcelBook, requiresKey, arginfo_class_ExcelBook_requiresKey, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	ZEND_ME(ExcelBook, load, arginfo_class_ExcelBook_load, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelBook, loadFile, arginfo_class_ExcelBook_loadFile, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelBook, save, arginfo_class_ExcelBook_save, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelBook, getSheet, arginfo_class_ExcelBook_getSheet, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelBook, getSheetByName, arginfo_class_ExcelBook_getSheetByName, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelBook, deleteSheet, arginfo_class_ExcelBook_deleteSheet, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelBook, activeSheet, arginfo_class_ExcelBook_activeSheet, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelBook, addSheet, arginfo_class_ExcelBook_addSheet, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelBook, copySheet, arginfo_class_ExcelBook_copySheet, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelBook, sheetCount, arginfo_class_ExcelBook_sheetCount, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelBook, getError, arginfo_class_ExcelBook_getError, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelBook, addFont, arginfo_class_ExcelBook_addFont, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelBook, addFormat, arginfo_class_ExcelBook_addFormat, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelBook, getAllFormats, arginfo_class_ExcelBook_getAllFormats, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelBook, addCustomFormat, arginfo_class_ExcelBook_addCustomFormat, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelBook, getCustomFormat, arginfo_class_ExcelBook_getCustomFormat, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelBook, packDate, arginfo_class_ExcelBook_packDate, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelBook, packDateValues, arginfo_class_ExcelBook_packDateValues, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelBook, unpackDate, arginfo_class_ExcelBook_unpackDate, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelBook, isDate1904, arginfo_class_ExcelBook_isDate1904, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelBook, setDate1904, arginfo_class_ExcelBook_setDate1904, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelBook, getActiveSheet, arginfo_class_ExcelBook_getActiveSheet, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelBook, getDefaultFont, arginfo_class_ExcelBook_getDefaultFont, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelBook, setDefaultFont, arginfo_class_ExcelBook_setDefaultFont, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelBook, setLocale, arginfo_class_ExcelBook_setLocale, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelBook, __construct, arginfo_class_ExcelBook___construct, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelBook, setActiveSheet, arginfo_class_ExcelBook_setActiveSheet, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelBook, addPictureFromFile, arginfo_class_ExcelBook_addPictureFromFile, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelBook, addPictureFromString, arginfo_class_ExcelBook_addPictureFromString, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelBook, rgbMode, arginfo_class_ExcelBook_rgbMode, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelBook, setRGBMode, arginfo_class_ExcelBook_setRGBMode, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelBook, colorPack, arginfo_class_ExcelBook_colorPack, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelBook, colorUnpack, arginfo_class_ExcelBook_colorUnpack, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelBook, getLibXlVersion, arginfo_class_ExcelBook_getLibXlVersion, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelBook, getPhpExcelVersion, arginfo_class_ExcelBook_getPhpExcelVersion, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelBook, loadInfo, arginfo_class_ExcelBook_loadInfo, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelBook, getSheetName, arginfo_class_ExcelBook_getSheetName, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelBook, addRichString, arginfo_class_ExcelBook_addRichString, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelBook, calcMode, arginfo_class_ExcelBook_calcMode, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelBook, setCalcMode, arginfo_class_ExcelBook_setCalcMode, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelBook, addConditionalFormat, arginfo_class_ExcelBook_addConditionalFormat, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelBook, addFormatFromStyle, arginfo_class_ExcelBook_addFormatFromStyle, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelBook, removeVBA, arginfo_class_ExcelBook_removeVBA, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelBook, removePrinterSettings, arginfo_class_ExcelBook_removePrinterSettings, ZEND_ACC_PUBLIC)
#if LIBXL_VERSION >= 0x05000000
	ZEND_ME(ExcelBook, setPassword, arginfo_class_ExcelBook_setPassword, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelBook, dpiAwareness, arginfo_class_ExcelBook_dpiAwareness, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelBook, setDpiAwareness, arginfo_class_ExcelBook_setDpiAwareness, ZEND_ACC_PUBLIC)
#endif
#if LIBXL_VERSION >= 0x05000100
	ZEND_ME(ExcelBook, loadInfoRaw, arginfo_class_ExcelBook_loadInfoRaw, ZEND_ACC_PUBLIC)
#endif
#if LIBXL_VERSION >= 0x05010000
	ZEND_ME(ExcelBook, errorCode, arginfo_class_ExcelBook_errorCode, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelBook, conditionalFormat, arginfo_class_ExcelBook_conditionalFormat, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelBook, conditionalFormatSize, arginfo_class_ExcelBook_conditionalFormatSize, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelBook, clear, arginfo_class_ExcelBook_clear, ZEND_ACC_PUBLIC)
#endif
	ZEND_ME(ExcelBook, coreProperties, arginfo_class_ExcelBook_coreProperties, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelBook, removeAllPhonetics, arginfo_class_ExcelBook_removeAllPhonetics, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelBook, biffVersion, arginfo_class_ExcelBook_biffVersion, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelBook, getRefR1C1, arginfo_class_ExcelBook_getRefR1C1, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelBook, setRefR1C1, arginfo_class_ExcelBook_setRefR1C1, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelBook, getPicture, arginfo_class_ExcelBook_getPicture, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelBook, getNumPictures, arginfo_class_ExcelBook_getNumPictures, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelBook, insertSheet, arginfo_class_ExcelBook_insertSheet, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelBook, isTemplate, arginfo_class_ExcelBook_isTemplate, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelBook, setTemplate, arginfo_class_ExcelBook_setTemplate, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelBook, sheetType, arginfo_class_ExcelBook_sheetType, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelBook, addPictureAsLink, arginfo_class_ExcelBook_addPictureAsLink, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelBook, moveSheet, arginfo_class_ExcelBook_moveSheet, ZEND_ACC_PUBLIC)
	ZEND_FE_END
};

static const zend_function_entry class_ExcelSheet_methods[] = {
	ZEND_ME(ExcelSheet, __construct, arginfo_class_ExcelSheet___construct, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelSheet, cellType, arginfo_class_ExcelSheet_cellType, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelSheet, cellFormat, arginfo_class_ExcelSheet_cellFormat, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelSheet, setCellFormat, arginfo_class_ExcelSheet_setCellFormat, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelSheet, readRow, arginfo_class_ExcelSheet_readRow, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelSheet, readCol, arginfo_class_ExcelSheet_readCol, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelSheet, read, arginfo_class_ExcelSheet_read, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelSheet, write, arginfo_class_ExcelSheet_write, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelSheet, writeRow, arginfo_class_ExcelSheet_writeRow, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelSheet, writeCol, arginfo_class_ExcelSheet_writeCol, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelSheet, isFormula, arginfo_class_ExcelSheet_isFormula, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelSheet, isDate, arginfo_class_ExcelSheet_isDate, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelSheet, insertRow, arginfo_class_ExcelSheet_insertRow, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelSheet, insertCol, arginfo_class_ExcelSheet_insertCol, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelSheet, removeRow, arginfo_class_ExcelSheet_removeRow, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelSheet, removeCol, arginfo_class_ExcelSheet_removeCol, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelSheet, colWidth, arginfo_class_ExcelSheet_colWidth, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelSheet, rowHeight, arginfo_class_ExcelSheet_rowHeight, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelSheet, readComment, arginfo_class_ExcelSheet_readComment, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelSheet, writeComment, arginfo_class_ExcelSheet_writeComment, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelSheet, setColWidth, arginfo_class_ExcelSheet_setColWidth, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelSheet, setRowHeight, arginfo_class_ExcelSheet_setRowHeight, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelSheet, getMerge, arginfo_class_ExcelSheet_getMerge, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelSheet, setMerge, arginfo_class_ExcelSheet_setMerge, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelSheet, deleteMerge, arginfo_class_ExcelSheet_deleteMerge, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelSheet, addPictureScaled, arginfo_class_ExcelSheet_addPictureScaled, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelSheet, addPictureDim, arginfo_class_ExcelSheet_addPictureDim, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelSheet, horPageBreak, arginfo_class_ExcelSheet_horPageBreak, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelSheet, verPageBreak, arginfo_class_ExcelSheet_verPageBreak, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelSheet, splitSheet, arginfo_class_ExcelSheet_splitSheet, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelSheet, groupRows, arginfo_class_ExcelSheet_groupRows, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelSheet, groupCols, arginfo_class_ExcelSheet_groupCols, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelSheet, clear, arginfo_class_ExcelSheet_clear, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelSheet, copy, arginfo_class_ExcelSheet_copy, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelSheet, firstRow, arginfo_class_ExcelSheet_firstRow, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelSheet, lastRow, arginfo_class_ExcelSheet_lastRow, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelSheet, firstCol, arginfo_class_ExcelSheet_firstCol, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelSheet, lastCol, arginfo_class_ExcelSheet_lastCol, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelSheet, displayGridlines, arginfo_class_ExcelSheet_displayGridlines, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelSheet, printGridlines, arginfo_class_ExcelSheet_printGridlines, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelSheet, setDisplayGridlines, arginfo_class_ExcelSheet_setDisplayGridlines, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelSheet, setHidden, arginfo_class_ExcelSheet_setHidden, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelSheet, isHidden, arginfo_class_ExcelSheet_isHidden, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelSheet, getTopLeftView, arginfo_class_ExcelSheet_getTopLeftView, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelSheet, setTopLeftView, arginfo_class_ExcelSheet_setTopLeftView, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelSheet, rowColToAddr, arginfo_class_ExcelSheet_rowColToAddr, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelSheet, addrToRowCol, arginfo_class_ExcelSheet_addrToRowCol, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelSheet, setPrintGridlines, arginfo_class_ExcelSheet_setPrintGridlines, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelSheet, zoom, arginfo_class_ExcelSheet_zoom, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelSheet, zoomPrint, arginfo_class_ExcelSheet_zoomPrint, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelSheet, setZoom, arginfo_class_ExcelSheet_setZoom, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelSheet, setZoomPrint, arginfo_class_ExcelSheet_setZoomPrint, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelSheet, setLandscape, arginfo_class_ExcelSheet_setLandscape, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelSheet, landscape, arginfo_class_ExcelSheet_landscape, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelSheet, paper, arginfo_class_ExcelSheet_paper, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelSheet, setPaper, arginfo_class_ExcelSheet_setPaper, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelSheet, header, arginfo_class_ExcelSheet_header, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelSheet, footer, arginfo_class_ExcelSheet_footer, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelSheet, setHeader, arginfo_class_ExcelSheet_setHeader, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelSheet, setFooter, arginfo_class_ExcelSheet_setFooter, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelSheet, headerMargin, arginfo_class_ExcelSheet_headerMargin, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelSheet, footerMargin, arginfo_class_ExcelSheet_footerMargin, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelSheet, hcenter, arginfo_class_ExcelSheet_hcenter, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelSheet, vcenter, arginfo_class_ExcelSheet_vcenter, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelSheet, setHCenter, arginfo_class_ExcelSheet_setHCenter, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelSheet, setVCenter, arginfo_class_ExcelSheet_setVCenter, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelSheet, marginLeft, arginfo_class_ExcelSheet_marginLeft, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelSheet, marginRight, arginfo_class_ExcelSheet_marginRight, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelSheet, marginTop, arginfo_class_ExcelSheet_marginTop, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelSheet, marginBottom, arginfo_class_ExcelSheet_marginBottom, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelSheet, setMarginLeft, arginfo_class_ExcelSheet_setMarginLeft, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelSheet, setMarginRight, arginfo_class_ExcelSheet_setMarginRight, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelSheet, setMarginTop, arginfo_class_ExcelSheet_setMarginTop, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelSheet, setMarginBottom, arginfo_class_ExcelSheet_setMarginBottom, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelSheet, printHeaders, arginfo_class_ExcelSheet_printHeaders, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelSheet, setPrintHeaders, arginfo_class_ExcelSheet_setPrintHeaders, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelSheet, name, arginfo_class_ExcelSheet_name, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelSheet, setName, arginfo_class_ExcelSheet_setName, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelSheet, setNamedRange, arginfo_class_ExcelSheet_setNamedRange, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelSheet, delNamedRange, arginfo_class_ExcelSheet_delNamedRange, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelSheet, setPrintRepeatRows, arginfo_class_ExcelSheet_setPrintRepeatRows, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelSheet, setPrintRepeatCols, arginfo_class_ExcelSheet_setPrintRepeatCols, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelSheet, getGroupSummaryBelow, arginfo_class_ExcelSheet_getGroupSummaryBelow, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelSheet, setGroupSummaryBelow, arginfo_class_ExcelSheet_setGroupSummaryBelow, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelSheet, getGroupSummaryRight, arginfo_class_ExcelSheet_getGroupSummaryRight, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelSheet, setGroupSummaryRight, arginfo_class_ExcelSheet_setGroupSummaryRight, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelSheet, setPrintFit, arginfo_class_ExcelSheet_setPrintFit, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelSheet, getPrintFit, arginfo_class_ExcelSheet_getPrintFit, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelSheet, getNamedRange, arginfo_class_ExcelSheet_getNamedRange, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelSheet, getIndexRange, arginfo_class_ExcelSheet_getIndexRange, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelSheet, namedRangeSize, arginfo_class_ExcelSheet_namedRangeSize, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelSheet, getVerPageBreak, arginfo_class_ExcelSheet_getVerPageBreak, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelSheet, getVerPageBreakSize, arginfo_class_ExcelSheet_getVerPageBreakSize, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelSheet, getHorPageBreak, arginfo_class_ExcelSheet_getHorPageBreak, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelSheet, getHorPageBreakSize, arginfo_class_ExcelSheet_getHorPageBreakSize, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelSheet, getPictureInfo, arginfo_class_ExcelSheet_getPictureInfo, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelSheet, getNumPictures, arginfo_class_ExcelSheet_getNumPictures, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelSheet, getRightToLeft, arginfo_class_ExcelSheet_getRightToLeft, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelSheet, setRightToLeft, arginfo_class_ExcelSheet_setRightToLeft, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelSheet, setPrintArea, arginfo_class_ExcelSheet_setPrintArea, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelSheet, clearPrintRepeats, arginfo_class_ExcelSheet_clearPrintRepeats, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelSheet, clearPrintArea, arginfo_class_ExcelSheet_clearPrintArea, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelSheet, protect, arginfo_class_ExcelSheet_protect, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelSheet, hyperlinkSize, arginfo_class_ExcelSheet_hyperlinkSize, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelSheet, hyperlink, arginfo_class_ExcelSheet_hyperlink, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelSheet, delHyperlink, arginfo_class_ExcelSheet_delHyperlink, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelSheet, addHyperlink, arginfo_class_ExcelSheet_addHyperlink, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelSheet, mergeSize, arginfo_class_ExcelSheet_mergeSize, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelSheet, merge, arginfo_class_ExcelSheet_merge, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelSheet, delMergeByIndex, arginfo_class_ExcelSheet_delMergeByIndex, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelSheet, splitInfo, arginfo_class_ExcelSheet_splitInfo, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelSheet, rowHidden, arginfo_class_ExcelSheet_rowHidden, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelSheet, setRowHidden, arginfo_class_ExcelSheet_setRowHidden, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelSheet, colHidden, arginfo_class_ExcelSheet_colHidden, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelSheet, setColHidden, arginfo_class_ExcelSheet_setColHidden, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelSheet, isLicensed, arginfo_class_ExcelSheet_isLicensed, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelSheet, setAutoFitArea, arginfo_class_ExcelSheet_setAutoFitArea, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelSheet, printRepeatRows, arginfo_class_ExcelSheet_printRepeatRows, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelSheet, printRepeatCols, arginfo_class_ExcelSheet_printRepeatCols, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelSheet, printArea, arginfo_class_ExcelSheet_printArea, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelSheet, setProtect, arginfo_class_ExcelSheet_setProtect, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelSheet, table, arginfo_class_ExcelSheet_table, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelSheet, setTabColor, arginfo_class_ExcelSheet_setTabColor, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelSheet, autoFilter, arginfo_class_ExcelSheet_autoFilter, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelSheet, applyFilter, arginfo_class_ExcelSheet_applyFilter, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelSheet, removeFilter, arginfo_class_ExcelSheet_removeFilter, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelSheet, addIgnoredError, arginfo_class_ExcelSheet_addIgnoredError, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelSheet, writeError, arginfo_class_ExcelSheet_writeError, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelSheet, removeComment, arginfo_class_ExcelSheet_removeComment, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelSheet, addDataValidation, arginfo_class_ExcelSheet_addDataValidation, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelSheet, addDataValidationDouble, arginfo_class_ExcelSheet_addDataValidationDouble, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelSheet, removeDataValidations, arginfo_class_ExcelSheet_removeDataValidations, ZEND_ACC_PUBLIC)
#if LIBXL_VERSION >= 0x05020000
	ZEND_ME(ExcelSheet, dataValidationSize, arginfo_class_ExcelSheet_dataValidationSize, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelSheet, dataValidation, arginfo_class_ExcelSheet_dataValidation, ZEND_ACC_PUBLIC)
#endif
	ZEND_ME(ExcelSheet, firstFilledRow, arginfo_class_ExcelSheet_firstFilledRow, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelSheet, lastFilledRow, arginfo_class_ExcelSheet_lastFilledRow, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelSheet, firstFilledCol, arginfo_class_ExcelSheet_firstFilledCol, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelSheet, lastFilledCol, arginfo_class_ExcelSheet_lastFilledCol, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelSheet, removePicture, arginfo_class_ExcelSheet_removePicture, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelSheet, removePictureByIndex, arginfo_class_ExcelSheet_removePictureByIndex, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelSheet, isRichStr, arginfo_class_ExcelSheet_isRichStr, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelSheet, readRichStr, arginfo_class_ExcelSheet_readRichStr, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelSheet, writeRichStr, arginfo_class_ExcelSheet_writeRichStr, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelSheet, formControlSize, arginfo_class_ExcelSheet_formControlSize, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelSheet, formControl, arginfo_class_ExcelSheet_formControl, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelSheet, getActiveCell, arginfo_class_ExcelSheet_getActiveCell, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelSheet, setActiveCell, arginfo_class_ExcelSheet_setActiveCell, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelSheet, selectionRange, arginfo_class_ExcelSheet_selectionRange, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelSheet, addSelectionRange, arginfo_class_ExcelSheet_addSelectionRange, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelSheet, removeSelection, arginfo_class_ExcelSheet_removeSelection, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelSheet, tabColor, arginfo_class_ExcelSheet_tabColor, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelSheet, getTabRgbColor, arginfo_class_ExcelSheet_getTabRgbColor, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelSheet, setTabRgbColor, arginfo_class_ExcelSheet_setTabRgbColor, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelSheet, hyperlinkIndex, arginfo_class_ExcelSheet_hyperlinkIndex, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelSheet, colWidthPx, arginfo_class_ExcelSheet_colWidthPx, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelSheet, rowHeightPx, arginfo_class_ExcelSheet_rowHeightPx, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelSheet, colFormat, arginfo_class_ExcelSheet_colFormat, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelSheet, rowFormat, arginfo_class_ExcelSheet_rowFormat, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelSheet, setColPx, arginfo_class_ExcelSheet_setColPx, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelSheet, setRowPx, arginfo_class_ExcelSheet_setRowPx, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelSheet, setBorder, arginfo_class_ExcelSheet_setBorder, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelSheet, addTable, arginfo_class_ExcelSheet_addTable, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelSheet, getTableByName, arginfo_class_ExcelSheet_getTableByName, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelSheet, getTableByIndex, arginfo_class_ExcelSheet_getTableByIndex, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelSheet, applyFilter2, arginfo_class_ExcelSheet_applyFilter2, ZEND_ACC_PUBLIC)
#if LIBXL_VERSION >= 0x05010000
	ZEND_ME(ExcelSheet, addConditionalFormatting, arginfo_class_ExcelSheet_addConditionalFormatting, ZEND_ACC_PUBLIC)
#endif
#if !(LIBXL_VERSION >= 0x05010000)
	ZEND_ME(ExcelSheet, addConditionalFormatting, arginfo_class_ExcelSheet_addConditionalFormatting, ZEND_ACC_PUBLIC)
#endif
#if LIBXL_VERSION >= 0x05010000
	ZEND_ME(ExcelSheet, conditionalFormatting, arginfo_class_ExcelSheet_conditionalFormatting, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelSheet, removeConditionalFormatting, arginfo_class_ExcelSheet_removeConditionalFormatting, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelSheet, conditionalFormattingSize, arginfo_class_ExcelSheet_conditionalFormattingSize, ZEND_ACC_PUBLIC)
#endif
	ZEND_FE_END
};

static const zend_function_entry class_ExcelFormat_methods[] = {
	ZEND_ME(ExcelFormat, __construct, arginfo_class_ExcelFormat___construct, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelFormat, setFont, arginfo_class_ExcelFormat_setFont, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelFormat, getFont, arginfo_class_ExcelFormat_getFont, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelFormat, numberFormat, arginfo_class_ExcelFormat_numberFormat, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelFormat, horizontalAlign, arginfo_class_ExcelFormat_horizontalAlign, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelFormat, verticalAlign, arginfo_class_ExcelFormat_verticalAlign, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelFormat, wrap, arginfo_class_ExcelFormat_wrap, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelFormat, rotate, arginfo_class_ExcelFormat_rotate, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelFormat, indent, arginfo_class_ExcelFormat_indent, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelFormat, shrinkToFit, arginfo_class_ExcelFormat_shrinkToFit, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelFormat, borderStyle, arginfo_class_ExcelFormat_borderStyle, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelFormat, borderColor, arginfo_class_ExcelFormat_borderColor, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelFormat, borderLeftStyle, arginfo_class_ExcelFormat_borderLeftStyle, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelFormat, borderLeftColor, arginfo_class_ExcelFormat_borderLeftColor, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelFormat, borderRightStyle, arginfo_class_ExcelFormat_borderRightStyle, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelFormat, borderRightColor, arginfo_class_ExcelFormat_borderRightColor, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelFormat, borderTopStyle, arginfo_class_ExcelFormat_borderTopStyle, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelFormat, borderTopColor, arginfo_class_ExcelFormat_borderTopColor, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelFormat, borderBottomStyle, arginfo_class_ExcelFormat_borderBottomStyle, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelFormat, borderBottomColor, arginfo_class_ExcelFormat_borderBottomColor, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelFormat, borderDiagonalStyle, arginfo_class_ExcelFormat_borderDiagonalStyle, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelFormat, borderDiagonalColor, arginfo_class_ExcelFormat_borderDiagonalColor, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelFormat, fillPattern, arginfo_class_ExcelFormat_fillPattern, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelFormat, patternForegroundColor, arginfo_class_ExcelFormat_patternForegroundColor, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelFormat, patternBackgroundColor, arginfo_class_ExcelFormat_patternBackgroundColor, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelFormat, locked, arginfo_class_ExcelFormat_locked, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelFormat, hidden, arginfo_class_ExcelFormat_hidden, ZEND_ACC_PUBLIC)
	ZEND_FE_END
};

static const zend_function_entry class_ExcelFont_methods[] = {
	ZEND_ME(ExcelFont, size, arginfo_class_ExcelFont_size, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelFont, italics, arginfo_class_ExcelFont_italics, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelFont, strike, arginfo_class_ExcelFont_strike, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelFont, bold, arginfo_class_ExcelFont_bold, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelFont, color, arginfo_class_ExcelFont_color, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelFont, mode, arginfo_class_ExcelFont_mode, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelFont, underline, arginfo_class_ExcelFont_underline, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelFont, name, arginfo_class_ExcelFont_name, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelFont, __construct, arginfo_class_ExcelFont___construct, ZEND_ACC_PUBLIC|ZEND_ACC_FINAL)
	ZEND_FE_END
};

static const zend_function_entry class_ExcelAutoFilter_methods[] = {
	ZEND_ME(ExcelAutoFilter, __construct, arginfo_class_ExcelAutoFilter___construct, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelAutoFilter, getRef, arginfo_class_ExcelAutoFilter_getRef, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelAutoFilter, setRef, arginfo_class_ExcelAutoFilter_setRef, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelAutoFilter, column, arginfo_class_ExcelAutoFilter_column, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelAutoFilter, columnSize, arginfo_class_ExcelAutoFilter_columnSize, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelAutoFilter, columnByIndex, arginfo_class_ExcelAutoFilter_columnByIndex, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelAutoFilter, getSortRange, arginfo_class_ExcelAutoFilter_getSortRange, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelAutoFilter, getSort, arginfo_class_ExcelAutoFilter_getSort, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelAutoFilter, setSort, arginfo_class_ExcelAutoFilter_setSort, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelAutoFilter, addSort, arginfo_class_ExcelAutoFilter_addSort, ZEND_ACC_PUBLIC)
	ZEND_FE_END
};

static const zend_function_entry class_ExcelFilterColumn_methods[] = {
	ZEND_ME(ExcelFilterColumn, __construct, arginfo_class_ExcelFilterColumn___construct, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelFilterColumn, index, arginfo_class_ExcelFilterColumn_index, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelFilterColumn, filterType, arginfo_class_ExcelFilterColumn_filterType, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelFilterColumn, filterSize, arginfo_class_ExcelFilterColumn_filterSize, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelFilterColumn, filter, arginfo_class_ExcelFilterColumn_filter, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelFilterColumn, addFilter, arginfo_class_ExcelFilterColumn_addFilter, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelFilterColumn, getTop10, arginfo_class_ExcelFilterColumn_getTop10, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelFilterColumn, setTop10, arginfo_class_ExcelFilterColumn_setTop10, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelFilterColumn, getCustomFilter, arginfo_class_ExcelFilterColumn_getCustomFilter, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelFilterColumn, setCustomFilter, arginfo_class_ExcelFilterColumn_setCustomFilter, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelFilterColumn, clear, arginfo_class_ExcelFilterColumn_clear, ZEND_ACC_PUBLIC)
	ZEND_FE_END
};

static const zend_function_entry class_ExcelRichString_methods[] = {
	ZEND_ME(ExcelRichString, __construct, arginfo_class_ExcelRichString___construct, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelRichString, addFont, arginfo_class_ExcelRichString_addFont, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelRichString, addText, arginfo_class_ExcelRichString_addText, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelRichString, getText, arginfo_class_ExcelRichString_getText, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelRichString, textSize, arginfo_class_ExcelRichString_textSize, ZEND_ACC_PUBLIC)
	ZEND_FE_END
};

static const zend_function_entry class_ExcelFormControl_methods[] = {
	ZEND_ME(ExcelFormControl, __construct, arginfo_class_ExcelFormControl___construct, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelFormControl, objectType, arginfo_class_ExcelFormControl_objectType, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelFormControl, checked, arginfo_class_ExcelFormControl_checked, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelFormControl, setChecked, arginfo_class_ExcelFormControl_setChecked, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelFormControl, fmlaGroup, arginfo_class_ExcelFormControl_fmlaGroup, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelFormControl, setFmlaGroup, arginfo_class_ExcelFormControl_setFmlaGroup, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelFormControl, fmlaLink, arginfo_class_ExcelFormControl_fmlaLink, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelFormControl, setFmlaLink, arginfo_class_ExcelFormControl_setFmlaLink, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelFormControl, fmlaRange, arginfo_class_ExcelFormControl_fmlaRange, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelFormControl, setFmlaRange, arginfo_class_ExcelFormControl_setFmlaRange, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelFormControl, fmlaTxbx, arginfo_class_ExcelFormControl_fmlaTxbx, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelFormControl, setFmlaTxbx, arginfo_class_ExcelFormControl_setFmlaTxbx, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelFormControl, name, arginfo_class_ExcelFormControl_name, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelFormControl, linkedCell, arginfo_class_ExcelFormControl_linkedCell, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelFormControl, listFillRange, arginfo_class_ExcelFormControl_listFillRange, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelFormControl, macro, arginfo_class_ExcelFormControl_macro, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelFormControl, altText, arginfo_class_ExcelFormControl_altText, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelFormControl, locked, arginfo_class_ExcelFormControl_locked, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelFormControl, defaultSize, arginfo_class_ExcelFormControl_defaultSize, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelFormControl, print, arginfo_class_ExcelFormControl_print, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelFormControl, disabled, arginfo_class_ExcelFormControl_disabled, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelFormControl, item, arginfo_class_ExcelFormControl_item, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelFormControl, itemSize, arginfo_class_ExcelFormControl_itemSize, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelFormControl, addItem, arginfo_class_ExcelFormControl_addItem, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelFormControl, insertItem, arginfo_class_ExcelFormControl_insertItem, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelFormControl, clearItems, arginfo_class_ExcelFormControl_clearItems, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelFormControl, dropLines, arginfo_class_ExcelFormControl_dropLines, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelFormControl, setDropLines, arginfo_class_ExcelFormControl_setDropLines, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelFormControl, dx, arginfo_class_ExcelFormControl_dx, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelFormControl, setDx, arginfo_class_ExcelFormControl_setDx, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelFormControl, firstButton, arginfo_class_ExcelFormControl_firstButton, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelFormControl, setFirstButton, arginfo_class_ExcelFormControl_setFirstButton, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelFormControl, horiz, arginfo_class_ExcelFormControl_horiz, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelFormControl, setHoriz, arginfo_class_ExcelFormControl_setHoriz, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelFormControl, inc, arginfo_class_ExcelFormControl_inc, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelFormControl, setInc, arginfo_class_ExcelFormControl_setInc, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelFormControl, getMax, arginfo_class_ExcelFormControl_getMax, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelFormControl, setMax, arginfo_class_ExcelFormControl_setMax, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelFormControl, getMin, arginfo_class_ExcelFormControl_getMin, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelFormControl, setMin, arginfo_class_ExcelFormControl_setMin, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelFormControl, multiSel, arginfo_class_ExcelFormControl_multiSel, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelFormControl, setMultiSel, arginfo_class_ExcelFormControl_setMultiSel, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelFormControl, sel, arginfo_class_ExcelFormControl_sel, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelFormControl, setSel, arginfo_class_ExcelFormControl_setSel, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelFormControl, fromAnchor, arginfo_class_ExcelFormControl_fromAnchor, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelFormControl, toAnchor, arginfo_class_ExcelFormControl_toAnchor, ZEND_ACC_PUBLIC)
	ZEND_FE_END
};

static const zend_function_entry class_ExcelConditionalFormat_methods[] = {
	ZEND_ME(ExcelConditionalFormat, __construct, arginfo_class_ExcelConditionalFormat___construct, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelConditionalFormat, font, arginfo_class_ExcelConditionalFormat_font, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelConditionalFormat, numFormat, arginfo_class_ExcelConditionalFormat_numFormat, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelConditionalFormat, setNumFormat, arginfo_class_ExcelConditionalFormat_setNumFormat, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelConditionalFormat, customNumFormat, arginfo_class_ExcelConditionalFormat_customNumFormat, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelConditionalFormat, setCustomNumFormat, arginfo_class_ExcelConditionalFormat_setCustomNumFormat, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelConditionalFormat, setBorder, arginfo_class_ExcelConditionalFormat_setBorder, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelConditionalFormat, setBorderColor, arginfo_class_ExcelConditionalFormat_setBorderColor, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelConditionalFormat, borderLeft, arginfo_class_ExcelConditionalFormat_borderLeft, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelConditionalFormat, setBorderLeft, arginfo_class_ExcelConditionalFormat_setBorderLeft, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelConditionalFormat, borderRight, arginfo_class_ExcelConditionalFormat_borderRight, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelConditionalFormat, setBorderRight, arginfo_class_ExcelConditionalFormat_setBorderRight, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelConditionalFormat, borderTop, arginfo_class_ExcelConditionalFormat_borderTop, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelConditionalFormat, setBorderTop, arginfo_class_ExcelConditionalFormat_setBorderTop, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelConditionalFormat, borderBottom, arginfo_class_ExcelConditionalFormat_borderBottom, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelConditionalFormat, setBorderBottom, arginfo_class_ExcelConditionalFormat_setBorderBottom, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelConditionalFormat, borderLeftColor, arginfo_class_ExcelConditionalFormat_borderLeftColor, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelConditionalFormat, setBorderLeftColor, arginfo_class_ExcelConditionalFormat_setBorderLeftColor, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelConditionalFormat, borderRightColor, arginfo_class_ExcelConditionalFormat_borderRightColor, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelConditionalFormat, setBorderRightColor, arginfo_class_ExcelConditionalFormat_setBorderRightColor, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelConditionalFormat, borderTopColor, arginfo_class_ExcelConditionalFormat_borderTopColor, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelConditionalFormat, setBorderTopColor, arginfo_class_ExcelConditionalFormat_setBorderTopColor, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelConditionalFormat, borderBottomColor, arginfo_class_ExcelConditionalFormat_borderBottomColor, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelConditionalFormat, setBorderBottomColor, arginfo_class_ExcelConditionalFormat_setBorderBottomColor, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelConditionalFormat, fillPattern, arginfo_class_ExcelConditionalFormat_fillPattern, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelConditionalFormat, setFillPattern, arginfo_class_ExcelConditionalFormat_setFillPattern, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelConditionalFormat, patternForegroundColor, arginfo_class_ExcelConditionalFormat_patternForegroundColor, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelConditionalFormat, setPatternForegroundColor, arginfo_class_ExcelConditionalFormat_setPatternForegroundColor, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelConditionalFormat, patternBackgroundColor, arginfo_class_ExcelConditionalFormat_patternBackgroundColor, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelConditionalFormat, setPatternBackgroundColor, arginfo_class_ExcelConditionalFormat_setPatternBackgroundColor, ZEND_ACC_PUBLIC)
	ZEND_FE_END
};

static const zend_function_entry class_ExcelConditionalFormatting_methods[] = {
#if LIBXL_VERSION >= 0x05010000
	ZEND_ME(ExcelConditionalFormatting, __construct, arginfo_class_ExcelConditionalFormatting___construct, ZEND_ACC_PUBLIC)
#endif
#if !(LIBXL_VERSION >= 0x05010000)
	ZEND_ME(ExcelConditionalFormatting, __construct, arginfo_class_ExcelConditionalFormatting___construct, ZEND_ACC_PUBLIC)
#endif
	ZEND_ME(ExcelConditionalFormatting, addRange, arginfo_class_ExcelConditionalFormatting_addRange, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelConditionalFormatting, addRule, arginfo_class_ExcelConditionalFormatting_addRule, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelConditionalFormatting, addTopRule, arginfo_class_ExcelConditionalFormatting_addTopRule, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelConditionalFormatting, addOpNumRule, arginfo_class_ExcelConditionalFormatting_addOpNumRule, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelConditionalFormatting, addOpStrRule, arginfo_class_ExcelConditionalFormatting_addOpStrRule, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelConditionalFormatting, addAboveAverageRule, arginfo_class_ExcelConditionalFormatting_addAboveAverageRule, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelConditionalFormatting, addTimePeriodRule, arginfo_class_ExcelConditionalFormatting_addTimePeriodRule, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelConditionalFormatting, add2ColorScaleRule, arginfo_class_ExcelConditionalFormatting_add2ColorScaleRule, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelConditionalFormatting, add2ColorScaleFormulaRule, arginfo_class_ExcelConditionalFormatting_add2ColorScaleFormulaRule, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelConditionalFormatting, add3ColorScaleRule, arginfo_class_ExcelConditionalFormatting_add3ColorScaleRule, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelConditionalFormatting, add3ColorScaleFormulaRule, arginfo_class_ExcelConditionalFormatting_add3ColorScaleFormulaRule, ZEND_ACC_PUBLIC)
	ZEND_FE_END
};

static const zend_function_entry class_ExcelCoreProperties_methods[] = {
	ZEND_ME(ExcelCoreProperties, __construct, arginfo_class_ExcelCoreProperties___construct, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelCoreProperties, title, arginfo_class_ExcelCoreProperties_title, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelCoreProperties, setTitle, arginfo_class_ExcelCoreProperties_setTitle, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelCoreProperties, subject, arginfo_class_ExcelCoreProperties_subject, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelCoreProperties, setSubject, arginfo_class_ExcelCoreProperties_setSubject, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelCoreProperties, creator, arginfo_class_ExcelCoreProperties_creator, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelCoreProperties, setCreator, arginfo_class_ExcelCoreProperties_setCreator, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelCoreProperties, lastModifiedBy, arginfo_class_ExcelCoreProperties_lastModifiedBy, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelCoreProperties, setLastModifiedBy, arginfo_class_ExcelCoreProperties_setLastModifiedBy, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelCoreProperties, created, arginfo_class_ExcelCoreProperties_created, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelCoreProperties, setCreated, arginfo_class_ExcelCoreProperties_setCreated, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelCoreProperties, modified, arginfo_class_ExcelCoreProperties_modified, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelCoreProperties, setModified, arginfo_class_ExcelCoreProperties_setModified, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelCoreProperties, tags, arginfo_class_ExcelCoreProperties_tags, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelCoreProperties, setTags, arginfo_class_ExcelCoreProperties_setTags, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelCoreProperties, categories, arginfo_class_ExcelCoreProperties_categories, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelCoreProperties, setCategories, arginfo_class_ExcelCoreProperties_setCategories, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelCoreProperties, comments, arginfo_class_ExcelCoreProperties_comments, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelCoreProperties, setComments, arginfo_class_ExcelCoreProperties_setComments, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelCoreProperties, createdAsDouble, arginfo_class_ExcelCoreProperties_createdAsDouble, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelCoreProperties, setCreatedAsDouble, arginfo_class_ExcelCoreProperties_setCreatedAsDouble, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelCoreProperties, modifiedAsDouble, arginfo_class_ExcelCoreProperties_modifiedAsDouble, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelCoreProperties, setModifiedAsDouble, arginfo_class_ExcelCoreProperties_setModifiedAsDouble, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelCoreProperties, removeAll, arginfo_class_ExcelCoreProperties_removeAll, ZEND_ACC_PUBLIC)
	ZEND_FE_END
};

static const zend_function_entry class_ExcelTable_methods[] = {
	ZEND_ME(ExcelTable, __construct, arginfo_class_ExcelTable___construct, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelTable, name, arginfo_class_ExcelTable_name, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelTable, setName, arginfo_class_ExcelTable_setName, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelTable, ref, arginfo_class_ExcelTable_ref, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelTable, setRef, arginfo_class_ExcelTable_setRef, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelTable, autoFilter, arginfo_class_ExcelTable_autoFilter, ZEND_ACC_PUBLIC)
#if LIBXL_VERSION >= 0x05020000
	ZEND_ME(ExcelTable, isAutoFilter, arginfo_class_ExcelTable_isAutoFilter, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelTable, removeFilter, arginfo_class_ExcelTable_removeFilter, ZEND_ACC_PUBLIC)
#endif
	ZEND_ME(ExcelTable, style, arginfo_class_ExcelTable_style, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelTable, setStyle, arginfo_class_ExcelTable_setStyle, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelTable, showRowStripes, arginfo_class_ExcelTable_showRowStripes, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelTable, setShowRowStripes, arginfo_class_ExcelTable_setShowRowStripes, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelTable, showColumnStripes, arginfo_class_ExcelTable_showColumnStripes, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelTable, setShowColumnStripes, arginfo_class_ExcelTable_setShowColumnStripes, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelTable, showFirstColumn, arginfo_class_ExcelTable_showFirstColumn, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelTable, setShowFirstColumn, arginfo_class_ExcelTable_setShowFirstColumn, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelTable, showLastColumn, arginfo_class_ExcelTable_showLastColumn, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelTable, setShowLastColumn, arginfo_class_ExcelTable_setShowLastColumn, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelTable, columnSize, arginfo_class_ExcelTable_columnSize, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelTable, columnName, arginfo_class_ExcelTable_columnName, ZEND_ACC_PUBLIC)
	ZEND_ME(ExcelTable, setColumnName, arginfo_class_ExcelTable_setColumnName, ZEND_ACC_PUBLIC)
	ZEND_FE_END
};

static zend_class_entry *register_class_ExcelBook(void)
{
	zend_class_entry ce, *class_entry;

	INIT_CLASS_ENTRY(ce, "ExcelBook", class_ExcelBook_methods);
	class_entry = zend_register_internal_class_with_flags(&ce, NULL, 0);

	return class_entry;
}

static zend_class_entry *register_class_ExcelSheet(void)
{
	zend_class_entry ce, *class_entry;

	INIT_CLASS_ENTRY(ce, "ExcelSheet", class_ExcelSheet_methods);
	class_entry = zend_register_internal_class_with_flags(&ce, NULL, 0);

	return class_entry;
}

static zend_class_entry *register_class_ExcelFormat(void)
{
	zend_class_entry ce, *class_entry;

	INIT_CLASS_ENTRY(ce, "ExcelFormat", class_ExcelFormat_methods);
	class_entry = zend_register_internal_class_with_flags(&ce, NULL, 0);

	return class_entry;
}

static zend_class_entry *register_class_ExcelFont(void)
{
	zend_class_entry ce, *class_entry;

	INIT_CLASS_ENTRY(ce, "ExcelFont", class_ExcelFont_methods);
	class_entry = zend_register_internal_class_with_flags(&ce, NULL, 0);

	return class_entry;
}

static zend_class_entry *register_class_ExcelAutoFilter(void)
{
	zend_class_entry ce, *class_entry;

	INIT_CLASS_ENTRY(ce, "ExcelAutoFilter", class_ExcelAutoFilter_methods);
	class_entry = zend_register_internal_class_with_flags(&ce, NULL, 0);

	return class_entry;
}

static zend_class_entry *register_class_ExcelFilterColumn(void)
{
	zend_class_entry ce, *class_entry;

	INIT_CLASS_ENTRY(ce, "ExcelFilterColumn", class_ExcelFilterColumn_methods);
	class_entry = zend_register_internal_class_with_flags(&ce, NULL, 0);

	return class_entry;
}

static zend_class_entry *register_class_ExcelRichString(void)
{
	zend_class_entry ce, *class_entry;

	INIT_CLASS_ENTRY(ce, "ExcelRichString", class_ExcelRichString_methods);
	class_entry = zend_register_internal_class_with_flags(&ce, NULL, 0);

	return class_entry;
}

static zend_class_entry *register_class_ExcelFormControl(void)
{
	zend_class_entry ce, *class_entry;

	INIT_CLASS_ENTRY(ce, "ExcelFormControl", class_ExcelFormControl_methods);
	class_entry = zend_register_internal_class_with_flags(&ce, NULL, 0);

	return class_entry;
}

static zend_class_entry *register_class_ExcelConditionalFormat(void)
{
	zend_class_entry ce, *class_entry;

	INIT_CLASS_ENTRY(ce, "ExcelConditionalFormat", class_ExcelConditionalFormat_methods);
	class_entry = zend_register_internal_class_with_flags(&ce, NULL, 0);

	return class_entry;
}

static zend_class_entry *register_class_ExcelConditionalFormatting(void)
{
	zend_class_entry ce, *class_entry;

	INIT_CLASS_ENTRY(ce, "ExcelConditionalFormatting", class_ExcelConditionalFormatting_methods);
	class_entry = zend_register_internal_class_with_flags(&ce, NULL, 0);

	return class_entry;
}

static zend_class_entry *register_class_ExcelCoreProperties(void)
{
	zend_class_entry ce, *class_entry;

	INIT_CLASS_ENTRY(ce, "ExcelCoreProperties", class_ExcelCoreProperties_methods);
	class_entry = zend_register_internal_class_with_flags(&ce, NULL, 0);

	return class_entry;
}

static zend_class_entry *register_class_ExcelTable(void)
{
	zend_class_entry ce, *class_entry;

	INIT_CLASS_ENTRY(ce, "ExcelTable", class_ExcelTable_methods);
	class_entry = zend_register_internal_class_with_flags(&ce, NULL, 0);

	return class_entry;
}
