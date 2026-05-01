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
// resets, but NOT on sheet-index shifts where libxl preserves the
// format table. __construct reuse exercises the reset path; moveSheet
// exercises the preserved-cache path. load()/clear() share the reset
// path but trigger pre-existing libxl-internal LSan reports on full
// suite runs, so cover them via __construct reuse here.
$bd->__construct(null, null, true);
$sd2 = $bd->addSheet("D2");
$sd2->write(1, 0, time(), null, ExcelFormat::AS_DATE);
echo "isDate after __construct reuse: " . var_export($sd2->isDate(1, 0), true) . "\n";

// moveSheet must preserve the cache.
$bm = new ExcelBook(null, null, true);
$am = $bm->addSheet("A");
$bm->addSheet("B");
$am->write(1, 0, time(), null, ExcelFormat::AS_DATE);
$baseM = count($bm->getAllFormats() ?: []);
$bm->moveSheet(1, 0);
$bm->moveSheet(0, 1);
$sM = $bm->getSheet(1);
$sM->write(1, 0, time(), null, ExcelFormat::AS_DATE);
$sM->write(2, 0, time(), null, ExcelFormat::AS_DATE);
echo "format delta across 2 moveSheet + 2 date writes: " . (count($bm->getAllFormats() ?: []) - $baseM) . "\n";

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

// Re-scan: addDataValidationDouble must initialize the optional val_2
// before passing it to libxl on the non-(NOT)BETWEEN branch — otherwise
// we feed garbage into the unused slot.
$bv = new ExcelBook(null, null, true);
$sv = $bv->addSheet("V");
var_dump($sv->addDataValidationDouble(0, ExcelSheet::VALIDATION_OP_EQUAL, 1, 1, 0, 0, 5.0));

// Standard `d` coercion semantics on val_2: numeric strings, ints,
// bools must coerce just like val_1 (which is a plain `d`). Arrays/
// objects must TypeError. Only null counts as "not supplied".
echo "val_2 numstring BETWEEN: ";
var_dump($sv->addDataValidationDouble(0, ExcelSheet::VALIDATION_OP_BETWEEN, 10, 10, 0, 0, "1.5", "10.5"));
echo "val_2 int BETWEEN: ";
var_dump($sv->addDataValidationDouble(0, ExcelSheet::VALIDATION_OP_BETWEEN, 11, 11, 0, 0, 1, 10));
echo "val_2 array TypeError: ";
try { $sv->addDataValidationDouble(0, ExcelSheet::VALIDATION_OP_BETWEEN, 12, 12, 0, 0, 5.0, []); echo "no err\n"; }
catch (TypeError $e) { echo "yes\n"; }

// Reflection-driven callers replaying getDefaultValue() must not bypass
// the BETWEEN/NOT-BETWEEN second-value guard. Stub default for $val_2
// is now `null`, treated as "not supplied".
foreach (["addDataValidation", "addDataValidationDouble"] as $m) {
    $r = new ReflectionMethod(ExcelSheet::class, $m);
    $is_double = ($m === "addDataValidationDouble");
    $args = [0, ExcelSheet::VALIDATION_OP_BETWEEN, 1, 1, 0, 0, $is_double ? 5.0 : "5"];
    foreach ($r->getParameters() as $i => $p) {
        if ($i < 7) continue;
        if ($p->isDefaultValueAvailable()) $args[] = $p->getDefaultValue();
    }
    echo "$m reflected BETWEEN: ";
    var_dump(@$r->invokeArgs($sv, $args));
}

// Re-scan: addDataValidation defaults in the stub must match C runtime
// initialisation (allow_blank=true, show_*=true, error_style=1).
$rd = new ReflectionMethod(ExcelSheet::class, "addDataValidation");
foreach ($rd->getParameters() as $p) {
    if (in_array($p->getName(), ["allow_blank", "show_inputmessage", "show_errormessage", "error_style"])) {
        echo $p->getName() . "=" . var_export($p->getDefaultValue(), true) . "\n";
    }
}

// Re-scan: activeSheet must default to the getter-mode sentinel (-1)
// so calling it with no args doesn't reset the active sheet to 0.
$ra = new ReflectionMethod(ExcelBook::class, "activeSheet");
echo "activeSheet \$sheet=" . var_export($ra->getParameters()[0]->getDefaultValue(), true) . "\n";

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
format delta across 2 moveSheet + 2 date writes: 0
readRow $start_col = 0
readRow $end_column = -1
readRow $read_formula = true
ExcelFormat: caught
ExcelFont: caught
ExcelSheet: caught
bool(true)
val_2 numstring BETWEEN: bool(true)
val_2 int BETWEEN: bool(true)
val_2 array TypeError: yes
addDataValidation reflected BETWEEN: bool(false)
addDataValidationDouble reflected BETWEEN: bool(false)
allow_blank=true
show_inputmessage=true
show_errormessage=true
error_style=1
activeSheet $sheet=-1
OK
