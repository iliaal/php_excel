--TEST--
Excel font tests
--SKIPIF--
<?php if (!extension_loaded("excel")) print "skip"; ?>
--FILE--
<?php 
	$x = new ExcelBook();
	$font = $x->addFont();
	
	var_dump(
		$font->name(),
		$font->size(),
		$font->underline(),
		$font->mode(),
		$font->color(),
		$font->bold(),
		$font->strike(),
		$font->italics()		
	);
	
	var_dump($font->name("Courier"));
	var_dump($font->size(10));
	var_dump($font->bold(true));
	var_dump($font->strike(true));
	var_dump($font->italics(true));
	var_dump($font->mode(ExcelFont::SUBSCRIPT));
	var_dump($font->mode(ExcelFont::SUPERSCRIPT) == ExcelFont::SUPERSCRIPT);
	
	var_dump($font->underline(ExcelFont::UNDERLINE_NONE));
	var_dump($font->underline(ExcelFont::UNDERLINE_SINGLE) == ExcelFont::UNDERLINE_SINGLE);
	var_dump($font->underline(ExcelFont::UNDERLINE_DOUBLE));
	var_dump($font->underline(ExcelFont::UNDERLINE_SINGLEACC) == ExcelFont::UNDERLINE_SINGLEACC);
	var_dump($font->underline(ExcelFont::UNDERLINE_DOUBLEACC) == ExcelFont::UNDERLINE_DOUBLEACC);
	
	var_dump($font->color(ExcelFormat::COLOR_PALEBLUE) == ExcelFormat::COLOR_PALEBLUE);
	var_dump($font->color(ExcelFormat::COLOR_DARKRED) == ExcelFormat::COLOR_DARKRED);
?>
--EXPECT--
string(5) "Arial"
int(10)
int(0)
int(0)
int(32767)
bool(false)
bool(false)
bool(false)
string(7) "Courier"
int(10)
bool(true)
bool(true)
bool(true)
int(2)
bool(true)
int(0)
bool(true)
int(2)
bool(true)
bool(true)
bool(true)
bool(true)
