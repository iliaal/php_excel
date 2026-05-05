--TEST--
R1C1 functions
--EXTENSIONS--
excel
--FILE--
<?php 
	$x = new ExcelBook(null, null, true);
	var_dump($x->getRefR1C1());
	$x->setRefR1C1(1);
	var_dump($x->getRefR1C1());
?>
--EXPECT--
bool(false)
bool(true)
