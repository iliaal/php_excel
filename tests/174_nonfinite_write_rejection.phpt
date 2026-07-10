--TEST--
Non-finite (NAN/INF) numbers are rejected on write instead of storing garbage
--EXTENSIONS--
excel
--DESCRIPTION--
A NAN/INF double serializes to a corrupt cell that reads back as garbage, and
the AS_DATE path cast to zend_long is undefined for a non-finite double. Plain,
AS_DATE, and AS_NUMERIC_STRING writes now reject a non-finite value; the
writeRow pre-scan rejects it too, keeping the row all-or-nothing.
--FILE--
<?php
$b = new ExcelBook(null, null, true);
$s = $b->addSheet("S");
foreach (["NAN" => NAN, "INF" => INF, "-INF" => -INF] as $label => $v) {
    echo "$label plain:    "; var_dump(@$s->write(1, 0, $v));
    echo "$label AS_DATE:  "; var_dump(@$s->write(1, 1, $v, null, ExcelFormat::AS_DATE));
}
echo "numstr INF:      "; var_dump(@$s->write(1, 2, "1e999", null, ExcelFormat::AS_NUMERIC_STRING));
echo "huge AS_DATE:    "; var_dump(@$s->write(1, 3, 1e300, null, ExcelFormat::AS_DATE));
echo "huge plain ok:   "; var_dump($s->write(2, 1, 1e300));
echo "finite ok:       "; var_dump($s->write(2, 0, 3.5));
for ($c = 0; $c < 3; $c++) $s->write(3, $c, "seed");
echo "writeRow ret:    "; var_dump(@$s->writeRow(3, [1.0, NAN, 2.0]));
echo "row3 col0 kept:  "; var_dump($s->read(3, 0));
echo "row3 col1 kept:  "; var_dump($s->read(3, 1));
?>
--EXPECT--
NAN plain:    bool(false)
NAN AS_DATE:  bool(false)
INF plain:    bool(false)
INF AS_DATE:  bool(false)
-INF plain:    bool(false)
-INF AS_DATE:  bool(false)
numstr INF:      bool(false)
huge AS_DATE:    bool(false)
huge plain ok:   bool(true)
finite ok:       bool(true)
writeRow ret:    bool(false)
row3 col0 kept:  string(4) "seed"
row3 col1 kept:  string(4) "seed"
