--TEST--
Point and pixel row-height setters preserve workbook bounds and validation order
--EXTENSIONS--
excel
--FILE--
<?php
function check($ok, $label) {
    if (!$ok) throw new RuntimeException($label);
}
function rejected($sheet, $method, $row, $height, $message) {
    $warnings = [];
    set_error_handler(function ($level, $text) use (&$warnings) {
        $warnings[] = $text;
        return true;
    });
    try {
        $result = $sheet->$method($row, $height);
    } finally {
        restore_error_handler();
    }
    check($result === false, "$method must return false");
    check($warnings === ["ExcelSheet::$method(): $message"],
        "$method warnings: " . json_encode($warnings));
}
class RowHeightBook extends ExcelBook {}
foreach ([false, true] as $xlsx) {
    $book = new RowHeightBook(null, null, $xlsx);
    $sheet = $book->addSheet('Rows');
    $max = $xlsx ? 1048575 : 65535;
    foreach (['setRowHeight' => 'rowHeight', 'setRowPx' => 'rowHeightPx'] as $setter => $getter) {
        // Both an ordinary row above the column limit and the last legal row
        // must use row bounds. Keep row 0 untouched for trial compatibility.
        foreach ([1, $xlsx ? 16384 : 256, $max] as $row) {
            check($sheet->$setter($row, 30) === true, "$setter valid row $row");
            check($sheet->$getter($row) == 30, "$getter valid row $row");
        }
        foreach ([-1, $max + 1, PHP_INT_MAX] as $row) {
            rejected($sheet, $setter, $row, 40, "Invalid row: $row");
        }
        check($sheet->$getter(1) == 30 && $sheet->$getter($max) == 30,
            "$setter invalid rows changed existing heights");
    }
    // Preserve the original order when more than one argument is invalid.
    rejected($sheet, 'setRowHeight', -1, NAN, 'Floating-point argument must be finite');
    rejected($sheet, 'setRowHeight', -1, -1, 'Invalid row: -1');
    rejected($sheet, 'setRowHeight', 1, -1, 'Height cannot be less then 0');
    rejected($sheet, 'setRowPx', -1, -1, 'Invalid row: -1');
    rejected($sheet, 'setRowPx', 1, -1, 'Argument out of int range');

    $book->addSheet('Temporary');
    check($book->deleteSheet(1), 'delete sheet');
    foreach (['setRowHeight', 'setRowPx'] as $setter) {
        rejected($sheet, $setter, $max + 1, 30, 'Invalid row: ' . ($max + 1));
        rejected($sheet, $setter, 1, 30,
            'Underlying ExcelBook sheet topology changed; wrapper must be re-fetched');
    }
    echo $xlsx ? "XLSX bounds and order OK\n" : "XLS bounds and order OK\n";
}
// Without a parent workbook, the shared validator retains the XLS fallback.
$sheet = (new ReflectionClass(ExcelSheet::class))->newInstanceWithoutConstructor();
foreach (['setRowHeight', 'setRowPx'] as $setter) {
    rejected($sheet, $setter, 65536, 30, 'Invalid row: 65536');
    rejected($sheet, $setter, 1, 30, "The sheet wasn't initialized");
}
echo "Uninitialized fallback OK\n";
?>
--EXPECT--
XLS bounds and order OK
XLSX bounds and order OK
Uninitialized fallback OK
