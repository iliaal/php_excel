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
	} catch (Throwable $e) {
		var_dump($e->getMessage());
	}

	try {
		$format = new ExcelFormat('cdsd');
	} catch (Throwable $e) {
		var_dump($e->getMessage());
	}

	echo "OK\n";
?>
--EXPECTF--
string(63) "ExcelFormat::__construct() expects exactly 1 parameter, 0 given"
string(94) "Argument 1 passed to ExcelFormat::__construct() must be an instance of ExcelBook, string given"
OK
