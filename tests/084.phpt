--TEST--
test Sheet::read(.., $format) issue #86
--SKIPIF--
<?php if (!extension_loaded("excel")) print "skip"; ?>
--FILE--
<?php

$book = new ExcelBook();
$sheet = $book->addSheet("Sheet1");

$sheet->read(1, 1, $format);

var_dump(
    $format->borderColor()
)

?>
--EXPECT--
bool(true)
