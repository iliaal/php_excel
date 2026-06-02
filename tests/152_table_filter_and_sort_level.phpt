--TEST--
Table::isAutoFilter, Table::removeFilter and AutoFilter::getSort($level) (libxl 5.2.0+)
--EXTENSIONS--
excel
--SKIPIF--
<?php if (!method_exists("ExcelTable", "isAutoFilter")) print "skip libxl 5.2.0+ required"; ?>
--FILE--
<?php
$book = new ExcelBook(null, null, true);
$sheet = $book->addSheet("S");
$sheet->write(1, 0, "H1"); $sheet->write(1, 1, "H2");
$sheet->write(2, 0, "a");  $sheet->write(2, 1, "b");
$sheet->write(3, 0, "c");  $sheet->write(3, 1, "d");

$table = new ExcelTable($sheet, "T1", 1, 3, 0, 1, true, 0);

// A header table carries an autofilter by default.
echo "isAutoFilter (initial): "; var_dump($table->isAutoFilter());
echo "removeFilter (): ";        var_dump($table->removeFilter());
echo "isAutoFilter (removed): "; var_dump($table->isAutoFilter());

// AutoFilter::getSort() gained an optional sort level in 5.2.
$af = $sheet->autoFilter();
$af->setRef(1, 3, 0, 1);
var_dump($af->setSort(0, false));
$sort = $af->getSort(0);
echo "getSort(0) column_index: " . $sort["column_index"] . "\n";
echo "getSort(0) descending: "   . $sort["descending"]   . "\n";
echo "getSort(5) (no such level): "; var_dump($af->getSort(5));

echo "OK\n";
?>
--EXPECT--
isAutoFilter (initial): bool(true)
removeFilter (): bool(true)
isAutoFilter (removed): bool(false)
bool(true)
getSort(0) column_index: 0
getSort(0) descending: 0
getSort(5) (no such level): bool(false)
OK
