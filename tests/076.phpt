--TEST--
Test the ExcelSheet::mergeSize(), ExcelSheet::merge(), ExcelSheet::delMergeByIndex() methods introduced in libxl 3.6.0.
--SKIPIF--
<?php
    if (!extension_loaded("excel")) die("skip - Excel extension not found");
    if (!in_array('mergeSize', get_class_methods('ExcelSheet'))) die("skip - ExcelSheet::mergeSize() missing");
    if ((bool) getenv("TRAVIS") === "true") die("skip - TravisCI w/o credentials");
?>
--FILE--
<?php 

// this test fails without credentials

$book = new ExcelBook();
$sheet = $book->addSheet('Sheet 1');

var_dump(
    $sheet->mergeSize(),
    $sheet->merge(0),
    $sheet->setMerge(2, 4, 6, 8),
    $sheet->mergeSize(),
    $sheet->merge(0),
    $sheet->delMergeByIndex(0),
    $sheet->delMergeByIndex(0),
    $sheet->merge(0),
    $sheet->mergeSize()
);

?>
--EXPECT--
int(0)
bool(false)
bool(true)
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
bool(true)
bool(false)
bool(false)
int(0)
