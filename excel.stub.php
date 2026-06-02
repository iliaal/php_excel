<?php

/** @generate-class-entries */

class ExcelBook
{
    public static function requiresKey(): bool {}  // ZPP ""
    public function load(string $data): bool {}  // ZPP "S"
    public function loadFile(string $filename): bool {}  // ZPP "S"
    public function save(string $filename = ""): string|true|false {}  // ZPP "|S"
    public function getSheet(int $sheet = 0): ExcelSheet|false {}  // ZPP "|l"
    public function getSheetByName(string $name, bool $case_insensitive = false): ExcelSheet|false {}  // ZPP "S|b"
    public function deleteSheet(int $sheet): bool {}  // ZPP "l"
    public function activeSheet(int $sheet = -1): int|false {}  // ZPP "|l"
    public function addSheet(string $name): ExcelSheet|false {}  // ZPP "S"
    public function copySheet(string $name, int $sheet_number): ExcelSheet|false {}  // ZPP "Sl"
    public function sheetCount(): int|false {}  // ZPP ""
    public function getError(): string|false {}  // ZPP ""
    public function addFont(?ExcelFont $font = null): ExcelFont|false {}  // ZPP "|O!"
    public function addFormat(?ExcelFormat $format = null): ExcelFormat|false {}  // ZPP "|O!"
    public function getAllFormats(): array|false {}  // ZPP ""
    public function addCustomFormat(string $format): int|false {}  // ZPP "S"
    public function getCustomFormat(int $id): string|false {}  // ZPP "l"
    public function packDate(int $timestamp): float|false {}  // ZPP "l"
    public function packDateValues(int $year, int $month, int $day, int $hour, int $min, int $sec): float|false {}  // ZPP "llllll"
    public function unpackDate(float $date): int|false {}  // ZPP "d"
    public function isDate1904(): bool {}  // ZPP ""
    public function setDate1904(bool $date_type): bool {}  // ZPP "b"
    public function getActiveSheet(): int|false {}  // ZPP ""
    public function getDefaultFont(): array|false {}  // ZPP ""
    public function setDefaultFont(string $font, int $font_size): mixed {}  // ZPP "Sl"
    public function setLocale(string $locale): mixed {}  // ZPP "S"
    public function __construct(?string $license_name = null, ?string $license_key = null, bool $excel_2007 = false) {}  // ZPP "|s!s!b"
    public function setActiveSheet(int $sheet): bool {}  // ZPP "l"
    public function addPictureFromFile(string $filename): int|false {}  // ZPP "S"
    public function addPictureFromString(string $data): int|false {}  // ZPP "S"
    public function rgbMode(): bool {}  // ZPP ""
    public function setRGBMode(bool $mode): bool {}  // ZPP "b"
    public function colorPack(int $r, int $g, int $b): int|false {}  // ZPP "lll"
    public function colorUnpack(int $color): array|false {}  // ZPP "l"
    public function getLibXlVersion(): string {}  // ZPP ""
    public function getPhpExcelVersion(): string {}  // ZPP ""
    public function loadInfo(string $filename): bool {}  // ZPP "S"
    public function getSheetName(int $index): string|false {}  // ZPP "l"
    public function addRichString(): ExcelRichString|false {}  // ZPP ""
    public function calcMode(): int|false {}  // ZPP ""
    public function setCalcMode(int $mode): bool {}  // ZPP "l"
    public function addConditionalFormat(): ExcelConditionalFormat|false {}  // ZPP ""
    public function addFormatFromStyle(int $style): ExcelFormat|false {}  // ZPP "l"
    public function removeVBA(): bool {}  // ZPP ""
    public function removePrinterSettings(): bool {}  // ZPP ""
#if LIBXL_VERSION >= 0x05000000
    public function setPassword(string $password): bool {}  // ZPP "S"
#endif
#if LIBXL_VERSION >= 0x05000000
    public function dpiAwareness(): int|false {}  // ZPP ""
#endif
#if LIBXL_VERSION >= 0x05000000
    public function setDpiAwareness(int $value): bool {}  // ZPP "l"
#endif
#if LIBXL_VERSION >= 0x05000100
    public function loadInfoRaw(string $data): bool {}  // ZPP "S"
#endif
#if LIBXL_VERSION >= 0x05010000
    public function errorCode(): int|false {}  // ZPP ""
#endif
#if LIBXL_VERSION >= 0x05010000
    public function conditionalFormat(int $index): ExcelConditionalFormat|false {}  // ZPP "l"
#endif
#if LIBXL_VERSION >= 0x05010000
    public function conditionalFormatSize(): int|false {}  // ZPP ""
#endif
#if LIBXL_VERSION >= 0x05010000
    public function clear(): bool {}  // ZPP ""
#endif
    public function coreProperties(): ExcelCoreProperties|false {}  // ZPP ""
    public function removeAllPhonetics(): bool {}  // ZPP ""
    public function biffVersion(): int|false {}  // ZPP ""
    public function getRefR1C1(): bool {}  // ZPP ""
    public function setRefR1C1(bool $active): bool {}  // ZPP "b"
    public function getPicture(int $index): array|false {}  // ZPP "l"
    public function getNumPictures(): int|false {}  // ZPP ""
    public function insertSheet(int $index, string $name, ?ExcelSheet $sheet = null): ExcelSheet|false {}  // ZPP "lS|O!"
    public function isTemplate(): bool {}  // ZPP ""
    public function setTemplate(bool $mode): bool {}  // ZPP "b"
    public function sheetType(int $sheet): int|false {}  // ZPP "l"
    public function addPictureAsLink(string $filename, bool $insert = false): int|false {}  // ZPP "S|b"
    public function moveSheet(int $src_index, int $dest_index): bool {}  // ZPP "ll"
}

class ExcelSheet
{
    public function __construct(ExcelBook $book, string $name) {}  // ZPP "OS"
    public function cellType(int $row, int $column): int|false {}  // ZPP "ll"
    public function cellFormat(int $row, int $column): ExcelFormat|false {}  // ZPP "ll"
    public function setCellFormat(int $row, int $column, ExcelFormat $format): mixed {}  // ZPP "llO"
    public function readRow(int $row, int $start_col = 0, int $end_column = -1, bool $read_formula = true): array|false {}  // ZPP "l|llb"
    public function readCol(int $column, int $start_row = 0, int $end_row = -1, bool $read_formula = true): array|false {}  // ZPP "l|llb"
    public function read(int $row, int $column, mixed &$format = null, bool $read_formula = true): mixed {}  // ZPP "ll|zb"
    public function write(int $row, int $column, mixed $data, ?ExcelFormat $format = null, int $datatype = 0): bool {}  // ZPP "llz|O!l"
    public function writeRow(int $row, array $data, int $start_column = 0, ?ExcelFormat $format = null): bool {}  // ZPP "la|lO!"
    public function writeCol(int $column, array $data, int $start_row = 0, ?ExcelFormat $format = null, int $data_type = 0): bool {}  // ZPP "la|lO!l"
    public function isFormula(int $row, int $column): bool {}  // ZPP "ll"
    public function isDate(int $row, int $column): bool {}  // ZPP "ll"
    public function insertRow(int $row_first, int $row_last): bool {}  // ZPP "ll"
    public function insertCol(int $col_first, int $col_last): bool {}  // ZPP "ll"
    public function removeRow(int $row_first, int $row_last): bool {}  // ZPP "ll"
    public function removeCol(int $col_first, int $col_last): bool {}  // ZPP "ll"
    public function colWidth(int $column): float|false {}  // ZPP ""
    public function rowHeight(int $row): float|false {}  // ZPP ""
    public function readComment(int $row, int $column): string|false {}  // ZPP "ll"
    public function writeComment(int $row, int $column, string $value, string $author, int $width, int $height): mixed {}  // ZPP "llSSll"
    public function setColWidth(int $column_start, int $column_end, float $width, bool $hidden = false, ?ExcelFormat $format = null): bool {}  // ZPP "lld|bO!"
    public function setRowHeight(int $row, float $height, ?ExcelFormat $format = null, bool $hidden = false): bool {}  // ZPP "ld|O!b"
    public function getMerge(int $row, int $column): array|false {}  // ZPP "ll"
    public function setMerge(int $row_start, int $row_end, int $col_start, int $col_end): bool {}  // ZPP "llll"
    public function deleteMerge(int $row, int $column): bool {}  // ZPP "ll"
    public function addPictureScaled(int $row, int $column, int $pic_id, float $scale, int $x_offset = 0, int $y_offset = 0, int $pos = 0): mixed {}  // ZPP "llld|lll"
    public function addPictureDim(int $row, int $column, int $pic_id, int $width, int $height, int $x_offset = 0, int $y_offset = 0, int $pos = 0): mixed {}  // ZPP "lllll|lll"
    public function horPageBreak(int $row, bool $break): bool {}  // ZPP "lb"
    public function verPageBreak(int $col, bool $break): bool {}  // ZPP "lb"
    public function splitSheet(int $row, int $column): mixed {}  // ZPP "ll"
    public function groupRows(int $start_row, int $end_row, bool $collapse = false): bool {}  // ZPP "ll|b"
    public function groupCols(int $start_column, int $end_column, bool $collapse = false): bool {}  // ZPP "ll|b"
    public function clear(int $row_s, int $row_e, int $col_s, int $col_e): mixed {}  // ZPP "llll"
    public function copy(int $row, int $col, int $to_row, int $to_col): bool {}  // ZPP "llll"
    public function firstRow(): int|false {}  // ZPP ""
    public function lastRow(): int|false {}  // ZPP ""
    public function firstCol(): int|false {}  // ZPP ""
    public function lastCol(): int|false {}  // ZPP ""
    public function displayGridlines(): bool {}  // ZPP ""
    public function printGridlines(): bool {}  // ZPP ""
    public function setDisplayGridlines(bool $value): mixed {}  // ZPP ""
    public function setHidden(bool $value): bool {}  // ZPP "b"
    public function isHidden(): bool {}  // ZPP ""
    public function getTopLeftView(): array|false {}  // ZPP ""
    public function setTopLeftView(int $row, int $column): bool {}  // ZPP "ll"
    public function rowColToAddr(int $row, int $column, bool $row_relative = false, bool $col_relative = false): string|false {}  // ZPP "ll|bb"
    public function addrToRowCol(string $cell_reference): array|false {}  // ZPP "S"
    public function setPrintGridlines(bool $value): mixed {}  // ZPP "b"
    public function zoom(): int|false {}  // ZPP ""
    public function zoomPrint(): int|false {}  // ZPP ""
    public function setZoom(int $value): mixed {}  // ZPP "l"
    public function setZoomPrint(int $value): mixed {}  // ZPP "l"
    public function setLandscape(bool $value): mixed {}  // ZPP "b"
    public function landscape(): bool {}  // ZPP ""
    public function paper(): int|false {}  // ZPP ""
    public function setPaper(int $value): mixed {}  // ZPP "l"
    public function header(): string|null|false {}  // ZPP ""
    public function footer(): string|null|false {}  // ZPP ""
    public function setHeader(string $header, float $margin): bool {}  // ZPP "Sd"
    public function setFooter(string $footer, float $margin): bool {}  // ZPP "Sd"
    public function headerMargin(): float|false {}  // ZPP ""
    public function footerMargin(): float|false {}  // ZPP ""
    public function hcenter(): bool {}  // ZPP ""
    public function vcenter(): bool {}  // ZPP ""
    public function setHCenter(bool $value): mixed {}  // ZPP "b"
    public function setVCenter(bool $value): mixed {}  // ZPP "b"
    public function marginLeft(): float|false {}  // ZPP ""
    public function marginRight(): float|false {}  // ZPP ""
    public function marginTop(): float|false {}  // ZPP ""
    public function marginBottom(): float|false {}  // ZPP ""
    public function setMarginLeft(float $value): mixed {}  // ZPP "d"
    public function setMarginRight(float $value): mixed {}  // ZPP "d"
    public function setMarginTop(float $value): mixed {}  // ZPP "d"
    public function setMarginBottom(float $value): mixed {}  // ZPP "d"
    public function printHeaders(): bool {}  // ZPP ""
    public function setPrintHeaders(bool $value): mixed {}  // ZPP "b"
    public function name(): string|null|false {}  // ZPP ""
    public function setName(string $name): mixed {}  // ZPP "S"
    public function setNamedRange(string $name, int $row, int $col, int $to_row, int $to_col, int $scope_id = 0): bool {}  // ZPP "Sllll|l"
    public function delNamedRange(string $name, int $scope_id = 0): bool {}  // ZPP "S|l"
    public function setPrintRepeatRows(int $row_start, int $row_end): bool {}  // ZPP "ll"
    public function setPrintRepeatCols(int $col_start, int $col_end): bool {}  // ZPP "ll"
    public function getGroupSummaryBelow(): bool {}  // ZPP ""
    public function setGroupSummaryBelow(bool $direction): bool {}  // ZPP "b"
    public function getGroupSummaryRight(): bool {}  // ZPP ""
    public function setGroupSummaryRight(bool $direction): bool {}  // ZPP "b"
    public function setPrintFit(int $wPages, int $hPages): bool {}  // ZPP "ll"
    public function getPrintFit(): array|false {}  // ZPP ""
    public function getNamedRange(string $name, int $scope_id = 0): array|false {}  // ZPP "S|l"
    public function getIndexRange(int $index): array|false {}  // ZPP "l"
    public function namedRangeSize(): int|false {}  // ZPP ""
    public function getVerPageBreak(int $index): int|false {}  // ZPP "l"
    public function getVerPageBreakSize(): int|false {}  // ZPP ""
    public function getHorPageBreak(int $index): int|false {}  // ZPP "l"
    public function getHorPageBreakSize(): int|false {}  // ZPP ""
    public function getPictureInfo(int $index): array|false {}  // ZPP "l"
    public function getNumPictures(): int|false {}  // ZPP ""
    public function getRightToLeft(): int|false {}  // ZPP ""
    public function setRightToLeft(int $mode): mixed {}  // ZPP "l"
    public function setPrintArea(int $row_first, int $row_last, int $col_first, int $col_last): bool {}  // ZPP "llll"
    public function clearPrintRepeats(): bool {}  // ZPP ""
    public function clearPrintArea(): bool {}  // ZPP ""
    public function protect(): bool {}  // ZPP ""
    public function hyperlinkSize(): int|false {}  // ZPP ""
    public function hyperlink(int $index): array|false {}  // ZPP "l"
    public function delHyperlink(int $index): bool {}  // ZPP "l"
    public function addHyperlink(string $hyperlink, int $row_first, int $row_last, int $col_first, int $col_last): bool {}  // ZPP "Sllll"
    public function mergeSize(): int|false {}  // ZPP ""
    public function merge(int $index): array|false {}  // ZPP "l"
    public function delMergeByIndex(int $index): bool {}  // ZPP "l"
    public function splitInfo(): array|false {}  // ZPP ""
    public function rowHidden(int $row): bool {}  // ZPP "l"
    public function setRowHidden(int $row, bool $hidden): bool {}  // ZPP "lb"
    public function colHidden(int $col): bool {}  // ZPP "l"
    public function setColHidden(int $col, bool $hidden): bool {}  // ZPP "lb"
    public function isLicensed(): bool {}  // ZPP ""
    public function setAutoFitArea(int $row_start = 0, int $row_end = 0, int $col_start = 0, int $col_end = 0): bool {}  // ZPP "|llll"
    public function printRepeatRows(): array|false {}  // ZPP ""
    public function printRepeatCols(): array|false {}  // ZPP ""
    public function printArea(): array|false {}  // ZPP ""
    public function setProtect(bool $value, string $password = "", int $enhancedProtection = 0): bool {}  // ZPP "b|Sl"
    public function table(int $index): array|false {}  // ZPP "l"
    public function setTabColor(int $color = 0): bool {}  // ZPP "|l"
    public function autoFilter(): ExcelAutoFilter|false {}  // ZPP ""
    public function applyFilter(): bool {}  // ZPP ""
    public function removeFilter(): bool {}  // ZPP ""
    public function addIgnoredError(int $iError, int $rowFirst = 0, int $colFirst = 0, int $rowLast = 0, int $colLast = 0): bool {}  // ZPP "l|llll"
    public function writeError(int $row, int $col, int $iError, ?ExcelFormat $format = null): mixed {}  // ZPP "lll|O!"
    public function removeComment(int $row, int $col): mixed {}  // ZPP "ll"
    public function addDataValidation(int $type, int $op, int $row_first, int $row_last, int $col_first, int $col_last, string $val_1, ?string $val_2 = null, bool $allow_blank = true, bool $hide_dropdown = false, bool $show_inputmessage = true, bool $show_errormessage = true, string $prompt_title = "", string $prompt = "", string $error_title = "", string $error = "", int $error_style = 1): bool {}  // ZPP "llllllS|S!bbbbSSSSl"
    public function addDataValidationDouble(int $type, int $op, int $row_first, int $row_last, int $col_first, int $col_last, float $val_1, ?float $val_2 = null, bool $allow_blank = true, bool $hide_dropdown = false, bool $show_inputmessage = true, bool $show_errormessage = true, string $prompt_title = "", string $prompt = "", string $error_title = "", string $error = "", int $error_style = 1): bool {}  // FAST_ZPP llllllD|D!bbbbSSSSl
    public function removeDataValidations(): bool {}  // ZPP ""
#if LIBXL_VERSION >= 0x05020000
    public function dataValidationSize(): int {}  // ZPP ""
    public function dataValidation(int $index): array|false {}  // ZPP "l"
#endif
    public function firstFilledRow(): int|false {}  // ZPP ""
    public function lastFilledRow(): int|false {}  // ZPP ""
    public function firstFilledCol(): int|false {}  // ZPP ""
    public function lastFilledCol(): int|false {}  // ZPP ""
    public function removePicture(int $row, int $col): bool {}  // ZPP "ll"
    public function removePictureByIndex(int $index): bool {}  // ZPP "l"
    public function isRichStr(int $row, int $col): bool {}  // ZPP ""
    public function readRichStr(int $row, int $col): ExcelRichString|false {}  // ZPP "ll"
    public function writeRichStr(int $row, int $col, ExcelRichString $richString, ?ExcelFormat $format = null): bool {}  // ZPP "llO|O!"
    public function formControlSize(): int|false {}  // ZPP ""
    public function formControl(int $index): ExcelFormControl|false {}  // ZPP "l"
    public function getActiveCell(): array|false {}  // ZPP ""
    public function setActiveCell(int $row, int $col): bool {}  // ZPP "ll"
    public function selectionRange(): string|null|false {}  // ZPP ""
    public function addSelectionRange(string $sqref): bool {}  // ZPP "S"
    public function removeSelection(): bool {}  // ZPP ""
    public function tabColor(): int|false {}  // ZPP ""
    public function getTabRgbColor(): array|false {}  // ZPP ""
    public function setTabRgbColor(int $red, int $green, int $blue): bool {}  // ZPP "lll"
    public function hyperlinkIndex(int $row, int $col): int|false {}  // ZPP "ll"
    public function colWidthPx(int $col): int|false {}  // ZPP "l"
    public function rowHeightPx(int $row): int|false {}  // ZPP "l"
    public function colFormat(int $col): ExcelFormat|false {}  // ZPP "l"
    public function rowFormat(int $row): ExcelFormat|false {}  // ZPP "l"
    public function setColPx(int $colFirst, int $colLast, int $widthPx, ?ExcelFormat $format = null, bool $hidden = false): bool {}  // ZPP "lll|O!b"
    public function setRowPx(int $row, int $heightPx, ?ExcelFormat $format = null, bool $hidden = false): bool {}  // ZPP "ll|O!b"
    public function setBorder(int $rowFirst, int $rowLast, int $colFirst, int $colLast, int $borderStyle, int $borderColor): bool {}  // ZPP "llllll"
    public function addTable(string $name, int $rowFirst, int $rowLast, int $colFirst, int $colLast, bool $hasHeaders = false, int $style = 0): ExcelTable|false {}  // ZPP "Sllll|bl"
    public function getTableByName(string $name): ExcelTable|false {}  // ZPP "S"
    public function getTableByIndex(int $index): ExcelTable|false {}  // ZPP "l"
    public function applyFilter2(ExcelAutoFilter $autoFilter): bool {}  // ZPP "O"
#if LIBXL_VERSION >= 0x05010000
    public function addConditionalFormatting(int $rowFirst, int $rowLast, int $colFirst, int $colLast): ExcelConditionalFormatting|false {}
#else
    public function addConditionalFormatting(): ExcelConditionalFormatting|false {}
#endif
#if LIBXL_VERSION >= 0x05010000
    public function conditionalFormatting(int $index): ExcelConditionalFormatting|false {}  // ZPP "l"
#endif
#if LIBXL_VERSION >= 0x05010000
    public function removeConditionalFormatting(int $index): bool {}  // ZPP "l"
#endif
#if LIBXL_VERSION >= 0x05010000
    public function conditionalFormattingSize(): int|false {}  // ZPP ""
#endif
}

class ExcelFormat
{
    public function __construct(ExcelBook $book) {}  // ZPP "O"
    public function setFont(ExcelFont $font): bool {}  // ZPP "O"
    public function getFont(): ExcelFont|false {}  // ZPP ""
    public function numberFormat(?int $format = null): int|false {}  // ZPP "|l!"
    public function horizontalAlign(?int $align_mode = null): int|false {}  // ZPP "|l!"
    public function verticalAlign(?int $align_mode = null): int|false {}  // ZPP "|l!"
    public function wrap(?bool $wrap = null): bool {}  // ZPP "|b!"
    public function rotate(?int $angle = null): int|false {}  // ZPP "|l!"
    public function indent(?int $indent = null): int|false {}  // ZPP "|l!"
    public function shrinkToFit(?bool $shrink = null): bool {}  // ZPP "|b!"
    public function borderStyle(?int $style = null): bool {}  // ZPP "|l!"
    public function borderColor(?int $color = null): bool {}  // ZPP "|l!"
    public function borderLeftStyle(?int $style = null): int|false {}  // ZPP "|l!"
    public function borderLeftColor(?int $color = null): int|false {}  // ZPP "|l!"
    public function borderRightStyle(?int $style = null): int|false {}  // ZPP "|l!"
    public function borderRightColor(?int $color = null): int|false {}  // ZPP "|l!"
    public function borderTopStyle(?int $style = null): int|false {}  // ZPP "|l!"
    public function borderTopColor(?int $color = null): int|false {}  // ZPP "|l!"
    public function borderBottomStyle(?int $style = null): int|false {}  // ZPP "|l!"
    public function borderBottomColor(?int $color = null): int|false {}  // ZPP "|l!"
    public function borderDiagonalStyle(?int $style = null): int|false {}  // ZPP "|l!"
    public function borderDiagonalColor(?int $color = null): int|false {}  // ZPP "|l!"
    public function fillPattern(?int $patern = null): int|false {}  // ZPP "|l!"
    public function patternForegroundColor(?int $color = null): int|false {}  // ZPP "|l!"
    public function patternBackgroundColor(?int $color = null): int|false {}  // ZPP "|l!"
    public function locked(?bool $locked = null): bool {}  // ZPP "|b!"
    public function hidden(?bool $hidden = null): bool {}  // ZPP "|b!"
}

class ExcelFont
{
    public function size(?int $size = null): int|false {}  // ZPP "|l!"
    public function italics(?bool $italics = null): bool {}  // ZPP "|b!"
    public function strike(?bool $strike = null): bool {}  // ZPP "|b!"
    public function bold(?bool $bold = null): bool {}  // ZPP "|b!"
    public function color(?int $color = null): int|false {}  // ZPP "|l!"
    public function mode(?int $mode = null): int|false {}  // ZPP "|l!"
    public function underline(?int $underline_style = null): int|false {}  // ZPP "|l!"
    public function name(?string $name = null): string|false {}  // ZPP "|S!"
    public final function __construct(ExcelBook $book) {}  // ZPP "O"
}

class ExcelAutoFilter
{
    public function __construct(ExcelSheet $sheet) {}  // ZPP "O"
    public function getRef(): array|false {}  // ZPP ""
    public function setRef(int $row_first, int $col_first, int $row_last, int $col_last): mixed {}  // ZPP "llll"
    public function column(int $colId): ExcelFilterColumn|false {}  // ZPP "l"
    public function columnSize(): int|false {}  // ZPP ""
    public function columnByIndex(int $index): ExcelFilterColumn|false {}  // ZPP "l"
    public function getSortRange(): array|false {}  // ZPP ""
    public function getSort(): array|false {}  // ZPP ""
    public function setSort(int $columnIndex, bool $descending): bool {}  // ZPP "lb"
    public function addSort(int $columnIndex, bool $descending): bool {}  // ZPP "lb"
}

class ExcelFilterColumn
{
    public function __construct(ExcelAutoFilter $autoFilter, int $colId) {}  // ZPP "Ol"
    public function index(): int|false {}  // ZPP ""
    public function filterType(): int|false {}  // ZPP ""
    public function filterSize(): int|false {}  // ZPP ""
    public function filter(int $index): string|false {}  // ZPP "l"
    public function addFilter(string $filterValue): bool {}  // ZPP "S"
    public function getTop10(): array|false {}  // ZPP ""
    public function setTop10(float $value, bool $top = false, bool $percent = false): bool {}  // ZPP "dbb"
    public function getCustomFilter(): array|false {}  // ZPP ""
    public function setCustomFilter(int $operator_1, string $value_1, int $operator_2 = 0, string $value_2 = "", bool $andOp = false): bool {}  // ZPP "lS|lSb"
    public function clear(): bool {}  // ZPP ""
}

class ExcelRichString
{
    public function __construct(ExcelBook $book) {}  // ZPP "O"
    public function addFont(?ExcelFont $font = null): ExcelFont|false {}  // ZPP "|O!"
    public function addText(string $text, ?ExcelFont $font = null): bool {}  // ZPP "S|O!"
    public function getText(int $index): array|false {}  // ZPP "l"
    public function textSize(): int|false {}  // ZPP ""
}

class ExcelFormControl
{
    public function __construct(ExcelSheet $sheet, int $index) {}  // ZPP "Ol"
    public function objectType(): mixed {}  // ZPP ""
    public function checked(): mixed {}  // ZPP ""
    public function setChecked(int $value): bool {}  // ZPP "l"
    public function fmlaGroup(): mixed {}  // ZPP ""
    public function setFmlaGroup(string $value): bool {}  // ZPP "S"
    public function fmlaLink(): mixed {}  // ZPP ""
    public function setFmlaLink(string $value): bool {}  // ZPP "S"
    public function fmlaRange(): mixed {}  // ZPP ""
    public function setFmlaRange(string $value): bool {}  // ZPP "S"
    public function fmlaTxbx(): mixed {}  // ZPP ""
    public function setFmlaTxbx(string $value): bool {}  // ZPP "S"
    public function name(): mixed {}  // ZPP ""
    public function linkedCell(): mixed {}  // ZPP ""
    public function listFillRange(): mixed {}  // ZPP ""
    public function macro(): mixed {}  // ZPP ""
    public function altText(): mixed {}  // ZPP ""
    public function locked(): mixed {}  // ZPP ""
    public function defaultSize(): mixed {}  // ZPP ""
    public function print(): mixed {}  // ZPP ""
    public function disabled(): mixed {}  // ZPP ""
    public function item(int $index): string|null|false {}  // ZPP "l"
    public function itemSize(): mixed {}  // ZPP ""
    public function addItem(string $value): bool {}  // ZPP "S"
    public function insertItem(int $index, string $value): bool {}  // ZPP "lS"
    public function clearItems(): mixed {}  // ZPP ""
    public function dropLines(): mixed {}  // ZPP ""
    public function setDropLines(int $value): bool {}  // ZPP "l"
    public function dx(): mixed {}  // ZPP ""
    public function setDx(int $value): bool {}  // ZPP "l"
    public function firstButton(): mixed {}  // ZPP ""
    public function setFirstButton(bool $value): bool {}  // ZPP "b"
    public function horiz(): mixed {}  // ZPP ""
    public function setHoriz(bool $value): bool {}  // ZPP "b"
    public function inc(): mixed {}  // ZPP ""
    public function setInc(int $value): bool {}  // ZPP "l"
    public function getMax(): mixed {}  // ZPP ""
    public function setMax(int $value): bool {}  // ZPP "l"
    public function getMin(): mixed {}  // ZPP ""
    public function setMin(int $value): bool {}  // ZPP "l"
    public function multiSel(): mixed {}  // ZPP ""
    public function setMultiSel(string $value): bool {}  // ZPP "S"
    public function sel(): mixed {}  // ZPP ""
    public function setSel(int $value): bool {}  // ZPP "l"
    public function fromAnchor(): mixed {}  // ZPP ""
    public function toAnchor(): mixed {}  // ZPP ""
}

class ExcelConditionalFormat
{
    public function __construct(ExcelBook $book) {}  // ZPP "O"
    public function font(): mixed {}  // ZPP ""
    public function numFormat(): mixed {}  // ZPP ""
    public function setNumFormat(int $value): bool {}  // ZPP "l"
    public function customNumFormat(): mixed {}  // ZPP ""
    public function setCustomNumFormat(string $value): bool {}  // ZPP "S"
    public function setBorder(int $value): bool {}  // ZPP "l"
    public function setBorderColor(int $value): bool {}  // ZPP "l"
    public function borderLeft(): mixed {}  // ZPP ""
    public function setBorderLeft(int $value): bool {}  // ZPP "l"
    public function borderRight(): mixed {}  // ZPP ""
    public function setBorderRight(int $value): bool {}  // ZPP "l"
    public function borderTop(): mixed {}  // ZPP ""
    public function setBorderTop(int $value): bool {}  // ZPP "l"
    public function borderBottom(): mixed {}  // ZPP ""
    public function setBorderBottom(int $value): bool {}  // ZPP "l"
    public function borderLeftColor(): mixed {}  // ZPP ""
    public function setBorderLeftColor(int $value): bool {}  // ZPP "l"
    public function borderRightColor(): mixed {}  // ZPP ""
    public function setBorderRightColor(int $value): bool {}  // ZPP "l"
    public function borderTopColor(): mixed {}  // ZPP ""
    public function setBorderTopColor(int $value): bool {}  // ZPP "l"
    public function borderBottomColor(): mixed {}  // ZPP ""
    public function setBorderBottomColor(int $value): bool {}  // ZPP "l"
    public function fillPattern(): mixed {}  // ZPP ""
    public function setFillPattern(int $value): bool {}  // ZPP "l"
    public function patternForegroundColor(): mixed {}  // ZPP ""
    public function setPatternForegroundColor(int $value): bool {}  // ZPP "l"
    public function patternBackgroundColor(): mixed {}  // ZPP ""
    public function setPatternBackgroundColor(int $value): bool {}  // ZPP "l"
}

class ExcelConditionalFormatting
{
#if LIBXL_VERSION >= 0x05010000
    public function __construct(ExcelSheet $sheet, int $rowFirst, int $rowLast, int $colFirst, int $colLast) {}  // ZPP "Ollll"
#else
    public function __construct(ExcelSheet $sheet) {}  // ZPP "O"
#endif
    public function addRange(int $rowFirst, int $rowLast, int $colFirst, int $colLast): bool {}  // ZPP "llll"
    public function addRule(int $type, ExcelConditionalFormat $cf, string $value, bool $stopIfTrue = false): bool {}  // ZPP "lOS|b"
    public function addTopRule(ExcelConditionalFormat $cf, int $value, bool $bottom, bool $percent, bool $stopIfTrue = false): bool {}  // ZPP "Olbb|b"
    public function addOpNumRule(int $op, ExcelConditionalFormat $cf, float $value1, float $value2, bool $stopIfTrue = false): bool {}  // ZPP "lOdd|b"
    public function addOpStrRule(int $op, ExcelConditionalFormat $cf, string $value1, string $value2, bool $stopIfTrue = false): bool {}  // ZPP "lOSS|b"
    public function addAboveAverageRule(ExcelConditionalFormat $cf, bool $above, bool $equal, int $stdDev, bool $stopIfTrue = false): bool {}  // ZPP "Obbl|b"
    public function addTimePeriodRule(ExcelConditionalFormat $cf, int $timePeriod, bool $stopIfTrue = false): bool {}  // ZPP "Ol|b"
    public function add2ColorScaleRule(int $minColor, int $maxColor, int $minType, float $minValue, int $maxType, float $maxValue, bool $stopIfTrue = false): bool {}  // ZPP "llldld|b"
    public function add2ColorScaleFormulaRule(int $minColor, int $maxColor, int $minType, string $minValue, int $maxType, string $maxValue, bool $stopIfTrue = false): bool {}  // ZPP "lllSlS|b"
    public function add3ColorScaleRule(int $minColor, int $midColor, int $maxColor, int $minType, float $minValue, int $midType, float $midValue, int $maxType, float $maxValue, bool $stopIfTrue = false): bool {}  // ZPP "lllldldld|b"
    public function add3ColorScaleFormulaRule(int $minColor, int $midColor, int $maxColor, int $minType, string $minValue, int $midType, string $midValue, int $maxType, string $maxValue, bool $stopIfTrue = false): bool {}  // ZPP "llllSlSlS|b"
}

class ExcelCoreProperties
{
    public function __construct(ExcelBook $book) {}  // ZPP "O"
    /* String getter/setter pairs from COREPROPERTIES_STRING_GETTER /
     * COREPROPERTIES_STRING_SETTER C macros (PE_RETURN_IS_STRING returns
     * NULL when the libxl call returns null, hence ?string). */
    public function title(): ?string {}
    public function setTitle(string $value): bool {}
    public function subject(): ?string {}
    public function setSubject(string $value): bool {}
    public function creator(): ?string {}
    public function setCreator(string $value): bool {}
    public function lastModifiedBy(): ?string {}
    public function setLastModifiedBy(string $value): bool {}
    public function created(): ?string {}
    public function setCreated(string $value): bool {}
    public function modified(): ?string {}
    public function setModified(string $value): bool {}
    public function tags(): ?string {}
    public function setTags(string $value): bool {}
    public function categories(): ?string {}
    public function setCategories(string $value): bool {}
    public function comments(): ?string {}
    public function setComments(string $value): bool {}
    public function createdAsDouble(): mixed {}
    public function setCreatedAsDouble(float $value): bool {}
    public function modifiedAsDouble(): mixed {}
    public function setModifiedAsDouble(float $value): bool {}
    public function removeAll(): mixed {}
}

class ExcelTable
{
    public function __construct(ExcelSheet $sheet, string $name, int $rowFirst, int $rowLast, int $colFirst, int $colLast, bool $hasHeaders = true, int $style = 0) {}  // ZPP "OSllll|bl"
    public function name(): mixed {}  // ZPP ""
    public function setName(string $value): bool {}  // ZPP "S"
    public function ref(): mixed {}  // ZPP ""
    public function setRef(string $value): bool {}  // ZPP "S"
    public function autoFilter(): mixed {}  // ZPP ""
#if LIBXL_VERSION >= 0x05020000
    public function isAutoFilter(): bool {}  // ZPP ""
    public function removeFilter(): bool {}  // ZPP ""
#endif
    public function style(): mixed {}  // ZPP ""
    public function setStyle(int $value): bool {}  // ZPP "l"
    public function showRowStripes(): mixed {}  // ZPP ""
    public function setShowRowStripes(bool $value): bool {}  // ZPP "b"
    public function showColumnStripes(): mixed {}  // ZPP ""
    public function setShowColumnStripes(bool $value): bool {}  // ZPP "b"
    public function showFirstColumn(): mixed {}  // ZPP ""
    public function setShowFirstColumn(bool $value): bool {}  // ZPP "b"
    public function showLastColumn(): mixed {}  // ZPP ""
    public function setShowLastColumn(bool $value): bool {}  // ZPP "b"
    public function columnSize(): mixed {}  // ZPP ""
    public function columnName(int $index): string|null|false {}  // ZPP "l"
    public function setColumnName(int $index, string $name): bool {}  // ZPP "lS"
}

