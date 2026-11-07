--TEST--
Data validations reject unsupported enum values and accept every documented constant
--EXTENSIONS--
excel
--FILE--
<?php
foreach (['addDataValidation' => ['1', '2'], 'addDataValidationDouble' => [1.0, 2.0]] as $method => $values) {
    $book = new ExcelBook(null, null, true);
    $sheet = $book->addSheet('Validation');
    $args = [ExcelSheet::VALIDATION_TYPE_WHOLE, ExcelSheet::VALIDATION_OP_EQUAL,
        1, 1, 0, 0, ...$values, true, false, true, true, '', '', '', '',
        ExcelSheet::VALIDATION_ERRSTYLE_WARNING];
    foreach (['type' => [0, 7], 'operator' => [1, 7], 'error style' => [16, 2]] as $name => [$index, $max]) {
        $warnings = [];
        set_error_handler(function ($level, $message) use (&$warnings) {
            $warnings[] = preg_replace('/^.*\(\): /', '', $message);
            return true;
        });
        $rejected = 0;
        foreach ([-1, $max + 1, 99, 2147483647] as $invalid) {
            $bad = $args;
            $bad[$index] = $invalid;
            $rejected += $sheet->$method(...$bad) === false;
        }
        restore_error_handler();
        echo "$method $name rejected=$rejected\n", implode("\n", $warnings), "\n";

        $accepted = 0;
        for ($valid = 0; $valid <= $max; $valid++) {
            $good = $args;
            $good[$index] = $valid;
            $accepted += $sheet->$method(...$good) === true;
        }
        echo "$method $name accepted=$accepted\n";
    }
    echo "$method defaults=";
    var_dump($sheet->$method(ExcelSheet::VALIDATION_TYPE_WHOLE,
        ExcelSheet::VALIDATION_OP_EQUAL, 1, 1, 0, 0, $values[0]));
}
?>
--EXPECT--
addDataValidation type rejected=4
Argument out of int range
Invalid data validation type
Invalid data validation type
Invalid data validation type
addDataValidation type accepted=8
addDataValidation operator rejected=4
Argument out of int range
Invalid data validation operator
Invalid data validation operator
Invalid data validation operator
addDataValidation operator accepted=8
addDataValidation error style rejected=4
Argument out of int range
Invalid data validation error style
Invalid data validation error style
Invalid data validation error style
addDataValidation error style accepted=3
addDataValidation defaults=bool(true)
addDataValidationDouble type rejected=4
Argument out of int range
Invalid data validation type
Invalid data validation type
Invalid data validation type
addDataValidationDouble type accepted=8
addDataValidationDouble operator rejected=4
Argument out of int range
Invalid data validation operator
Invalid data validation operator
Invalid data validation operator
addDataValidationDouble operator accepted=8
addDataValidationDouble error style rejected=4
Argument out of int range
Invalid data validation error style
Invalid data validation error style
Invalid data validation error style
addDataValidationDouble error style accepted=3
addDataValidationDouble defaults=bool(true)
