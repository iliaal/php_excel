--TEST--
Path operations use PHP streams or reject path-only LibXL APIs while open_basedir is active
--EXTENSIONS--
excel
--SKIPIF--
<?php if (!method_exists(ExcelBook::class, "loadInfoRaw")) print "skip LibXL 5.0.1+ required"; ?>
--FILE--
<?php
$source = __DIR__ . "/189_source.xlsx";
$saved = __DIR__ . "/189_saved.xlsx";

$book = new ExcelBook(null, null, true);
$sheet = $book->addSheet("source");
$sheet->write(1, 0, "value");
var_dump($book->save($source));

ini_set("open_basedir", __DIR__);

$loaded = new ExcelBook(null, null, true);
var_dump($loaded->loadFile($source));
var_dump($loaded->getSheet(0)->read(1, 0));

if (method_exists(ExcelBook::class, "loadFilePartially")) {
    $partial = new ExcelBook(null, null, true);
    var_dump($partial->loadFilePartially($source, 0, 1, 1));
}

$info = new ExcelBook(null, null, true);
var_dump($info->loadInfo($source));
var_dump($info->getSheetName(0));

$pictures = new ExcelBook(null, null, true);
var_dump($pictures->addPictureFromFile(__DIR__ . "/phplogo.jpg") >= 0);

$output = new ExcelBook(null, null, true);
$output->addSheet("saved")->write(1, 0, "ok");
var_dump($output->save($saved));
var_dump(substr(file_get_contents($saved), 0, 2));

set_error_handler(static fn() => true);
if (method_exists(ExcelBook::class, "loadFileWithoutEmptyCells")) {
    var_dump($output->loadFileWithoutEmptyCells($source));
}
var_dump($output->addPictureAsLink(__DIR__ . "/phplogo.jpg", true));
restore_error_handler();

unlink($source);
unlink($saved);
?>
--EXPECT--
bool(true)
bool(true)
string(5) "value"
bool(true)
bool(true)
string(6) "source"
bool(true)
bool(true)
string(2) "PK"
bool(false)
bool(false)
