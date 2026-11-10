--TEST--
Fill patterns keep independent foreground/background colors through XLS/XLSX save/load
--EXTENSIONS--
excel
--FILE--
<?php
function check(bool $condition, string $label): void {
    if (!$condition) {
        echo "FAIL: $label\n";
    }
}

function checkFill(ExcelFormat $format, int $pattern, int $foreground, int $background, string $label): void {
    check($format->fillPattern() === $pattern, "$label pattern");
    check($format->patternForegroundColor() === $foreground, "$label foreground");
    check($format->patternBackgroundColor() === $background, "$label background");
}

$patterns = [];
foreach ((new ReflectionClass(ExcelFormat::class))->getConstants() as $name => $value) {
    if (strpos($name, 'FILLPATTERN_') === 0) {
        $patterns[] = $value;
    }
}
$colors = [ExcelFormat::COLOR_RED, ExcelFormat::COLOR_BLUE, ExcelFormat::COLOR_GREEN,
    ExcelFormat::COLOR_YELLOW, ExcelFormat::COLOR_BLACK, ExcelFormat::COLOR_WHITE];
check(count($patterns) > 0, 'fill pattern constants present');

foreach ([false, true] as $xlsx) {
    $kind = $xlsx ? 'XLSX' : 'XLS';
    $book = new ExcelBook(null, null, $xlsx);
    $sheet = $book->addSheet('Fill colors');
    $cases = [];
    foreach ($patterns as $index => $pattern) {
        $row = $index + 1;
        $foreground = $colors[$index % count($colors)];
        $background = $colors[($index + 1) % count($colors)];
        // Both setter orders must preserve the other color. Two cells per
        // pattern stay comfortably below the LibXL trial read limit.
        foreach ([0, 1] as $column) {
            $format = $book->addFormat();
            check($format->fillPattern($pattern) === $pattern, 'set pattern');
            if ($column === 0) {
                check($format->patternForegroundColor($foreground) === $foreground, 'set foreground first');
                check($format->patternBackgroundColor($background) === $background, 'set background second');
            } else {
                check($format->patternBackgroundColor($background) === $background, 'set background first');
                check($format->patternForegroundColor($foreground) === $foreground, 'set foreground second');
            }
            checkFill($format, $pattern, $foreground, $background, "$kind source $row,$column");
            $value = "fill $row,$column";
            if ($column === 0) {
                check($sheet->write($row, $column, $value, $format), 'write formatted cell');
            } else {
                check($sheet->write($row, $column, $value), 'write cell');
                $sheet->setCellFormat($row, $column, $format);
            }
            $cases[] = [$row, $column, $value, $pattern, $foreground, $background];
        }
    }

    $loaded = new ExcelBook(null, null, $xlsx);
    check($loaded->load($book->save()), 'reload workbook');
    foreach ([$sheet, $loaded->getSheet(0)] as $stage => $readSheet) {
        foreach ($cases as [$row, $column, $value, $pattern, $foreground, $background]) {
            $format = null;
            check($readSheet->read($row, $column, $format) === $value, 'read cell');
            checkFill($format, $pattern, $foreground, $background, "$kind stage $stage read $row,$column");
            checkFill($readSheet->cellFormat($row, $column), $pattern, $foreground, $background,
                "$kind stage $stage cellFormat $row,$column");
        }
    }
    echo $xlsx ? "XLSX complete\n" : "XLS complete\n";
}
?>
--EXPECT--
XLS complete
XLSX complete
