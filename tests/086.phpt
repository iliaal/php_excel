--TEST--
test Sheet::isLicensed()
--SKIPIF--
<?php if (!extension_loaded("excel") || !ExcelBook::requiresKey()) print "skip"; ?>
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

?>
--EXPECT--
bool(true)
bool(false)
