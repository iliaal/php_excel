--TEST--
Table::isAutoFilter and Table::removeFilter (libxl 5.2.0+)
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

echo "OK\n";
?>
--EXPECT--
isAutoFilter (initial): bool(true)
removeFilter (): bool(true)
isAutoFilter (removed): bool(false)
OK
