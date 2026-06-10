--TEST--
ExcelFormat::AS_TEXT writes strings verbatim; explicit dtype is not hijacked by the '=' sniff
--EXTENSIONS--
excel
--FILE--
<?php
$b = new ExcelBook(null, null, true);
$s = $b->addSheet("S1");

// default behavior unchanged: leading '=' promotes to a formula
$s->write(1, 0, "=2+3");
var_dump($s->isFormula(1, 0));

// AS_TEXT writes verbatim: no formula promotion
$s->write(2, 0, "=2+3", null, ExcelFormat::AS_TEXT);
var_dump($s->isFormula(2, 0));
var_dump($s->read(2, 0));

// AS_TEXT keeps a leading quote instead of stripping it
$s->write(3, 0, "'quoted", null, ExcelFormat::AS_TEXT);
var_dump($s->read(3, 0));

// AS_TEXT skips numeric coercion
$s->write(4, 0, "123", null, ExcelFormat::AS_TEXT);
var_dump($s->read(4, 0));

// an explicit non-formula dtype is honored, not overridden by the sniff
$s->write(5, 0, "=2+3", null, ExcelFormat::AS_NUMERIC_STRING);
var_dump($s->isFormula(5, 0));
var_dump($s->read(5, 0));

// explicit AS_FORMULA still writes a formula
$s->write(6, 0, "=2+3", null, ExcelFormat::AS_FORMULA);
var_dump($s->isFormula(6, 0));

// writeCol passes the dtype through; default writeCol still promotes
$s->writeCol(1, ["=a", "=b"], 1, null, ExcelFormat::AS_TEXT);
var_dump($s->read(1, 1), $s->isFormula(1, 1), $s->read(2, 1));
$s->writeCol(2, ["=2+3"], 1);
var_dump($s->isFormula(1, 2));

// survives a save/load round-trip as a literal string
$buf = $b->save();
$b2 = new ExcelBook(null, null, true);
$b2->load($buf);
$s2 = $b2->getSheet(0);
var_dump($s2->read(2, 0), $s2->isFormula(2, 0));

echo "OK\n";
?>
--EXPECT--
bool(true)
bool(false)
string(4) "=2+3"
string(7) "'quoted"
string(3) "123"
bool(false)
string(4) "=2+3"
bool(true)
string(2) "=a"
bool(false)
string(2) "=b"
bool(true)
string(4) "=2+3"
bool(false)
OK
