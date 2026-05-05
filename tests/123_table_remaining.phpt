--TEST--
Table: setRef, setColumnName, showColumnStripes, setShowColumnStripes, showFirstColumn, setShowFirstColumn, showLastColumn, setShowLastColumn
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

$t = $sheet->addTable("T1", 1, 2, 0, 1);

// setRef
var_dump($t->setRef("A1:B5"));
var_dump(is_string($t->ref()));

// setColumnName
var_dump($t->setColumnName(0, "NewName"));
var_dump($t->columnName(0));

// showColumnStripes
var_dump($t->showColumnStripes());
var_dump($t->setShowColumnStripes(true));
var_dump($t->showColumnStripes());
var_dump($t->setShowColumnStripes(false));
var_dump($t->showColumnStripes());

// showFirstColumn
var_dump($t->showFirstColumn());
var_dump($t->setShowFirstColumn(true));
var_dump($t->showFirstColumn());

// showLastColumn
var_dump($t->showLastColumn());
var_dump($t->setShowLastColumn(true));
var_dump($t->showLastColumn());

echo "OK\n";
?>
--EXPECT--
bool(true)
bool(true)
bool(true)
string(7) "NewName"
bool(false)
bool(true)
bool(true)
bool(true)
bool(false)
bool(false)
bool(true)
bool(true)
bool(false)
bool(true)
bool(true)
OK
