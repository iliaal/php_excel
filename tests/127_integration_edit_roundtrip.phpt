--TEST--
Integration: save workbook, reload, modify cells, save again, verify modifications
--EXTENSIONS--
excel
--FILE--
<?php
// Step 1: Create and save
$book = new ExcelBook(null, null, true);
$sheet = $book->addSheet("Data");
$sheet->write(1, 0, "original");
$sheet->write(1, 1, 100);
$sheet->write(2, 0, "keep");
$sheet->write(2, 1, 200);

$tmp = tempnam(sys_get_temp_dir(), "xl") . ".xlsx";
$book->save($tmp);

// Step 2: Load, modify, save
$book2 = new ExcelBook(null, null, true);
$book2->loadFile($tmp);
$s = $book2->getSheet(0);
echo "before: " . $s->read(1, 0) . " = " . $s->read(1, 1) . "\n";

$s->write(1, 0, "modified");
$s->write(1, 1, 999);
$book2->save($tmp);

// Step 3: Reload and verify
$book3 = new ExcelBook(null, null, true);
$book3->loadFile($tmp);
$s2 = $book3->getSheet(0);
echo "after: " . $s2->read(1, 0) . " = " . $s2->read(1, 1) . "\n";
echo "untouched: " . $s2->read(2, 0) . " = " . $s2->read(2, 1) . "\n";

unlink($tmp);
echo "OK\n";
?>
--EXPECT--
before: original = 100
after: modified = 999
untouched: keep = 200
OK
