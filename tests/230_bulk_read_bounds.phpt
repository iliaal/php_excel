--TEST--
Dense and sparse bulk readers preserve default, inclusive and invalid bounds
--EXTENSIONS--
excel
--FILE--
<?php
foreach ([false, true] as $xlsx) {
    $book = new ExcelBook(null, null, $xlsx);
    $sheet = $book->addSheet('Bounds');
    $sheet->write(1, 1, 'a');
    $sheet->write(2, 2, 'b');

    foreach (['readRow', 'readCol', 'readSparseRow', 'readSparseCol'] as $method) {
        echo ($xlsx ? 'xlsx ' : 'xls '), $method, "\n";
        // Both axes contain an empty cell followed by 'b' from index 1.
        $sparse = str_contains($method, 'Sparse');
        $last = str_ends_with($method, 'Row') ? $sheet->lastCol() : $sheet->lastRow();
        $fixedLast = str_ends_with($method, 'Row') ? $sheet->lastRow() : $sheet->lastCol();
        $expected = array_fill(0, $last - 1, '');
        $expected[1] = 'b';
        var_dump($sheet->$method(2, 1) === ($sparse ? [2 => 'b'] : $expected));
        var_dump($sheet->$method(2, 2, 2) === ($sparse ? [2 => 'b'] : ['b']));
        // The used-range end is exclusive, but explicit one-past reads
        // remain accepted for compatibility (including the fixed axis).
        var_dump($sheet->$method(2, $last, $last) === ($sparse ? [] : ['']));
        var_dump($sheet->$method($fixedLast, 2, 2) === ($sparse ? [] : ['']));
        $warnings = [];
        set_error_handler(static function ($severity, $message) use (&$warnings) {
            $warnings[] = [$severity, $message];
            return true;
        });
        foreach ([[-1, 2], [$last + 1, $last + 1], [2, 1], [1, $last + 1], [1, -2], [$last, -1]] as [$start, $end]) {
            var_dump($sheet->$method(2, $start, $end));
        }
        restore_error_handler();
        $axis = str_ends_with($method, 'Row') ? 'column' : 'row';
        $expectedWarnings = [];
        foreach ([['starting', -1], ['starting', $last + 1], ['ending', 1], ['ending', $last + 1], ['ending', -2], ['ending', $last - 1]] as [$bound, $value]) {
            $expectedWarnings[] = [E_WARNING, "ExcelSheet::$method(): Invalid $bound $axis number '$value'"];
        }
        var_dump($warnings === $expectedWarnings);
    }
    $empty = $book->addSheet('Empty');
    // LibXL may give a fresh sheet a nonempty used range. Compare dense
    // and sparse defaults only when that axis really has an empty range.
    foreach ([['readRow', 'readSparseRow', 'lastCol'], ['readCol', 'readSparseCol', 'lastRow']] as [$dense, $sparse, $last]) {
        if ($empty->$last() === 0) {
            if ($empty->$dense(0) !== [] || $empty->$sparse(0) !== []) {
                echo "empty default mismatch\n";
            }
        }
    }
}
echo "OK\n";
?>
--EXPECT--
xls readRow
bool(true)
bool(true)
bool(true)
bool(true)
bool(false)
bool(false)
bool(false)
bool(false)
bool(false)
bool(false)
bool(true)
xls readCol
bool(true)
bool(true)
bool(true)
bool(true)
bool(false)
bool(false)
bool(false)
bool(false)
bool(false)
bool(false)
bool(true)
xls readSparseRow
bool(true)
bool(true)
bool(true)
bool(true)
bool(false)
bool(false)
bool(false)
bool(false)
bool(false)
bool(false)
bool(true)
xls readSparseCol
bool(true)
bool(true)
bool(true)
bool(true)
bool(false)
bool(false)
bool(false)
bool(false)
bool(false)
bool(false)
bool(true)
xlsx readRow
bool(true)
bool(true)
bool(true)
bool(true)
bool(false)
bool(false)
bool(false)
bool(false)
bool(false)
bool(false)
bool(true)
xlsx readCol
bool(true)
bool(true)
bool(true)
bool(true)
bool(false)
bool(false)
bool(false)
bool(false)
bool(false)
bool(false)
bool(true)
xlsx readSparseRow
bool(true)
bool(true)
bool(true)
bool(true)
bool(false)
bool(false)
bool(false)
bool(false)
bool(false)
bool(false)
bool(true)
xlsx readSparseCol
bool(true)
bool(true)
bool(true)
bool(true)
bool(false)
bool(false)
bool(false)
bool(false)
bool(false)
bool(false)
bool(true)
OK
