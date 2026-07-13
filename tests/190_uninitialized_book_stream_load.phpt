--TEST--
Stream-backed load paths release the read buffer when the book handle is missing
--EXTENSIONS--
excel
--FILE--
<?php
// A book that bypassed __construct has no libxl handle. The stream branches
// fetch the handle only after reading the source, so the bail path must
// release the buffer it already owns (observable as a debug-build leak).
$book = (new ReflectionClass(ExcelBook::class))->newInstanceWithoutConstructor();

var_dump(@$book->loadFile("data://text/plain,not-a-workbook"));

if (method_exists($book, "loadFilePartially")) {
    var_dump(@$book->loadFilePartially("data://text/plain,not-a-workbook", 0, 0, 10));
} else {
    echo "bool(false)\n";
}

var_dump(@$book->addPictureFromFile("data://text/plain,not-a-picture"));

var_dump(@$book->loadInfo("data://text/plain,not-a-workbook"));

echo "OK\n";
?>
--EXPECT--
bool(false)
bool(false)
bool(false)
bool(false)
OK
