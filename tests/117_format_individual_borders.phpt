--TEST--
Format: borderLeftStyle/Color, borderRightStyle/Color, borderTopStyle/Color, borderBottomStyle/Color
--EXTENSIONS--
excel
--FILE--
<?php
$book = new ExcelBook(null, null, true);

$fmt = $book->addFormat();

// borderLeft
$fmt->borderLeftStyle(1);
var_dump($fmt->borderLeftStyle());
$fmt->borderLeftColor(10);
var_dump($fmt->borderLeftColor());

// borderRight
$fmt->borderRightStyle(2);
var_dump($fmt->borderRightStyle());
$fmt->borderRightColor(11);
var_dump($fmt->borderRightColor());

// borderTop
$fmt->borderTopStyle(3);
var_dump($fmt->borderTopStyle());
$fmt->borderTopColor(12);
var_dump($fmt->borderTopColor());

// borderBottom
$fmt->borderBottomStyle(4);
var_dump($fmt->borderBottomStyle());
$fmt->borderBottomColor(13);
var_dump($fmt->borderBottomColor());

echo "OK\n";
?>
--EXPECT--
int(1)
int(10)
int(2)
int(11)
int(3)
int(12)
int(4)
int(13)
OK
