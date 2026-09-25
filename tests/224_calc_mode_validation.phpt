--TEST--
ExcelBook::setCalcMode() accepts only documented calculation modes
--EXTENSIONS--
excel
--FILE--
<?php
$book = new ExcelBook(null, null, true);
foreach ([
    ExcelBook::CALCMODE_MANUAL,
    ExcelBook::CALCMODE_AUTO,
    ExcelBook::CALCMODE_AUTONOTABLE,
] as $mode) {
    var_dump($book->setCalcMode($mode));
    var_dump($book->calcMode());
}

set_error_handler(static fn(): bool => true);
foreach ([3, 100, -1, PHP_INT_MAX] as $mode) {
    var_dump($book->setCalcMode($mode));
    var_dump($book->calcMode());
}
restore_error_handler();
echo "OK\n";
?>
--EXPECT--
bool(true)
int(0)
bool(true)
int(1)
bool(true)
int(2)
bool(false)
int(2)
bool(false)
int(2)
bool(false)
int(2)
bool(false)
int(2)
OK
