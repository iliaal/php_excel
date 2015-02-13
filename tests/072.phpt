--TEST--
Book::isTemplate() / Book::setTemplate()
--SKIPIF--
<?php
    if (!extension_loaded("excel")) die("skip - Excel extension not found");
    if (!in_array('isTemplate', get_class_methods('ExcelBook'))) die("skip - ExcelBook::isTemplate() missing");
?>
--FILE--
<?php 
	$x = new ExcelBook();
	var_dump($x->isTemplate());
	$x->setTemplate(true);
	var_dump($x->isTemplate());
	$x->setTemplate(false);
	var_dump($x->isTemplate());
?>
--EXPECT--
bool(false)
bool(true)
bool(false)
