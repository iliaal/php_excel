--TEST--
Integration: multiple sheets, copySheet, deleteSheet, sheet manipulation
--EXTENSIONS--
excel
--FILE--
<?php
$book = new ExcelBook(null, null, true);

$s1 = $book->addSheet("First");
$s1->write(1, 0, "sheet1");
$s2 = $book->addSheet("Second");
$s2->write(1, 0, "sheet2");
$s3 = $book->addSheet("Third");
$s3->write(1, 0, "sheet3");

echo "count: " . $book->sheetCount() . "\n";

// Copy sheet
$s4 = $book->copySheet("FirstCopy", 0);
echo "after copy: " . $book->sheetCount() . "\n";
echo "copy name: " . $s4->name() . "\n";
echo "copy data: " . $s4->read(1, 0) . "\n";

// Delete sheet
var_dump($book->deleteSheet(2));
echo "after delete: " . $book->sheetCount() . "\n";

// Insert sheet
$s5 = $book->insertSheet(0, "Inserted");
$s5->write(1, 0, "inserted");
echo "after insert: " . $book->sheetCount() . "\n";
echo "sheet0 name: " . $book->getSheet(0)->name() . "\n";

// Active sheet
$book->setActiveSheet(1);
echo "active: " . $book->getActiveSheet() . "\n";

// Sheet type
echo "type: " . $book->sheetType(0) . "\n";

echo "OK\n";
?>
--EXPECT--
count: 3
after copy: 4
copy name: FirstCopy
copy data: sheet1
bool(true)
after delete: 3
after insert: 4
sheet0 name: Inserted
active: 1
type: 0
OK
