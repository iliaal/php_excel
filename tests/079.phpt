--TEST--
Book::sheetType()
--SKIPIF--
<?php if (!extension_loaded("excel") || !in_array('sheetType', get_class_methods('ExcelBook'))) print "skip"; ?>
--FILE--
<?php 
$book = new ExcelBook();

var_dump(
    $book->sheetType(0)
);

$sheet = $book->addSheet('Sheet 1');

var_dump(
    $book->sheetType(0)
);
?>
--EXPECT--
int(2)
int(0)