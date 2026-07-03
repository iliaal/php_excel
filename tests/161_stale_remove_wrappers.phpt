--TEST--
Remove APIs invalidate stale child wrappers without invalidating owners
--EXTENSIONS--
excel
--SKIPIF--
<?php
if (!method_exists("ExcelTable", "removeFilter") || !method_exists("ExcelSheet", "removeConditionalFormatting")) {
    print "skip libxl 5.2.0+ required";
}
?>
--FILE--
<?php
function dump_result(string $label, mixed $value): void {
    echo $label . ": ";
    var_dump($value);
}

$book = new ExcelBook(null, null, true);
$sheet = $book->addSheet("SheetFilter");
$sheet->write(1, 0, "Name");
$sheet->write(2, 0, "A");

$af = $sheet->autoFilter();
$af->setRef(1, 2, 0, 0);
$fc = $af->column(0);

dump_result("sheet removeFilter", $sheet->removeFilter());
dump_result("stale sheet autofilter", @$af->getRef());
dump_result("stale sheet filtercolumn", @$fc->index());
dump_result("sheet after removeFilter", $sheet->write(3, 0, "still usable"));
dump_result("new sheet autofilter", $sheet->autoFilter() instanceof ExcelAutoFilter);

$book = new ExcelBook(null, null, true);
$sheet = $book->addSheet("TableFilter");
$sheet->write(1, 0, "H1");
$sheet->write(1, 1, "H2");
$sheet->write(2, 0, "a");
$sheet->write(2, 1, "b");

$table = new ExcelTable($sheet, "T1", 1, 2, 0, 1, true, 0);
$af = $table->autoFilter();
$af->setRef(1, 2, 0, 1);
$fc = $af->column(0);

dump_result("table removeFilter", $table->removeFilter());
dump_result("stale table autofilter", @$af->getRef());
dump_result("stale table filtercolumn", @$fc->index());
dump_result("table after removeFilter", $table->isAutoFilter());
dump_result("sheet after table removeFilter", $sheet->write(3, 0, "still usable"));

$book = new ExcelBook(null, null, true);
$sheet = $book->addSheet("CF");
$sheet->write(1, 0, 10);

$cfing = $sheet->addConditionalFormatting(1, 1, 0, 0);
$cf = $book->addConditionalFormat();
$cf->setFillPattern(1);
$cfing->addRule(1, $cf, "5");

dump_result("removeConditionalFormatting", $sheet->removeConditionalFormatting(0));
dump_result("stale conditional formatting", @$cfing->addRange(2, 2, 0, 0));
dump_result("sheet after removeConditionalFormatting", $sheet->write(2, 0, 20));
dump_result("new conditional formatting", $sheet->addConditionalFormatting(2, 2, 0, 0) instanceof ExcelConditionalFormatting);

echo "OK\n";
?>
--EXPECT--
sheet removeFilter: bool(true)
stale sheet autofilter: bool(false)
stale sheet filtercolumn: bool(false)
sheet after removeFilter: bool(true)
new sheet autofilter: bool(true)
table removeFilter: bool(true)
stale table autofilter: bool(false)
stale table filtercolumn: bool(false)
table after removeFilter: bool(false)
sheet after table removeFilter: bool(true)
removeConditionalFormatting: bool(true)
stale conditional formatting: bool(false)
sheet after removeConditionalFormatting: bool(true)
new conditional formatting: bool(true)
OK
