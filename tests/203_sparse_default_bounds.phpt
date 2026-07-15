--TEST--
Sparse default reads begin at the first used row or column
--EXTENSIONS--
excel
--FILE--
<?php
$book = new ExcelBook(null, null, true);

$columnSheet = $book->addSheet('Column');
$columnSheet->write(1048575, 0, 'tail');
var_dump($columnSheet->readSparseCol(0) === [1048575 => 'tail']);
var_dump($columnSheet->readSparseCol(0, 1, 10) === []);

$rowSheet = $book->addSheet('Row');
$rowSheet->write(1, 16383, 'tail');
var_dump($rowSheet->readSparseRow(1) === [16383 => 'tail']);
var_dump($rowSheet->readSparseRow(1, 0, 10) === []);
?>
--EXPECT--
bool(true)
bool(true)
bool(true)
bool(true)
