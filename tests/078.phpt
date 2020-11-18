--TEST--
Sheet::colHidden(), Sheet::rowHidden(), Sheet::setColHidden() and Sheet::setRowHidden()
--SKIPIF--
<?php if (!extension_loaded("excel") || !in_array('rowHidden', get_class_methods('ExcelSheet'))) print "skip"; ?>
--FILE--
<?php 
$book = new ExcelBook();

$sheet = $book->addSheet('Sheet 1');

var_dump(
    $sheet->rowHidden(0),
    $sheet->colHidden(0),
    $sheet->rowHidden(null),
    $sheet->colHidden(null),
    $sheet->setRowHidden(1),
    $sheet->setColHidden(1),
    $sheet->setRowHidden(1, true),
    $sheet->setRowHidden(-1, true),
    $sheet->setRowHidden(null, true),
    $sheet->setColHidden(1, true),
    $sheet->setColHidden(-1, true),
    $sheet->setColHidden(null, true),
    $sheet->rowHidden(1),
    $sheet->colHidden(1),
    $sheet->rowHidden(0),
    $sheet->colHidden(0),
    $sheet->setRowHidden(null, false),
    $sheet->setColHidden(null, false),
    $sheet->rowHidden(0),
    $sheet->colHidden(0)
);
?>
--EXPECTF--
Warning: ExcelSheet::setRowHidden() expects exactly 2 parameters, 1 given in %s on line %d

Warning: ExcelSheet::setColHidden() expects exactly 2 parameters, 1 given in %s on line %d
bool(false)
bool(false)
bool(false)
bool(false)
bool(false)
bool(false)
bool(true)
bool(false)
bool(true)
bool(true)
bool(false)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(false)
bool(false)
