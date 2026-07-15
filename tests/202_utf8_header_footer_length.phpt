--TEST--
Header and footer limits count UTF-8 characters rather than encoded bytes
--EXTENSIONS--
excel
--FILE--
<?php
$book = new ExcelBook(null, null, true);
$book->setLocale('UTF-8');
$sheet = $book->addSheet('Sheet1');
$withinLimit = str_repeat('é', 255);
$overLimit = str_repeat('é', 256);

var_dump($sheet->setHeader($withinLimit, 0.5));
var_dump($sheet->header() === $withinLimit);
var_dump(@$sheet->setHeader($overLimit, 0.5));
var_dump($sheet->setFooter($withinLimit, 0.5));
var_dump($sheet->footer() === $withinLimit);
var_dump(@$sheet->setFooter($overLimit, 0.5));
?>
--EXPECT--
bool(true)
bool(true)
bool(false)
bool(true)
bool(true)
bool(false)
