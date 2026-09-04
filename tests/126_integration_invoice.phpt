--TEST--
Integration: create invoice workbook with fonts, formats, formulas, dates, save/reload/verify
--EXTENSIONS--
excel
--FILE--
<?php
$book = new ExcelBook(null, null, true);
$book->setLocale('UTF-8');

// Fonts
$boldFont = $book->addFont();
$boldFont->bold(true);

$titleFont = $book->addFont();
$titleFont->size(16);
$titleFont->bold(true);

// Formats
$titleFmt = $book->addFormat();
$titleFmt->setFont($titleFont);

$headerFmt = $book->addFormat();
$headerFmt->horizontalAlign(ExcelFormat::ALIGNH_CENTER);
$headerFmt->borderStyle(ExcelFormat::BORDERSTYLE_THIN);
$headerFmt->setFont($boldFont);
$headerFmt->fillPattern(ExcelFormat::FILLPATTERN_SOLID);
$headerFmt->patternForegroundColor(ExcelFormat::COLOR_TAN);

$currencyFmt = $book->addFormat();
$currencyFmt->numberFormat(ExcelFormat::NUMFORMAT_CURRENCY_NEGBRA);
$currencyFmt->borderLeftStyle(ExcelFormat::BORDERSTYLE_THIN);
$currencyFmt->borderRightStyle(ExcelFormat::BORDERSTYLE_THIN);

$dateFmt = $book->addFormat();
$dateFmt->numberFormat(ExcelFormat::NUMFORMAT_DATE);

// Sheet 1: Invoice
$sheet = $book->addSheet("Invoice");

$sheet->write(1, 1, "Invoice #3568", $titleFmt);
$sheet->write(3, 1, "Name: John Smith");
$sheet->write(4, 1, "Date:");
$sheet->write(4, 2, $book->packDateValues(2024, 8, 15, 0, 0, 0), $dateFmt);

$sheet->write(6, 1, "Description", $headerFmt);
$sheet->write(6, 2, "Amount", $headerFmt);

$sheet->write(7, 1, "Ball-Point Pens", $currencyFmt);
$sheet->write(7, 2, 85.0, $currencyFmt);
$sheet->write(8, 1, "T-Shirts", $currencyFmt);
$sheet->write(8, 2, 150.0, $currencyFmt);
$sheet->write(9, 1, "Tea Cups", $currencyFmt);
$sheet->write(9, 2, 45.0, $currencyFmt);

$sheet->write(10, 2, "=SUM(C8:C10)");

$sheet->setColWidth(1, 1, 30);
$sheet->setColWidth(2, 2, 15);

// Sheet 2: Custom formats
$sheet2 = $book->addSheet("Formats");
$customFmt = $book->addCustomFormat("0.000");
$fmt = $book->addFormat();
$fmt->numberFormat($customFmt);
$sheet2->write(1, 0, 25.718, $fmt);

// Save
$tmp = tempnam(sys_get_temp_dir(), "xl") . ".xlsx";
var_dump($book->save($tmp));
echo "sheetCount: " . $book->sheetCount() . "\n";

// Reload and verify
$book2 = new ExcelBook(null, null, true);
var_dump($book2->loadFile($tmp));
echo "loaded sheetCount: " . $book2->sheetCount() . "\n";

$s1 = $book2->getSheet(0);
echo "sheet name: " . $s1->name() . "\n";

// Read back values
echo "title: " . $s1->read(1, 1) . "\n";
echo "name: " . $s1->read(3, 1) . "\n";

// Read formatted cells with format ref
$fmt = null;
$val = $s1->read(7, 2, $fmt);
echo "amount: " . $val . "\n";
echo "amount format: " . $fmt->numberFormat() . "\n";

// Read formula
echo "formula cell: " . $s1->read(10, 2) . "\n";
echo "isFormula: " . var_export($s1->isFormula(10, 2), true) . "\n";

// Verify col width
echo "col1 width > 20: " . var_export($s1->colWidth(1) > 20, true) . "\n";

// Sheet 2
$s2 = $book2->getSheet(1);
echo "sheet2 name: " . $s2->name() . "\n";
echo "custom format value: " . $s2->read(1, 0) . "\n";

unlink($tmp);
echo "OK\n";
?>
--EXPECT--
bool(true)
sheetCount: 2
bool(true)
loaded sheetCount: 2
sheet name: Invoice
title: Invoice #3568
name: Name: John Smith
amount: 85
amount format: 5
formula cell: SUM(C8:C10)
isFormula: true
col1 width > 20: true
sheet2 name: Formats
custom format value: 25.718
OK
