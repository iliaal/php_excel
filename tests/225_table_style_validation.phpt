--TEST--
Excel table style inputs are restricted to documented TABLESTYLE values
--EXTENSIONS--
excel
--FILE--
<?php
$book = new ExcelBook(null, null, true);
$sheet = $book->addSheet('Tables');
$styles = [
    ExcelTable::TABLESTYLE_NONE,
    ExcelTable::TABLESTYLE_LIGHT1,
    ExcelTable::TABLESTYLE_LIGHT2,
    ExcelTable::TABLESTYLE_LIGHT3,
    ExcelTable::TABLESTYLE_LIGHT4,
    ExcelTable::TABLESTYLE_LIGHT5,
    ExcelTable::TABLESTYLE_LIGHT6,
    ExcelTable::TABLESTYLE_LIGHT7,
    ExcelTable::TABLESTYLE_LIGHT8,
    ExcelTable::TABLESTYLE_LIGHT9,
    ExcelTable::TABLESTYLE_LIGHT10,
    ExcelTable::TABLESTYLE_MEDIUM1,
    ExcelTable::TABLESTYLE_MEDIUM2,
    ExcelTable::TABLESTYLE_MEDIUM3,
    ExcelTable::TABLESTYLE_DARK1,
    ExcelTable::TABLESTYLE_DARK2,
    ExcelTable::TABLESTYLE_DARK3,
];

$table = $sheet->addTable('Setter', 1, 2, 0, 0);
$valid = true;
foreach ($styles as $style) {
    $valid = $table->setStyle($style) && $table->style() === $style && $valid;
}
var_dump($valid);

foreach ($styles as $index => $style) {
    $row = 10 + $index * 3;
    $added = $sheet->addTable('Added' . $index, $row, $row + 1, 0, 0, true, $style);
    $constructed = new ExcelTable($sheet, 'Built' . $index, $row, $row + 1, 1, 1, true, $style);
    var_dump($added instanceof ExcelTable && $added->style() === $style);
    var_dump($constructed->style() === $style);
}

$table->setStyle(ExcelTable::TABLESTYLE_DARK3);
set_error_handler(static fn(): bool => true);
foreach ([999, -1, PHP_INT_MAX] as $style) {
    var_dump($table->setStyle($style));
    var_dump($table->style());
    var_dump($sheet->addTable('Invalid' . $style, 100, 101, 0, 0, true, $style));
    try {
        new ExcelTable($sheet, 'InvalidCtor' . $style, 100, 101, 0, 0, true, $style);
        var_dump(false);
    } catch (Throwable $e) {
        var_dump($e instanceof Exception);
    }
}
restore_error_handler();

set_error_handler(static function (int $severity, string $message): never {
    throw new ErrorException($message);
});
try {
    new ExcelTable($sheet, 'WarningChaining', 100, 101, 0, 0, true, 999);
    var_dump(false);
} catch (Throwable $e) {
    var_dump($e::class);
    var_dump($e->getMessage());
    var_dump($e->getPrevious());
}
restore_error_handler();
var_dump($sheet->getTableByName('Invalid999'));
echo "OK\n";
?>
--EXPECT--
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(false)
int(52)
bool(false)
bool(true)
bool(false)
int(52)
bool(false)
bool(true)
bool(false)
int(52)
bool(false)
bool(true)
string(9) "Exception"
string(19) "Invalid table style"
NULL
bool(false)
OK
