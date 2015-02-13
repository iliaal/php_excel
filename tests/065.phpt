--TEST--
Book:biffVersion();
--SKIPIF--
<?php if (!extension_loaded("excel")) die("skip - Excel extension not found"); ?>
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
