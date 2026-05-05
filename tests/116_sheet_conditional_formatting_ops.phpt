--TEST--
Sheet: conditionalFormatting, removeConditionalFormatting
--EXTENSIONS--
excel
--SKIPIF--
<?php if (!method_exists("ExcelBook", "conditionalFormatSize")) print "skip"; ?>
--FILE--
<?php
$book = new ExcelBook(null, null, true);
$sheet = $book->addSheet("Sheet1");
$sheet->write(1, 0, 10);

// start with zero conditional formattings
var_dump($sheet->conditionalFormattingSize());

// add one
$cfing = $sheet->addConditionalFormatting(1, 1, 0, 0);
$cf = $book->addConditionalFormat();
$cf->setFillPattern(1);
$cfing->addRule(1, $cf, "5");

var_dump($sheet->conditionalFormattingSize());

// retrieve by index
$cfing2 = $sheet->conditionalFormatting(0);
var_dump($cfing2 instanceof ExcelConditionalFormatting);

// invalid index returns false
var_dump($sheet->conditionalFormatting(999));

// remove
var_dump($sheet->removeConditionalFormatting(0));
var_dump($sheet->conditionalFormattingSize());

echo "OK\n";
?>
--EXPECT--
int(0)
int(1)
bool(true)
bool(false)
bool(true)
int(0)
OK
