--TEST--
ExcelSheet::writeError() accepts only writable error types
--EXTENSIONS--
excel
--FILE--
<?php
$book = new ExcelBook(null, null, true);
$sheet = $book->addSheet('Errors');
$valid = [
    ExcelSheet::ERRORTYPE_NULL,
    ExcelSheet::ERRORTYPE_DIV_0,
    ExcelSheet::ERRORTYPE_VALUE,
    ExcelSheet::ERRORTYPE_REF,
    ExcelSheet::ERRORTYPE_NAME,
    ExcelSheet::ERRORTYPE_NUM,
    ExcelSheet::ERRORTYPE_NA,
];
foreach ($valid as $row => $code) {
    var_dump($sheet->writeError($row + 1, 0, $code));
    var_dump($sheet->cellType($row + 1, 0) === 5);
    var_dump($sheet->read($row + 1, 0) === $code);
}

// The registered ERRORTYPE_NOERROR sentinel is not writable data and is
// rejected with the documented diagnostic.
$sheet->write(20, 0, 'keep');
$warning = null;
set_error_handler(function (int $severity, string $message) use (&$warning): bool {
    $warning = $message;
    return true;
});
var_dump($sheet->writeError(20, 0, ExcelSheet::ERRORTYPE_NOERROR));
restore_error_handler();
var_dump($warning);
var_dump($sheet->read(20, 0));

$invalid = [
	1,
    43,
    100,
    1000,
    2147483647,
    -1,
    2147483648,
];
foreach ($invalid as $index => $code) {
    $row = 20 + $index;
    $sheet->write($row, 0, 'keep');
    set_error_handler(static fn(): bool => true);
    var_dump($sheet->writeError($row, 0, $code));
    restore_error_handler();
    var_dump($sheet->read($row, 0));
    var_dump($sheet->cellType($row, 0));
}
echo "OK\n";
?>
--EXPECTF--
NULL
bool(true)
bool(true)
NULL
bool(true)
bool(true)
NULL
bool(true)
bool(true)
NULL
bool(true)
bool(true)
NULL
bool(true)
bool(true)
NULL
bool(true)
bool(true)
NULL
bool(true)
bool(true)
bool(false)
string(%d) "ExcelSheet::writeError(): Invalid error type"
string(4) "keep"
bool(false)
string(4) "keep"
int(2)
bool(false)
string(4) "keep"
int(2)
bool(false)
string(4) "keep"
int(2)
bool(false)
string(4) "keep"
int(2)
bool(false)
string(4) "keep"
int(2)
bool(false)
string(4) "keep"
int(2)
bool(false)
string(4) "keep"
int(2)
OK
