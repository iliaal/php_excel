--TEST--
Sheet::addrToRowCol() / Sheet::rowColToAddr()
--SKIPIF--
<?php
    if (!extension_loaded("excel")) die("skip - Excel extension not found");
    if (!in_array('addrToRowCol', get_class_methods('ExcelSheet'))) die("skip - ExcelSheet::addrToRowCol() missing");
?>
--FILE--
<?php 
	$x = new ExcelBook();
	$s = $x->addSheet("Sheet 1");
	var_dump($s->write(1, 1, "Test"));
	$data = $s->rowColToAddr(1,1);

	var_dump($data, $s->addrToRowCol($data));
	var_dump($s->addrToRowCol(""));
?>
--EXPECTF--
bool(true)
string(2) "B2"
array(4) {
  ["row"]=>
  int(1)
  ["column"]=>
  int(1)
  ["col_relative"]=>
  bool(true)
  ["row_relative"]=>
  bool(true)
}

Warning: ExcelSheet::addrToRowCol(): Cell reference cannot be empty in %s on line %d
bool(false)
