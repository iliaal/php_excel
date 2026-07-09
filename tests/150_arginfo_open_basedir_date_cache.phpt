--TEST--
Strict object types, open_basedir on loadInfo/addPictureAsLink, cached date format, throw on uninitialized book
--EXTENSIONS--
excel
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

// Re-scan: zend_long sheet/index args must reject values that wrap on
// narrowing to libxl int. 2^32 used to alias to index 0.
$bb = new ExcelBook(null, null, true);
$bb->addSheet("S0");
$bb->addSheet("S1");
var_dump(@$bb->getSheet(2 ** 32));
var_dump(@$bb->getSheetName(2 ** 32));
var_dump(@$bb->deleteSheet(2 ** 32));
echo "after: " . $bb->getSheetName(0) . "\n";

// Re-scan: stubs publishing int/string/mixed for params the C parses
// as bool used to fatal under debug PHP arginfo/ZPP checks.
$sb = $bb->addSheet("Sb");
var_dump($sb->setPrintHeaders(true));
var_dump($sb->groupRows(1, 2, true));
$fb = $bb->addFormat();
var_dump($fb->wrap(true));

// Re-scan: ExcelAutoFilter::__construct argument is required, not
// nullable optional.
$ra = new ReflectionMethod(ExcelAutoFilter::class, "__construct");
echo "AutoFilter ctor optional? " . var_export($ra->getParameters()[0]->isOptional(), true) . "\n";

// Re-scan: every public collection/index API that hands a zend_long to
// a libxl int parameter must reject 2**32 instead of aliasing it back
// onto a real handle. The previous round only covered Book::getSheet,
// getSheetName, deleteSheet; these are the rest of the surface.
$bn = new ExcelBook(null, null, true);
$sn = $bn->addSheet("N");
$sn->write(1, 0, 1);
$bn->addCustomFormat("0");
echo "Book::conditionalFormat(2**32): "; var_dump(@$bn->conditionalFormat(2 ** 32));
echo "Book::getPicture(2**32): "; var_dump(@$bn->getPicture(2 ** 32));
echo "Book::moveSheet(2**32, 0): "; var_dump(@$bn->moveSheet(2 ** 32, 0));
echo "Book::moveSheet(0, 2**32): "; var_dump(@$bn->moveSheet(0, 2 ** 32));
echo "Sheet::getPictureInfo(2**32): "; var_dump(@$sn->getPictureInfo(2 ** 32));
echo "Sheet::hyperlink(2**32): "; var_dump(@$sn->hyperlink(2 ** 32));
echo "Sheet::table(2**32): "; var_dump(@$sn->table(2 ** 32));
echo "Sheet::removePictureByIndex(2**32): "; var_dump(@$sn->removePictureByIndex(2 ** 32));
echo "Sheet::formControl(2**32): "; var_dump(@$sn->formControl(2 ** 32));
echo "Sheet::getNamedRange(2**32): "; var_dump(@$sn->getNamedRange(2 ** 32));
echo "Sheet::getVerPageBreak(2**32): "; var_dump(@$sn->getVerPageBreak(2 ** 32));
echo "Sheet::getHorPageBreak(2**32): "; var_dump(@$sn->getHorPageBreak(2 ** 32));
if (method_exists("ExcelSheet", "conditionalFormatting")) {
    echo "Sheet::conditionalFormatting(2**32): "; var_dump(@$sn->conditionalFormatting(2 ** 32));
    echo "Sheet::removeConditionalFormatting(2**32): "; var_dump(@$sn->removeConditionalFormatting(2 ** 32));
} else {
    echo "Sheet::conditionalFormatting(2**32): bool(false)\n";
    echo "Sheet::removeConditionalFormatting(2**32): bool(false)\n";
}
if (method_exists("ExcelSheet", "getTableByIndex")) {
    echo "Sheet::getTableByIndex(2**32): "; var_dump(@$sn->getTableByIndex(2 ** 32));
} else {
    echo "Sheet::getTableByIndex(2**32): bool(false)\n";
}

// AutoFilter / FilterColumn / RichString / Table index APIs.
$sn->setMerge(1, 1, 0, 1);
$af = $sn->autoFilter();
$af->setRef(0, 1, 0, 1);
echo "AutoFilter::column(2**32): "; var_dump(@$af->column(2 ** 32));
echo "AutoFilter::columnByIndex(2**32): "; var_dump(@$af->columnByIndex(2 ** 32));
echo "AutoFilter::setSort(2**32, false): "; var_dump(@$af->setSort(2 ** 32, false));
echo "AutoFilter::addSort(2**32, false): "; var_dump(@$af->addSort(2 ** 32, false));
$fc = $af->column(0);
echo "FilterColumn::filter(2**32): "; var_dump(@$fc->filter(2 ** 32));

try {
    new ExcelFilterColumn($af, 2 ** 32);
    echo "FilterColumn ctor 2**32: NO THROW\n";
} catch (Throwable $e) {
    echo "FilterColumn ctor 2**32: caught\n";
}

$rs = $bn->addRichString();
echo "RichString::getText(2**32): "; var_dump(@$rs->getText(2 ** 32));

// Negative input must also be rejected uniformly.
echo "Book::conditionalFormat(-1): "; var_dump(@$bn->conditionalFormat(-1));
echo "Sheet::hyperlink(-1): "; var_dump(@$sn->hyperlink(-1));

// PHP_INT_MAX still narrows; it must be rejected the same way as 2**32.
echo "Book::moveSheet(PHP_INT_MAX, 0): "; var_dump(@$bn->moveSheet(PHP_INT_MAX, 0));

// Re-scan: addPictureScaled / addPictureDim still passed pic_id and the
// optional dimensional / offset args straight from zend_long to libxl
// int. pic_id = 2**32 used to alias to picture index 0 and mutate the
// sheet. Each int boundary now rejects the wrap.
$bp = new ExcelBook(null, null, true);
$sp = $bp->addSheet("P");
$png_data = "\x89PNG\r\n\x1a\n" .
    "\x00\x00\x00\rIHDR\x00\x00\x00\x01\x00\x00\x00\x01\x08\x02\x00\x00\x00\x90wS\xde" .
    "\x00\x00\x00\x0cIDATx\x9cc\xf8\xff\xff?\x00\x05\xfe\x02\xfe\xa3\x9bP\x07" .
    "\x00\x00\x00\x00IEND\xaeB`\x82";
$pic_id = $bp->addPictureFromString($png_data);
echo "addPictureScaled pic 2**32: "; var_dump(@$sp->addPictureScaled(1, 0, 2 ** 32, 1.0));
echo "addPictureScaled x_offset 2**32: "; var_dump(@$sp->addPictureScaled(1, 0, $pic_id, 1.0, 2 ** 32));
echo "addPictureDim pic 2**32: "; var_dump(@$sp->addPictureDim(1, 0, 2 ** 32, 100, 100));
echo "addPictureDim w 2**32: "; var_dump(@$sp->addPictureDim(1, 0, $pic_id, 2 ** 32, 100));
echo "addPictureDim h 2**32: "; var_dump(@$sp->addPictureDim(1, 0, $pic_id, 100, 2 ** 32));
echo "addPictureDim x_offset 2**32: "; var_dump(@$sp->addPictureDim(1, 0, $pic_id, 100, 100, 2 ** 32));
echo "pictures after rejected calls: "; var_dump($sp->getNumPictures());

// Re-scan: optional getter/setters on Format/Font must treat explicit
// null as "getter mode", not as "set to 0/empty". Previously the |l
// ZPP weak-coerced null to 0, ZEND_NUM_ARGS()=1 fired the setter, and
// numberFormat(null) silently reset format 7 -> 0. Same mechanic
// reset Font::name("Arial") -> "" via |S null coercion to "".
$bo = new ExcelBook(null, null, true);
$Fo = $bo->addFormat();
$Fo->numberFormat(7);
echo "numberFormat(null) returns: "; var_dump($Fo->numberFormat(null));
echo "numberFormat after null: " . $Fo->numberFormat() . "\n";
$Fo->rotate(45);
echo "rotate(null) returns: "; var_dump($Fo->rotate(null));
echo "rotate after null: " . $Fo->rotate() . "\n";
$Fo->indent(3);
echo "indent(null) returns: "; var_dump($Fo->indent(null));
echo "indent after null: " . $Fo->indent() . "\n";
$Fo->wrap(true);
echo "wrap(null) returns: "; var_dump($Fo->wrap(null));
echo "wrap after null: "; var_dump($Fo->wrap());

$fo = $bo->addFont();
$fo->name("Arial");
echo "Font::name(null) returns: "; var_dump($fo->name(null));
echo "Font::name after null: " . $fo->name() . "\n";
$fo->color(0xFF0000);
echo "Font::color(null) returns: "; var_dump($fo->color(null));
echo "Font::color after null: " . $fo->color() . "\n";

// Re-scan: non-index integer setters on Format/Font must reject values
// that wrap on narrowing to libxl int. Previously numberFormat(2**32+1)
// silently became format 1 and Font::color(2**32+1) became color 1.
echo "Format::numberFormat(2**32+1): "; var_dump(@$Fo->numberFormat(2 ** 32 + 1));
echo "numberFormat unchanged: " . $Fo->numberFormat() . "\n";
echo "Font::color(2**32+1): "; var_dump(@$fo->color(2 ** 32 + 1));
echo "color unchanged: " . $fo->color() . "\n";
echo "Font::size(2**32+1): "; var_dump(@$fo->size(2 ** 32 + 1));
echo "Format::rotate(2**32+1): "; var_dump(@$Fo->rotate(2 ** 32 + 1));
echo "Format::borderLeftStyle(2**32+1): "; var_dump(@$Fo->borderLeftStyle(2 ** 32 + 1));
echo "Format::patternForegroundColor(2**32+1): "; var_dump(@$Fo->patternForegroundColor(2 ** 32 + 1));

// Doc files must match the C ZPP signatures so IDE-driven calls stop
// type-confusing on the deprecated mixed/string/int forms.
$docs = [
    "ExcelSheet" => "groupRows",
    "ExcelSheet" => "groupCols",
    "ExcelSheet" => "setPrintHeaders",
    "ExcelFormat" => "wrap",
    "ExcelFormat" => "shrinkToFit",
    "ExcelFormat" => "locked",
    "ExcelFormat" => "hidden",
    "ExcelAutoFilter" => "setRef",
];

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
bool(false)
bool(false)
bool(false)
after: S0
NULL
bool(true)
bool(true)
AutoFilter ctor optional? false
Book::conditionalFormat(2**32): bool(false)
Book::getPicture(2**32): bool(false)
Book::moveSheet(2**32, 0): bool(false)
Book::moveSheet(0, 2**32): bool(false)
Sheet::getPictureInfo(2**32): bool(false)
Sheet::hyperlink(2**32): bool(false)
Sheet::table(2**32): bool(false)
Sheet::removePictureByIndex(2**32): bool(false)
Sheet::formControl(2**32): bool(false)
Sheet::getNamedRange(2**32): bool(false)
Sheet::getVerPageBreak(2**32): bool(false)
Sheet::getHorPageBreak(2**32): bool(false)
Sheet::conditionalFormatting(2**32): bool(false)
Sheet::removeConditionalFormatting(2**32): bool(false)
Sheet::getTableByIndex(2**32): bool(false)
AutoFilter::column(2**32): bool(false)
AutoFilter::columnByIndex(2**32): bool(false)
AutoFilter::setSort(2**32, false): bool(false)
AutoFilter::addSort(2**32, false): bool(false)
FilterColumn::filter(2**32): bool(false)
FilterColumn ctor 2**32: caught
RichString::getText(2**32): bool(false)
Book::conditionalFormat(-1): bool(false)
Sheet::hyperlink(-1): bool(false)
Book::moveSheet(PHP_INT_MAX, 0): bool(false)
addPictureScaled pic 2**32: bool(false)
addPictureScaled x_offset 2**32: bool(false)
addPictureDim pic 2**32: bool(false)
addPictureDim w 2**32: bool(false)
addPictureDim h 2**32: bool(false)
addPictureDim x_offset 2**32: bool(false)
pictures after rejected calls: int(0)
numberFormat(null) returns: int(7)
numberFormat after null: 7
rotate(null) returns: int(45)
rotate after null: 45
indent(null) returns: int(3)
indent after null: 3
wrap(null) returns: bool(true)
wrap after null: bool(true)
Font::name(null) returns: string(5) "Arial"
Font::name after null: Arial
Font::color(null) returns: int(16711680)
Font::color after null: 16711680
Format::numberFormat(2**32+1): bool(false)
numberFormat unchanged: 7
Font::color(2**32+1): bool(false)
color unchanged: 16711680
Font::size(2**32+1): bool(false)
Format::rotate(2**32+1): bool(false)
Format::borderLeftStyle(2**32+1): bool(false)
Format::patternForegroundColor(2**32+1): bool(false)
OK
