--TEST--
Text Wrap Test
--INI--
date.timezone=America/Toronto
--SKIPIF--
<?php if (!extension_loaded("excel")) print "skip"; ?>
--FILE--
<?php 
	$x = new ExcelBook();

	$s = $x->addSheet("Sheet 1");

	$oClass = new ReflectionClass('ExcelFormat');

	$data = str_repeat("my test data", 10);;

	$format = $x->addFormat();
	$format->wrap(false);

	var_dump($s->write(1, 0, $data, $format));
	var_dump($x->getError());

	$format = $x->addFormat();
	$format->wrap(true);

	var_dump($s->write(2, 0, $data, $format));
	var_dump($x->getError());

	for($i = 1; $i < 3; $i++) {
		$format = '';
		$s->read($i, 0, $format);
		echo (int)$format->wrap() . "\n";
	}



	echo "OK\n";
?>
--EXPECT--
bool(true)
bool(false)
bool(true)
bool(false)
0
1
OK
