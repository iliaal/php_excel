--TEST--
NUL guards on color-scale formula rules and addDataValidationDouble; coordinate validation on rowColToAddr/setActiveCell/setPrintArea/setMerge/clear/copy/etc.
--SKIPIF--
<?php if (!extension_loaded("excel")) print "skip"; ?>
--FILE--
<?php
$b = new ExcelBook(null, null, true);
$s = $b->addSheet("S");

// rowColToAddr with PHP_INT_MAX must reject (was returning "A")
var_dump(@$s->rowColToAddr(PHP_INT_MAX, 0));

// setActiveCell with PHP_INT_MAX must reject (was returning true)
var_dump(@$s->setActiveCell(PHP_INT_MAX, 0));
var_dump(@$s->setActiveCell(0, PHP_INT_MAX));

// setPrintArea with out-of-range coordinates
var_dump(@$s->setPrintArea(PHP_INT_MAX, PHP_INT_MAX, 0, 0));
var_dump(@$s->setPrintArea(0, 0, PHP_INT_MAX, PHP_INT_MAX));

// setMerge / clear / copy with out-of-range coords
var_dump(@$s->setMerge(PHP_INT_MAX, PHP_INT_MAX, 0, 0));
var_dump(@$s->clear(PHP_INT_MAX, PHP_INT_MAX, 0, 0));
var_dump(@$s->copy(PHP_INT_MAX, 0, 1, 0));
var_dump(@$s->copy(0, 0, PHP_INT_MAX, 0));

// readComment / writeComment / removeComment / removePicture / hyperlinkIndex
var_dump(@$s->readComment(PHP_INT_MAX, 0));
var_dump(@$s->removeComment(PHP_INT_MAX, 0));
var_dump(@$s->removePicture(PHP_INT_MAX, 0));
var_dump(@$s->hyperlinkIndex(PHP_INT_MAX, 0));

// setColPx / setRowPx
var_dump(@$s->setColPx(PHP_INT_MAX, 0, 50));
var_dump(@$s->setRowPx(PHP_INT_MAX, 50));

// setBorder
var_dump(@$s->setBorder(PHP_INT_MAX, 0, 0, 0, 0, 0));
var_dump(@$s->setBorder(0, 0, PHP_INT_MAX, 0, 0, 0));

// addDataValidationDouble: NUL in prompt; out-of-range coords
if (method_exists("ExcelSheet", "addDataValidationDouble")) {
    var_dump(@$s->addDataValidationDouble(1, 1, 1, 1, 0, 0, 1.0, 2.0, true, false, true, true, "title\0BAD", "p", "et", "e", 1));
    var_dump(@$s->addDataValidationDouble(1, 1, PHP_INT_MAX, PHP_INT_MAX, 0, 0, 1.0, 2.0));
} else {
    echo "bool(false)\nbool(false)\n";
}

// add2/3ColorScaleFormulaRule with NUL value (libxl 5.1.0+: 4-arg
// addConditionalFormatting). On older libxl the test still has to print
// the same number of "bool(false)" lines so EXPECT matches both builds.
if (method_exists("ExcelBook", "conditionalFormatSize")) {
    $cf = $b->addConditionalFormat();
    $cfing = $s->addConditionalFormatting(1, 3, 0, 0);
    var_dump(@$cfing->add2ColorScaleFormulaRule(0xFFFFFF, 0xFF0000, 1, "A1\0BAD", 2, "A2"));
    var_dump(@$cfing->add3ColorScaleFormulaRule(0xFFFFFF, 0xCCCCCC, 0xFF0000, 1, "A1", 2, "A2\0BAD", 3, "A3"));
    var_dump(@$s->addConditionalFormatting(PHP_INT_MAX, PHP_INT_MAX, 0, 0));
} else {
    echo "bool(false)\nbool(false)\nbool(false)\n";
}

// Single-axis validators
var_dump(@$s->setRowHidden(PHP_INT_MAX, true));
var_dump(@$s->setColHidden(PHP_INT_MAX, true));
var_dump(@$s->rowHidden(PHP_INT_MAX));
var_dump(@$s->colHidden(PHP_INT_MAX));
var_dump(@$s->colWidth(PHP_INT_MAX));
var_dump(@$s->rowHeight(PHP_INT_MAX));
var_dump(@$s->colWidthPx(PHP_INT_MAX));
var_dump(@$s->rowHeightPx(PHP_INT_MAX));
var_dump(@$s->colFormat(PHP_INT_MAX));
var_dump(@$s->rowFormat(PHP_INT_MAX));
var_dump(@$s->groupRows(0, PHP_INT_MAX));
var_dump(@$s->groupCols(0, PHP_INT_MAX));

// Setters
var_dump(@$s->setAutoFitArea(PHP_INT_MAX, 0, 0, 0));
var_dump(@$s->addIgnoredError(0, 0, 0, PHP_INT_MAX, 0));

// XLS book — 70000 row out of XLS range
$xls = new ExcelBook();
$xs = $xls->addSheet("X");
var_dump(@$xs->colWidth(300));   // 300 > 255 col limit on XLS
var_dump(@$xs->rowHeight(70000));// 70000 > 65535 row limit on XLS
var_dump(@$xs->setRowHidden(70000, true));
var_dump(@$xs->setColHidden(300, true));

// Page-break coordinate validation (CR-002)
var_dump(@$s->horPageBreak(1048576, true));   // XLSX: row > max
var_dump(@$s->verPageBreak(16384, true));     // XLSX: col > max
var_dump(@$xs->horPageBreak(70000, true));    // XLS: row > max
var_dump(@$xs->verPageBreak(300, true));      // XLS: col > max

// addHyperlink workbook-aware (CR-003)
var_dump(@$s->addHyperlink("http://x", 1048576, 1048576, 0, 0));  // XLSX: row > max
var_dump(@$xs->addHyperlink("http://x", 70000, 70000, 0, 0));     // XLS: row > max
var_dump(@$xs->addHyperlink("http://x", 0, 0, 300, 300));         // XLS: col > max

// ExcelConditionalFormatting::__construct on 5.1.0+ now throws on bad coords (CR-001)
if (method_exists("ExcelBook", "conditionalFormatSize")) {
    $sheet2 = $b->addSheet("CF");
    try {
        $cf = new ExcelConditionalFormatting($sheet2, 1, 1, 0, 0);
        echo "5-arg valid: ok\n";
    } catch (Throwable $e) {
        echo "5-arg valid: failed\n";
    }
    try {
        new ExcelConditionalFormatting($sheet2, PHP_INT_MAX, PHP_INT_MAX, 0, 0);
        echo "5-arg bad: no throw (BUG)\n";
    } catch (Exception $e) {
        echo "5-arg bad: caught\n";
    }
} else {
    echo "5-arg valid: ok\n";
    echo "5-arg bad: caught\n";
}

echo "OK\n";
?>
--EXPECT--
bool(false)
bool(false)
bool(false)
bool(false)
bool(false)
bool(false)
bool(false)
bool(false)
bool(false)
bool(false)
bool(false)
bool(false)
bool(false)
bool(false)
bool(false)
bool(false)
bool(false)
bool(false)
bool(false)
bool(false)
bool(false)
bool(false)
bool(false)
bool(false)
bool(false)
bool(false)
bool(false)
bool(false)
bool(false)
bool(false)
bool(false)
bool(false)
bool(false)
bool(false)
bool(false)
bool(false)
bool(false)
bool(false)
bool(false)
bool(false)
bool(false)
bool(false)
bool(false)
bool(false)
bool(false)
bool(false)
bool(false)
5-arg valid: ok
5-arg bad: caught
OK
