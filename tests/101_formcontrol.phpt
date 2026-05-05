--TEST--
ExcelFormControl: formControlSize on empty sheet
--EXTENSIONS--
excel
--FILE--
<?php
$book = new ExcelBook(null, null, true);
$sheet = $book->addSheet("Sheet1");

var_dump($sheet->formControlSize());

echo "OK\n";
?>
--EXPECT--
int(0)
OK
