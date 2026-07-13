--TEST--
ExcelSheet::write does not query a native book after a validation warning callback
--EXTENSIONS--
excel
--FILE--
<?php
$book = new ExcelBook(null, null, true);
$sheet = $book->addSheet('S');
set_error_handler(static function () use ($book): bool {
    $book->__construct(null, null, true);
    return true;
});

var_dump($sheet->write(1, 0, new stdClass()));
restore_error_handler();
var_dump($book->addSheet('after') instanceof ExcelSheet);
echo "OK\n";
?>
--EXPECT--
bool(false)
bool(true)
OK
