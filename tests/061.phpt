--TEST--
getPrintFit() / setPrintFit() tests
--SKIPIF--
<?php if (!extension_loaded("excel")) die("skip - Excel extension not found"); ?>
--FILE--
<?php 
	$x = new ExcelBook();
	$s = $x->addSheet("Sheet A");

	var_dump($s->getPrintFit());
	var_dump($s->setPrintFit(200, 300));
	var_dump($s->getPrintFit());
?>
--EXPECT--
bool(false)
bool(true)
array(2) {
  ["width"]=>
  int(200)
  ["height"]=>
  int(300)
}
