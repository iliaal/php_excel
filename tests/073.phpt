--TEST--
Sheet::getRightToLeft() / Sheet::setRightToLeft()
--SKIPIF--
<?php
    if (!extension_loaded("excel")) die("skip - Excel extension not found");
    if (!in_array('getRightToLeft', get_class_methods('ExcelSheet'))) die("skip - ExcelSheet::getRightToLeft() missing");
?>
--FILE--
<?php 
	$x = new ExcelBook();
	$s = $x->addSheet("Sheet 1");
	
	var_dump($s->getRightToLeft() == ExcelSheet::LEFT_TO_RIGHT);
	$s->setRightToLeft(ExcelSheet::RIGHT_TO_LEFT);
	var_dump($s->getRightToLeft() == ExcelSheet::RIGHT_TO_LEFT);
	$s->setRightToLeft(ExcelSheet::LEFT_TO_RIGHT);
	var_dump($s->getRightToLeft() == ExcelSheet::LEFT_TO_RIGHT);
?>
--EXPECT--
bool(true)
bool(true)
bool(true)
