--TEST--
Floating AS_DATE timestamps respect PHP integer width before single and bulk writes
--EXTENSIONS--
excel
--INI--
date.timezone=UTC
--FILE--
<?php
function writeDates($sheet, $method, $values) {
    if ($method === 'write') {
        return $sheet->write(2, 0, $values[1], null, ExcelFormat::AS_DATE);
    }
    if ($method === 'writeRow') {
        return $sheet->writeRow(2, $values, 0, null, ExcelFormat::AS_DATE);
    }
    return $sheet->writeCol(0, $values, 2, null, ExcelFormat::AS_DATE);
}
$upper = -(float) PHP_INT_MIN;
// At 64-bit magnitudes, 1 is smaller than one representable double step.
$step = PHP_INT_SIZE === 4 ? 1.0 : 2048.0;
foreach ([false, true] as $xlsx) {
    foreach (['write', 'writeRow', 'writeCol'] as $method) {
        $book = new ExcelBook(null, null, $xlsx);
        $sheet = $book->addSheet('Dates');
        $sheet->write(2, 0, 'keep first');
        $sheet->write(2, 1, 'keep row');
        $sheet->write(3, 0, 'keep column');
        $warnings = 0;
        set_error_handler(function () use (&$warnings) { ++$warnings; return true; });
        foreach ([$upper, $upper + $step, -$upper - $step, INF, -INF, NAN] as $invalid) {
            var_dump(writeDates($sheet, $method, [1700000000.0, $invalid]) === false);
            var_dump($sheet->read(2, 0) === 'keep first'
                && $sheet->read(2, 1) === 'keep row'
                && $sheet->read(3, 0) === 'keep column');
        }
        restore_error_handler();
        var_dump($warnings === 6);
        set_error_handler(function () { throw new RuntimeException('rejected'); });
        try {
            writeDates($sheet, $method, [1700000000.0, $upper]);
            echo "missing exception\n";
        } catch (RuntimeException $e) {
            echo $e->getMessage(), "\n";
        }
        restore_error_handler();
        // Last second within the 32-bit positive range remains a valid float.
        var_dump(writeDates($sheet, $method, [1700000000.0, 2147483647.0]));
        $row = $method === 'writeCol' ? 3 : 2;
        $col = $method === 'writeRow' ? 1 : 0;
        var_dump($sheet->read($row, $col) === 2147483647);
        var_dump(writeDates($sheet, $method, [1700000000.0, -2147483648.0]));
        var_dump($sheet->read($row, $col) === -2147483647 - 1);
        $saved = $book->save();
        $loaded = new ExcelBook(null, null, $xlsx);
        var_dump(is_string($saved) && $loaded->load($saved));
        var_dump($loaded->getSheet(0)->read($row, $col) === -2147483647 - 1);
    }
}
?>
--EXPECT--
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
rejected
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
rejected
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
rejected
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
rejected
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
rejected
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
rejected
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
