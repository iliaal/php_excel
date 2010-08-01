--TEST--
Format constructor test
--INI--
date.timezone=America/Toronto
--SKIPIF--
<?php if (!extension_loaded("excel")) print "skip"; ?>
--FILE--
<?php 
	$x = new ExcelBook();

	try {
		$format = new ExcelFormat();
	} catch (Exception $e) {
		var_dump($e->getMessage());
	}

	try {
		$format = new ExcelFormat('cdsd');
	} catch (Exception $e) {
		var_dump($e->getMessage());
	}

	echo "OK\n";
?>
--EXPECT--
string(63) "ExcelFormat::__construct() expects exactly 1 parameter, 0 given"
string(76) "ExcelFormat::__construct() expects parameter 1 to be ExcelBook, string given"
OK
