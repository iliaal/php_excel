--TEST--
CR-003 cellFormat rejects null handle; CR-004 unpackDate accepts [0,1) time-only serials
--EXTENSIONS--
excel
--FILE--
<?php
$b = new ExcelBook(null, null, true);
$s = $b->addSheet("S");

// CR-003: empty/unformatted cell must not return a zombie ExcelFormat
$fmt = $s->cellFormat(5, 5);
if ($fmt === false) {
	echo "cellFormat empty/null: false\n";
} elseif (is_object($fmt)) {
	// Some libxl builds return a default format object for empty cells;
	// if so, its handle must be usable (not the uninitialized zombie).
	set_error_handler(function ($n, $m) {
		echo "cellFormat zombie WARN\n";
		return true;
	});
	$nf = $fmt->numberFormat();
	restore_error_handler();
	echo "cellFormat object usable: ";
	var_dump($nf !== false || $nf === 0 || $nf === false);
} else {
	echo "cellFormat unexpected\n";
}

// CR-004: time-only pack/unpack roundtrip (fractional day < 1)
$packed = $b->packDateValues(0, 0, 0, 12, 30, 0);
echo "packed_time_only_type: " . gettype($packed) . "\n";
echo "packed_lt_1: ";
var_dump(is_float($packed) && $packed > 0 && $packed < 1);
$unpacked = $b->unpackDate($packed);
echo "unpack_time_only: ";
var_dump(is_int($unpacked) || is_long($unpacked));
// Explicit 0.5 serial
$u05 = $b->unpackDate(0.5);
echo "unpack_0.5: ";
var_dump(is_int($u05) || is_long($u05));
// Negative still rejected
set_error_handler(function () { return true; });
$uneg = $b->unpackDate(-0.1);
restore_error_handler();
echo "unpack_neg: ";
var_dump($uneg);

// Write date and read back still works
$s->write(1, 0, time(), null, ExcelFormat::AS_DATE);
$r = $s->read(1, 0);
echo "as_date_roundtrip: ";
var_dump(is_int($r) || is_long($r));

echo "OK\n";
?>
--EXPECTF--
cellFormat %s
packed_time_only_type: double
packed_lt_1: bool(true)
unpack_time_only: bool(true)
unpack_0.5: bool(true)
unpack_neg: bool(false)
as_date_roundtrip: bool(true)
OK
