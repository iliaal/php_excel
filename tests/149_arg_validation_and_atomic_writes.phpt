--TEST--
Scalar format type checks, stale-arg validation, moveSheet generation, atomic writeRow/writeCol, nullable arginfo
--EXTENSIONS--
excel
--SKIPIF--
<?php
/* Uses 4-arg ExcelSheet::addConditionalFormatting (libxl 5.1.0+) and related CF rule APIs. */
if ((new ReflectionMethod("ExcelSheet", "addConditionalFormatting"))->getNumberOfParameters() < 4) {
	print "skip libxl 5.1.0+ required";
}
?>
--FILE--
<?php
$b = new ExcelBook(null, null, true);
$s = $b->addSheet("S");

// CR-001: scalar format must raise TypeError, not crash
foreach ([
    "setColWidth"  => fn() => $s->setColWidth(0, 5, 10.0, false, "not a format"),
    "setRowHeight" => fn() => $s->setRowHeight(1, 15.0, "not a format"),
] as $name => $call) {
    try { $call(); echo "$name: NO ERROR\n"; }
    catch (TypeError $e) { echo "$name: TypeError\n"; }
}

// CR-002: stale object arguments must be detected before libxl call
$staleSheet = $s;
$staleAF = $staleSheet->autoFilter();
$staleRS = $b->addRichString();
$staleCF = $b->addConditionalFormat();
$cfingForRules = $staleSheet->addConditionalFormatting(1, 3, 0, 0);

$b2 = new ExcelBook(null, null, true);
$b2->addSheet("T");
$raw = $b2->save();
var_dump($b->load($raw));

$s2 = $b->addSheet("S2");
var_dump(@$s2->applyFilter2($staleAF));
var_dump(@$s2->writeRichStr(1, 0, $staleRS));

$cfing2 = $s2->addConditionalFormatting(1, 3, 0, 0);
var_dump(@$cfing2->addRule(1, $staleCF, "A1"));

// CR-003: moveSheet must invalidate sheet wrappers
$b = new ExcelBook(null, null, true);
$a = $b->addSheet("A");
$bs = $b->addSheet("B");
$fmt = $b->addFormat();
$font = $b->addFont();
$fmt->numberFormat(ExcelFormat::NUMFORMAT_TEXT);
echo "before: ", $bs->name(), "\n";
$b->moveSheet(1, 0);
try {
    echo "after: ", @$bs->name(), "\n";
} catch (Throwable $e) {
    echo "after: ", $e->getMessage(), "\n";
}
echo "format after move: "; var_dump($fmt->numberFormat() === ExcelFormat::NUMFORMAT_TEXT);
echo "font after move: "; var_dump(is_int($font->size()));
$b->deleteSheet(0);
echo "format after delete: "; var_dump($fmt->numberFormat() === ExcelFormat::NUMFORMAT_TEXT);
echo "font after delete: "; var_dump(is_int($font->size()));

// CR-004: writeRow / writeCol must reject overflowing run before any write
$xls = new ExcelBook();
$xs = $xls->addSheet("X");
var_dump(@$xs->writeRow(1, ["a", "b"], 255));   // start col 255, 2 cells -> 256 (>255)
echo "cell at start: ";
var_dump($xs->read(1, 255));                    // must still be empty (no partial write)
var_dump(@$xs->writeCol(0, ["a", "b"], 65535)); // start row 65535, 2 cells -> 65536
echo "cell at start: ";
var_dump($xs->read(65535, 0));

// Re-scan CR-001: extreme starts must not trigger signed overflow.
// Validate-start-then-capacity ordering keeps the (max - start) math
// safe under UBSan even for PHP_INT_MIN / PHP_INT_MAX inputs.
var_dump(@$xs->writeRow(1, [], PHP_INT_MIN));
var_dump(@$xs->writeRow(1, ["a", "b"], PHP_INT_MIN));
var_dump(@$xs->writeRow(1, ["a", "b"], PHP_INT_MAX));
var_dump(@$xs->writeCol(0, ["a", "b"], PHP_INT_MIN));
var_dump(@$xs->writeCol(0, ["a", "b"], PHP_INT_MAX));

// Re-scan CR-002: writeCol's first arg is a column, not a row.
$r = new ReflectionMethod(ExcelSheet::class, "writeCol");
echo "writeCol[0]: " . $r->getParameters()[0]->getName() . "\n";

// CR-006: nullable parameters must accept explicit null
$bookN = new ExcelBook(null, null, true);
var_dump($bookN->addFont(null) instanceof ExcelFont);
var_dump($bookN->addFormat(null) instanceof ExcelFormat);
$sheetN = $bookN->addSheet("N");
var_dump($sheetN->writeRow(1, ["x"], 0, null));

echo "OK\n";
?>
--EXPECT--
setColWidth: TypeError
setRowHeight: TypeError
bool(true)
bool(false)
bool(false)
bool(false)
before: B
after: 
format after move: bool(true)
font after move: bool(true)
format after delete: bool(true)
font after delete: bool(true)
bool(false)
cell at start: string(0) ""
bool(false)
cell at start: string(0) ""
bool(false)
bool(false)
bool(false)
bool(false)
bool(false)
writeCol[0]: column
bool(true)
bool(true)
bool(true)
OK
