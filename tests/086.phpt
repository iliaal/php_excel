--TEST--
test Sheet::isLicensed()
--SKIPIF--
<?php
    if (!extension_loaded("excel")) die("skip - Excel extension not found");
    if ((bool) getenv("TRAVIS") === "true") die("skip - TravisCI w/o credentials");
    if (!in_array('isLicensed', get_class_methods('ExcelSheet'))) die("skip - ExcelSheet::isLicensed() missing");
?>
--FILE--
<?php

// requires correct php.ini settings
// excel.license_name="<NAME>" and excel.license_key="<KEY>"
$book = new ExcelBook();
$sheet = $book->addSheet("Sheet1");
var_dump(
    $sheet->isLicensed()
);

$book = new ExcelBook('x', 'y');
$sheet = $book->addSheet("Sheet1");
var_dump(
    $sheet->isLicensed()
);

// not working - improper chaining?
// var_dump(
//     (new \ExcelBook())->addSheet("foo")->isLicensed()
// );

?>
--EXPECT--
bool(true)
bool(false)
