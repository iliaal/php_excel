--TEST--
rgbMode()/setRGBMode() tests
--INI--
date.timezone=America/Toronto
--SKIPIF--
<?php
	if (!extension_loaded("excel")) print "skip";
	if (!method_exists(new ExcelBook(), "rgbMode")) print "skip";
?>
--FILE--
<?php 
	$x = new ExcelBook(null,null,1);

	var_dump($x->rgbMode());
	$x->setRGBMode(1);
	var_dump($x->rgbMode());
	$x->setRGBMode(0);
	var_dump($x->rgbMode());

	echo "OK\n";
?>
--EXPECT--
bool(false)
bool(true)
bool(false)
OK
