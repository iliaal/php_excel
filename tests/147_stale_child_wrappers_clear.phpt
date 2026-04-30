--TEST--
Book::clear() invalidates existing child wrappers (libxl 5.1.0+)
--SKIPIF--
<?php if (!extension_loaded("excel") || !method_exists("ExcelBook", "clear")) print "skip"; ?>
--FILE--
<?php
$b = new ExcelBook(null, null, true);
$s = $b->addSheet("S");
$f = $b->addFormat();

var_dump($s->write(1, 0, "before-clear"));
var_dump($b->clear());
var_dump($s->write(1, 0, "after-clear"));
var_dump($f->numberFormat());

echo "OK\n";
?>
--EXPECTF--
bool(true)
bool(true)

Warning: ExcelSheet::write(): Underlying ExcelBook handle is stale (parent was reloaded, cleared, or reinitialized) in %s on line %d
bool(false)

Warning: ExcelFormat::numberFormat(): Underlying ExcelBook handle is stale (parent was reloaded, cleared, or reinitialized) in %s on line %d
bool(false)
OK
