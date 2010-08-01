--TEST--
Sheet Name Manipulation
--INI--
date.timezone=America/Toronto
--SKIPIF--
<?php if (!extension_loaded("excel")) print "skip"; ?>
--FILE--
<?php 
	$x = new ExcelBook();

	$s = $x->addSheet("Sheet 1");

	$s->write(1, 1, "Test");

	var_dump($s->name());
	$s->setName("New Name");
	var_dump($x->getError(), $s->name());


	
	echo "OK\n";
?>
--EXPECT--
string(7) "Sheet 1"
bool(false)
string(8) "New Name"
OK
