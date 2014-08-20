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

class MyExcelSheet extends ExcelSheet
{
    // nothing
}

$sheet4 = new MyExcelSheet($book, 'Table3');

// due to the internal call of xlBookGetSheet()
// you will receive type ExcelSheet instead of type MyExcelSheet
$sheet5 = $book->getSheetByName('Table3');

var_dump(
    get_class($sheet1),
    get_class($sheet2),
    get_class($sheet3),
    get_class($sheet4),
    get_class($sheet5)
);

?>
--EXPECT--
bool(true)
string(25) "ExcelSheet::__construct()"
bool(true)
string(4) "Test"
int(2)
string(10) "ExcelSheet"
string(10) "ExcelSheet"
string(10) "ExcelSheet"
string(12) "MyExcelSheet"
string(10) "ExcelSheet"
