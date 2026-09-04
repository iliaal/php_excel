--TEST--
Book: setPassword, dpiAwareness, loadInfoRaw, errorCode, conditionalFormatSize, clear (libxl 5.x)
--EXTENSIONS--
excel
--SKIPIF--
<?php
/* Newest-tier gate: this file spans three libxl generations (setPassword/dpiAwareness,
 * loadInfoRaw, errorCode/conditionalFormatSize/clear); errorCode is the youngest, so its
 * presence implies the rest. A partial tier would pass SKIPIF yet fatal on a missing method. */
if (!method_exists("ExcelBook", "errorCode")) print "skip"; ?>
--FILE--
<?php
$book = new ExcelBook(null, null, true);
$sheet = $book->addSheet("Test");
$sheet->write(1, 0, "data");

// setPassword
$book->setPassword("secret");
echo "setPassword: OK\n";

// dpiAwareness / setDpiAwareness
$orig = $book->dpiAwareness();
var_dump(is_int($orig));
$book->setDpiAwareness(1);
var_dump($book->dpiAwareness());

// save and loadInfoRaw
$tmp = tempnam(sys_get_temp_dir(), "xl") . ".xlsx";
$book->save($tmp);
$raw = file_get_contents($tmp);

$book2 = new ExcelBook(null, null, true);
var_dump($book2->loadInfoRaw($raw));
var_dump($book2->sheetCount());

// errorCode
var_dump($book->errorCode());

// conditionalFormatSize
var_dump($book->conditionalFormatSize());

// clear
$book->clear();
var_dump($book->sheetCount());

unlink($tmp);
echo "OK\n";
?>
--EXPECT--
setPassword: OK
bool(true)
int(1)
bool(true)
int(1)
int(0)
int(0)
int(0)
OK
