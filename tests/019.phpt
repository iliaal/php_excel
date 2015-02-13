--TEST--
Font clone test
--INI--
date.timezone=America/Toronto
--SKIPIF--
<?php if (!extension_loaded("excel")) die("skip - Excel extension not found"); ?>
--FILE--
<?php 
	$x = new ExcelBook();

	$font = new ExcelFont($x);

	$fontc = $font;
	$font->bold(1);
	$fontb = clone $font;

	$font->italics(1);

	var_dump($font->italics(), $fontb->italics(), $fontc->italics());
	var_dump($font->bold(), $fontb->bold(), $fontc->bold());

	echo "OK\n";
?>
--EXPECT--
bool(true)
bool(false)
bool(true)
bool(true)
bool(true)
bool(true)
OK
