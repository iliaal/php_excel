--TEST--
ExcelConditionalFormatting: addRange, addRule, conditionalFormattingSize
--EXTENSIONS--
excel
--SKIPIF--
<?php if (!method_exists("ExcelBook", "conditionalFormatSize")) print "skip"; ?>
--FILE--
<?php
$book = new ExcelBook(null, null, true);
$sheet = $book->addSheet("Sheet1");

$sheet->write(1, 0, 10);
$sheet->write(2, 0, 20);
$sheet->write(3, 0, 30);

var_dump($sheet->conditionalFormattingSize());

$cfing = $sheet->addConditionalFormatting(1, 3, 0, 0);
var_dump($cfing instanceof ExcelConditionalFormatting);

var_dump($cfing->addRange(4, 6, 0, 0));

$cf = $book->addConditionalFormat();
$cf->setPatternForegroundColor(0x00FF00);
$cf->setFillPattern(1);

var_dump($cfing->addRule(1, $cf, "10"));

var_dump($sheet->conditionalFormattingSize());

echo "OK\n";
?>
--EXPECT--
int(0)
bool(true)
bool(true)
bool(true)
int(1)
OK
