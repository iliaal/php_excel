--TEST--
ExcelConditionalFormatting constructor rejects inverted ranges
--EXTENSIONS--
excel
--SKIPIF--
<?php if ((new ReflectionMethod('ExcelConditionalFormatting', '__construct'))->getNumberOfParameters() < 5) print 'skip libxl 5.1.0+ required'; ?>
--FILE--
<?php
$book = new ExcelBook(null, null, true);
$sheet = $book->addSheet('S');

foreach ([[10, 5, 0, 1], [0, 5, 3, 1]] as $range) {
    try {
        new ExcelConditionalFormatting($sheet, ...$range);
        echo "accepted\n";
    } catch (Throwable $e) {
        echo "rejected\n";
    }
}

var_dump(new ExcelConditionalFormatting($sheet, 0, 5, 0, 1) instanceof ExcelConditionalFormatting);
echo "OK\n";
?>
--EXPECT--
rejected
rejected
bool(true)
OK
