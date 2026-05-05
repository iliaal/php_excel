--TEST--
Cell Copy
--INI--
date.timezone=America/Toronto
--EXTENSIONS--
excel
--FILE--
<?php 
	$x = new ExcelBook();

	$s = $x->addSheet("Sheet 1");

	$s->write(1, 1, "Test");

	$s->copy(1,1,2,2);
	var_dump($x->getError());


	
	echo "OK\n";
?>
--EXPECT--
bool(false)
OK
