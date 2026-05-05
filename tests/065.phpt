--TEST--
Book:biffVersion();
--EXTENSIONS--
excel
--FILE--
<?php 
	$x = new ExcelBook();
	var_dump($x->biffVersion());
	
	$x = new ExcelBook(null, null, true);
	var_dump($x->biffVersion());
?>
--EXPECT--
int(1536)
bool(false)
