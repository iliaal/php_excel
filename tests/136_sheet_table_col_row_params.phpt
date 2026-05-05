--TEST--
Sheet: addTable hasHeaders/style params, setColPx/setRowPx format+hidden, writeRichStr with format, setAutoFitArea no-arg reset
--EXTENSIONS--
excel
--FILE--
<?php
$book = new ExcelBook(null, null, true);
$sheet = $book->addSheet("Sheet1");

// --- addTable with hasHeaders and style ---
$sheet->write(1, 0, "Name");
$sheet->write(1, 1, "Value");
$sheet->write(2, 0, "A");
$sheet->write(2, 1, 1);
$sheet->write(3, 0, "B");
$sheet->write(3, 1, 2);

$table = $sheet->addTable("StyledTable", 1, 3, 0, 1, true, 5);
var_dump($table instanceof ExcelTable);
var_dump($table->style());
var_dump($table->columnSize());
var_dump($table->columnName(0));

// addTable without headers
$sheet->write(5, 0, "X");
$sheet->write(5, 1, 10);
$sheet->write(6, 0, "Y");
$sheet->write(6, 1, 20);

$table2 = $sheet->addTable("NoHeaderTable", 5, 6, 0, 1, false);
var_dump($table2 instanceof ExcelTable);
var_dump($table2->columnSize());

// --- setColPx with format and hidden ---
$fmt = $book->addFormat();
$fmt->fillPattern(ExcelFormat::FILLPATTERN_SOLID);

var_dump($sheet->setColPx(3, 3, 120, $fmt, true));
var_dump($sheet->colWidthPx(3));
var_dump($sheet->colHidden(3));

// setColPx with format only (no hidden)
var_dump($sheet->setColPx(4, 4, 80, $fmt));
var_dump($sheet->colWidthPx(4));
var_dump($sheet->colHidden(4));

// setColPx with null format and hidden
var_dump($sheet->setColPx(5, 5, 100, null, true));
var_dump($sheet->colWidthPx(5));
var_dump($sheet->colHidden(5));

// --- setRowPx with format and hidden ---
var_dump($sheet->setRowPx(10, 50, $fmt, true));
var_dump($sheet->rowHeightPx(10));
var_dump($sheet->rowHidden(10));

// setRowPx with format only
var_dump($sheet->setRowPx(11, 35, $fmt));
var_dump($sheet->rowHeightPx(11));
var_dump($sheet->rowHidden(11));

// setRowPx with null format and hidden
var_dump($sheet->setRowPx(12, 45, null, true));
var_dump($sheet->rowHeightPx(12));
var_dump($sheet->rowHidden(12));

// --- writeRichStr with format ---
$rs = $book->addRichString();
$font = $rs->addFont();
$font->bold(true);
$rs->addText("Bold", $font);
$rs->addText(" normal");

$cellFmt = $book->addFormat();
$cellFmt->fillPattern(ExcelFormat::FILLPATTERN_SOLID);

var_dump($sheet->writeRichStr(15, 0, $rs, $cellFmt));
var_dump($sheet->isRichStr(15, 0));
$readBack = $sheet->readRichStr(15, 0);
var_dump($readBack instanceof ExcelRichString);
var_dump($readBack->textSize());
$t0 = $readBack->getText(0);
var_dump($t0['text']);

// writeRichStr without format (already tested in 100, but confirm it still works)
var_dump($sheet->writeRichStr(16, 0, $rs));
var_dump($sheet->isRichStr(16, 0));

// --- setAutoFitArea no-arg reset ---
// Set a specific area first
var_dump($sheet->setAutoFitArea(2, 5, 1, 3));
// Reset to defaults by calling with no args
var_dump($sheet->setAutoFitArea());

echo "OK\n";
?>
--EXPECT--
bool(true)
int(5)
int(2)
string(4) "Name"
bool(true)
int(2)
bool(true)
int(120)
bool(true)
bool(true)
int(80)
bool(false)
bool(true)
int(100)
bool(true)
bool(true)
int(50)
bool(true)
bool(true)
int(35)
bool(false)
bool(true)
int(45)
bool(true)
bool(true)
bool(true)
bool(true)
int(2)
string(4) "Bold"
bool(true)
bool(true)
bool(true)
bool(true)
OK
