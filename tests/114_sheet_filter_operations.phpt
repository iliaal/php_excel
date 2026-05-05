--TEST--
Sheet: applyFilter, removeFilter, applyFilter2, table
--EXTENSIONS--
excel
--FILE--
<?php
$book = new ExcelBook(null, null, true);
$sheet = $book->addSheet("Sheet1");

$sheet->write(1, 0, "Name");
$sheet->write(1, 1, "Value");
$sheet->write(2, 0, "A");
$sheet->write(2, 1, 10);
$sheet->write(3, 0, "B");
$sheet->write(3, 1, 20);

// applyFilter / removeFilter
var_dump($sheet->applyFilter());
var_dump($sheet->removeFilter());

// applyFilter2 with AutoFilter object
$af = $sheet->autoFilter();
$af->setRef(1, 3, 0, 1);
var_dump($sheet->applyFilter2($af));

// Sheet::table - add a table then retrieve info
$t = $sheet->addTable("T1", 1, 3, 0, 1);
$info = $sheet->table(0);
var_dump($info["name"]);
var_dump($info["row_first"]);
var_dump($info["row_last"]);
var_dump($info["col_first"]);
var_dump($info["col_last"]);
var_dump($info["header_row_count"]);
var_dump($info["totals_row_count"]);

// table with invalid index returns false
var_dump($sheet->table(999));

echo "OK\n";
?>
--EXPECT--
bool(true)
bool(true)
bool(true)
string(2) "T1"
int(1)
int(3)
int(0)
int(1)
int(1)
int(0)
bool(false)
OK
