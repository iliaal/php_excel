--TEST--
Book::writeRow() with reference
--SKIPIF--
<?php if (!extension_loaded("excel")) print "skip"; ?>
--FILE--
<?php
	$data = array (
		0 => 'test ref value'
	);

	$xlBook = new \ExcelBook();
	$xlSheet = $xlBook->addSheet('Sheet');

	var_dump($data);

	array_walk($data, function(&$field) {
		$field = (string) $field;
	});

	var_dump($data);

	$xlSheet->writeRow(1, $data);

	echo "OK\n";

?>
--EXPECT--
array(1) {
  [0]=>
  string(14) "test ref value"
}
array(1) {
  [0]=>
  string(14) "test ref value"
}
OK
