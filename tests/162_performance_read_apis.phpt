--TEST--
ExcelSheet range and sparse read APIs
--EXTENSIONS--
excel
--FILE--
<?php
$book = new ExcelBook(null, null, true);
$sheet = $book->addSheet("Read APIs");

$sheet->write(1, 0, "A1");
$sheet->write(1, 2, null);
$sheet->write(1, 3, "A4");
$sheet->write(2, 1, 22);
$sheet->write(2, 3, "=10*2");
$sheet->write(3, 0, true);
$sheet->write(3, 3, "");

$tmp = tempnam("/tmp", "xl_range_") . ".xlsx";
$book->save($tmp);

$book2 = new ExcelBook(null, null, true);
$book2->loadFile($tmp);
$sheet2 = $book2->getSheet(0);

echo "range formula=true:\n";
var_dump($sheet2->readRange(1, 2, 0, 3, true));

echo "range formula=false:\n";
var_dump($sheet2->readRange(1, 2, 0, 3, false));

echo "sparse row:\n";
var_dump($sheet2->readSparseRow(1, 0, 3, false));

echo "sparse col:\n";
var_dump($sheet2->readSparseCol(3, 1, 3, true));

echo "invalid range:\n";
var_dump(@$sheet2->readRange(2, 1, 0, 1));

unlink($tmp);
echo "OK\n";
?>
--EXPECT--
range formula=true:
array(2) {
  [0]=>
  array(4) {
    [0]=>
    string(2) "A1"
    [1]=>
    string(0) ""
    [2]=>
    string(0) ""
    [3]=>
    string(2) "A4"
  }
  [1]=>
  array(4) {
    [0]=>
    string(0) ""
    [1]=>
    float(22)
    [2]=>
    string(0) ""
    [3]=>
    string(4) "10*2"
  }
}
range formula=false:
array(2) {
  [0]=>
  array(4) {
    [0]=>
    string(2) "A1"
    [1]=>
    string(0) ""
    [2]=>
    string(0) ""
    [3]=>
    string(2) "A4"
  }
  [1]=>
  array(4) {
    [0]=>
    string(0) ""
    [1]=>
    float(22)
    [2]=>
    string(0) ""
    [3]=>
    float(0)
  }
}
sparse row:
array(2) {
  [0]=>
  string(2) "A1"
  [3]=>
  string(2) "A4"
}
sparse col:
array(3) {
  [1]=>
  string(2) "A4"
  [2]=>
  string(4) "10*2"
  [3]=>
  string(0) ""
}
invalid range:
bool(false)
OK
