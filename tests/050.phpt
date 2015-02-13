--TEST--
Column Write
--INI--
date.timezone=America/Toronto
--SKIPIF--
<?php
    if (!extension_loaded("excel")) die("skip - Excel extension not found");
    if (boolval(getenv('TRAVIS_CI'))) die("skip - TravisCI w/o credentials");
?>
--FILE--
<?php
	$x = new ExcelBook();

	$s = $x->addSheet("Sheet 1");

	$data = array(true, 1.222, 434324, "fsdfasDF", NULL, "", false, -3321, -77.3321, "a a a a a aa");

	var_dump($s->writeCol(0, $data));
	var_dump($s->writeCol(1, $data, 5));

	$format = $x->addFormat();
	$format->borderStyle(ExcelFormat::BORDERSTYLE_THIN);

	var_dump($s->writeCol(2, $data, 0, $format));

	var_dump($s->writeCol(-1, $data));
	var_dump($s->writeCol(1, $data, -1));

	var_dump($s->readCol(0));
	var_dump($s->readCol(1));
	var_dump($s->readCol(2));

	echo "OK\n";
?>
--EXPECTF--
bool(true)
bool(true)
bool(true)

Warning: ExcelSheet::writeCol(): Invalid column number '-1' in %s on line %d
bool(false)

Warning: ExcelSheet::writeCol(): Invalid starting row number '-1' in %s on line %d
bool(false)
array(15) {
  [0]=>
  bool(true)
  [1]=>
  float(1.222)
  [2]=>
  float(434324)
  [3]=>
  string(8) "fsdfasDF"
  [4]=>
  NULL
  [5]=>
  string(0) ""
  [6]=>
  bool(false)
  [7]=>
  float(-3321)
  [8]=>
  float(-77.3321)
  [9]=>
  string(12) "a a a a a aa"
  [10]=>
  string(0) ""
  [11]=>
  string(0) ""
  [12]=>
  string(0) ""
  [13]=>
  string(0) ""
  [14]=>
  string(0) ""
}
array(15) {
  [0]=>
  string(0) ""
  [1]=>
  string(0) ""
  [2]=>
  string(0) ""
  [3]=>
  string(0) ""
  [4]=>
  string(0) ""
  [5]=>
  bool(true)
  [6]=>
  float(1.222)
  [7]=>
  float(434324)
  [8]=>
  string(8) "fsdfasDF"
  [9]=>
  NULL
  [10]=>
  string(0) ""
  [11]=>
  bool(false)
  [12]=>
  float(-3321)
  [13]=>
  float(-77.3321)
  [14]=>
  string(12) "a a a a a aa"
}
array(15) {
  [0]=>
  bool(true)
  [1]=>
  float(1.222)
  [2]=>
  float(434324)
  [3]=>
  string(8) "fsdfasDF"
  [4]=>
  NULL
  [5]=>
  string(0) ""
  [6]=>
  bool(false)
  [7]=>
  float(-3321)
  [8]=>
  float(-77.3321)
  [9]=>
  string(12) "a a a a a aa"
  [10]=>
  string(0) ""
  [11]=>
  string(0) ""
  [12]=>
  string(0) ""
  [13]=>
  string(0) ""
  [14]=>
  string(0) ""
}
OK