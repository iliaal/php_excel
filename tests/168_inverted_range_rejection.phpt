--TEST--
Inverted (first > last) ranges are rejected on hyperlink / setRef / conditional formatting / data validation / clear
--EXTENSIONS--
excel
--DESCRIPTION--
CR-017: EXCEL_VALIDATE_ROW_RANGE / COL_RANGE only checked sheet bounds, not
order. Methods funneling through them (addHyperlink, AutoFilter::setRef,
addConditionalFormatting, addDataValidation, clear) accepted inverted ranges
and stored garbage coordinates. setMerge/insertRow were already rejected by
libxl; the macros now reject the whole family up front. Equal endpoints
(single row/column/cell) stay valid.

Conditional-formatting branch needs the 4-arg addConditionalFormatting
signature (libxl 5.1.0+); on older builds that line is soft-skipped so the
rest of the suite still runs on the minimum supported libxl.
--FILE--
<?php
$b = new ExcelBook(null, null, true);
$s = $b->addSheet("S");
$s->write(1, 0, "a");

echo "hyperlink inv rows:   "; var_dump(@$s->addHyperlink("http://x", 10, 5, 0, 1));
echo "hyperlink stored:     "; var_dump(@$s->hyperlink(0));
echo "hyperlink inv cols:   "; var_dump(@$s->addHyperlink("http://x", 1, 3, 10, 2));

$af = $s->autoFilter();
echo "setRef inv rows:      "; var_dump(@$af->setRef(10, 5, 0, 2));
echo "setRef getRef:        "; var_dump(@$af->getRef());

if ((new ReflectionMethod("ExcelSheet", "addConditionalFormatting"))->getNumberOfParameters() >= 4) {
	echo "condFormatting inv:   "; var_dump(@$s->addConditionalFormatting(20, 10, 5, 1));
} else {
	echo "condFormatting inv:   bool(false)\n";
}
echo "dataValidation inv:   "; var_dump(@$s->addDataValidation(0, ExcelSheet::VALIDATION_OP_EQUAL, 10, 5, 0, 1, "5"));
echo "clear inv rows:       "; var_dump(@$s->clear(10, 5, 0, 1));
echo "insertRow inv:        "; var_dump(@$s->insertRow(20, 15));

echo "--- ordered / single-cell still valid ---\n";
echo "hyperlink ordered:    "; var_dump(@$s->addHyperlink("http://ok", 1, 3, 0, 1));
echo "hyperlink single:     "; var_dump(@$s->addHyperlink("http://ok", 2, 2, 2, 2));
echo "setRef ordered:       "; var_dump(@$af->setRef(1, 3, 0, 2));
echo "OK\n";
?>
--EXPECT--
hyperlink inv rows:   bool(false)
hyperlink stored:     bool(false)
hyperlink inv cols:   bool(false)
setRef inv rows:      bool(false)
setRef getRef:        bool(false)
condFormatting inv:   bool(false)
dataValidation inv:   bool(false)
clear inv rows:       bool(false)
insertRow inv:        bool(false)
--- ordered / single-cell still valid ---
hyperlink ordered:    bool(true)
hyperlink single:     bool(true)
setRef ordered:       NULL
OK
