--TEST--
Column Heading Printing
--INI--
date.timezone=America/Toronto
--SKIPIF--
<?php if (!extension_loaded("excel")) die("skip - Excel extension not found"); ?>
--FILE--
<?php 
	$x = new ExcelBook();

	$s = $x->addSheet("Sheet 1");

	$s->write(1, 1, "Test");

	var_dump(
			$s->printHeaders(),
			$s->setPrintHeaders(true),
			$s->printHeaders()
	);

	echo "OK\n";
?>
--EXPECT--
bool(false)
NULL
bool(true)
OK
