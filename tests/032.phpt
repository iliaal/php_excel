--TEST--
Sheet Split
--INI--
date.timezone=America/Toronto
--SKIPIF--
<?php if (!extension_loaded("excel")) die("skip - Excel extension not found"); ?>
--FILE--
<?php 
	$x = new ExcelBook();

	$s = $x->addSheet("Sheet 1");

	$s->splitSheet(10, 10);
	var_dump($x->getError());


	
	echo "OK\n";
?>
--EXPECT--
bool(false)
OK
