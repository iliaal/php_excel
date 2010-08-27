--TEST--
getAllFormats() tests
--INI--
date.timezone=America/Toronto
--SKIPIF--
<?php
	if (!extension_loaded("excel")) print "skip";
	if (!method_exists(new ExcelBook(), "getAllFormats")) print "skip";
?>
--FILE--
<?php 
	$x = new ExcelBook();

	var_dump($x->getAllFormats(), $x->getError());

	$f1 = $x->addFormat();
	$f1->borderStyle(ExcelFormat::BORDERSTYLE_DASHED);

	$f2 = $x->addFormat();
	$f2->horizontalAlign(ExcelFormat::ALIGNH_RIGHT);

	var_dump($x->getAllFormats());

	echo "OK\n";
?>
--EXPECT--
array(21) {
  [0]=>
  &object(ExcelFormat)#2 (0) {
  }
  [1]=>
  &object(ExcelFormat)#3 (0) {
  }
  [2]=>
  &object(ExcelFormat)#4 (0) {
  }
  [3]=>
  &object(ExcelFormat)#5 (0) {
  }
  [4]=>
  &object(ExcelFormat)#6 (0) {
  }
  [5]=>
  &object(ExcelFormat)#7 (0) {
  }
  [6]=>
  &object(ExcelFormat)#8 (0) {
  }
  [7]=>
  &object(ExcelFormat)#9 (0) {
  }
  [8]=>
  &object(ExcelFormat)#10 (0) {
  }
  [9]=>
  &object(ExcelFormat)#11 (0) {
  }
  [10]=>
  &object(ExcelFormat)#12 (0) {
  }
  [11]=>
  &object(ExcelFormat)#13 (0) {
  }
  [12]=>
  &object(ExcelFormat)#14 (0) {
  }
  [13]=>
  &object(ExcelFormat)#15 (0) {
  }
  [14]=>
  &object(ExcelFormat)#16 (0) {
  }
  [15]=>
  &object(ExcelFormat)#17 (0) {
  }
  [16]=>
  &object(ExcelFormat)#18 (0) {
  }
  [17]=>
  &object(ExcelFormat)#19 (0) {
  }
  [18]=>
  &object(ExcelFormat)#20 (0) {
  }
  [19]=>
  &object(ExcelFormat)#21 (0) {
  }
  [20]=>
  &object(ExcelFormat)#22 (0) {
  }
}
bool(false)
array(23) {
  [0]=>
  &object(ExcelFormat)#20 (0) {
  }
  [1]=>
  &object(ExcelFormat)#19 (0) {
  }
  [2]=>
  &object(ExcelFormat)#18 (0) {
  }
  [3]=>
  &object(ExcelFormat)#17 (0) {
  }
  [4]=>
  &object(ExcelFormat)#16 (0) {
  }
  [5]=>
  &object(ExcelFormat)#15 (0) {
  }
  [6]=>
  &object(ExcelFormat)#14 (0) {
  }
  [7]=>
  &object(ExcelFormat)#13 (0) {
  }
  [8]=>
  &object(ExcelFormat)#12 (0) {
  }
  [9]=>
  &object(ExcelFormat)#11 (0) {
  }
  [10]=>
  &object(ExcelFormat)#10 (0) {
  }
  [11]=>
  &object(ExcelFormat)#9 (0) {
  }
  [12]=>
  &object(ExcelFormat)#8 (0) {
  }
  [13]=>
  &object(ExcelFormat)#7 (0) {
  }
  [14]=>
  &object(ExcelFormat)#6 (0) {
  }
  [15]=>
  &object(ExcelFormat)#5 (0) {
  }
  [16]=>
  &object(ExcelFormat)#4 (0) {
  }
  [17]=>
  &object(ExcelFormat)#3 (0) {
  }
  [18]=>
  &object(ExcelFormat)#2 (0) {
  }
  [19]=>
  &object(ExcelFormat)#23 (0) {
  }
  [20]=>
  &object(ExcelFormat)#24 (0) {
  }
  [21]=>
  &object(ExcelFormat)#25 (0) {
  }
  [22]=>
  &object(ExcelFormat)#26 (0) {
  }
}
OK
