--TEST--
Date format tests
--INI--
date.timezone=America/Toronto
--SKIPIF--
<?php if (!extension_loaded("excel")) die("skip - Excel extension not found"); ?>
--FILE--
<?php 
	$x = new ExcelBook();

	$time = mktime(5,43,11,1,21,1980);
	
	$s = $x->addSheet("Sheet 1");

	$row = 1;

	$oClass = new ReflectionClass('ExcelFormat');

	$date_constants = array(
		"NUMFORMAT_DATE",
		"NUMFORMAT_CUSTOM_D_MON_YY",
		"NUMFORMAT_CUSTOM_D_MON",
		"NUMFORMAT_CUSTOM_MON_YY",
		"NUMFORMAT_CUSTOM_HMM_AM",
		"NUMFORMAT_CUSTOM_HMMSS_AM",
		"NUMFORMAT_CUSTOM_HMM",
		"NUMFORMAT_CUSTOM_HMMSS",
		"NUMFORMAT_CUSTOM_MDYYYY_HMM",
		"NUMFORMAT_CUSTOM_MMSS",
		"NUMFORMAT_CUSTOM_H0MMSS",
		"NUMFORMAT_CUSTOM_MMSS0"
	);

	foreach ($oClass->getConstants() as $c => $val) {
		if (!in_array($c, $date_constants)) {
			continue;
		}
	
		$format = $x->addFormat();
		$format->numberFormat($val);

		var_dump($s->write($row, 0, $c));
		var_dump($x->getError());
		var_dump($s->write($row, 1, $time, $format, ExcelFormat::AS_DATE));
		var_dump($x->getError());
		++$row;
	}

	for($i = 1; $i < $row; $i++) {
		echo $s->read($i, 0) . " >> " . $s->read($i, 1) . "\n";
		var_dump($x->getError());
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
bool(true)
bool(false)
bool(true)
bool(false)
bool(true)
bool(false)
bool(true)
bool(false)
NUMFORMAT_DATE >> 317299391
bool(false)
NUMFORMAT_CUSTOM_D_MON_YY >> 317299391
bool(false)
NUMFORMAT_CUSTOM_D_MON >> 317299391
bool(false)
NUMFORMAT_CUSTOM_MON_YY >> 317299391
bool(false)
NUMFORMAT_CUSTOM_HMM_AM >> 317299391
bool(false)
NUMFORMAT_CUSTOM_HMMSS_AM >> 317299391
bool(false)
NUMFORMAT_CUSTOM_HMM >> 317299391
bool(false)
NUMFORMAT_CUSTOM_HMMSS >> 317299391
bool(false)
NUMFORMAT_CUSTOM_MDYYYY_HMM >> 317299391
bool(false)
NUMFORMAT_CUSTOM_MMSS >> 317299391
bool(false)
NUMFORMAT_CUSTOM_H0MMSS >> 317299391
bool(false)
NUMFORMAT_CUSTOM_MMSS0 >> 317299391
bool(false)
OK
