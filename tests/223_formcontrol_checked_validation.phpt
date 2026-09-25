--TEST--
ExcelFormControl::setChecked() accepts only documented checked states
--EXTENSIONS--
excel
--FILE--
<?php
$book = new ExcelBook(null, null, true);
var_dump($book->loadFile(__DIR__ . '/formcontrols.xlsx'));
$sheet = $book->getSheet(0);
$checkbox = $sheet->formControl(0);

foreach ([
    ExcelFormControl::CHECKEDTYPE_UNCHECKED,
    ExcelFormControl::CHECKEDTYPE_CHECKED,
    ExcelFormControl::CHECKEDTYPE_MIXED,
] as $state) {
    var_dump($checkbox->setChecked($state));
}
var_dump($checkbox->checked());

$warnings = 0;
set_error_handler(function () use (&$warnings): bool {
    $warnings++;
    return true;
});
for ($index = 0; $index < $sheet->formControlSize(); $index++) {
    $control = $sheet->formControl($index);
    $before = $control->checked();
    var_dump($control->setChecked(3));
    var_dump($control->checked() === $before);
}
$checkbox = $sheet->formControl(0);
$before = $checkbox->checked();
var_dump($checkbox->setChecked(-1));
var_dump($checkbox->checked() === $before);
var_dump($checkbox->setChecked(PHP_INT_MAX));
var_dump($checkbox->checked() === $before);
restore_error_handler();
var_dump($warnings);
echo "OK\n";
?>
--EXPECT--
bool(true)
bool(true)
bool(true)
bool(true)
int(2)
bool(false)
bool(true)
bool(false)
bool(true)
bool(false)
bool(true)
bool(false)
bool(true)
bool(false)
bool(true)
bool(false)
bool(true)
bool(false)
bool(true)
bool(false)
bool(true)
bool(false)
bool(true)
int(9)
OK
