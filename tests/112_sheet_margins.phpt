--TEST--
Sheet: marginLeft/Right/Top/Bottom and setMarginLeft/Right/Top/Bottom
--EXTENSIONS--
excel
--FILE--
<?php
$book = new ExcelBook(null, null, true);
$sheet = $book->addSheet("Sheet1");

// defaults are floats
var_dump(is_float($sheet->marginLeft()));
var_dump(is_float($sheet->marginRight()));
var_dump(is_float($sheet->marginTop()));
var_dump(is_float($sheet->marginBottom()));

// set and verify
$sheet->setMarginLeft(1.5);
$sheet->setMarginRight(2.0);
$sheet->setMarginTop(0.5);
$sheet->setMarginBottom(0.75);
var_dump($sheet->marginLeft());
var_dump($sheet->marginRight());
var_dump($sheet->marginTop());
var_dump($sheet->marginBottom());

// set to zero
$sheet->setMarginLeft(0.0);
var_dump($sheet->marginLeft());

echo "OK\n";
?>
--EXPECT--
bool(true)
bool(true)
bool(true)
bool(true)
float(1.5)
float(2)
float(0.5)
float(0.75)
float(0)
OK
