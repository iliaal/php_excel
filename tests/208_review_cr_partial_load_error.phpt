--TEST--
CR-002 loadPartially/loadFilePartially surface libxl error on failure
--SKIPIF--
<?php
if (!extension_loaded('excel')) die('skip excel missing');
if (!method_exists('ExcelBook', 'loadPartially')) die('skip libxl 5.0+ required');
?>
--FILE--
<?php
$b = new ExcelBook(null, null, true);
set_error_handler(function ($n, $m) {
	echo "WARN:" . (str_contains($m, 'Failed to load workbook') ? 'has_msg' : 'other') . "\n";
	return true;
});
$r = $b->loadPartially("not-a-valid-xlsx-payload", 0, 0, 10);
echo "loadPartially: ";
var_dump($r);
restore_error_handler();
echo "OK\n";
?>
--EXPECT--
WARN:has_msg
loadPartially: bool(false)
OK
