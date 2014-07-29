--TEST--
Book::packDateValues()
--SKIPIF--
<?php if (!extension_loaded("excel") || !in_array('packDateValues', get_class_methods('ExcelBook'))) print "skip"; ?>
--FILE--
<?php 
	$x = new ExcelBook();
	var_dump(
		$x->packDateValues(),
		$x->packDateValues(2013, 10, 12, 1, 10, 30),
		$x->packDateValues(-1, 10, 12, 1, 10, 30),
		$x->packDateValues(2013, -10, 12, 1, 10, 30),
		$x->packDateValues(2013, 10, -12, 1, 10, 30),
		$x->packDateValues(2013, 10, 12, -1, 10, 30),
		$x->packDateValues(2013, 10, 12, 1, -10, 30),
		$x->packDateValues(2013, 10, 12, 1, 10, -30)
	);
?>
--EXPECTF--
Warning: ExcelBook::packDateValues() expects exactly 6 parameters, 0 given in %s on line %d

Warning: ExcelBook::packDateValues(): Invalid '-1' value for year in %s on line %d

Warning: ExcelBook::packDateValues(): Invalid '-10' value for month in %s on line %d

Warning: ExcelBook::packDateValues(): Invalid '-12' value for day in %s on line %d

Warning: ExcelBook::packDateValues(): Invalid '-1' value for hour in %s on line %d

Warning: ExcelBook::packDateValues(): Invalid '-10' value for minute in %s on line %d

Warning: ExcelBook::packDateValues(): Invalid '-30' value for second in %s on line %d
bool(false)
float(41559.048958333)
bool(false)
bool(false)
bool(false)
bool(false)
bool(false)
bool(false)
