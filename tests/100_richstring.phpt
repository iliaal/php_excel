--TEST--
ExcelRichString: create, addFont, addText, textSize, getText, write/read
--EXTENSIONS--
excel
--FILE--
<?php
$book = new ExcelBook(null, null, true);
$sheet = $book->addSheet("Sheet1");

$rs = $book->addRichString();
var_dump($rs instanceof ExcelRichString);

$font = $rs->addFont();
var_dump($font instanceof ExcelFont);
$font->bold(true);
$font->size(14);

var_dump($rs->addText("Hello ", $font));
var_dump($rs->addText("World"));

var_dump($rs->textSize());

$t0 = $rs->getText(0);
var_dump($t0['text']);
var_dump($t0['font'] instanceof ExcelFont);

$t1 = $rs->getText(1);
var_dump($t1['text']);

var_dump($sheet->writeRichStr(1, 0, $rs));
$rs2 = $sheet->readRichStr(1, 0);
var_dump($rs2 instanceof ExcelRichString);
var_dump($rs2->textSize());
$rt0 = $rs2->getText(0);
var_dump($rt0['text']);

echo "OK\n";
?>
--EXPECT--
bool(true)
bool(true)
bool(true)
bool(true)
int(2)
string(6) "Hello "
bool(true)
string(5) "World"
bool(true)
bool(true)
int(2)
string(6) "Hello "
OK
