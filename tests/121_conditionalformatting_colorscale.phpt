--TEST--
ConditionalFormatting: add2ColorScaleRule, add2ColorScaleFormulaRule, add3ColorScaleRule, add3ColorScaleFormulaRule
--EXTENSIONS--
excel
--SKIPIF--
<?php if (!method_exists("ExcelBook", "conditionalFormatSize")) print "skip"; ?>
--FILE--
<?php
$book = new ExcelBook(null, null, true);
$sheet = $book->addSheet("Sheet1");

$sheet->write(1, 0, 10);
$sheet->write(2, 0, 50);
$sheet->write(3, 0, 90);

$cfing = $sheet->addConditionalFormatting(1, 3, 0, 0);

// add2ColorScaleRule(minColor, maxColor, minType, minVal, maxType, maxVal, [stopIfTrue])
var_dump($cfing->add2ColorScaleRule(0xFF0000, 0x00FF00, 0, 0.0, 0, 100.0));

// add2ColorScaleFormulaRule(minColor, maxColor, minType, minVal, maxType, maxVal, [stopIfTrue])
var_dump($cfing->add2ColorScaleFormulaRule(0xFF0000, 0x00FF00, 0, "0", 0, "100"));

// add3ColorScaleRule(minColor, midColor, maxColor, minType, minVal, midType, midVal, maxType, maxVal, [stopIfTrue])
var_dump($cfing->add3ColorScaleRule(0xFF0000, 0xFFFF00, 0x00FF00, 0, 0.0, 0, 50.0, 0, 100.0));

// add3ColorScaleFormulaRule
var_dump($cfing->add3ColorScaleFormulaRule(0xFF0000, 0xFFFF00, 0x00FF00, 0, "0", 0, "50", 0, "100"));

// with stopIfTrue = true
var_dump($cfing->add2ColorScaleRule(0x0000FF, 0x00FFFF, 0, 0.0, 0, 100.0, true));

echo "OK\n";
?>
--EXPECT--
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
OK
