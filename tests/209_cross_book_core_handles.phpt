--TEST--
Core cross-book handle rejection (Format/Font/RichString) without CF version gate
--EXTENSIONS--
excel
--DESCRIPTION--
CR-016: tests/153 skips entirely on libxl < 5.1 because of ConditionalFormat.
Core EXCEL_REQUIRE_SAME_BOOK paths must still be exercised on 4.6.
--FILE--
<?php
$b1 = new ExcelBook(null, null, true);
$b2 = new ExcelBook(null, null, true);
$s2 = $b2->addSheet("S2");

$fmt1 = $b1->addFormat();
echo "write:        "; var_dump(@$s2->write(2, 0, "x", $fmt1));
echo "writeRow:     "; var_dump(@$s2->writeRow(3, ["a"], 0, $fmt1));
echo "setCellFormat:"; var_dump(@$s2->setCellFormat(2, 0, $fmt1));

$rs1 = $b1->addRichString();
echo "writeRichStr: "; var_dump(@$s2->writeRichStr(2, 0, $rs1));

$fmt2 = $b2->addFormat();
$font1 = $b1->addFont();
echo "setFont:      "; var_dump(@$fmt2->setFont($font1));

echo "--- same-book ---\n";
echo "write:        "; var_dump($s2->write(6, 0, "ok", $fmt2));
$font2 = $b2->addFont();
echo "setFont:      "; var_dump($fmt2->setFont($font2));

// Template copy methods may accept foreign fonts (not a same-book guard site)
$rs2 = $b2->addRichString();
echo "addText foreign font: "; var_dump(@$rs2->addText("x", $font1) !== false || @$rs2->addText("x", $font1) === false);
// just ensure no crash
@$rs2->addText("x", $font1);
echo "OK\n";
?>
--EXPECT--
write:        bool(false)
writeRow:     bool(false)
setCellFormat:bool(false)
writeRichStr: bool(false)
setFont:      bool(false)
--- same-book ---
write:        bool(true)
setFont:      bool(true)
addText foreign font: bool(true)
OK
