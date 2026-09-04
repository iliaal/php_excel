--TEST--
Book: isDate1904, setDate1904, getActiveSheet, getSheetName, load, loadInfo
--EXTENSIONS--
excel
--FILE--
<?php
$book = new ExcelBook(null, null, true);
$sheet = $book->addSheet("First");

// isDate1904 / setDate1904
var_dump($book->isDate1904());
var_dump($book->setDate1904(true));
var_dump($book->isDate1904());
var_dump($book->setDate1904(false));
var_dump($book->isDate1904());

// getActiveSheet
var_dump($book->getActiveSheet());
$book->setActiveSheet(0);
var_dump($book->getActiveSheet());

// getSheetName (0-based index)
$book->addSheet("Second");
$book->addSheet("Third");
var_dump($book->getSheetName(0));
var_dump($book->getSheetName(1));
var_dump($book->getSheetName(2));

// load (from raw data)
$sheet->write(1, 0, "test_data");
$tmpf = tempnam(sys_get_temp_dir(), "xls") . ".xlsx";
$book->save($tmpf);
$data = file_get_contents($tmpf);

$book2 = new ExcelBook(null, null, true);
var_dump($book2->load($data));
$s2 = $book2->getSheet(0);
var_dump($s2->read(1, 0));

// loadInfo
$book3 = new ExcelBook(null, null, true);
var_dump($book3->loadInfo($tmpf));
var_dump($book3->sheetCount());
unlink($tmpf);

echo "OK\n";
?>
--EXPECT--
bool(false)
bool(true)
bool(true)
bool(true)
bool(false)
int(0)
int(0)
string(5) "First"
string(6) "Second"
string(5) "Third"
bool(true)
string(9) "test_data"
bool(true)
int(3)
OK
