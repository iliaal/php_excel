--TEST--
New Book methods: calcMode, setCalcMode, addFormatFromStyle, removeVBA, removePrinterSettings, dpiAwareness, setDpiAwareness, removeAllPhonetics
--SKIPIF--
<?php if (!extension_loaded("excel") || !method_exists("ExcelBook", "dpiAwareness")) print "skip"; ?>
--FILE--
<?php
$book = new ExcelBook(null, null, true);
$sheet = $book->addSheet("Sheet1");

$cm = $book->calcMode();
var_dump(is_int($cm));
var_dump($book->setCalcMode(1));
var_dump($book->calcMode());
var_dump($book->setCalcMode(0));
var_dump($book->calcMode());

$fmt = $book->addFormatFromStyle(0);
var_dump($fmt instanceof ExcelFormat);

var_dump($book->removeVBA());
var_dump($book->removePrinterSettings());

$dpi = $book->dpiAwareness();
var_dump(is_int($dpi));
var_dump($book->setDpiAwareness(1));
var_dump($book->dpiAwareness());

var_dump($book->removeAllPhonetics());

echo "OK\n";
?>
--EXPECT--
bool(true)
bool(true)
int(1)
bool(true)
int(0)
bool(true)
bool(false)
bool(false)
bool(true)
bool(true)
int(1)
bool(true)
OK
