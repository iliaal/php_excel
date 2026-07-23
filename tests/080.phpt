--TEST--
Sheet::writeCol() data type argument
--EXTENSIONS--
excel
--SKIPIF--
<?php if (!in_array('writeCol', get_class_methods('ExcelSheet'))) print "skip"; ?>
--FILE--
<?php
$book = new ExcelBook();
$sheet = $book->addSheet("Sheet 1");

$time = mktime(5, 0, 0, 1, 21, 1980);

$dateFormat = new ExcelFormat($book);
$dateFormat->numberFormat(ExcelFormat::NUMFORMAT_DATE);

// Explicit dtypes require homogeneous zval types (all-or-nothing).
// Formula / numeric-string: strings only. Date: integers only.
$sheet->writeCol(2, ["2", "3", (string) $time], 2, null, ExcelFormat::AS_FORMULA);
$sheet->writeCol(3, ["2", "3", (string) $time], 2, null, ExcelFormat::AS_NUMERIC_STRING);
$sheet->writeCol(4, [2, 3, $time], 2, $dateFormat, ExcelFormat::AS_DATE);

var_dump(
    $sheet->isFormula(2, 2),
    $sheet->isFormula(3, 2),
    $sheet->isFormula(4, 2),

    $sheet->isDate(2, 2),
    $sheet->isDate(3, 2),
    $sheet->isDate(4, 2),

    $sheet->isFormula(2, 3),
    $sheet->isFormula(3, 3),
    $sheet->isFormula(4, 3),

    $sheet->isDate(2, 3),
    $sheet->isDate(3, 3),
    $sheet->isDate(4, 3),

    $sheet->isFormula(2, 4),
    $sheet->isFormula(3, 4),
    $sheet->isFormula(4, 4),

    $sheet->isDate(2, 4),
    $sheet->isDate(3, 4),
    $sheet->isDate(4, 4)
);

// Mismatched dtype+zval is rejected with no partial write
var_dump(@$sheet->writeCol(5, [2, "3", $time], 2, null, ExcelFormat::AS_FORMULA));
?>
--EXPECT--
bool(true)
bool(true)
bool(true)
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
bool(true)
bool(true)
bool(true)
bool(false)
