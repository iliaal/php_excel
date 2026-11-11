--TEST--
Horizontal and vertical page breaks use independent row/column coordinates and boolean removal
--EXTENSIONS--
excel
--FILE--
<?php
function check(bool $condition, string $label): void {
    if (!$condition) {
        echo "FAIL: $label\n";
    }
}

function checkBreaks(ExcelSheet $sheet, array $rows, array $columns, string $label): void {
    foreach ([['Hor', $rows], ['Ver', $columns]] as [$axis, $coordinates]) {
        $size = 'get' . $axis . 'PageBreakSize';
        $get = 'get' . $axis . 'PageBreak';
        check($sheet->$size() === count($coordinates), "$label $axis count");
        foreach ($coordinates as $index => $coordinate) {
            check($sheet->$get($index) === $coordinate, "$label $axis index $index");
        }
        check($sheet->$get(count($coordinates)) === -1, "$label $axis missing index");
    }
}

foreach ([false, true] as $xlsx) {
    $kind = $xlsx ? 'XLSX' : 'XLS';
    $book = new ExcelBook(null, null, $xlsx);
    $sheet = $book->addSheet('Page breaks');
    check($sheet->write(1, 0, 'content'), "$kind write");
    checkBreaks($sheet, [], [], "$kind empty");
    // Neither the list positions nor the two axes share coordinates.
    check($sheet->horPageBreak(10, true), "$kind set row 10");
    check($sheet->horPageBreak(30, true), "$kind set row 30");
    check($sheet->verPageBreak(5, true), "$kind set column 5");
    checkBreaks($sheet, [10, 30], [5], "$kind set");

    $loaded = new ExcelBook(null, null, $xlsx);
    check($loaded->load($book->save()), "$kind reload");
    $sheet = $loaded->getSheet(0);
    checkBreaks($sheet, [10, 30], [5], "$kind loaded");
    check($sheet->horPageBreak(10, false), "$kind remove row 10");
    checkBreaks($sheet, [30], [5], "$kind remove one row");
    check($sheet->verPageBreak(5, false), "$kind remove column 5");
    checkBreaks($sheet, [30], [], "$kind remove column");
    check($sheet->horPageBreak(30, false), "$kind remove row 30");
    checkBreaks($sheet, [], [], "$kind remove final row");
    echo "$kind complete\n";
}
?>
--EXPECT--
XLS complete
XLSX complete
