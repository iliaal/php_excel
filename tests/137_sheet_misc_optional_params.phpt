--TEST--
Sheet: addIgnoredError 1-arg mode, setColWidth format+hidden, setRowHeight format+hidden
--EXTENSIONS--
excel
--FILE--
<?php
$book = new ExcelBook(null, null, true);
$sheet = $book->addSheet("Sheet1");

// --- addIgnoredError with just the error type (1-arg) ---
// When called with only iError, defaults are rowFirst=0, colFirst=0, rowLast=0, colLast=0
$sheet->write(1, 0, "=1+1");
var_dump($sheet->addIgnoredError(ExcelSheet::IERR_NUMBER_STORED_AS_TEXT));

// With all explicit args for comparison
var_dump($sheet->addIgnoredError(ExcelSheet::IERR_EVAL_ERROR, 1, 0, 3, 2));

// Different error types with 1-arg
var_dump($sheet->addIgnoredError(ExcelSheet::IERR_EMPTY_CELLREF));
var_dump($sheet->addIgnoredError(ExcelSheet::IERR_INCONSIST_FMLA));

// --- setColWidth with hidden and format ---
// Signature: setColWidth(colFirst, colLast, width, hidden, format)
$fmt = $book->addFormat();
$fmt->fillPattern(ExcelFormat::FILLPATTERN_SOLID);

// Set column with hidden=true and format
var_dump($sheet->setColWidth(2, 2, 20.0, true, $fmt));
$w = $sheet->colWidth(2);
var_dump($w > 0);
var_dump($sheet->colHidden(2));

// Set column with hidden=false and format
var_dump($sheet->setColWidth(3, 3, 15.0, false, $fmt));
var_dump($sheet->colHidden(3));

// Set column with hidden only (no format)
var_dump($sheet->setColWidth(4, 4, 12.0, true));
var_dump($sheet->colHidden(4));

// Set column range with format
var_dump($sheet->setColWidth(5, 7, 18.0, false, $fmt));
var_dump($sheet->colHidden(5));
var_dump($sheet->colHidden(7));

// --- setRowHeight with format and hidden ---
// Signature: setRowHeight(row, height, format, hidden)
var_dump($sheet->setRowHeight(5, 30.0, $fmt, true));
$h = $sheet->rowHeight(5);
var_dump($h > 0);
var_dump($sheet->rowHidden(5));

// Set row with format, not hidden
var_dump($sheet->setRowHeight(6, 25.0, $fmt, false));
var_dump($sheet->rowHidden(6));

// Set row with format only (no hidden param)
var_dump($sheet->setRowHeight(7, 20.0, $fmt));
var_dump($sheet->rowHidden(7));

// Set row height without format or hidden (sanity check)
var_dump($sheet->setRowHeight(8, 18.0));
$h2 = $sheet->rowHeight(8);
var_dump($h2 > 0);

// Verify actual width/height values persisted correctly
$w2 = $sheet->colWidth(3);
var_dump($w2 > 10);
$w3 = $sheet->colWidth(5);
var_dump($w3 > 10);
$h3 = $sheet->rowHeight(6);
var_dump($h3 > 20);
$h4 = $sheet->rowHeight(7);
var_dump($h4 > 15);

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
bool(false)
bool(true)
bool(true)
bool(true)
bool(false)
bool(false)
bool(true)
bool(true)
bool(true)
bool(true)
bool(false)
bool(true)
bool(false)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
OK
