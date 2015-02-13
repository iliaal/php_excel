--TEST--
A bug that casues numberic fields with custom formats to be dectected as dates
--SKIPIF--
<?php if (!extension_loaded("excel")) die("skip - Excel extension not found"); ?>
--FILE--
<?php 
$book = new ExcelBook();

$formatString = '#,##0.00_);[Red](#,##0.00)';
$cfid = $book->addCustomFormat($formatString);

$number_format = $book->addFormat();
$number_format->numberFormat($cfid);

$sheet = $book->addSheet('Sheet 1');
$sheet->write(1, 0, 10, $number_format);

var_dump(
	$sheet->isDate(1, 0),
	$sheet->read(1, 0)
);
?>
--EXPECT--
bool(false)
float(10)
