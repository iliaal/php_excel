--TEST--
Sheet::write/writeRow/writeCol return false for unsupported zval types
--EXTENSIONS--
excel
--FILE--
<?php
$b = new ExcelBook(null, null, true);
$s = $b->addSheet("S");

// Array
var_dump(@$s->write(1, 0, [1, 2, 3]));
// Object
var_dump(@$s->write(2, 0, new stdClass));
// Resource
$fp = fopen("php://memory", "r");
var_dump(@$s->write(3, 0, $fp));
fclose($fp);

// writeRow with one bad value should stop and return false
var_dump(@$s->writeRow(4, ["ok", []]));

// writeCol with one bad value should stop and return false
var_dump(@$s->writeCol(0, ["ok", new stdClass]));

echo "OK\n";
?>
--EXPECT--
bool(false)
bool(false)
bool(false)
bool(false)
bool(false)
OK
