--TEST--
Date conversion follows PHP's timezone and rejects timestamps outside PHP's integer range
--EXTENSIONS--
excel
--ENV--
TZ=UTC
--INI--
date.timezone=America/New_York
--FILE--
<?php
$book = new ExcelBook();
foreach (['1969-12-31 23:59:59', '2024-01-15 12:30:45', '2024-07-15 12:30:45'] as $date) {
    $timestamp = strtotime($date);
    $parts = array_map('intval', explode(' ', date('Y n j G i s', $timestamp)));
    $serial = $book->packDateValues(...$parts);
    var_dump($book->unpackDate($serial) === $timestamp);
    var_dump($book->packDate($timestamp) === $serial);
}
var_dump($book->unpackDate(0.0));
var_dump($book->unpackDate(0.5));
var_dump($book->unpackDate($book->packDateValues(0, 0, 0, 12, 30, 0)));
date_default_timezone_set('UTC');
$beyond32 = $book->unpackDate($book->packDateValues(2038, 1, 19, 3, 14, 8));
var_dump(PHP_INT_SIZE === 4 ? $beyond32 === false : $beyond32 === 2147483648);
var_dump($book->unpackDate($book->packDateValues(2038, 1, 19, 3, 14, 7)) === 2147483647);
var_dump($book->unpackDate($book->packDateValues(1901, 12, 13, 20, 45, 52)) === -2147483647 - 1);
$before32 = $book->unpackDate($book->packDateValues(1901, 12, 13, 20, 45, 51));
var_dump(PHP_INT_SIZE === 4 ? $before32 === false : $before32 === -2147483649);
?>
--EXPECT--
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
int(0)
int(43200)
int(45000)
bool(true)
bool(true)
bool(true)
bool(true)
