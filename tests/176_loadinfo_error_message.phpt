--TEST--
loadInfo()/loadInfoRaw() surface the libxl error message on failure
--EXTENSIONS--
excel
--SKIPIF--
<?php
/* loadInfoRaw is compiled only for libxl >= 5.0.1 (0x05000100). */
if (!method_exists("ExcelBook", "loadInfoRaw")) {
	print "skip libxl 5.0.1+ required";
}
?>
--DESCRIPTION--
loadInfo() and loadInfoRaw() previously returned a bare false on libxl failure
with no diagnostic, unlike load()/loadFile(). They now emit an E_WARNING
carrying xlBookErrorMessage() so the failure is diagnosable.
--FILE--
<?php
$w = [];
set_error_handler(function ($n, $s) use (&$w) { $w[] = $s; return true; });
$b = new ExcelBook(null, null, true);
var_dump($b->loadInfo(__DIR__ . "/nope-176.xlsx"));
$b2 = new ExcelBook(null, null, true);
var_dump($b2->loadInfoRaw("definitely not a valid xlsx payload"));
restore_error_handler();
$diag = 0;
foreach ($w as $m) if (strpos($m, "Failed to load workbook") !== false) $diag++;
echo "diagnostics: $diag\n";
?>
--EXPECT--
bool(false)
bool(false)
diagnostics: 2
