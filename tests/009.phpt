--TEST--
Text rotation preserves all angles and vertical text through XLS/XLSX save/load
--EXTENSIONS--
excel
--FILE--
<?php
function check(bool $condition, string $label): void {
    if (!$condition) {
        throw new RuntimeException($label);
    }
}

$angles = range(0, 180);
$angles[] = 255; // Vertical text is the one supported value above 180.
foreach ([false, true] as $xlsx) {
    $kind = $xlsx ? 'XLSX' : 'XLS';
    $book = new ExcelBook(null, null, $xlsx);
    $sheet = $book->addSheet('Rotation');
    foreach ($angles as $index => $angle) {
        // Row 0 is reserved by the trial library; 182 cells stay below its limit.
        $row = $index + 1;
        $format = $book->addFormat();
        check($format->rotate($angle) === $angle, "$kind set rotation $angle");
        check($format->rotate() === $angle, "$kind get rotation $angle");
        check($format->rotate(null) === $angle, "$kind null getter $angle");
        check($sheet->write($row, 0, "angle $angle", $format), "$kind write $angle");
        check($book->getError() === false, "$kind write error $angle");
    }

    // The last format is attached to the vertical-text cell. Invalid setters
    // must neither accept the gaps around 255 nor change that stored rotation.
    foreach ([-1, 181, 254, 256, PHP_INT_MAX] as $angle) {
        $warnings = [];
        set_error_handler(function ($level, $message) use (&$warnings) {
            $warnings[] = $message;
            return true;
        });
        try {
            $result = $format->rotate($angle);
        } finally {
            restore_error_handler();
        }
        check($result === false, "$kind reject rotation $angle");
        check($warnings === [
            'ExcelFormat::rotate(): Rotation can be a number between 0 and 180 or 255',
        ], "$kind warning for rotation $angle");
        check($format->rotate() === 255, "$kind preserve vertical text after $angle");
    }

    $saved = $book->save();
    check(is_string($saved) && $saved !== '', "$kind save workbook");
    $loaded = new ExcelBook(null, null, $xlsx);
    check($loaded->load($saved), "$kind reload workbook");
    foreach ([$sheet, $loaded->getSheet(0)] as $stage => $readSheet) {
        foreach ($angles as $index => $angle) {
            $row = $index + 1;
            $readFormat = '';
            check($readSheet->read($row, 0, $readFormat) === "angle $angle",
                "$kind stage $stage read $angle");
            check($readFormat->rotate() === $angle,
                "$kind stage $stage rotation $angle");
            check($readSheet->cellFormat($row, 0)->rotate() === $angle,
                "$kind stage $stage cellFormat $angle");
        }
    }
    echo "$kind all rotations OK\n";
}
?>
--EXPECT--
XLS all rotations OK
XLSX all rotations OK
