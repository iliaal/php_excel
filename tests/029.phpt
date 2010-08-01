--TEST--
Width/Height Checks
--INI--
date.timezone=America/Toronto
--SKIPIF--
<?php if (!extension_loaded("excel")) print "skip"; ?>
--FILE--
<?php 
	$x = new ExcelBook();

	$s = $x->addSheet("Sheet 1");

	$s->setColWidth(1, 1, 55);
	$s->setRowHeight(4, 50);

	var_dump($s->colWidth(1));
	var_dump($s->rowHeight(4));


	
	echo "OK\n";
?>
--EXPECT--
float(55)
float(50)
OK
