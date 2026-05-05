--TEST--
Book::packDateValues()
--EXTENSIONS--
excel
--SKIPIF--
<?php if (!in_array('packDateValues', get_class_methods('ExcelBook'))) print "skip"; ?>
--FILE--
<?php
	$x = new ExcelBook();

	try {
		$x->packDateValues();
	} catch (\ArgumentCountError $e) {
		echo $e->getMessage() . "\n";
	}

	var_dump(
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
ExcelBook::packDateValues() expects exactly 6 arguments, 0 given

Warning: ExcelBook::packDateValues(): Invalid '-1' value for year in %s on line %d

Warning: ExcelBook::packDateValues(): Invalid '-10' value for month in %s on line %d

Warning: ExcelBook::packDateValues(): Invalid '-12' value for day in %s on line %d

Warning: ExcelBook::packDateValues(): Invalid '-1' value for hour in %s on line %d

Warning: ExcelBook::packDateValues(): Invalid '-10' value for minute in %s on line %d

Warning: ExcelBook::packDateValues(): Invalid '-30' value for second in %s on line %d
float(41559.04895833%s)
bool(false)
bool(false)
bool(false)
bool(false)
bool(false)
bool(false)
