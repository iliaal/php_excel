--TEST--
Book::isTemplate() / Book::setTemplate()
--EXTENSIONS--
excel
--SKIPIF--
<?php if (!in_array('isTemplate', get_class_methods('ExcelBook'))) print "skip"; ?>
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
