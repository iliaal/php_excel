--TEST--
Cell Postion Identification
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
		$s->firstRow(),
		$s->lastRow(),
		$s->firstCol(),
		$s->lastCol()
	);
	

	
	echo "OK\n";
?>
--EXPECT--
int(1)
int(13)
int(1)
int(12)
OK
