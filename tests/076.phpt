--TEST--
Test the ExcelSheet::mergeSize(), ExcelSheet::merge(), ExcelSheet::delMergeByIndex() methods introduced in libxl 3.6.0.
--EXTENSIONS--
excel
--SKIPIF--
<?php if (!in_array('mergeSize', get_class_methods('ExcelSheet'))) print "skip"; ?>
--FILE--
<?php
$book = new ExcelBook();

$sheet = $book->addSheet('Sheet 1');

$initialMerges = $sheet->mergeSize();

$sheet->setMerge(2, 4, 6, 8);

var_dump($sheet->mergeSize() - $initialMerges);

$idx = $sheet->mergeSize() - 1;
var_dump($sheet->merge($idx));

$sheet->delMergeByIndex($idx);

var_dump($sheet->mergeSize() - $initialMerges);

?>
--EXPECT--
int(1)
array(4) {
  ["row_first"]=>
  int(2)
  ["row_last"]=>
  int(4)
  ["col_first"]=>
  int(6)
  ["col_last"]=>
  int(8)
}
int(0)
