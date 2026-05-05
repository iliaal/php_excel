--TEST--
Misc optional params: Format::borderColor setter, FilterColumn::setCustomFilter 2-arg, RichString::addFont copy, Sheet::addTable with TABLESTYLE constant
--EXTENSIONS--
excel
--FILE--
<?php
// --- Format::borderColor($color) setter mode ---
$book = new ExcelBook(null, null, true);
$fmt = $book->addFormat();

var_dump($fmt->borderColor(10));

var_dump($fmt->borderLeftColor());
var_dump($fmt->borderRightColor());
var_dump($fmt->borderTopColor());
var_dump($fmt->borderBottomColor());

// --- FilterColumn::setCustomFilter 2-arg mode (single condition) ---
$sheet = $book->addSheet("Filter");

$sheet->write(1, 0, "Name");
$sheet->write(2, 0, "Alice");
$sheet->write(3, 0, "Bob");
$sheet->write(4, 0, "Charlie");

$af = $sheet->autoFilter();
$af->setRef(1, 4, 0, 0);

$fc = $af->column(0);
$fc->setCustomFilter(ExcelFilterColumn::OPERATOR_EQUAL, "Alice");
var_dump($fc->filterType() === ExcelFilterColumn::FILTER_CUSTOM);

$cf = $fc->getCustomFilter();
var_dump($cf['operator_1'] === ExcelFilterColumn::OPERATOR_EQUAL);
var_dump($cf['value_1']);

// --- RichString::addFont($existingFont) copy mode ---
$rs = $book->addRichString();

$srcFont = $rs->addFont();
$srcFont->bold(true);
$srcFont->size(18);

$bookFont = $book->addFont();
$bookFont->bold(true);
$bookFont->size(24);

$copiedFont = $rs->addFont($bookFont);
var_dump($copiedFont instanceof ExcelFont);
var_dump($copiedFont->bold());
var_dump($copiedFont->size());

$rs->addText("Source ", $srcFont);
$rs->addText("Copied", $copiedFont);
var_dump($rs->textSize());

// --- Sheet::addTable with TABLESTYLE constant ---
$sheet2 = $book->addSheet("Tables");

$sheet2->write(1, 0, "Col1");
$sheet2->write(1, 1, "Col2");
$sheet2->write(2, 0, "X");
$sheet2->write(2, 1, 1);
$sheet2->write(3, 0, "Y");
$sheet2->write(3, 1, 2);

$table = $sheet2->addTable("DarkTable", 1, 3, 0, 1, true, ExcelTable::TABLESTYLE_DARK1);
var_dump($table instanceof ExcelTable);
var_dump($table->style());

echo "OK\n";
?>
--EXPECT--
bool(true)
int(10)
int(10)
int(10)
int(10)
bool(true)
bool(true)
string(5) "Alice"
bool(true)
bool(true)
int(24)
int(2)
bool(true)
int(50)
OK
