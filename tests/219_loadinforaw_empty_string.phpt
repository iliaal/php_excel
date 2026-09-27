--TEST--
Book::loadInfoRaw() rejects an empty data string
--EXTENSIONS--
excel
--SKIPIF--
<?php
if (!method_exists(ExcelBook::class, 'loadInfoRaw')) {
    print 'skip LibXL 5.0.1+ required';
}
?>
--DESCRIPTION--
loadInfoRaw() validated only the UINT_MAX size bound, so an empty string was
forwarded to LibXL and surfaced a confusing "readHeader: file is corrupt"
error. It now fails fast like load()/loadFile() do for empty input.
--FILE--
<?php
$b = new ExcelBook(null, null, true);
var_dump($b->loadInfoRaw(''));
// A real payload still loads its info.
$data = file_get_contents(__DIR__ . '/formcontrols.xlsx');
if ($data === false) {
    echo "skip fixture missing\n";
    exit;
}
var_dump($b->loadInfoRaw($data));
var_dump($b->sheetCount() > 0);
?>
--EXPECT--
bool(false)
bool(true)
bool(true)
