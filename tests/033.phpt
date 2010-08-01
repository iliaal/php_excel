--TEST--
Row/Column Grouping
--INI--
date.timezone=America/Toronto
--SKIPIF--
<?php if (!extension_loaded("excel")) print "skip"; ?>
--FILE--
<?php 
	$x = new ExcelBook();

	$s = $x->addSheet("Sheet 1");

	for ($i = 0; $i < 10; $i++) {
		$s->write(1, $i, "Row 1");
		$s->write(2, $i, "Row 2");
		$s->write(5, $i, "Row 3");
		$s->write(6, $i, "Row 4");
	}

	var_dump($s->groupRows(1,2), $x->getError());
	var_dump($s->groupRows(5,6, true), $x->getError());

	var_dump($s->groupCols(1,2), $x->getError());
	var_dump($s->groupCols(5,6, true), $x->getError());


	
	echo "OK\n";
?>
--EXPECT--
bool(true)
bool(false)
bool(true)
bool(false)
bool(true)
bool(false)
bool(true)
bool(false)
OK
