--TEST--
New range related functions
--SKIPIF--
<?php if (!extension_loaded("excel")) die("skip - Excel extension not found"); ?>
--FILE--
<?php 
	$x = new ExcelBook();
	$s = $x->addSheet("Sheet 1");

	var_dump($s->setNamedRange("test", 1, 1, 10, 10));
	var_dump($s->setNamedRange("test2", 20, 20, 30, 30));

	var_dump($s->getNamedRange("test"));
	var_dump($s->getNamedRange("nothing"));
	
	var_dump($s->getIndexRange(1));
	var_dump($s->getIndexRange(100));

	var_dump($s->namedRangeSize());

	var_dump($s->delNamedRange("test"));
	var_dump($s->delNamedRange("test2"));

	echo "OK\n";
?>
--EXPECT--
bool(true)
bool(true)
array(5) {
  ["row_first"]=>
  int(1)
  ["row_last"]=>
  int(1)
  ["col_first"]=>
  int(10)
  ["col_last"]=>
  int(10)
  ["hidden"]=>
  bool(false)
}
bool(false)
array(6) {
  ["row_first"]=>
  int(20)
  ["row_last"]=>
  int(20)
  ["col_first"]=>
  int(30)
  ["col_last"]=>
  int(30)
  ["hidden"]=>
  bool(false)
  ["scope"]=>
  int(-1)
}
bool(false)
int(2)
bool(true)
bool(true)
OK
