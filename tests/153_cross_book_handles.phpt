--TEST--
Child wrapper handles from one ExcelBook are rejected when used with another
--EXTENSIONS--
excel
--SKIPIF--
<?php
/* The ConditionalFormat cross-book case needs the 4-arg addConditionalFormatting
 * (libxl 5.1.0+); on 4.6.0 that method takes no arguments. */
if ((new ReflectionMethod("ExcelSheet", "addConditionalFormatting"))->getNumberOfParameters() < 4) print "skip libxl 5.1.0+ required";
?>
--DESCRIPTION--
libxl Format/Font/RichString/AutoFilter/ConditionalFormat handles index into
the workbook that created them. Applying one to a different workbook silently
corrupts output and dangles once the source book is freed. Every use-site must
reject a foreign handle and accept a same-book one. (Template-copy methods such
as Book::addFormat/addFont legitimately accept a foreign handle and are not
covered here.)
--FILE--
<?php
$b1 = new ExcelBook(null, null, true);
$b2 = new ExcelBook(null, null, true);
$s2 = $b2->addSheet("S2");

// Format from b1 against b2's sheet -> all rejected.
$fmt1 = $b1->addFormat();
echo "write:        "; var_dump(@$s2->write(2, 0, "x", $fmt1));
echo "writeRow:     "; var_dump(@$s2->writeRow(3, ["a"], 0, $fmt1));
echo "writeCol:     "; var_dump(@$s2->writeCol(0, ["a"], 4, $fmt1));
echo "setCellFormat:"; var_dump(@$s2->setCellFormat(2, 0, $fmt1));
echo "writeError:   "; var_dump(@$s2->writeError(2, 0, 0, $fmt1));
echo "setColWidth:  "; var_dump(@$s2->setColWidth(0, 1, 10, false, $fmt1));
echo "setRowHeight: "; var_dump(@$s2->setRowHeight(1, 20, $fmt1));
echo "setColPx:     "; var_dump(@$s2->setColPx(0, 1, 64, $fmt1));
echo "setRowPx:     "; var_dump(@$s2->setRowPx(1, 20, $fmt1));

// RichString from b1
$rs1 = $b1->addRichString();
echo "writeRichStr: "; var_dump(@$s2->writeRichStr(2, 0, $rs1));

// AutoFilter from b1
$af1 = $b1->addSheet("A1")->autoFilter();
echo "applyFilter2: "; var_dump(@$s2->applyFilter2($af1));

// Font from b1 against a b2 format
$fmt2 = $b2->addFormat();
$font1 = $b1->addFont();
echo "setFont:      "; var_dump(@$fmt2->setFont($font1));

// ConditionalFormat from b1 against b2's conditional formatting
$cf1 = $b1->addConditionalFormat();
$cfing = $s2->addConditionalFormatting(1, 5, 0, 0);
echo "addRule:      "; var_dump(@$cfing->addRule(0x100, $cf1, "10"));

echo "--- same-book accepted ---\n";
echo "write:        "; var_dump($s2->write(6, 0, "ok", $fmt2));
$font2 = $b2->addFont();
echo "setFont:      "; var_dump($fmt2->setFont($font2));
$cf2 = $b2->addConditionalFormat();
echo "addRule:      "; var_dump($cfing->addRule(0x100, $cf2, "10"));

echo "OK\n";
?>
--EXPECT--
write:        bool(false)
writeRow:     bool(false)
writeCol:     bool(false)
setCellFormat:bool(false)
writeError:   bool(false)
setColWidth:  bool(false)
setRowHeight: bool(false)
setColPx:     bool(false)
setRowPx:     bool(false)
writeRichStr: bool(false)
applyFilter2: bool(false)
setFont:      bool(false)
addRule:      bool(false)
--- same-book accepted ---
write:        bool(true)
setFont:      bool(true)
addRule:      bool(true)
OK
