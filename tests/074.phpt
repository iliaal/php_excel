--TEST--
Test the SKIP_NULLS options, that skips null when writing data
--SKIPIF--
<?php if (!extension_loaded("excel")) print "skip"; ?>
--FILE--
<?php 
$book = new ExcelBook();

$sheet = $book->addSheet('Sheet 1');

$sheet->write(0, 0, NULL);
$sheet->writeRow(1, array(1, NULL));

var_dump(
	$sheet->read(0, 0),
	$sheet->readRow(1)
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
