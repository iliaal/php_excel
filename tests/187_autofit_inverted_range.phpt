--TEST--
ExcelSheet::setAutoFitArea rejects inverted finite ranges
--EXTENSIONS--
excel
--FILE--
<?php
$book = new ExcelBook(null, null, true);
$sheet = $book->addSheet('S');

var_dump(@$sheet->setAutoFitArea(10, 5, 0, 1));
var_dump(@$sheet->setAutoFitArea(0, 5, 3, 1));
var_dump($sheet->setAutoFitArea(0, -1, 0, -1));
var_dump($sheet->setAutoFitArea(0, 5, 0, 1));
echo "OK\n";
?>
--EXPECT--
bool(false)
bool(false)
bool(true)
bool(true)
OK
