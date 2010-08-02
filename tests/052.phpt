--TEST--
setCellFormat() tests
--INI--
date.timezone=America/Toronto
--SKIPIF--
<?php
	if (!extension_loaded("excel")) print "skip";
	$x = new ExcelBook();
	if (!method_exists($x->addSheet("a"), "setCellFormat")) print "skip";
?>
--FILE--
<?php 
	$x = new ExcelBook();

	$s = $x->addSheet("Sheet 1");

	$s->write(1, 1, "test");
	
	$f = $x->addFormat();
	$f->borderStyle(ExcelFormat::BORDERSTYLE_DASHED);
	$f->horizontalAlign(ExcelFormat::ALIGNH_RIGHT);

	$s->setCellFormat(1,1,$f);
	var_dump($x->getError());
	
	$fo = $s->cellFormat(1,1);
	var_dump($fo->horizontalAlign(), $fo->borderStyle());

	echo "OK\n";
?>
--EXPECT--
bool(false)
int(3)
bool(true)
OK
