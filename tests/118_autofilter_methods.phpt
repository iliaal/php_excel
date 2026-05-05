--TEST--
AutoFilter: getRef, setRef, getSort, setSort, getSortRange, columnSize
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

$af = $sheet->autoFilter();
var_dump($af instanceof ExcelAutoFilter);

// setRef / getRef
$af->setRef(1, 3, 0, 1);
$ref = $af->getRef();
var_dump($ref["row_first"]);
var_dump($ref["col_first"]);

// setSort / getSort
var_dump($af->setSort(0, false));
$sort = $af->getSort();
var_dump($sort["column_index"]);
var_dump($sort["descending"]);

var_dump($af->setSort(1, true));
$sort2 = $af->getSort();
var_dump($sort2["column_index"]);
var_dump($sort2["descending"]);

// getSortRange
$sr = $af->getSortRange();
var_dump(is_array($sr));
var_dump(isset($sr["row_first"]));

// columnSize (no columns with filters set yet via column())
var_dump($af->columnSize());

echo "OK\n";
?>
--EXPECT--
bool(true)
int(1)
int(3)
bool(true)
int(0)
int(0)
bool(true)
int(1)
int(1)
bool(true)
bool(true)
int(0)
OK
