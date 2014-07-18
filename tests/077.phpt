--TEST--
Test the ExcelSheet::splitInfo() method introduced in libxl 3.6.0.
--SKIPIF--
<?php if (!extension_loaded("excel")) print "skip"; ?>
--FILE--
<?php 
$book = new ExcelBook();

$sheet = $book->addSheet('Sheet 1');

var_dump(
    $sheet->splitInfo(),
    $sheet->splitSheet(5, 2),
    $sheet->splitInfo(),
    $sheet->splitSheet(null, null),
    $sheet->splitInfo()
);

?>
--EXPECT--
bool(false)
NULL
array(2) {
  ["row"]=>
  int(5)
  ["col"]=>
  int(2)
}
NULL
array(2) {
  ["row"]=>
  int(0)
  ["col"]=>
  int(0)
}
