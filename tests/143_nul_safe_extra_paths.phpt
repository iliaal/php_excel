--TEST--
NUL guards on cell writes, sheet name lookup, file paths, and license args
--EXTENSIONS--
excel
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

// Constructor license args must reject NUL by throwing — PHP ignores
// constructor return values, so RETURN_FALSE would leave the caller
// with a usable object built from rejected input.
try {
    new ExcelBook("name\0evil", "key", true);
    echo "name nul: no exception\n";
} catch (Exception $e) {
    echo "name nul: " . $e->getMessage() . "\n";
}
try {
    new ExcelBook("name", "key\0evil", true);
    echo "key nul: no exception\n";
} catch (Exception $e) {
    echo "key nul: " . $e->getMessage() . "\n";
}

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
name nul: License name/key must not contain NUL bytes
key nul: License name/key must not contain NUL bytes
OK
