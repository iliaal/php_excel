--TEST--
Blank formula writes store readable empty strings and follow excel.skip_empty
--EXTENSIONS--
excel
--INI--
excel.skip_empty=0
--FILE--
<?php
function check(bool $ok, string $what): void {
    if (!$ok) {
        throw new Exception($what);
    }
}
$cases = [
    ['', ExcelFormat::AS_FORMULA],
    ['=', ExcelFormat::AS_FORMULA],
    [" \t", ExcelFormat::AS_FORMULA],
    ['=', null],
    ["= \r\n", null],
];
foreach ([false, true] as $xlsx) {
    foreach ([0, 1, 2] as $mode) {
        ini_set('excel.skip_empty', (string) $mode);
        foreach (['cell', 'row'] as $writer) {
            $book = new ExcelBook(null, null, $xlsx);
            $sheet = $book->addSheet('Blank formulas');
            foreach ($cases as $i => [$value, $type]) {
                $row = 2 + $i;
                check($sheet->write($row, 1, 'keep'), 'seed');
                if ($writer === 'row') {
                    $ok = $type === null
                        ? $sheet->writeRow($row, [$value], 1)
                        : $sheet->writeRow($row, [$value], 1, null, $type);
                } else {
                    $ok = $type === null
                        ? $sheet->write($row, 1, $value)
                        : $sheet->write($row, 1, $value, null, $type);
                }
                $label = sprintf('%s mode %d %s %s', $xlsx ? 'xlsx' : 'xls', $mode, $writer, json_encode($value));
                check($ok, "$label: write failed");
                check(!$sheet->isFormula($row, 1), "$label: stored a formula");
                check($sheet->read($row, 1) === ($mode === 2 ? 'keep' : ''), "$label: wrong value");
            }
            check($sheet->write(9, 1, 7), 'operand');
            check($sheet->write(9, 2, '= B10*2 '), 'formula');
            check($sheet->isFormula(9, 2), 'non-blank formula kept');
        }
    }
    echo $xlsx ? "XLSX OK\n" : "XLS OK\n";
}
?>
--EXPECT--
XLS OK
XLSX OK
