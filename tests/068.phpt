--TEST--
Sheet::isHidden() / Sheet::setHidden()
--SKIPIF--
<?php
    if (!extension_loaded("excel")) die("skip - Excel extension not found");
    if (!in_array('isHidden', get_class_methods('ExcelSheet'))) die("skip - ExcelSheet::isHidden() missing");
?>
--FILE--
<?php 
	$x = new ExcelBook();
	$s = $x->addSheet("Sheet 1");
	$s2 = $x->addSheet("Sheet 2");
	var_dump(
		$s->isHidden(),
		$s->setHidden(true),
		$s->isHidden(),
		$s->setHidden(false),
		$s->isHidden()
	);

	$y =  new ExcelBook();
	$s = $y->addSheet("Sheet 1");
	var_dump($s->setHidden(true));
?>
--EXPECT--
bool(false)
bool(true)
bool(true)
bool(true)
bool(false)
bool(false)
