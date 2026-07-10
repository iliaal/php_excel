--TEST--
Docs API reference signatures match corrected public signatures
--EXTENSIONS--
excel
--FILE--
<?php
$checks = [
    ["ExcelBook.php", "public function save(string \$filename = ''): string|bool"],
    ["ExcelBook.php", "public function loadPartially(string \$data, int \$sheet_index, int \$row_first, int \$row_last, bool \$keep_all_sheets = false): bool"],
    ["ExcelBook.php", "public function loadFilePartially(string \$filename, int \$sheet_index, int \$row_first, int \$row_last, bool \$keep_all_sheets = false): bool"],
    ["ExcelBook.php", "public function loadFileWithoutEmptyCells(string \$filename): bool"],
    ["ExcelBook.php", "public function insertSheet(int \$index, string \$name, ?ExcelSheet \$sheet = null): ExcelSheet|false"],
    ["ExcelSheet.php", "public function readRange(int \$row_start, int \$row_end, int \$col_start, int \$col_end, bool \$read_formula = true): array|false"],
    ["ExcelSheet.php", "public function readSparseRow(int \$row, int \$start_col = 0, int \$end_column = -1, bool \$read_formula = true): array|false"],
    ["ExcelSheet.php", "public function readSparseCol(int \$column, int \$start_row = 0, int \$end_row = -1, bool \$read_formula = true): array|false"],
    ["ExcelSheet.php", "public function writeRow(int \$row, array \$data, int \$start_column = 0, ?ExcelFormat \$format = null, int \$data_type = -1): bool"],
    ["ExcelSheet.php", "public function setNamedRange(string \$name, int \$row_first, int \$row_last, int \$col_first, int \$col_last, int \$scope_id = -1): bool"],
    ["ExcelSheet.php", "public function delNamedRange(string \$name, int \$scope_id = -1): bool"],
    ["ExcelSheet.php", "public function getNamedRange(string \$name, int \$scope_id = -1): array|false"],
    ["ExcelFilterColumn.php", "public function setCustomFilter(int \$operator_1, string \$value_1, int \$operator_2 = -1, ?string \$value_2 = null, bool \$andOp = false): bool"],
    // AutoFilter::setRef arg order is (row_first, row_last, col_first, col_last)
    ["ExcelAutoFilter.php", "public function setRef(int \$row_first, int \$row_last, int \$col_first, int \$col_last): mixed"],
    // insert/remove row/col take exactly two ints (no phantom $update_named_ranges)
    ["ExcelSheet.php", "public function insertRow(int \$row_first, int \$row_last): bool"],
    ["ExcelSheet.php", "public function insertCol(int \$col_first, int \$col_last): bool"],
    ["ExcelSheet.php", "public function removeRow(int \$row_first, int \$row_last): bool"],
    ["ExcelSheet.php", "public function removeCol(int \$col_first, int \$col_last): bool"],
    // ConditionalFormatting rule methods: $stopIfTrue is bool, values are required and typed
    ["ExcelConditionalFormatting.php", "public function addRule(int \$type, ExcelConditionalFormat \$cf, string \$value, bool \$stopIfTrue = false): bool"],
    ["ExcelConditionalFormatting.php", "public function addTopRule(ExcelConditionalFormat \$cf, int \$value, bool \$bottom, bool \$percent, bool \$stopIfTrue = false): bool"],
    ["ExcelConditionalFormatting.php", "public function addOpNumRule(int \$op, ExcelConditionalFormat \$cf, float \$value1, float \$value2, bool \$stopIfTrue = false): bool"],
    ["ExcelConditionalFormatting.php", "public function addOpStrRule(int \$op, ExcelConditionalFormat \$cf, string \$value1, string \$value2, bool \$stopIfTrue = false): bool"],
    ["ExcelConditionalFormatting.php", "public function addAboveAverageRule(ExcelConditionalFormat \$cf, bool \$above, bool \$equal, int \$stdDev, bool \$stopIfTrue = false): bool"],
    ["ExcelConditionalFormatting.php", "public function addTimePeriodRule(ExcelConditionalFormat \$cf, int \$timePeriod, bool \$stopIfTrue = false): bool"],
    // removeComment is implemented and now documented (was doc-lagged)
    ["ExcelSheet.php", "public function removeComment(int \$row, int \$col): mixed"],
    // addConditionalFormatting takes required ints (no null defaults) on libxl 5.1+
    ["ExcelSheet.php", "public function addConditionalFormatting(int \$rowFirst, int \$rowLast, int \$colFirst, int \$colLast): ExcelConditionalFormatting|false"],
    // dataValidationSize returns int|false, matching the stub
    ["ExcelSheet.php", "public function dataValidationSize(): int|false"],
    // CoreProperties string getters return string|null|false, matching the stub
    ["ExcelCoreProperties.php", "public function title(): string|null|false"],
    ["ExcelCoreProperties.php", "public function categories(): string|null|false"],
];

foreach ($checks as [$file, $needle]) {
    $doc = file_get_contents(__DIR__ . "/../docs/" . $file);
    echo $file . ": " . (str_contains($doc, $needle) ? "OK" : "MISSING") . "\n";
}
?>
--EXPECT--
ExcelBook.php: OK
ExcelBook.php: OK
ExcelBook.php: OK
ExcelBook.php: OK
ExcelBook.php: OK
ExcelSheet.php: OK
ExcelSheet.php: OK
ExcelSheet.php: OK
ExcelSheet.php: OK
ExcelSheet.php: OK
ExcelSheet.php: OK
ExcelSheet.php: OK
ExcelFilterColumn.php: OK
ExcelAutoFilter.php: OK
ExcelSheet.php: OK
ExcelSheet.php: OK
ExcelSheet.php: OK
ExcelSheet.php: OK
ExcelConditionalFormatting.php: OK
ExcelConditionalFormatting.php: OK
ExcelConditionalFormatting.php: OK
ExcelConditionalFormatting.php: OK
ExcelConditionalFormatting.php: OK
ExcelConditionalFormatting.php: OK
ExcelSheet.php: OK
ExcelSheet.php: OK
ExcelSheet.php: OK
ExcelCoreProperties.php: OK
ExcelCoreProperties.php: OK
