--TEST--
test Book::save() and Book::loadFile()
--SKIPIF--
<?php if (!extension_loaded("excel")) die("skip - Excel extension not found"); ?>
--FILE--
<?php

$tmp_filename = dirname(__FILE__).'/test088.xls';

$book = new ExcelBook();
$sheet = $book->addSheet('Sheet1');
$sheet->write(1, 1, 'foo');
$book->save($tmp_filename);

$book = new ExcelBook();
$book->loadFile($tmp_filename);
$sheet = $book->getSheet(0);

var_dump($sheet->read(1, 1));

echo 'OK';

?>
--CLEAN--
<?php
$tmp_filename = dirname(__FILE__).'/test088.xls';

if (is_file($tmp_filename)) {
    unlink($tmp_filename);
}
?>
--EXPECT--
string(3) "foo"
OK
