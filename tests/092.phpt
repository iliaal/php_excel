--TEST--
Book::getLibXlVersion()
--EXTENSIONS--
excel
--SKIPIF--
<?php if (!in_array('getLibXlVersion', get_class_methods('ExcelBook'))) print "skip"; ?>
--FILE--
<?php 
$book = new ExcelBook();

var_dump(
    is_numeric($book->getLibXlVersion())
);

?>
--EXPECT--
bool(true)