--TEST--
ConditionalFormatting: stopIfTrue parameter on addRule, addOpNumRule, addOpStrRule, addAboveAverageRule, addTimePeriodRule, add2ColorScaleFormulaRule, add3ColorScaleRule, add3ColorScaleFormulaRule
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

$cfing = $sheet->addConditionalFormatting(1, 3, 0, 0);

// addRule(type, cf, value, stopIfTrue)
$cf1 = $book->addConditionalFormat();
$cf1->setFillPattern(1);
var_dump($cfing->addRule(0, $cf1, "A1", true));

// addOpNumRule(op, cf, v1, v2, stopIfTrue)
$cf2 = $book->addConditionalFormat();
$cf2->setFillPattern(1);
var_dump($cfing->addOpNumRule(1, $cf2, 5.0, 25.0, true));

// addOpStrRule(op, cf, v1, v2, stopIfTrue)
$cf3 = $book->addConditionalFormat();
$cf3->setFillPattern(1);
var_dump($cfing->addOpStrRule(1, $cf3, "5", "25", true));

// addAboveAverageRule(cf, above, equal, stdDev, stopIfTrue)
$cf4 = $book->addConditionalFormat();
$cf4->setFillPattern(1);
var_dump($cfing->addAboveAverageRule($cf4, true, true, 0, true));

// addTimePeriodRule(cf, timePeriod, stopIfTrue)
$cf5 = $book->addConditionalFormat();
$cf5->setFillPattern(1);
var_dump($cfing->addTimePeriodRule($cf5, 0, true));

// add2ColorScaleFormulaRule(minColor, maxColor, minType, minVal, maxType, maxVal, stopIfTrue)
var_dump($cfing->add2ColorScaleFormulaRule(0xFF0000, 0x00FF00, 0, "0", 0, "100", true));

// add3ColorScaleRule(minColor, midColor, maxColor, minType, minVal, midType, midVal, maxType, maxVal, stopIfTrue)
var_dump($cfing->add3ColorScaleRule(0xFF0000, 0xFFFF00, 0x00FF00, 0, 0.0, 0, 50.0, 0, 100.0, true));

// add3ColorScaleFormulaRule(minColor, midColor, maxColor, minType, minVal, midType, midVal, maxType, maxVal, stopIfTrue)
var_dump($cfing->add3ColorScaleFormulaRule(0xFF0000, 0xFFFF00, 0x00FF00, 0, "0", 0, "50", 0, "100", true));

echo "OK\n";
?>
--EXPECT--
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
OK
