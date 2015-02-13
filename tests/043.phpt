--TEST--
Printing Alignment
--INI--
date.timezone=America/Toronto
--SKIPIF--
<?php if (!extension_loaded("excel")) die("skip - Excel extension not found"); ?>
--FILE--
<?php 
	$x = new ExcelBook();

	$s = $x->addSheet("Sheet 1");

	$s->write(1, 1, "Test");

	var_dump($s->hcenter(), $s->vcenter());
	
	var_dump($s->setHCenter(true), $s->setVCenter(true));

	var_dump($s->hcenter(), $s->vcenter());


	
	echo "OK\n";
?>
--EXPECT--
bool(false)
bool(false)
NULL
NULL
bool(true)
bool(true)
OK
