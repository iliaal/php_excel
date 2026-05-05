--TEST--
Print Area tests
--EXTENSIONS--
excel
--FILE--
<?php 
	$x = new ExcelBook();
	$s = $x->addSheet("Sheet 1");

	$s->setPrintRepeatRows(1, 1);
	
	$s->clearPrintRepeats();
	
	$s->setPrintRepeatCols(10, 12);

	$s->clearPrintArea();

	echo "OK\n";
?>
--EXPECT--
OK
