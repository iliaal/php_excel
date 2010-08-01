--TEST--
Cell Format Detection
--INI--
date.timezone=America/Toronto
--SKIPIF--
<?php if (!extension_loaded("excel")) print "skip"; ?>
--FILE--
<?php 
	$x = new ExcelBook();

	$s = $x->addSheet("Sheet 1");

	$data = array(true, 1.222, 434324, "fsdfasDF", NULL, "");
	foreach ($data as $k => $v) {
		$s->write($k, 1, $v);
		var_dump($x->getError());
	}

	$type = array();
	$oClass = new ReflectionClass('ExcelSheet');
	foreach ($oClass->getConstants() as $c => $val) {
		if (strpos($c, 'CELLTYPE_') !== 0) {
			continue;
		}
		$type[$val] = $c;
	}

	for ($i = 0, $c = count($data); $i < $c; $i++) {
		var_dump($type[$s->cellType($i, 1)]);
	}

	$format = $x->addFormat();
	$format->numberFormat(ExcelFormat::NUMFORMAT_CUSTOM_D_MON_YY);

	$format2 = $x->addFormat();
	$format2->numberFormat(ExcelFormat::NUMFORMAT_CUSTOM_HMMSS);

	$time = time();
	$data = array(
					array($time, ExcelFormat::AS_DATE, $format),
					array("SUM(B4:B4)", ExcelFormat::AS_FORMULA),
					array($time, ExcelFormat::AS_DATE, $format2),
	);

	foreach ($data as $k => $v) {
		var_dump($s->write(4, $k, $v[0], (isset($v[2]) ? $v[2] : NULL), $v[1]));
		var_dump($x->getError());
	}
	for ($i = 0, $c = count($data); $i < $c; $i++) {
		var_dump($s->isFormula(4, $i));
		var_dump($s->isDate(4, $i));
	}

	echo "OK\n";
?>
--EXPECT--
bool(false)
bool(false)
bool(false)
bool(false)
bool(false)
bool(false)
string(16) "CELLTYPE_BOOLEAN"
string(15) "CELLTYPE_NUMBER"
string(15) "CELLTYPE_NUMBER"
string(15) "CELLTYPE_STRING"
string(14) "CELLTYPE_BLANK"
string(15) "CELLTYPE_STRING"
bool(true)
bool(false)
bool(true)
bool(false)
bool(true)
bool(false)
bool(false)
bool(true)
bool(true)
bool(false)
bool(false)
bool(true)
OK
