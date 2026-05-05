--TEST--
Font constructor test
--INI--
date.timezone=America/Toronto
--EXTENSIONS--
excel
--FILE--
<?php
	$x = new ExcelBook();

	try {
		$format = new ExcelFont();
	} catch (\Throwable $e) {
		echo get_class($e) . "\n";
	}

	try {
		$format = new ExcelFont('cdsd');
	} catch (\Throwable $e) {
		echo get_class($e) . "\n";
	}
	echo "OK\n";
?>
--EXPECT--
ArgumentCountError
TypeError
OK
