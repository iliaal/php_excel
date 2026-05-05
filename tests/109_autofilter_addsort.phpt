--TEST--
AutoFilter::addSort via Table::autoFilter
--EXTENSIONS--
excel
--FILE--
<?php
$book = new ExcelBook(null, null, true);
$sheet = $book->addSheet("Sheet1");

$sheet->write(1, 0, "Name");
$sheet->write(1, 1, "Value");
$sheet->write(2, 0, "B");
$sheet->write(2, 1, 2);
$sheet->write(3, 0, "A");
$sheet->write(3, 1, 1);

$table = $sheet->addTable("T1", 1, 3, 0, 1);
$af = $table->autoFilter();
var_dump($af instanceof ExcelAutoFilter);

var_dump($af->addSort(0, false));

echo "OK\n";
?>
--EXPECT--
bool(true)
bool(true)
OK
