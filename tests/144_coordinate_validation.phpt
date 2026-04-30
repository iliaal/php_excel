--TEST--
Coordinate validation rejects negative and oversized row/column on read paths
--SKIPIF--
<?php if (!extension_loaded("excel")) print "skip"; ?>
--FILE--
<?php
$b = new ExcelBook(null, null, true);
$s = $b->addSheet("S");
$s->write(1, 0, "ok");

// Negative
var_dump(@$s->read(-1, 0));
var_dump(@$s->cellType(-1, 0));
var_dump(@$s->isDate(-1, 0));
var_dump(@$s->isFormula(-1, 0));

// Oversized vs INT_MAX
var_dump(@$s->read(PHP_INT_MAX, 0));
var_dump(@$s->cellType(PHP_INT_MAX, 0));
var_dump(@$s->isDate(0, PHP_INT_MAX));
var_dump(@$s->isFormula(0, PHP_INT_MAX));

// Just past Excel limits — read paths must also reject these (libxl
// silently returns empty otherwise).
var_dump(@$s->read(2000000, 0));
var_dump(@$s->cellType(2000000, 0));
var_dump(@$s->read(0, 100000));
var_dump(@$s->cellType(0, 100000));

// Write paths
var_dump(@$s->write(-1, 0, "x"));
var_dump(@$s->write(0, PHP_INT_MAX, "x"));
var_dump(@$s->write(2000000, 0, "x"));

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
OK
