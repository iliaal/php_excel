--TEST--
Named Range test
--SKIPIF--
<?php if (!extension_loaded("excel")) die("skip - Excel extension not found"); ?>
--FILE--
<?php 
	$x = new ExcelBook();
	$s = $x->addSheet("Sheet 1");

	var_dump($s->setNamedRange("test", 1, 1, 10, 10));
	var_dump($s->delNamedRange("test"));

	echo "OK\n";
?>
--EXPECT--
bool(true)
bool(true)
OK
