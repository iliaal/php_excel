--TEST--
Conditional-formatting operator rules reject unsupported enums before adding a rule
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
foreach (['addOpNumRule' => [1.0, 2.0], 'addOpStrRule' => ['a', 'b']] as $method => $values) {
    $warnings = [];
    set_error_handler(function ($level, $message) use (&$warnings) {
        $warnings[] = preg_replace('/^.*\(\): /', '', $message);
        return true;
    });
    $rejected = 0;
    foreach ([-1, 12, 99, 2147483647] as $op) {
        $rejected += $rules->$method($op, $format, ...$values) === false;
    }
    restore_error_handler();
    echo "$method rejected=$rejected\n", implode("\n", $warnings), "\n";
    $accepted = 0;
    for ($op = 0; $op <= 11; $op++) {
        $accepted += $rules->$method($op, $format, ...$values) === true;
    }
    echo "$method accepted=$accepted\n";
}
?>
--EXPECT--
addOpNumRule rejected=4
Argument out of int range
Invalid conditional formatting operator
Invalid conditional formatting operator
Invalid conditional formatting operator
addOpNumRule accepted=12
addOpStrRule rejected=4
Argument out of int range
Invalid conditional formatting operator
Invalid conditional formatting operator
Invalid conditional formatting operator
addOpStrRule accepted=12
