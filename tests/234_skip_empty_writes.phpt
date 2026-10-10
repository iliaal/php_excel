--TEST--
Skipped empty writes preserve cells and bulk positions in XLS and XLSX
--EXTENSIONS--
excel
--INI--
excel.skip_empty=0
--FILE--
<?php
function check(bool $ok): void {
    if (!$ok) {
        throw new Exception('Unexpected skip_empty behavior');
    }
}
foreach ([false, true] as $xlsx) {
    echo $xlsx ? "XLSX\n" : "XLS\n";
    foreach ([0, 1, 2] as $mode) {
        ini_set('excel.skip_empty', (string) $mode);
        foreach (['cell', 'row', 'col'] as $writer) {
            $book = new ExcelBook(null, null, $xlsx);
            $sheet = $book->addSheet('Skip empty');
            $original = $book->addFormat();
            $original->numberFormat(ExcelFormat::NUMFORMAT_NUMBER_D2);
            $replacement = $book->addFormat();
            $replacement->numberFormat(ExcelFormat::NUMFORMAT_GENERAL);
            $values = [null, '', 0, false, '0'];
            foreach ($values as $i => $value) {
                $row = $writer === 'col' ? 2 + $i : 2;
                $col = $writer === 'col' ? 2 : 2 + $i;
                check($sheet->write($row, $col, 'keep', $original));
            }
            if ($writer === 'row') {
                check($sheet->writeRow(2, $values, 2, $replacement));
            } elseif ($writer === 'col') {
                check($sheet->writeCol(2, $values, 2, $replacement));
            } else {
                foreach ($values as $i => $value) {
                    check($sheet->write(2, 2 + $i, $value, $replacement));
                }
            }
            $expected = [$mode > 0 ? 'keep' : null, $mode === 2 ? 'keep' : '', 0.0, false, '0'];
            foreach ($expected as $i => $value) {
                $row = $writer === 'col' ? 2 + $i : 2;
                $col = $writer === 'col' ? 2 : 2 + $i;
                $format = null;
                check($sheet->read($row, $col, $format) === $value);
                $skipped = ($i === 0 && $mode > 0) || ($i === 1 && $mode === 2);
                check($format->numberFormat() === ($skipped
                    ? ExcelFormat::NUMFORMAT_NUMBER_D2 : ExcelFormat::NUMFORMAT_GENERAL));
            }
            // Explicit text bypasses empty-string skipping, but not null skipping.
            check($sheet->write(9, 2, 'keep', $original));
            $nextRow = $writer === 'col' ? 10 : 9;
            $nextCol = $writer === 'col' ? 2 : 3;
            check($sheet->write($nextRow, $nextCol, 'keep', $original));
            if ($writer === 'row') {
                check($sheet->writeRow(9, ['', null], 2, $replacement, ExcelFormat::AS_TEXT));
            } elseif ($writer === 'col') {
                check($sheet->writeCol(2, ['', null], 9, $replacement, ExcelFormat::AS_TEXT));
            } else {
                check($sheet->write(9, 2, '', $replacement, ExcelFormat::AS_TEXT));
                check($sheet->write(9, 3, null, $replacement, ExcelFormat::AS_TEXT));
            }
            check($sheet->read(9, 2) === '');
            check($sheet->read($nextRow, $nextCol) === ($mode > 0 ? 'keep' : null));
            // Explicit numeric strings follow the default empty-string rule.
            check($sheet->write(11, 2, 'keep', $original));
            check($sheet->write(11, 2, '', $replacement, ExcelFormat::AS_NUMERIC_STRING));
            check($sheet->read(11, 2) === ($mode === 2 ? 'keep' : ''));
            // A lone apostrophe is unescaped to an empty string after the skip decision.
            check($sheet->write(12, 2, 'keep', $original));
            check($sheet->write(12, 2, "'", $replacement));
            check($sheet->read(12, 2) === '');
        }
        echo "mode $mode: values, formats, positions and explicit text OK\n";
    }
}
?>
--EXPECT--
XLS
mode 0: values, formats, positions and explicit text OK
mode 1: values, formats, positions and explicit text OK
mode 2: values, formats, positions and explicit text OK
XLSX
mode 0: values, formats, positions and explicit text OK
mode 1: values, formats, positions and explicit text OK
mode 2: values, formats, positions and explicit text OK
