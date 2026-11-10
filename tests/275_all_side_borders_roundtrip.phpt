--TEST--
All-side border setters update every side and survive XLS/XLSX save/load
--EXTENSIONS--
excel
--FILE--
<?php
function check(bool $condition, string $label): void {
    if (!$condition) {
        echo "FAIL: $label\n";
    }
}

$sides = ['Left', 'Right', 'Top', 'Bottom'];
$cases = [
    [ExcelFormat::BORDERSTYLE_NONE, ExcelFormat::COLOR_RED],
    [ExcelFormat::BORDERSTYLE_THIN, ExcelFormat::COLOR_BLUE],
    [ExcelFormat::BORDERSTYLE_DASHED, ExcelFormat::COLOR_BLACK],
    [ExcelFormat::BORDERSTYLE_SLANTDASHDOT, ExcelFormat::COLOR_GREEN],
];
foreach ([false, true] as $xlsx) {
    $book = new ExcelBook(null, null, $xlsx);
    $sheet = $book->addSheet('All-side borders');
    foreach ($cases as $column => [$style, $color]) {
        $format = $book->addFormat();
        // Seed every side so resetting to NONE must perform a real write.
        foreach ($sides as $side) {
            $styleMethod = 'border' . $side . 'Style';
            $colorMethod = 'border' . $side . 'Color';
            check($format->$styleMethod(ExcelFormat::BORDERSTYLE_DOUBLE) === ExcelFormat::BORDERSTYLE_DOUBLE, "seed $side style");
            check($format->$colorMethod(ExcelFormat::COLOR_YELLOW) === ExcelFormat::COLOR_YELLOW, "seed $side color");
        }
        check($format->borderStyle($style) === true, 'set all styles');
        check($format->borderColor($color) === true, 'set all colors');
        foreach ($sides as $side) {
            $styleMethod = 'border' . $side . 'Style';
            $colorMethod = 'border' . $side . 'Color';
            check($format->$styleMethod() === $style, "set $side style");
            check($format->$colorMethod() === $color, "set $side color");
        }
        // Neither omission nor explicit null may reset a write-only setter.
        foreach (['borderStyle', 'borderColor'] as $method) {
            check($format->$method() === true, "$method omitted");
            check($format->$method(null) === true, "$method null");
        }
        if ($column % 2 === 0) {
            check($sheet->write(1, $column, 'border ' . $column, $format), 'write formatted cell');
        } else {
            check($sheet->write(1, $column, 'border ' . $column), 'write cell');
            $sheet->setCellFormat(1, $column, $format);
        }
    }

    $loaded = new ExcelBook(null, null, $xlsx);
    check($loaded->load($book->save()), 'reload workbook');
    $restored = $loaded->getSheet(0);
    foreach ($cases as $column => [$style, $color]) {
        $format = null;
        check($restored->read(1, $column, $format) === 'border ' . $column, 'read cell');
        foreach ($sides as $side) {
            $styleMethod = 'border' . $side . 'Style';
            $colorMethod = 'border' . $side . 'Color';
            check($format->$styleMethod() === $style, "saved $side style");
            check($format->$colorMethod() === $color, "saved $side color");
        }
    }
    echo $xlsx ? "XLSX complete\n" : "XLS complete\n";
}
?>
--EXPECT--
XLS complete
XLSX complete
