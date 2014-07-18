--TEST--
Test the ExcelSheet::hyperlinkSize(), ExcelSheet::hyperlink(), ExcelSheet::delHyperlink(), ExcelSheet::addHyperlink() methods introduced in libxl 3.6.0.
--SKIPIF--
<?php if (!extension_loaded("excel")) print "skip"; ?>
--FILE--
<?php 
$book = new ExcelBook();

$sheet = $book->addSheet('Sheet 1');

var_dump(
    $sheet->hyperlinkSize(),
    $sheet->addHyperlink('http://example.org', 1, 1, 1, 1),
    $sheet->hyperlinkSize(),
    $sheet->hyperlink(0),
    $sheet->hyperlink(1),
    $sheet->delHyperlink(0),
    $sheet->delHyperlink(0),
    $sheet->hyperlinkSize()
);

?>
--EXPECT--
int(0)
NULL
int(1)
array(5) {
  ["hyperlink"]=>
  string(18) "http://example.org"
  ["row_first"]=>
  int(1)
  ["row_last"]=>
  int(1)
  ["col_first"]=>
  int(1)
  ["col_last"]=>
  int(1)
}
bool(false)
bool(true)
bool(false)
int(0)
