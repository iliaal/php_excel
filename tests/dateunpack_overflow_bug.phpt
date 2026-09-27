--TEST--
Excel date pack/unpack overflow tests
--EXTENSIONS--
excel
--INI--
date.timezone=UTC
--FILE--
<?php
$book = new ExcelBook();
$serial = $book->packDateValues(2100, 1, 1, 0, 0, 0);
$timestamp = $book->unpackDate($serial);
if (PHP_INT_SIZE === 4) {
    var_dump($timestamp === false);
} else {
    var_dump($timestamp === 4102444800 && $book->packDate($timestamp) === $serial);
}
var_dump($book->unpackDate($book->packDate(2147483647)) === 2147483647);
?>
--EXPECT--
bool(true)
bool(true)
