--TEST--
Sheet protection test
--INI--
date.timezone=America/Toronto
--SKIPIF--
<?php if (!extension_loaded("excel")) print "skip"; ?>
--FILE--
<?php 
	$x = new ExcelBook();

	$s = $x->addSheet("Sheet 1");

	$s->write(1, 1, "Test");

	var_dump(
			$s->protect(),
			$s->setProtect(true),
			$s->protect()
	);
	
	echo "OK\n";
?>
--EXPECT--
bool(false)
NULL
bool(true)
OK
