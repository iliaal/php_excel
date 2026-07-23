--TEST--
load() / loadFile() / save() surface the libxl error message on failure
--EXTENSIONS--
excel
--DESCRIPTION--
CR-019: load and save previously returned a bare false on libxl failure with
no diagnostic, unlike Sheet::write which surfaces xlBookErrorMessage(). They
now emit an E_WARNING carrying the underlying libxl error so failures are
diagnosable without a separate getError() call.
--FILE--
<?php
$warnings = [];
set_error_handler(function ($n, $s) use (&$warnings) { $warnings[] = $s; return true; });

$b = new ExcelBook(null, null, true);
$r1 = $b->load("this is not a valid xlsx payload");

$b2 = new ExcelBook(null, null, true);
$r2 = $b2->loadFile(__DIR__ . "/does-not-exist-172.xlsx");

$b3 = new ExcelBook(null, null, true);
$r3 = $b3->save("/no/such/directory/172.xlsx");

restore_error_handler();

echo "load ret:     "; var_dump($r1);
echo "loadFile ret: "; var_dump($r2);
echo "save ret:     "; var_dump($r3);
echo "warnings:     " . count($warnings) . "\n";
foreach (["load", "loadFile", "save"] as $i => $op) {
    $w = $warnings[$i] ?? "";
    $expect = $op === "save" ? "Failed to save workbook:" : "Failed to load workbook:";
    // save may fail at stream open (after SaveRaw) with a staged path diagnostic,
    // or at SaveRaw with an embedded libxl message — both start with the same prefix.
    echo "$op warning carries libxl error: " .
        var_export(str_contains($w, $expect) && strlen($w) > strlen($expect) + 5, true) . "\n";
}
echo "OK\n";
?>
--EXPECT--
load ret:     bool(false)
loadFile ret: bool(false)
save ret:     bool(false)
warnings:     3
load warning carries libxl error: true
loadFile warning carries libxl error: true
save warning carries libxl error: true
OK
