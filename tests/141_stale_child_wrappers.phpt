--TEST--
Stale child wrappers refuse to use libxl handles after invalidating book operations
--SKIPIF--
<?php if (!extension_loaded("excel")) print "skip"; ?>
--FILE--
<?php
// load() invalidates existing sheet wrappers
$b = new ExcelBook(null, null, true);
$s = $b->addSheet("S");
var_dump($s->write(1, 0, "before"));

$b2 = new ExcelBook(null, null, true);
$b2->addSheet("T");
$raw = $b2->save();

var_dump($b->load($raw));
var_dump($s->write(1, 0, "after-load"));

// load() invalidates existing format wrappers
$b = new ExcelBook(null, null, true);
$f = $b->addFormat();

$b2 = new ExcelBook(null, null, true);
$b2->addSheet("T");
$raw = $b2->save();

var_dump($b->load($raw));
var_dump($f->numberFormat());

// clear() invalidates existing sheet wrappers
$b = new ExcelBook(null, null, true);
$s = $b->addSheet("S");
var_dump($s->write(1, 0, "before-clear"));
var_dump($b->clear());
var_dump($s->write(1, 0, "after-clear"));

// __construct reuse invalidates existing children
$b = new ExcelBook(null, null, true);
$s = $b->addSheet("S");
var_dump($s->write(1, 0, "before-reconstruct"));
$b->__construct(null, null, true);
var_dump($s->write(1, 0, "after-reconstruct"));

echo "OK\n";
?>
--EXPECTF--
bool(true)
bool(true)

Warning: ExcelSheet::write(): Underlying ExcelBook handle is stale (parent was reloaded, cleared, or reinitialized) in %s on line %d
bool(false)
bool(true)

Warning: ExcelFormat::numberFormat(): Underlying ExcelBook handle is stale (parent was reloaded, cleared, or reinitialized) in %s on line %d
bool(false)
bool(true)
bool(true)

Warning: ExcelSheet::write(): Underlying ExcelBook handle is stale (parent was reloaded, cleared, or reinitialized) in %s on line %d
bool(false)
bool(true)

Warning: ExcelSheet::write(): Underlying ExcelBook handle is stale (parent was reloaded, cleared, or reinitialized) in %s on line %d
bool(false)
OK
