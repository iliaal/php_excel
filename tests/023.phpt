--TEST--
Excel Sheet Manipulation
--INI--
date.timezone=America/Toronto
--SKIPIF--
<?php if (!extension_loaded("excel")) print "skip"; ?>
--FILE--
<?php 
	$x = new ExcelBook();

	$s = $x->addSheet("Sheet 1");
	var_dump($s->write(1, 1, 'Sheet #1'));
	var_dump($x->getError());

	$s2 = $x->copySheet("Sheet 2 (copy of 1)", 0);
	var_dump($s2, $x->getError());

	var_dump($x->sheetCount());

	var_dump($x->deleteSheet(2), $x->deleteSheet(1));

	var_dump($x->getSheet(0));

	$s3 = $x->copySheet("Sheet 3 (copy of 1)", 0);
	var_dump($s2, $x->getError());

	var_dump($x->activeSheet());
	
	$x->setActiveSheet(1);
	
	var_dump($s3->write(2, 2, 'Sheet 2 (copy)'));
	var_dump($x->getError());

	$x->setActiveSheet(1);

	var_dump($s->write(3, 3, 'Sheet 1 (back to start)'));
	var_dump($x->getError());



	echo "OK\n";
?>
--EXPECT--
bool(true)
bool(false)
object(ExcelSheet)#3 (0) {
}
bool(false)
int(2)
bool(false)
bool(true)
object(ExcelSheet)#4 (0) {
}
object(ExcelSheet)#3 (0) {
}
bool(false)
int(0)
bool(true)
bool(false)
bool(true)
bool(false)
OK
