--TEST--
Test the SKIP_NULLS options, that skips null when writing data
--SKIPIF--
<?php if (!extension_loaded("excel")) die("skip - Excel extension not found"); ?>
--FILE--
<?php 
$book = new ExcelBook();

$sheet = $book->addSheet('Sheet 1');

$sheet->write(1, 0, NULL);
$sheet->writeRow(2, array(1, NULL));

var_dump(
	$sheet->read(1, 0),
	$sheet->readRow(2)
);
?>
--EXPECT--
NULL
array(2) {
  [0]=>
  float(1)
  [1]=>
  NULL
}
