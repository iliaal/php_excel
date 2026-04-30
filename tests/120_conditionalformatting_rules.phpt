--TEST--
ConditionalFormatting: addTopRule, addOpNumRule, addOpStrRule, addAboveAverageRule, addTimePeriodRule
--SKIPIF--
<?php if (!extension_loaded("excel") || !method_exists("ExcelBook", "conditionalFormatSize")) print "skip"; ?>
--FILE--
<?php
$book = new ExcelBook(null, null, true);
$sheet = $book->addSheet("Sheet1");

$sheet->write(1, 0, 10);
$sheet->write(2, 0, 20);
$sheet->write(3, 0, 30);

$cfing = $sheet->addConditionalFormatting(1, 3, 0, 0);

// addTopRule(cf, value, bottom, percent, [stopIfTrue])
$cf1 = $book->addConditionalFormat();
$cf1->setFillPattern(1);
$cf1->setPatternForegroundColor(0xFF0000);
var_dump($cfing->addTopRule($cf1, 10, false, false));

// addOpNumRule(op, cf, v1, v2, [stopIfTrue])
$cf2 = $book->addConditionalFormat();
$cf2->setFillPattern(1);
var_dump($cfing->addOpNumRule(1, $cf2, 10.0, 30.0));

// addOpStrRule(op, cf, v1, v2, [stopIfTrue])
$cf3 = $book->addConditionalFormat();
$cf3->setFillPattern(1);
var_dump($cfing->addOpStrRule(1, $cf3, "10", "30"));

// addAboveAverageRule(cf, above, equal, stdDev, [stopIfTrue])
$cf4 = $book->addConditionalFormat();
$cf4->setFillPattern(1);
var_dump($cfing->addAboveAverageRule($cf4, true, false, 0));

// addTimePeriodRule(cf, timePeriod, [stopIfTrue])
$cf5 = $book->addConditionalFormat();
$cf5->setFillPattern(1);
var_dump($cfing->addTimePeriodRule($cf5, 0));

// with stopIfTrue = true
$cf6 = $book->addConditionalFormat();
$cf6->setFillPattern(1);
var_dump($cfing->addTopRule($cf6, 5, true, true, true));

echo "OK\n";
?>
--EXPECT--
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
OK
