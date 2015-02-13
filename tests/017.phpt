--TEST--
Hidden Format
--INI--
date.timezone=America/Toronto
--SKIPIF--
<?php if (!extension_loaded("excel")) die("skip - Excel extension not found"); ?>
--FILE--
<?php 
	$x = new ExcelBook();

	$s = $x->addSheet("Sheet 1");

	$data = "Heading";

	$format = $x->addFormat();
	$format->hidden(true);

	var_dump($s->write(1, 1, $data, $format));
	var_dump($x->getError());

	var_dump($s->write(2, 0, $data, $format));
	var_dump($x->getError());

	$s->read(2, 0, $format);
	echo (int)$format->hidden() . "\n";

	$s->read(1, 1, $format);
	echo (int)$format->hidden() . "\n";



	echo "OK\n";
?>
--EXPECT--
bool(true)
bool(false)
bool(true)
bool(false)
1
1
OK
