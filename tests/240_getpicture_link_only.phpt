--TEST--
getPicture() returns empty data and the picture type for link-only XLSX pictures
--EXTENSIONS--
excel
--FILE--
<?php
$png = tempnam(sys_get_temp_dir(), 'pxl') . '.png';
file_put_contents($png, base64_decode('iVBORw0KGgoAAAANSUhEUgAAAAEAAAABCAYAAAAfFcSJAAAADUlEQVR42mNkYPhfDwAChwGA60e6kgAAAABJRU5ErkJggg=='));
$book = new ExcelBook(null, null, true);
$linked = $book->addPictureAsLink($png, false);
$jpeg = $book->addPictureAsLink(__DIR__ . '/phplogo.jpg', false);
$embedded = $book->addPictureAsLink($png, true);
foreach ([$linked, $jpeg, $embedded] as $id) {
    $pic = $book->getPicture($id);
    var_dump(array_keys($pic), $pic['type'], strlen($pic['data']));
}
var_dump($book->getPicture($embedded)['data'] === file_get_contents($png));
var_dump($book->getPicture($linked)['type'] === ExcelBook::PICTURETYPE_PNG);
var_dump($book->getPicture($jpeg)['type'] === ExcelBook::PICTURETYPE_JPEG);
unlink($png);
unlink(substr($png, 0, -4));
?>
--EXPECT--
array(2) {
  [0]=>
  string(4) "data"
  [1]=>
  string(4) "type"
}
int(0)
int(0)
array(2) {
  [0]=>
  string(4) "data"
  [1]=>
  string(4) "type"
}
int(1)
int(0)
array(2) {
  [0]=>
  string(4) "data"
  [1]=>
  string(4) "type"
}
int(0)
int(70)
bool(true)
bool(true)
bool(true)
