--TEST--
Named Range test
--EXTENSIONS--
excel
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
