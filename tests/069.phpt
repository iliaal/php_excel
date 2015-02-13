--TEST--
Sheet::setTopLeftView() / Sheet::getTopLeftView()
--SKIPIF--
<?php
    if (!extension_loaded("excel")) die("skip - Excel extension not found");
    if (!in_array('getTopLeftView', get_class_methods('ExcelSheet'))) die("skip - ExcelSheet::getTopLeftView() missing");
?>
--FILE--
<?php 
	$x = new ExcelBook();
	$s = $x->addSheet("Sheet 1");
	var_dump(
		$s->getTopLeftView(),
		$s->setTopLeftView(2,2),
		$s->getTopLeftView()
	);
?>
--EXPECT--
array(2) {
  ["row"]=>
  int(0)
  ["column"]=>
  int(0)
}
bool(true)
array(2) {
  ["row"]=>
  int(2)
  ["column"]=>
  int(2)
}
