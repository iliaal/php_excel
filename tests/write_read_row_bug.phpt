--TEST--
A bug with using readRow/writeRow combination to handle NULL values
--SKIPIF--
<?php if (!extension_loaded("excel")) print "skip"; ?>
--FILE--
<?php 
$book = new ExcelBook();
$ws = $book->addSheet("foo");

$row = array('a', 'b', 'c', NULL, 1, 2, 3);
var_dump(
	$ws->writeRow(1, $row),
	$ws->readRow(1)
);
?>
--EXPECT--
bool(true)
array(7) {
  [0]=>
  string(1) "a"
  [1]=>
  string(1) "b"
  [2]=>
  string(1) "c"
  [3]=>
  NULL
  [4]=>
  float(1)
  [5]=>
  float(2)
  [6]=>
  float(3)
}
