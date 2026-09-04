--TEST--
ExcelSheet optional params: addPictureScaled, addPictureDim, named ranges with scope, setProtect
--EXTENSIONS--
excel
--FILE--
<?php
$book = new ExcelBook(null, null, true);
$sheet = $book->addSheet("Pictures");

$pic_file = dirname(__FILE__) . "/phplogo.jpg";
$pic = $book->addPictureFromFile($pic_file);
echo "pic_id: ";
var_dump(is_int($pic) && $pic >= 0);

// addPictureScaled with x_offset, y_offset, pos parameters
$sheet->addPictureScaled(1, 1, $pic, 0.5, 10, 20, 0);
echo "addPictureScaled with offsets+pos: ";
var_dump($book->getError());

$sheet->addPictureScaled(3, 1, $pic, 0.25, 5, 15, 1);
echo "addPictureScaled with pos=1: ";
var_dump($book->getError());

// addPictureDim with x_offset, y_offset, pos parameters
$sheet->addPictureDim(5, 1, $pic, 200, 150, 10, 20, 0);
echo "addPictureDim with offsets+pos: ";
var_dump($book->getError());

$sheet->addPictureDim(7, 1, $pic, 100, 75, 5, 10, 1);
echo "addPictureDim with pos=1: ";
var_dump($book->getError());

// setNamedRange with scope_id
$sheet2 = $book->addSheet("Ranges");
$sheet2->write(1, 0, "A");
$sheet2->write(1, 1, "B");
$sheet2->write(2, 0, "C");
$sheet2->write(2, 1, "D");

$result = $sheet2->setNamedRange("test_range", 1, 2, 0, 1, ExcelBook::SCOPE_WORKBOOK);
echo "setNamedRange with scope: ";
var_dump($result);

// getNamedRange with scope_id
$range = $sheet2->getNamedRange("test_range", ExcelBook::SCOPE_WORKBOOK);
echo "getNamedRange with scope: ";
var_dump($range);

// delNamedRange with scope_id
$del = $sheet2->delNamedRange("test_range", ExcelBook::SCOPE_WORKBOOK);
echo "delNamedRange with scope: ";
var_dump($del);

// verify the range is gone
$gone = $sheet2->getNamedRange("test_range", ExcelBook::SCOPE_WORKBOOK);
echo "getNamedRange after delete: ";
var_dump($gone);

// setProtect with password
$sheet3 = $book->addSheet("Protected");
$result = $sheet3->setProtect(true, "secret");
echo "setProtect with password: ";
var_dump($result);
echo "protect state: ";
var_dump($sheet3->protect());

// setProtect with password and enhancedProtection
$sheet4 = $book->addSheet("ProtectedEnhanced");
$result = $sheet4->setProtect(true, "pass123", ExcelSheet::PROT_ALL);
echo "setProtect with enhanced: ";
var_dump($result);
echo "protect state (enhanced): ";
var_dump($sheet4->protect());

// verify protection persists after save+reload
$tmp = tempnam(sys_get_temp_dir(), "xl_protect_") . ".xlsx";
$book->save($tmp);

$book2 = new ExcelBook(null, null, true);
$book2->loadFile($tmp);

$s3 = $book2->getSheet(2);
echo "protect after reload: ";
var_dump($s3->protect());

$s4 = $book2->getSheet(3);
echo "protect (enhanced) after reload: ";
var_dump($s4->protect());

// disable protection
$s3->setProtect(false);
echo "protect after disable: ";
var_dump($s3->protect());

unlink($tmp);
echo "OK\n";
?>
--EXPECT--
pic_id: bool(true)
addPictureScaled with offsets+pos: bool(false)
addPictureScaled with pos=1: bool(false)
addPictureDim with offsets+pos: bool(false)
addPictureDim with pos=1: bool(false)
setNamedRange with scope: bool(true)
getNamedRange with scope: array(5) {
  ["row_first"]=>
  int(1)
  ["row_last"]=>
  int(2)
  ["col_first"]=>
  int(0)
  ["col_last"]=>
  int(1)
  ["hidden"]=>
  bool(false)
}
delNamedRange with scope: bool(true)
getNamedRange after delete: bool(false)
setProtect with password: bool(true)
protect state: bool(true)
setProtect with enhanced: bool(true)
protect state (enhanced): bool(true)
protect after reload: bool(true)
protect (enhanced) after reload: bool(true)
protect after disable: bool(false)
OK
