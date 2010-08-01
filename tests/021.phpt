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

	echo "OK\n";
?>
--EXPECT--
string(61) "ExcelFont::__construct() expects exactly 1 parameter, 0 given"
string(74) "ExcelFont::__construct() expects parameter 1 to be ExcelBook, string given"
OK
