--TEST--
Sheet: removePicture, removePictureByIndex, formControl (edge cases on empty sheet)
--EXTENSIONS--
excel
--FILE--
<?php
$book = new ExcelBook(null, null, true);
$sheet = $book->addSheet("Sheet1");

// removePicture on cell with no picture
var_dump($sheet->removePicture(1, 0));

// removePictureByIndex with no pictures
var_dump($sheet->removePictureByIndex(0));

// formControl on index with no form controls
var_dump($sheet->formControlSize());
var_dump($sheet->formControl(0));

echo "OK\n";
?>
--EXPECT--
bool(false)
bool(false)
int(0)
bool(false)
OK
