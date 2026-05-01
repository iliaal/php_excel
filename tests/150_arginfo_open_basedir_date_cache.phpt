--TEST--
Strict object types, open_basedir on loadInfo/addPictureAsLink, cached date format, throw on uninitialized book
--SKIPIF--
<?php if (!extension_loaded("excel")) print "skip"; ?>
--FILE--
<?php
$b = new ExcelBook(null, null, true);
$s = $b->addSheet("S");

// CR-001: stdClass cannot impersonate Excel objects. The third argument
// to insertSheet is ?ExcelSheet — passing stdClass must hit the typed
// ZPP boundary, not silently reach FROM_OBJECT.
foreach ([
    "setCellFormat" => fn() => $s->setCellFormat(1, 0, new stdClass),
    "writeError"    => fn() => $s->writeError(1, 0, 1, new stdClass),
    "insertSheet"   => fn() => $b->insertSheet(0, "X", new stdClass),
] as $name => $call) {
    try { $call(); echo "$name: NO TYPEERROR\n"; }
    catch (TypeError $e) { echo "$name: TypeError\n"; }
}

// CR-002: open_basedir is enforced on loadInfo / addPictureAsLink
ini_set("open_basedir", __DIR__);
var_dump(@$b->loadInfo("/etc/hostname"));
var_dump(@$b->addPictureAsLink("/etc/hostname", true));
ini_restore("open_basedir");

// CR-003: date writes without explicit format share one cached format
$bd = new ExcelBook(null, null, true);
$sd = $bd->addSheet("D");
$before = count($bd->getAllFormats() ?: []);
for ($i = 0; $i < 5; $i++) {
    $sd->write(1 + $i, 0, time(), null, ExcelFormat::AS_DATE);
}
$after = count($bd->getAllFormats() ?: []);
echo "format delta: " . ($after - $before) . "\n";

// Re-scan: the cached default_date_format must be cleared on book-state
// resets. Otherwise post-reset AS_DATE writes reuse a freed handle and
// the cell records as a raw double instead of a date. Use __construct
// reuse to drive the reset; load() and clear() take the same code path
// (php_excel_book_bump_generation), but exercising them here triggers
// pre-existing libxl-internal leaks that LSan flags during full-suite
// runs.
$bd->__construct(null, null, true);
$sd2 = $bd->addSheet("D2");
$sd2->write(1, 0, time(), null, ExcelFormat::AS_DATE);
echo "isDate after __construct reuse: " . var_export($sd2->isDate(1, 0), true) . "\n";

// CR-004: reflection defaults match C
$r = new ReflectionMethod(ExcelSheet::class, "readRow");
foreach ($r->getParameters() as $p) {
    if ($p->isDefaultValueAvailable()) {
        echo "readRow \$" . $p->getName() . " = " . var_export($p->getDefaultValue(), true) . "\n";
    }
}

// CR-005: book-backed constructors throw on uninitialized book instead of
// returning an unusable child wrapper
$rc = new ReflectionClass(ExcelBook::class);
$uninit = $rc->newInstanceWithoutConstructor();
foreach ([
    "ExcelFormat" => fn() => new ExcelFormat($uninit),
    "ExcelFont"   => fn() => new ExcelFont($uninit),
    "ExcelSheet"  => fn() => new ExcelSheet($uninit, "x"),
] as $name => $call) {
    try { $call(); echo "$name: NO THROW\n"; }
    catch (Throwable $e) { echo "$name: caught\n"; }
}

echo "OK\n";
?>
--EXPECT--
setCellFormat: TypeError
writeError: TypeError
insertSheet: TypeError
bool(false)
bool(false)
format delta: 1
isDate after __construct reuse: true
readRow $start_col = 0
readRow $end_column = -1
readRow $read_formula = true
ExcelFormat: caught
ExcelFont: caught
ExcelSheet: caught
OK
