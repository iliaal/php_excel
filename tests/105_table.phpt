--TEST--
ExcelTable: addTable, name, setName, ref, columnSize, columnName, style, showRowStripes, getTableByName, getTableByIndex (xlsx only)
--EXTENSIONS--
excel
--FILE--
<?php
$book = new ExcelBook(null, null, true);
$sheet = $book->addSheet("Sheet1");

$sheet->write(1, 0, "Name");
$sheet->write(1, 1, "Value");
$sheet->write(2, 0, "A");
$sheet->write(2, 1, 1);
$sheet->write(3, 0, "B");
$sheet->write(3, 1, 2);

$table = $sheet->addTable("TestTable", 1, 3, 0, 1);
var_dump($table instanceof ExcelTable);

var_dump($table->name());
var_dump($table->setName("RenamedTable"));
var_dump($table->name());

$ref = $table->ref();
var_dump(is_string($ref));

var_dump($table->columnSize());
var_dump($table->columnName(0));
var_dump($table->columnName(1));

var_dump($table->style());
var_dump($table->setStyle(1));

var_dump($table->showRowStripes());
var_dump($table->setShowRowStripes(false));
var_dump($table->showRowStripes());
var_dump($table->setShowRowStripes(true));
var_dump($table->showRowStripes());

$t2 = $sheet->getTableByName("RenamedTable");
var_dump($t2 instanceof ExcelTable);
var_dump($t2->name());

$t3 = $sheet->getTableByIndex(0);
var_dump($t3 instanceof ExcelTable);
var_dump($t3->name());

echo "OK\n";
?>
--EXPECT--
bool(true)
string(9) "TestTable"
bool(true)
string(12) "RenamedTable"
bool(true)
int(2)
string(4) "Name"
string(5) "Value"
int(0)
bool(true)
bool(true)
bool(true)
bool(false)
bool(true)
bool(true)
bool(true)
string(12) "RenamedTable"
bool(true)
string(12) "RenamedTable"
OK
