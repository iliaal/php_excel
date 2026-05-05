--TEST--
Sheet pixel-based sizing: setColPx, setRowPx, colWidthPx, rowHeightPx
--EXTENSIONS--
excel
--FILE--
<?php
$book = new ExcelBook(null, null, true);
$sheet = $book->addSheet("Sheet1");

var_dump($sheet->setColPx(0, 0, 150));
$w = $sheet->colWidthPx(0);
var_dump($w);

var_dump($sheet->setRowPx(1, 40));
$h = $sheet->rowHeightPx(1);
var_dump($h);

echo "OK\n";
?>
--EXPECT--
bool(true)
int(150)
bool(true)
int(40)
OK
