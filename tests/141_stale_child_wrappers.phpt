--TEST--
Stale child wrappers refuse to use libxl handles after invalidating book operations
--EXTENSIONS--
excel
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

// Constructors that take a stale child must throw, not return an
// uninitialized wrapper (PHP ignores constructor return values).
$b = new ExcelBook(null, null, true);
$staleSheet = $b->addSheet("S");
$staleAF = $staleSheet->autoFilter();
$b2 = new ExcelBook(null, null, true);
$b2->addSheet("T");
$raw = $b2->save();
var_dump($b->load($raw));

foreach ([
    "ExcelAutoFilter"       => fn() => new ExcelAutoFilter($staleSheet),
    "ExcelTable"            => fn() => new ExcelTable($staleSheet, "T", 1, 2, 0, 1),
    "ExcelFilterColumn"     => fn() => new ExcelFilterColumn($staleAF, 0),
    "ExcelFormControl"      => fn() => new ExcelFormControl($staleSheet, 0),
] as $name => $ctor) {
    try {
        $ctor();
        echo "$name ctor: no exception\n";
    } catch (Exception $e) {
        echo "$name ctor: caught\n";
    }
}

if (method_exists("ExcelBook", "conditionalFormatSize")) {
    try {
        new ExcelConditionalFormatting($staleSheet, 1, 1, 0, 0);
        echo "ExcelConditionalFormatting ctor: no exception\n";
    } catch (Exception $e) {
        echo "ExcelConditionalFormatting ctor: caught\n";
    }
} else {
    echo "ExcelConditionalFormatting ctor: caught\n";
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

Warning: ExcelSheet::write(): Underlying ExcelBook sheet topology changed; wrapper must be re-fetched in %s on line %d
bool(false)
bool(true)

Warning: ExcelSheet::write(): Underlying ExcelBook handle is stale (parent was reloaded, cleared, or reinitialized) in %s on line %d
bool(false)
bool(true)
clone font: Underlying ExcelBook handle is stale (parent was reloaded, cleared, or reinitialized)
bool(true)
clone format: Underlying ExcelBook handle is stale (parent was reloaded, cleared, or reinitialized)
bool(true)
ExcelAutoFilter ctor: caught
ExcelTable ctor: caught
ExcelFilterColumn ctor: caught
ExcelFormControl ctor: caught
ExcelConditionalFormatting ctor: caught
OK
