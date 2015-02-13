--TEST--
Sheet Page Breaks
--SKIPIF--
<?php if (!extension_loaded("excel")) die("skip - Excel extension not found"); ?>
--FILE--
<?php 
	$x = new ExcelBook();

	$s = $x->addSheet("Sheet 1");

	var_dump($s->horPageBreak(10, 1));
	
	var_dump($s->verPageBreak(10, 1));

	var_dump($s->horPageBreak(20, 1));
	
	var_dump($s->verPageBreak(20, 1));

	var_dump($s->horPageBreak(20, 0));
	
	var_dump($s->verPageBreak(20, 0));


	
	echo "OK\n";
?>
--EXPECT--
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
OK
