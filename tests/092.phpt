--TEST--
Book::getLibXlVersion()
--SKIPIF--
<?php if (!extension_loaded("excel") || !in_array('getLibXlVersion', get_class_methods('ExcelBook'))) print "skip"; ?>
--FILE--
<?php 
$book = new ExcelBook();

var_dump(
    is_numeric($book->getLibXlVersion())
);

?>
--EXPECT--
bool(true)