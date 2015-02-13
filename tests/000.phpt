--TEST--
LibXL licensed version vs trial version test
--INI--
date.timezone=America/Toronto
--SKIPIF--
<?php
    if (!extension_loaded("excel")) die("skip - Excel extension not found");
    if (boolval(getenv('TRAVIS_CI'))) die("skip - TravisCI w/o credentials");
?>
--FILE--
<?php
    echo 'TRAVIS_CI' . PHP_EOL;
    var_dump(getenv('TRAVIS_CI'));
    
    echo 'TRAVIS' . PHP_EOL;
    var_dump(getenv('TRAVIS'));
    
    echo 'TRAVIS_PHP_VERSION' . PHP_EOL;
    var_dump(getenv('TRAVIS_PHP_VERSION'));
    
    echo 'TRAVIS_2' . PHP_EOL;
    var_dump($_ENV['TRAVIS']);
    
    echo 'TRAVIS_CI_2' . PHP_EOL;
    var_dump($_ENV['TRAVIS_CI']);
    
    echo 'TRAVIS_PHP_VERSION_2' . PHP_EOL;
    var_dump($_ENV['TRAVIS_PHP_VERSION']);

    $data = array("foo");
    
    $x = new ExcelBook('bar', 'baz');
	$s = $x->addSheet("Sheet1");
	var_dump($s->writeRow(1, $data));
	var_dump($s->readRow(0));
    
	$x = new ExcelBook();
	$s = $x->addSheet("Sheet1");
	var_dump($s->writeRow(1, $data));
	var_dump($s->readRow(0));
    
    $x = new ExcelBook(null, null);
    $s = $x->addSheet("Sheet1");
	var_dump($s->writeRow(1, $data));
	var_dump($s->readRow(0));
    
    $x = new ExcelBook(ini_get('excel.license_name'), ini_get('excel.license_key'));
    $s = $x->addSheet("Sheet1");
	var_dump($s->writeRow(1, $data));
	var_dump($s->readRow(0));

	echo "OK\n";
?>
--EXPECT--
bool(true)
array(1) {
  [0]=>
  string(92) "Created by LibXL trial version. Please buy the LibXL full version for removing this message."
}
bool(true)
array(1) {
  [0]=>
  string(0) ""
}
bool(true)
array(1) {
  [0]=>
  string(0) ""
}
bool(true)
array(1) {
  [0]=>
  string(0) ""
}
OK
