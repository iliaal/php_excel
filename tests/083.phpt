--TEST--
Sheet::__construct()
--SKIPIF--
<?php if (!extension_loaded("excel")) print "skip"; ?>
--FILE--
<?php

$book = new ExcelBook();

// $sheet0 = new ExcelSheet(null, 'Table1');
// $sheet0 = new ExcelSheet($book, null);

$sheet1 = new ExcelSheet($book, 'Table1');
$sheet2 = $book->getSheetByName('Table1');

var_dump(
    $sheet1->write(1, 1, 'ExcelSheet::__construct()'),
    $sheet2->read(1, 1),
    $sheet2->write(2, 2, 'Test'),
    $sheet1->read(2, 2)
);

$sheet3 = $book->addSheet('Table2');

var_dump(
    $book->sheetCount()
);

?>
--EXPECT--
bool(true)
string(25) "ExcelSheet::__construct()"
bool(true)
string(4) "Test"
int(2)
