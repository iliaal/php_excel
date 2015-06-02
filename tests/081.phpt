--TEST--
Sheet::write***() implicit formula parsing with =
--SKIPIF--
<?php if (!extension_loaded("excel")) print "skip"; ?>
--FILE--
<?php
$book = new ExcelBook();
$sheet = $book->addSheet("Sheet 1");

$sheet->write(1, 1, '=3+2+1');
$sheet->write(2, 2, '3+2+1');

var_dump(
    $sheet->isFormula(1, 1),
    $sheet->isFormula(2, 2)
);
?>
--EXPECT--
bool(true)
bool(false)
