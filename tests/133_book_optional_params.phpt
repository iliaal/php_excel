--TEST--
ExcelBook optional parameters: addFont copy, addFormat copy, save buffer, getSheet default, activeSheet setter
--EXTENSIONS--
excel
--FILE--
<?php

// --- ExcelBook::addFont($existingFont) - copy mode ---

$book = new ExcelBook(null, null, true);

$font1 = $book->addFont();
$font1->bold(true);
$font1->size(16);
$font1->color(8); // COLOR_RED
$font1->name("Arial");

$font2 = $book->addFont($font1);
echo "font copy bold: " . var_export($font2->bold(), true) . "\n";
echo "font copy size: " . $font2->size() . "\n";
echo "font copy color: " . $font2->color() . "\n";
echo "font copy name: " . $font2->name() . "\n";

// Verify it's an independent copy: modifying copy doesn't affect original
$font2->size(20);
echo "original size after copy change: " . $font1->size() . "\n";
echo "copy size after change: " . $font2->size() . "\n";

// --- ExcelBook::addFormat($existingFormat) - copy mode ---

$book2 = new ExcelBook(null, null, true);

$fmt1 = $book2->addFormat();
$fmt1->numberFormat(1); // 1 = "0" format
$fmt1->horizontalAlign(2); // ALIGNH_CENTER
$fmt1->wrap(true);

$fmt2 = $book2->addFormat($fmt1);
echo "format copy numberFormat: " . $fmt2->numberFormat() . "\n";
echo "format copy horizontalAlign: " . $fmt2->horizontalAlign() . "\n";
echo "format copy wrap: " . var_export($fmt2->wrap(), true) . "\n";

// Verify independence
$fmt2->numberFormat(2);
echo "original numberFormat after copy change: " . $fmt1->numberFormat() . "\n";

// --- ExcelBook::save() - no-arg buffer mode ---

$book3 = new ExcelBook(null, null, true);
$sheet3 = $book3->addSheet("BufferTest");
$sheet3->write(1, 0, "buffer_data");

$buffer = $book3->save();
var_dump(is_string($buffer));
var_dump(strlen($buffer) > 0);

// Verify buffer is valid by loading it into a new book
$book3b = new ExcelBook(null, null, true);
$book3b->load($buffer);
$sheet3b = $book3b->getSheet(0);
echo "reloaded: " . $sheet3b->read(1, 0) . "\n";

// --- ExcelBook::getSheet() - no-arg default (sheet 0) ---

$book4 = new ExcelBook(null, null, true);
$s0 = $book4->addSheet("SheetZero");
$s0->write(1, 0, "first_sheet");
$book4->addSheet("SheetOne");

$defaultSheet = $book4->getSheet();
echo "getSheet() name: " . $defaultSheet->name() . "\n";
echo "getSheet() data: " . $defaultSheet->read(1, 0) . "\n";

// --- ExcelBook::activeSheet($index) - setter mode ---
// activeSheet() with no args returns the active sheet index (getter).
// activeSheet($index) sets the active sheet and returns the new index (setter).

$book5 = new ExcelBook(null, null, true);
$book5->addSheet("A");
$book5->addSheet("B");
$book5->addSheet("C");

echo "initial active: " . $book5->activeSheet() . "\n";

$result = $book5->activeSheet(2);
echo "after set to 2: " . $result . "\n";
echo "verify via getActiveSheet: " . $book5->getActiveSheet() . "\n";

$result = $book5->activeSheet(0);
echo "after set to 0: " . $result . "\n";
echo "verify via getActiveSheet: " . $book5->getActiveSheet() . "\n";

echo "OK\n";
?>
--EXPECT--
font copy bold: true
font copy size: 16
font copy color: 8
font copy name: Arial
original size after copy change: 16
copy size after change: 20
format copy numberFormat: 1
format copy horizontalAlign: 2
format copy wrap: true
original numberFormat after copy change: 1
bool(true)
bool(true)
reloaded: buffer_data
getSheet() name: SheetZero
getSheet() data: first_sheet
initial active: 0
after set to 2: 2
verify via getActiveSheet: 2
after set to 0: 0
verify via getActiveSheet: 0
OK
