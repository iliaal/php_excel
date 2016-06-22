--TEST--
Font constructor test
--INI--
date.timezone=America/Toronto
--SKIPIF--
<?php if (!extension_loaded("excel")) print "skip"; ?>
--FILE--
<?php 
	$x = new ExcelBook();

	try {
		$format = new ExcelFont();
	} catch (Exception $e) {
		var_dump($e->getMessage());
	}

	try {
		$format = new ExcelFont('cdsd');
	} catch (Exception $e) {
		var_dump($e->getMessage());
	}
?>
--EXPECTF--
string(61) "ExcelFont::__construct() expects exactly 1 parameter, 0 given"

Fatal error: Uncaught TypeError: Argument 1 passed to ExcelFont::__construct() must be an instance of ExcelBook, string given in %s:%d
Stack trace:
#0 %s(%d): ExcelFont->__construct('cdsd')
#1 {main}
  thrown in %s on line %d
