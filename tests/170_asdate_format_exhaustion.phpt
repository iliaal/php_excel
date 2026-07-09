--TEST--
AS_DATE fails closed when a default date format cannot be allocated (style table exhausted)
--EXTENSIONS--
excel
--DESCRIPTION--
CR-008: on the AS_DATE path with no explicit format, the book lazily creates
a shared date format. If xlBookAddFormat returns NULL (the xlsx style table
tops out near 65k formats), the cell was written as a bare number yet reported
success. It now fails closed (write returns false).
--FILE--
<?php
$b = new ExcelBook(null, null, true);
$s = $b->addSheet("S");

// Exhaust the workbook style table. The xlsx format-record cap (~65k) is a
// file-format limit, not a trial watermark, so this is deterministic.
$exhausted = false;
for ($i = 0; $i < 70000; $i++) {
    if ($b->addFormat() === false) { $exhausted = true; break; }
}
echo "exhausted: "; var_dump($exhausted);

// No default date format can be allocated now -> fail closed, do not write a
// bare number and claim success.
$ret = @$s->write(1, 0, 1700000000, null, ExcelFormat::AS_DATE);
echo "write AS_DATE: "; var_dump($ret);
echo "isDate:        "; var_dump($s->isDate(1, 0));
echo "OK\n";
?>
--EXPECT--
exhausted: bool(true)
write AS_DATE: bool(false)
isDate:        bool(false)
OK
