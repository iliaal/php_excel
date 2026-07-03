--TEST--
ExcelBook partial and without-empty-cell load APIs
--EXTENSIONS--
excel
--SKIPIF--
<?php if (!method_exists('ExcelBook', 'loadPartially')) print "skip"; ?>
--FILE--
<?php
$book = new ExcelBook(null, null, true);
$s1 = $book->addSheet("First");
$s2 = $book->addSheet("Second");

$s1->write(1, 0, "first-r1");
$s1->write(5, 0, "first-r5");
$s2->write(1, 0, "second-r1");
$s2->write(5, 0, "second-r5");

$tmp = tempnam("/tmp", "xl_partial_") . ".xlsx";
$book->save($tmp);
$raw = file_get_contents($tmp);

$rawBook = new ExcelBook(null, null, true);
var_dump($rawBook->loadPartially($raw, 1, 1, 2, false));
echo "raw sheet count: " . $rawBook->sheetCount() . "\n";
$rawSheet = $rawBook->getSheet(0);
echo "raw loaded row 1: " . $rawSheet->read(1, 0) . "\n";
echo "raw skipped row 5: ";
var_dump($rawSheet->read(5, 0));

$fileBook = new ExcelBook(null, null, true);
var_dump($fileBook->loadFilePartially($tmp, 0, 1, 2, true));
echo "file sheet count: " . $fileBook->sheetCount() . "\n";
echo "file loaded row 1: " . $fileBook->getSheet(0)->read(1, 0) . "\n";
echo "file other sheet name: " . $fileBook->getSheetName(1) . "\n";

$compactBook = new ExcelBook(null, null, true);
var_dump($compactBook->loadFileWithoutEmptyCells($tmp));
echo "compact sheet count: " . $compactBook->sheetCount() . "\n";
echo "compact row 1: " . $compactBook->getSheet(0)->read(1, 0) . "\n";

unlink($tmp);
echo "OK\n";
?>
--EXPECT--
bool(true)
raw sheet count: 1
raw loaded row 1: second-r1
raw skipped row 5: string(0) ""
bool(true)
file sheet count: 2
file loaded row 1: first-r1
file other sheet name: Second
bool(true)
compact sheet count: 2
compact row 1: first-r1
OK
