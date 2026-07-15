--TEST--
ExcelSheet::addrToRowCol accepts only complete in-range A1 references
--EXTENSIONS--
excel
--FILE--
<?php
$xlsx = new ExcelBook(null, null, true);
$sheet = $xlsx->addSheet('Sheet1');

var_dump($sheet->addrToRowCol('A1'));
var_dump($sheet->addrToRowCol('$XFD$1048576'));
var_dump($sheet->addrToRowCol('iv65536'));

$invalid = ['junk', '1', 'A1junk', 'A1 ', 'XFE1', 'A1048577', 'A0', '$1', 'A$', 'A-1', 'ZZZZZZ999999999999999999'];
$accepted = [];
foreach ($invalid as $reference) {
    if (@$sheet->addrToRowCol($reference) !== false) {
        $accepted[] = $reference;
    }
}
var_dump($accepted);

$xls = new ExcelBook();
$xlsSheet = $xls->addSheet('Sheet1');
var_dump($xlsSheet->addrToRowCol('IV65536'));
var_dump(@$xlsSheet->addrToRowCol('IW1'));
?>
--EXPECT--
array(4) {
  ["row"]=>
  int(0)
  ["column"]=>
  int(0)
  ["col_relative"]=>
  bool(true)
  ["row_relative"]=>
  bool(true)
}
array(4) {
  ["row"]=>
  int(1048575)
  ["column"]=>
  int(16383)
  ["col_relative"]=>
  bool(false)
  ["row_relative"]=>
  bool(false)
}
array(4) {
  ["row"]=>
  int(65535)
  ["column"]=>
  int(255)
  ["col_relative"]=>
  bool(true)
  ["row_relative"]=>
  bool(true)
}
array(0) {
}
array(4) {
  ["row"]=>
  int(65535)
  ["column"]=>
  int(255)
  ["col_relative"]=>
  bool(true)
  ["row_relative"]=>
  bool(true)
}
bool(false)
