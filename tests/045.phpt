--TEST--
Column Heading Printing
--INI--
date.timezone=America/Toronto
--SKIPIF--
<?php if (!extension_loaded("excel")) print "skip"; ?>
--FILE--
<?php 
	$x = new ExcelBook();

	$s = $x->addSheet("Sheet 1");

	$s->write(1, 1, "Test");

	var_dump($s->printHeaders());


	
	echo "OK\n";
?>
--EXPECT--
bool(false)
OK
