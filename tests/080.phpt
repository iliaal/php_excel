--TEST--
Sheet::writeCol() data type argument
--INI--
date.timezone=America/Toronto
--SKIPIF--
<?php
    if (!extension_loaded("excel")) die("skip - Excel extension not found");
    if (!in_array('writeCol', get_class_methods('ExcelSheet'))) die("skip - ExcelSheet::writeCol() missing");
?>
--FILE--
<?php
$book = new ExcelBook();
$sheet = $book->addSheet("Sheet 1");

$time = mktime(5, 0, 0, 1, 21, 1980);
$data = [2, "3", $time];

$dateFormat = new ExcelFormat($book);
$dateFormat->numberFormat(ExcelFormat::NUMFORMAT_DATE);

$sheet->writeCol(2, $data, 2, null, ExcelFormat::AS_FORMULA);
$sheet->writeCol(3, $data, 2, null, ExcelFormat::AS_NUMERIC_STRING);
$sheet->writeCol(4, $data, 2, $dateFormat, ExcelFormat::AS_DATE);

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

$sheet->writeCol(2, array_map(function($item){return (string) $item;}, $data), 2, null, ExcelFormat::AS_FORMULA);
$sheet->writeCol(4, array_map(function($item){return (int) $item;}, $data), 2, $dateFormat, ExcelFormat::AS_DATE);

var_dump(
    $sheet->isFormula(2, 2),
    $sheet->isFormula(3, 2),
    $sheet->isFormula(4, 2),
    
    $sheet->isDate(2, 2),
    $sheet->isDate(3, 2),
    $sheet->isDate(4, 2),
    
    $sheet->isFormula(2, 4),
    $sheet->isFormula(3, 4),
    $sheet->isFormula(4, 4),
    
    $sheet->isDate(2, 4),
    $sheet->isDate(3, 4),
    $sheet->isDate(4, 4)
);
?>
--EXPECT--
bool(false)
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
bool(false)
bool(true)
bool(false)
bool(true)
bool(true)
bool(true)
bool(true)
bool(false)
bool(false)
bool(false)
bool(false)
bool(false)
bool(false)
bool(true)
bool(true)
bool(true)
