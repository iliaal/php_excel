--TEST--
ConditionalFormat: border getters (borderLeft/Right/Top/Bottom, borderLeftColor/RightColor/TopColor/BottomColor), customNumFormat
--EXTENSIONS--
excel
--FILE--
<?php
$book = new ExcelBook(null, null, true);

$cf = $book->addConditionalFormat();

// border style setters/getters
$cf->setBorderLeft(2);
$cf->setBorderRight(3);
$cf->setBorderTop(4);
$cf->setBorderBottom(5);
var_dump($cf->borderLeft());
var_dump($cf->borderRight());
var_dump($cf->borderTop());
var_dump($cf->borderBottom());

// border color setters/getters
$cf->setBorderLeftColor(10);
$cf->setBorderRightColor(11);
$cf->setBorderTopColor(12);
$cf->setBorderBottomColor(13);
var_dump($cf->borderLeftColor());
var_dump($cf->borderRightColor());
var_dump($cf->borderTopColor());
var_dump($cf->borderBottomColor());

// customNumFormat (starts null)
var_dump($cf->customNumFormat());
$cf->setCustomNumFormat("#,##0.00");
var_dump($cf->customNumFormat());

echo "OK\n";
?>
--EXPECT--
int(2)
int(3)
int(4)
int(5)
int(10)
int(11)
int(12)
int(13)
NULL
string(8) "#,##0.00"
OK
