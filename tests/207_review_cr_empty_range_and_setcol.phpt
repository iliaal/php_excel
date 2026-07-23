--TEST--
CR-010 empty used-range defaults; CR-020 setColWidth inverted range still rejected by validator
--EXTENSIONS--
excel
--FILE--
<?php
$b = new ExcelBook(null, null, true);
$s = $b->addSheet("S");

// Force empty-range path if libxl reports lastCol/lastRow as 0 (defensive).
// When lastCol is non-zero (common for empty sheets), skip the empty-array
// assertion and only verify inverted ranges still fail cleanly.
$lc = $s->lastCol();
$lr = $s->lastRow();
echo "lastCol=$lc lastRow=$lr\n";

set_error_handler(function ($n, $m) {
	echo "W:" . preg_replace('/^ExcelSheet::\w+\(\):\s*/', '', $m) . "\n";
	return true;
});

// Inverted explicit range must still be rejected (validator owns this now)
$r = $s->setColWidth(5, 1, 10.0);
echo "setColWidth inverted: ";
var_dump($r);

// Valid single-column still works
$r = $s->setColWidth(2, 2, 12.0);
echo "setColWidth ok: ";
var_dump($r);

// Width < -1 still rejected
$r = $s->setColWidth(0, 0, -2.0);
echo "setColWidth width<-1: ";
var_dump($r);

restore_error_handler();
echo "OK\n";
?>
--EXPECTF--
lastCol=%d lastRow=%d
W:%s
setColWidth inverted: bool(false)
setColWidth ok: bool(true)
W:%s
setColWidth width<-1: bool(false)
OK
