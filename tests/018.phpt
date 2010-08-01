--TEST--
Format clone test
--INI--
date.timezone=America/Toronto
--SKIPIF--
<?php if (!extension_loaded("excel")) print "skip"; ?>
--FILE--
<?php 
	$x = new ExcelBook();

	$format = new ExcelFormat($x);

	$formatc = $format;
	$format->shrinkToFit(true);
	$formatb = clone $format;

	$format->indent(1);

	var_dump($format->indent(), $formatb->indent(), $formatc->indent());
	var_dump($format->shrinkToFit(), $formatb->shrinkToFit(), $formatc->shrinkToFit());

	echo "OK\n";
?>
--EXPECT--
int(1)
int(0)
int(1)
bool(true)
bool(true)
bool(true)
OK
