--TEST--
FilterColumn: index, filterType, filterSize, filter, addFilter, clear, getTop10, setTop10, getCustomFilter, setCustomFilter; AutoFilter::columnByIndex
--EXTENSIONS--
excel
--FILE--
<?php
$book = new ExcelBook(null, null, true);
$sheet = $book->addSheet("Test");

$sheet->write(1, 0, "Name");
$sheet->write(1, 1, "Value");
$sheet->write(2, 0, "Alice");
$sheet->write(2, 1, 10);
$sheet->write(3, 0, "Bob");
$sheet->write(3, 1, 20);

$af = $sheet->autoFilter();
$af->setRef(1, 3, 0, 1);

// Get filter column
$fc = $af->column(0);
var_dump($fc instanceof ExcelFilterColumn);
var_dump($fc->index());
var_dump($fc->filterSize());

// addFilter / filter / filterSize
$fc->addFilter("Alice");
var_dump($fc->filterSize());
var_dump($fc->filter(0));

// clear
$fc->clear();
var_dump($fc->filterSize());

// setTop10 / getTop10
$fc->setTop10(10.0, true, false);
var_dump($fc->filterType() === ExcelFilterColumn::FILTER_TOP10);
$top10 = $fc->getTop10();
var_dump($top10['value']);
var_dump($top10['top']);
var_dump($top10['percent']);

// setCustomFilter / getCustomFilter
$fc->clear();
$fc->setCustomFilter(ExcelFilterColumn::OPERATOR_EQUAL, "Bob", ExcelFilterColumn::OPERATOR_EQUAL, "Alice", false);
var_dump($fc->filterType() === ExcelFilterColumn::FILTER_CUSTOM);
$cf = $fc->getCustomFilter();
var_dump($cf['value_1']);
var_dump($cf['value_2']);
var_dump($cf['and_operator']);

// columnByIndex
var_dump($af->columnSize());
$fc2 = $af->columnByIndex(0);
var_dump($fc2 instanceof ExcelFilterColumn);
var_dump($fc2->index());

echo "OK\n";
?>
--EXPECT--
bool(true)
int(0)
int(0)
int(1)
string(5) "Alice"
int(0)
bool(true)
float(10)
bool(true)
bool(false)
bool(true)
string(3) "Bob"
string(5) "Alice"
bool(false)
int(1)
bool(true)
int(0)
OK
