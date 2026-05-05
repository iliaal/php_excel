--TEST--
Book::getPhpExcelVersion()
--EXTENSIONS--
excel
--SKIPIF--
<?php if (!in_array('getPhpExcelVersion', get_class_methods('ExcelBook'))) print "skip"; ?>
--FILE--
<?php 
$book = new ExcelBook();

var_dump(
    !is_null($book->getPhpExcelVersion())
);

?>
--EXPECT--
bool(true)