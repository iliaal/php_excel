--TEST--
Parent object reference counting: child objects keep parents alive after unset
--EXTENSIONS--
excel
--FILE--
<?php
// ExcelSheet keeps ExcelBook alive
$sheet = (new ExcelBook(null, null, true))->addSheet("test");
$sheet->write(1, 0, "alive");
echo "Sheet: " . $sheet->read(1, 0) . "\n";

// ExcelFont keeps ExcelBook alive
$font = (new ExcelBook(null, null, true))->addFont();
echo "Font size: " . $font->size() . "\n";

// ExcelFormat keeps ExcelBook alive
$fmt = (new ExcelBook(null, null, true))->addFormat();
echo "Format: " . $fmt->numberFormat() . "\n";

// ExcelRichString keeps ExcelBook alive
$rs = (new ExcelBook(null, null, true))->addRichString();
echo "RichString textSize: " . $rs->textSize() . "\n";

// ExcelConditionalFormat keeps ExcelBook alive
$cf = (new ExcelBook(null, null, true))->addConditionalFormat();
echo "CF numFormat: " . $cf->numFormat() . "\n";

// ExcelCoreProperties keeps ExcelBook alive
$b = new ExcelBook(null, null, true);
$cp = $b->coreProperties();
unset($b);
$cp->setTitle("test");
echo "CoreProperties: " . $cp->title() . "\n";

// ExcelAutoFilter keeps ExcelSheet (and transitively ExcelBook) alive
$b = new ExcelBook(null, null, true);
$s = $b->addSheet("test");
$af = $s->autoFilter();
unset($b, $s);
echo "AutoFilter colSize: " . $af->columnSize() . "\n";

// ExcelTable keeps ExcelSheet alive
$b = new ExcelBook(null, null, true);
$s = $b->addSheet("test");
$s->write(1, 0, "H1");
$s->write(1, 1, "H2");
$s->write(2, 0, "v1");
$s->write(2, 1, "v2");
$t = $s->addTable("MyTable", 1, 2, 0, 1);
unset($b, $s);
echo "Table name: " . $t->name() . "\n";

// ExcelFilterColumn keeps ExcelAutoFilter alive
$b = new ExcelBook(null, null, true);
$s = $b->addSheet("test");
$af = $s->autoFilter();
$af->setRef(1, 3, 0, 1);
$fc = $af->column(0);
unset($b, $s, $af);
echo "FilterColumn index: " . $fc->index() . "\n";

// Font from Format::getFont keeps Format alive
$b = new ExcelBook(null, null, true);
$fmt = $b->addFormat();
$fmt->setFont($b->addFont());
$font = $fmt->getFont();
unset($b, $fmt);
echo "Font from format: " . $font->size() . "\n";

// Format clone keeps parent alive
$fmt = (new ExcelBook(null, null, true))->addFormat();
$clone = clone $fmt;
unset($fmt);
echo "Cloned format: " . $clone->numberFormat() . "\n";

echo "ALL OK\n";
?>
--EXPECT--
Sheet: alive
Font size: 11
Format: 0
RichString textSize: 0
CF numFormat: 0
CoreProperties: test
AutoFilter colSize: 0
Table name: MyTable
FilterColumn index: 0
Font from format: 11
Cloned format: 0
ALL OK
