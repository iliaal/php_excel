--TEST--
Cell Merge Tests
--INI--
date.timezone=America/Toronto
--SKIPIF--
<?php if (!extension_loaded("excel")) die("skip - Excel extension not found"); ?>
--FILE--
<?php 
	$x = new ExcelBook();

	$s = $x->addSheet("Sheet 1");

	var_dump($s->setMerge(1,10,1,5), $x->getError());

	var_dump($s->getMerge(1,1), $x->getError());

	var_dump($s->deleteMerge(1,1), $x->getError());

	var_dump($s->getMerge(1,1), $x->getError());


	
	echo "OK\n";
?>
--EXPECT--
bool(true)
bool(false)
array(4) {
  ["row_first"]=>
  int(1)
  ["row_last"]=>
  int(10)
  ["col_first"]=>
  int(1)
  ["col_last"]=>
  int(5)
}
bool(false)
bool(true)
bool(false)
bool(false)
string(32) "this cell isn't in merging block"
OK
