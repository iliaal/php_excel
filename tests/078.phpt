--TEST--
Sheet::colHidden(), Sheet::rowHidden(), Sheet::setColHidden() and Sheet::setRowHidden()
--EXTENSIONS--
excel
--SKIPIF--
<?php if (!in_array('rowHidden', get_class_methods('ExcelSheet'))) print "skip"; ?>
--FILE--
<?php
$book = new ExcelBook();

$sheet = $book->addSheet('Sheet 1');

var_dump(
    $sheet->rowHidden(1),
    $sheet->colHidden(1),
    $sheet->setRowHidden(1, true),
    $sheet->setColHidden(1, true),
    $sheet->rowHidden(1),
    $sheet->colHidden(1),
    $sheet->setRowHidden(1, false),
    $sheet->setColHidden(1, false),
    $sheet->rowHidden(1),
    $sheet->colHidden(1)
);
?>
--EXPECT--
bool(false)
bool(false)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(false)
bool(false)
