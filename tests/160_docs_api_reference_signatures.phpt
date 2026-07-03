--TEST--
Docs API reference signatures match corrected public signatures
--EXTENSIONS--
excel
--FILE--
<?php
$checks = [
    ["ExcelBook.php", "public function save(string \$filename = ''): string|true|false"],
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
