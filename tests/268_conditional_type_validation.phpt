--TEST--
Conditional-formatting rule and color-scale types reject unsupported enums
--EXTENSIONS--
excel
--FILE--
<?php
$book = new ExcelBook(null, null, true);
$sheet = $book->addSheet('S');
$rules = (new ReflectionMethod(ExcelSheet::class, 'addConditionalFormatting'))->getNumberOfParameters()
    ? $sheet->addConditionalFormatting(1, 2, 0, 0)
    : $sheet->addConditionalFormatting();
$format = $book->addConditionalFormat();
$cases = [
    'addRule' => [[0, $format, 'A2'], [0], 10],
    'add2ColorScaleRule' => [[2, 3, 0, 1.0, 1, 2.0], [2, 4], 5],
    'add2ColorScaleFormulaRule' => [[2, 3, 0, 'A2', 1, 'A3'], [2, 4], 5],
    'add3ColorScaleRule' => [[2, 3, 4, 0, 1.0, 3, 2.0, 1, 3.0], [3, 5, 7], 5],
    'add3ColorScaleFormulaRule' => [[2, 3, 4, 0, 'A2', 3, 'A3', 1, 'A4'], [3, 5, 7], 5],
];
foreach ($cases as $method => [$args, $positions, $last]) {
    $warning = null;
    set_error_handler(function ($level, $message) use (&$warning) {
        $warning = preg_replace('/^.*\(\): /', '', $message);
        return true;
    });
    $rejected = $accepted = 0;
    foreach ($positions as $position) {
        foreach ([-1, $last + 1, 99, 2147483647] as $type) {
            $call = $args;
            $call[$position] = $type;
            $warning = null;
            $expected = $type < 0 ? 'Argument out of int range'
                : ($method === 'addRule' ? 'Invalid conditional formatting type'
                    : 'Invalid conditional formatting value type');
            $rejected += $rules->$method(...$call) === false && $warning === $expected;
        }
        for ($type = 0; $type <= $last; $type++) {
            $call = $args;
            $call[$position] = $type;
            $warning = null;
            $accepted += $rules->$method(...$call) === true && $warning === null;
        }
    }
    restore_error_handler();
    echo "$method rejected=$rejected accepted=$accepted\n";
}
?>
--EXPECT--
addRule rejected=4 accepted=11
add2ColorScaleRule rejected=8 accepted=12
add2ColorScaleFormulaRule rejected=8 accepted=12
add3ColorScaleRule rejected=12 accepted=18
add3ColorScaleFormulaRule rejected=12 accepted=18
