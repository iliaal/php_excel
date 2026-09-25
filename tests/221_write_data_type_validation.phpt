--TEST--
ExcelSheet write APIs reject unknown data types without modifying cells
--EXTENSIONS--
excel
--FILE--
<?php
$book = new ExcelBook(null, null, true);
$sheet = $book->addSheet('Data');
$sheet->write(1, 0, 'keep');

set_error_handler(static fn(): bool => true);
foreach ([0, 5, 99, -2] as $i => $dtype) {
    $row = 3 + $i;
    var_dump($sheet->write($row, 0, '=1+1', null, $dtype));
    var_dump($sheet->writeRow($row + 10, ['first', 'second'], 0, null, $dtype));
    var_dump($sheet->writeCol(0, ['first', 'second'], $row + 20, null, $dtype));
}
restore_error_handler();

var_dump($sheet->read(1, 0));
foreach ([3, 4, 5, 6] as $row) {
    var_dump($sheet->cellType($row, 0));
    var_dump($sheet->cellType($row, 1));
}
foreach ([23, 24, 25, 26] as $row) {
    var_dump($sheet->cellType($row, 0));
    var_dump($sheet->cellType($row + 1, 0));
}

var_dump($sheet->write(1, 1, '=1+1'));
var_dump($sheet->isFormula(1, 1));
var_dump($sheet->write(1, 2, '=1+1', null, ExcelFormat::AS_TEXT));
var_dump($sheet->isFormula(1, 2));
var_dump($sheet->write(1, 3, '=1+1', null, ExcelFormat::AS_FORMULA));
var_dump($sheet->isFormula(1, 3));
var_dump($sheet->write(1, 4, '123', null, ExcelFormat::AS_NUMERIC_STRING));
var_dump($sheet->cellType(1, 4));
var_dump($sheet->write(1, 5, 1700000000, null, ExcelFormat::AS_DATE));
var_dump($sheet->isDate(1, 5));
echo "OK\n";
?>
--EXPECT--
bool(false)
bool(false)
bool(false)
bool(false)
bool(false)
bool(false)
bool(false)
bool(false)
bool(false)
bool(false)
bool(false)
bool(false)
string(4) "keep"
int(0)
int(0)
int(0)
int(0)
int(0)
int(0)
int(0)
int(0)
int(0)
int(0)
int(0)
int(0)
int(0)
int(0)
int(0)
int(0)
bool(true)
bool(true)
bool(true)
bool(false)
bool(true)
bool(true)
bool(true)
int(1)
bool(true)
bool(true)
OK
