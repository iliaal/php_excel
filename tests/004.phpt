--TEST--
Complex Type write tests
--INI--
date.timezone=America/Toronto
--SKIPIF--
<?php if (!extension_loaded("excel")) print "skip"; ?>
--FILE--
<?php 
	$x = new ExcelBook();

	$time = mktime(5,0,0,1,21,1980);

	$format = $x->addFormat();
	$format->numberFormat(ExcelFormat::NUMFORMAT_CUSTOM_D_MON_YY);

	$format2 = $x->addFormat();
	$format2->numberFormat(ExcelFormat::NUMFORMAT_CUSTOM_HMMSS);

	$data = array(
					"date" => array($time, ExcelFormat::AS_DATE, $format),
					"formula" => array("SUM(B4:B4)", ExcelFormat::AS_FORMULA),
					"numeric_textA" => array("32321", ExcelFormat::AS_NUMERIC_STRING),
					"numeric_textB" => array("AV32321", ExcelFormat::AS_NUMERIC_STRING),
					"time" => array($time, ExcelFormat::AS_DATE, $format2),
	);

	$s = $x->addSheet("Sheet 1");
	
	$row = 1;
	foreach ($data as $k => $v) {
		var_dump($s->write($row, 0, $k));
		var_dump($x->getError());
		var_dump($s->write($row, 1, $v[0], (isset($v[2]) ? $v[2] : NULL), $v[1]));
		var_dump($x->getError());
		++$row;
	}
	


	echo "OK\n";
?>
--EXPECT--
bool(true)
bool(false)
bool(true)
bool(false)
bool(true)
bool(false)
bool(true)
bool(false)
bool(true)
bool(false)
bool(true)
bool(false)
bool(true)
bool(false)
bool(true)
bool(false)
bool(true)
bool(false)
bool(true)
bool(false)
OK
