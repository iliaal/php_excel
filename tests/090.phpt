--TEST--
test Book::setLocale()
--EXTENSIONS--
excel
--FILE--
<?php

$book = new ExcelBook();
$book->setLocale('UTF-8');

$sheet = $book->addSheet('Sheet1');

echo 'OK';

?>
--EXPECT--
OK
