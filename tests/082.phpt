--TEST--
Sheet::write***() auto date format
--SKIPIF--
<?php if (!extension_loaded("excel")) print "skip"; ?>
--FILE--
<?php
$book = new ExcelBook();
$sheet = $book->addSheet("Sheet 1");

$sheet->write(1, 1, (new \DateTime())->getTimestamp(), null, \ExcelFormat::AS_DATE);

var_dump(
    $sheet->isDate(1, 1)
);
?>
--EXPECT--
bool(true)
