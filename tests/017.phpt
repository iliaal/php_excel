--TEST--
Hidden Format
--INI--
date.timezone=America/Toronto
--SKIPIF--
<?php if (!extension_loaded("excel")) print "skip"; ?>
--FILE--
<?php 
	$x = new ExcelBook();

	$s = $x->addSheet("Sheet 1");

	$data = "Heading";

	$format = $x->addFormat();
	$format->hidden(true);

	var_dump($s->write(0, 1, $data, $format));
	var_dump($x->getError());

	var_dump($s->write(1, 0, $data, $format));
	var_dump($x->getError());

	$s->read(1, 0, $format);
	echo (int)$format->hidden() . "\n";

	$s->read(0, 1, $format);
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
