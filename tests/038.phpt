--TEST--
Grid Lines Test
--INI--
date.timezone=America/Toronto
--SKIPIF--
<?php if (!extension_loaded("excel")) die("skip - Excel extension not found"); ?>
--FILE--
<?php 
	$x = new ExcelBook();

	$s = $x->addSheet("Sheet 1");

	$s->write(1, 1, "Test");
	$s->write(12, 11, "Test 2");

	var_dump(
		$s->displayGridlines(),
		$s->setDisplayGridlines(false),
		$s->displayGridlines()
	);

	var_dump(
		$s->printGridlines(),
		$s->setPrintGridlines(true),
		$s->printGridlines()
	);
	

	
	echo "OK\n";
?>
--EXPECT--
bool(true)
NULL
bool(false)
bool(false)
NULL
bool(true)
OK
