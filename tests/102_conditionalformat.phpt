--TEST--
ExcelConditionalFormat: create via Book::addConditionalFormat, font, numFormat, border, fill
--EXTENSIONS--
excel
--FILE--
<?php
$book = new ExcelBook(null, null, true);

$cf = $book->addConditionalFormat();
var_dump($cf instanceof ExcelConditionalFormat);

$font = $cf->font();
var_dump($font instanceof ExcelFont);

var_dump($cf->numFormat());
var_dump($cf->setNumFormat(1));
var_dump($cf->numFormat());

var_dump($cf->setBorder(1));
var_dump($cf->setBorderColor(0xFF0000));

var_dump($cf->setBorderLeft(2));
var_dump($cf->setBorderRight(2));
var_dump($cf->setBorderTop(2));
var_dump($cf->setBorderBottom(2));

var_dump($cf->setBorderLeftColor(10));
var_dump($cf->setBorderRightColor(11));
var_dump($cf->setBorderTopColor(12));
var_dump($cf->setBorderBottomColor(13));

var_dump($cf->fillPattern());
var_dump($cf->setFillPattern(1));
var_dump($cf->fillPattern());

var_dump($cf->setPatternForegroundColor(0x00FF00));
var_dump($cf->setPatternBackgroundColor(0x0000FF));

echo "OK\n";
?>
--EXPECT--
bool(true)
bool(true)
int(0)
bool(true)
int(1)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
int(0)
bool(true)
int(1)
bool(true)
bool(true)
OK
