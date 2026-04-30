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

// deleteSheet() invalidates existing sheet wrappers
$b = new ExcelBook(null, null, true);
$s = $b->addSheet("S");
$b->addSheet("keepalive"); // libxl needs at least one sheet remaining
var_dump($s->write(1, 0, "before-delete"));
var_dump($b->deleteSheet(0));
var_dump($s->write(1, 0, "after-delete"));

// __construct reuse invalidates existing children
$b = new ExcelBook(null, null, true);
$s = $b->addSheet("S");
var_dump($s->write(1, 0, "before-reconstruct"));
$b->__construct(null, null, true);
var_dump($s->write(1, 0, "after-reconstruct"));

// Stale ExcelFont clone after load() throws
$b = new ExcelBook(null, null, true);
$font = $b->addFont();
$b2 = new ExcelBook(null, null, true);
$b2->addSheet("T");
$raw = $b2->save();
var_dump($b->load($raw));
try {
    $c = clone $font;
    echo "clone font: no exception\n";
} catch (Exception $e) {
    echo "clone font: " . $e->getMessage() . "\n";
}

// Stale ExcelFormat clone after load() throws
$b = new ExcelBook(null, null, true);
$fmt = $b->addFormat();
$b2 = new ExcelBook(null, null, true);
$b2->addSheet("T");
$raw = $b2->save();
var_dump($b->load($raw));
try {
    $c = clone $fmt;
    echo "clone format: no exception\n";
} catch (Exception $e) {
    echo "clone format: " . $e->getMessage() . "\n";
}

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
bool(true)
clone font: Underlying ExcelBook handle is stale (parent was reloaded, cleared, or reinitialized)
bool(true)
clone format: Underlying ExcelBook handle is stale (parent was reloaded, cleared, or reinitialized)
OK
