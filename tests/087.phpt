--TEST--
ExcelSheet::setAutoFitArea(), ExcelSheet::printRepeatCols(), ExcelSheet::printRepeatRows(), ExcelSheet::printArea()
--INI--
date.timezone=America/Toronto
--SKIPIF--
<?php if (!extension_loaded("excel")) print "skip"; ?>
--FILE--
<?php 
    $xb = new ExcelBook(null, null, true);
    $xb->setLocale('UTF-8');
    $xs = new ExcelSheet($xb, 'test');
    $xs->setAutoFitArea(0, 0, 2, 2);


    var_dump($xs->setAutoFitArea(-1, -1, -2, null));
    var_dump($xs->setAutoFitArea(0, -1, -2, null));
    var_dump($xs->setAutoFitArea(0, 0, -2, null));
    var_dump($xs->setAutoFitArea(0, 0, -1, null));
    var_dump($xs->setAutoFitArea(0, 0, -1, -1));
    var_dump($xs->setAutoFitArea(0, 0, 0, 0));
    var_dump($xs->setAutoFitArea(0, 1, 5, 7));
    var_dump($xs->setAutoFitArea(5, 7, 0, 0));

    var_dump($xs->printRepeatCols());
    var_dump($xs->printRepeatRows());
    var_dump($xs->printArea());


    $xs->setPrintArea(0, 3, 0, 7);
    $xs->setPrintRepeatRows(0, 1);
    $xs->setPrintRepeatCols(0, 2);
    var_dump($xs->printRepeatCols());
    var_dump($xs->printRepeatRows());
    var_dump($xs->printArea());

	echo "OK\n";
?>
--EXPECT--
bool(false)
bool(false)
bool(false)
bool(false)
bool(false)
bool(true)
bool(true)
bool(true)
bool(false)
bool(false)
bool(false)
array(2) {
  ["col_start"]=>
  int(0)
  ["col_end"]=>
  int(2)
}
array(2) {
  ["row_start"]=>
  int(0)
  ["row_end"]=>
  int(1)
}
array(4) {
  ["row_start"]=>
  int(0)
  ["col_start"]=>
  int(3)
  ["row_end"]=>
  int(0)
  ["col_end"]=>
  int(7)
}
OK