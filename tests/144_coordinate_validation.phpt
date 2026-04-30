--TEST--
Coordinate validation rejects out-of-range row/column per book type
--SKIPIF--
<?php if (!extension_loaded("excel")) print "skip"; ?>
--FILE--
<?php
// XLSX: 1048576 rows x 16384 cols
$b = new ExcelBook(null, null, true);
$s = $b->addSheet("S");
$s->write(1, 0, "ok");

// Negative
var_dump(@$s->read(-1, 0));
var_dump(@$s->cellType(-1, 0));
var_dump(@$s->isDate(-1, 0));
var_dump(@$s->isFormula(-1, 0));

// PHP_INT_MAX
var_dump(@$s->read(PHP_INT_MAX, 0));
var_dump(@$s->cellType(PHP_INT_MAX, 0));
var_dump(@$s->isDate(0, PHP_INT_MAX));
var_dump(@$s->isFormula(0, PHP_INT_MAX));

// Just past XLSX limits
var_dump(@$s->read(2000000, 0));
var_dump(@$s->cellType(2000000, 0));
var_dump(@$s->read(0, 100000));
var_dump(@$s->cellType(0, 100000));

// XLSX writes
var_dump(@$s->write(-1, 0, "x"));
var_dump(@$s->write(0, PHP_INT_MAX, "x"));
var_dump(@$s->write(2000000, 0, "x"));

echo "XLS\n";

// XLS: 65536 rows x 256 cols. Reads beyond these limits should also fail.
$b = new ExcelBook();
$s = $b->addSheet("S");

var_dump(@$s->read(70000, 0));
var_dump(@$s->cellType(70000, 0));
var_dump(@$s->read(0, 256));
var_dump(@$s->cellType(0, 256));
var_dump(@$s->write(70000, 0, "x"));
var_dump(@$s->write(0, 256, "x"));

// XLS still accepts in-range coordinates (use row 1+ to stay trial-compatible)
var_dump($s->write(1, 0, "x"));
var_dump($s->write(65535, 255, "x"));

// Row-range / col-range methods must validate against the right axis.
// On an XLS book, row 256 and column 256 are both legal rows but only
// 0..255 are legal columns; conflating them rejects valid row inputs.
var_dump($s->insertRow(256, 256));      // valid XLS rows
var_dump($s->insertRow(65535, 65535));  // valid edge
var_dump(@$s->insertRow(65536, 65536)); // out of range
var_dump($s->insertCol(255, 255));      // valid edge
var_dump(@$s->insertCol(256, 256));     // out of range
var_dump($s->removeRow(1000, 1000));
var_dump(@$s->removeRow(80000, 80000));

// XLSX row-range/col-range
$b2 = new ExcelBook(null, null, true);
$s2 = $b2->addSheet("S");
var_dump($s2->insertRow(16384, 16384));      // valid XLSX rows
var_dump($s2->insertRow(1048575, 1048575));  // valid edge
var_dump(@$s2->insertRow(1048576, 1048576)); // out of range
var_dump($s2->insertCol(16383, 16383));      // valid edge
var_dump(@$s2->insertCol(16384, 16384));     // out of range

echo "OK\n";
?>
--EXPECT--
bool(false)
bool(false)
bool(false)
bool(false)
bool(false)
bool(false)
bool(false)
bool(false)
bool(false)
bool(false)
bool(false)
bool(false)
bool(false)
bool(false)
bool(false)
XLS
bool(false)
bool(false)
bool(false)
bool(false)
bool(false)
bool(false)
bool(true)
bool(true)
bool(true)
bool(true)
bool(false)
bool(true)
bool(false)
bool(true)
bool(false)
bool(true)
bool(true)
bool(false)
bool(true)
bool(false)
OK
