--TEST--
LibXL licensed version vs trial version test
--INI--
date.timezone=America/Toronto
--EXTENSIONS--
excel
--SKIPIF--
<?php if (!extension_loaded("excel") || !ExcelBook::requiresKey() || !ini_get("excel.license_name") || !ini_get("excel.license_key")) print "skip"; ?>
--FILE--
<?php
    $data = array("foo");

    $x = new ExcelBook('bar', 'baz');
	$s = $x->addSheet("Sheet1");
	var_dump($s->writeRow(1, $data));
	var_dump($s->readRow(1));

	$x = new ExcelBook();
	$s = $x->addSheet("Sheet1");
	var_dump($s->writeRow(1, $data));
	var_dump($s->readRow(1));

    $x = new ExcelBook(ini_get('excel.license_name'), ini_get('excel.license_key'));
    $s = $x->addSheet("Sheet1");
	var_dump($s->writeRow(1, $data));
	var_dump($s->readRow(1));

	echo "OK\n";
?>
--EXPECT--
bool(true)
array(1) {
  [0]=>
  string(3) "foo"
}
bool(true)
array(1) {
  [0]=>
  string(3) "foo"
}
bool(true)
array(1) {
  [0]=>
  string(3) "foo"
}
OK
