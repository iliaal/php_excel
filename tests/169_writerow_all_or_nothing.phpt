--TEST--
writeRow / writeCol are all-or-nothing on a PHP-side value error (no partial commit)
--EXTENSIONS--
excel
--DESCRIPTION--
CR-009: a bad element mid-array (unsupported type, embedded NUL) aborted the
write loop after earlier cells were already committed. A pre-scan now rejects
the whole row/column up front so no cell is modified. The column-overflow
preflight (test 149) stays atomic; libxl-side failures remain out of scope.
--FILE--
<?php
$b = new ExcelBook(null, null, true);
$s = $b->addSheet("S");

// Seed sentinels so a partial apply would be visible.
for ($c = 0; $c < 3; $c++) $s->write(1, $c, "seed");
$ret = @$s->writeRow(1, ["ok", [], "never"]);   // [] is unsupported mid-array
echo "writeRow ret:  "; var_dump($ret);
echo "row1 col0:     "; var_dump($s->read(1, 0));
echo "row1 col1:     "; var_dump($s->read(1, 1));
echo "row1 col2:     "; var_dump($s->read(1, 2));

// Embedded NUL mid-array, into an empty row.
$ret = @$s->writeRow(2, ["a", "b\0c", "d"]);
echo "writeRow NUL:  "; var_dump($ret);
echo "row2 col0:     "; var_dump($s->read(2, 0));

// writeCol symmetric.
for ($r = 3; $r < 6; $r++) $s->write($r, 0, "seed");
$ret = @$s->writeCol(0, ["x", new stdClass, "z"], 3);
echo "writeCol ret:  "; var_dump($ret);
echo "col rows 3-5:  "; var_dump($s->read(3, 0), $s->read(4, 0), $s->read(5, 0));

// Valid arrays still write fully.
echo "valid writeRow: "; var_dump($s->writeRow(7, ["p", "q", "r"]));
echo "row7:           "; var_dump($s->read(7, 0), $s->read(7, 1), $s->read(7, 2));
echo "OK\n";
?>
--EXPECT--
writeRow ret:  bool(false)
row1 col0:     string(4) "seed"
row1 col1:     string(4) "seed"
row1 col2:     string(4) "seed"
writeRow NUL:  bool(false)
row2 col0:     string(0) ""
writeCol ret:  bool(false)
col rows 3-5:  string(4) "seed"
string(4) "seed"
string(4) "seed"
valid writeRow: bool(true)
row7:           string(1) "p"
string(1) "q"
string(1) "r"
OK
