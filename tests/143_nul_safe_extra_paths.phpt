--TEST--
NUL guards on cell writes, sheet name lookup, file paths, and license args
--SKIPIF--
<?php if (!extension_loaded("excel")) print "skip"; ?>
--FILE--
<?php
$b = new ExcelBook(null, null, true);
$s = $b->addSheet("safe");

// write with embedded NUL must fail, not silently truncate
var_dump(@$s->write(1, 0, "abc\0evil"));

// writeRow must fail on first NUL value
var_dump(@$s->writeRow(2, ["ok", "abc\0evil"]));

// writeCol same
var_dump(@$s->writeCol(0, ["ok", "abc\0evil"]));

// getSheetByName must reject embedded NUL (not match a truncated prefix)
var_dump(@$b->getSheetByName("safe\0evil"));

// loadFile must reject NUL path
$tmp = tempnam(sys_get_temp_dir(), "excel_nul_");
var_dump(@$b->loadFile($tmp . "\0suffix"));

// save must reject NUL path
var_dump(@$b->save($tmp . "\0suffix"));

// addPictureFromFile must reject NUL path
var_dump(@$b->addPictureFromFile(__DIR__ . "/phplogo.jpg\0suffix"));

unlink($tmp);

// Constructor license args must reject NUL
var_dump(@new ExcelBook("name\0evil", "key", true));
var_dump(@new ExcelBook("name", "key\0evil", true));

echo "OK\n";
?>
--EXPECT--
bool(false)
bool(false)
bool(false)
bool(false)
bool(false)
bool(false)
bool(false)
object(ExcelBook)#3 (0) {
}
object(ExcelBook)#3 (0) {
}
OK
