--TEST--
Existing sheet wrappers remain attached to their sheet after insertion
--EXTENSIONS--
excel
--FILE--
<?php
$book = new ExcelBook(null, null, true);
$sheet = $book->addSheet('original');
$sheet->write(1, 0, 'kept');

var_dump($book->insertSheet(0, 'inserted') instanceof ExcelSheet);
var_dump($sheet->read(1, 0));
var_dump($sheet->name());
echo "OK\n";
?>
--EXPECT--
bool(true)
string(4) "kept"
string(8) "original"
OK
