--TEST--
Shrink to fit
--INI--
date.timezone=America/Toronto
--SKIPIF--
<?php if (!extension_loaded("excel")) die("skip - Excel extension not found"); ?>
--FILE--
<?php 
	$x = new ExcelBook();

	$s = $x->addSheet("Sheet 1");

	$data = "Test";

	$format = $x->addFormat();
	$format->shrinkToFit(true);

	var_dump($s->write(1, 0, $data, $format));
	var_dump($x->getError());

	var_dump($s->write(2, 0, $data));
	var_dump($x->getError());



	for($i = 1; $i < 3; $i++) {
		$format = '';
		$s->read($i, 0, $format);
		echo (int)$format->shrinkToFit() . "\n";
	}

	echo "OK\n";
?>
--EXPECT--
bool(true)
bool(false)
bool(true)
bool(false)
1
0
OK
