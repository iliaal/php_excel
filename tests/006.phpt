--TEST--
Numeric Data format tests (part 1)
--INI--
date.timezone=America/Toronto
--EXTENSIONS--
excel
--FILE--
<?php
	$x = new ExcelBook();

	$s = $x->addSheet("Sheet 1");

	$row = 1;

	$oClass = new ReflectionClass('ExcelFormat');

	$constants = array(
		"NUMFORMAT_GENERAL",
		"NUMFORMAT_NUMBER",
		"NUMFORMAT_NUMBER_D2",
		"NUMFORMAT_NUMBER_SEP",
		"NUMFORMAT_NUMBER_SEP_D2",
		"NUMFORMAT_CURRENCY_NEGBRA",
		"NUMFORMAT_CURRENCY_NEGBRARED",
		"NUMFORMAT_CURRENCY_D2_NEGBRA",
		"NUMFORMAT_CURRENCY_D2_NEGBRARED",
		"NUMFORMAT_PERCENT",
		"NUMFORMAT_PERCENT_D2",
		"NUMFORMAT_SCIENTIFIC_D2",
	);

	$numbers = array(100, -100, 100.99, -100.99, 0.99, -0.99, 242342343, -242342343);

	foreach ($oClass->getConstants() as $c => $val) {
		if (!in_array($c, $constants)) {
			continue;
		}

		var_dump($s->write($row, 0, $c));
		var_dump($x->getError());

		foreach ($numbers as $k => $v) {
			$format = $x->addFormat();
			$format->numberFormat($val);
			var_dump($s->write($row, ($k + 1), $v, $format));
			var_dump($x->getError());
		}
		++$row;
	}

	for($i = 1; $i < $row; $i++) {
		echo $s->read($i, 0) . " >> ";
		foreach ($numbers as $k => $v) {
			$format = '';
			echo "(" . $s->read($i, ($k + 1), $format) . " via " . $format->numberFormat() . " " . $x->getError() . " ) -- ";
		}
		echo "\n";
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
NUMFORMAT_GENERAL >> (100 via 0  ) -- (-100 via 0  ) -- (100.99 via 0  ) -- (-100.99 via 0  ) -- (0.99 via 0  ) -- (-0.99 via 0  ) -- (242342343 via 0  ) -- (-242342343 via 0  ) -- 
NUMFORMAT_NUMBER >> (100 via 1  ) -- (-100 via 1  ) -- (100.99 via 1  ) -- (-100.99 via 1  ) -- (0.99 via 1  ) -- (-0.99 via 1  ) -- (242342343 via 1  ) -- (-242342343 via 1  ) -- 
NUMFORMAT_NUMBER_D2 >> (100 via 2  ) -- (-100 via 2  ) -- (100.99 via 2  ) -- (-100.99 via 2  ) -- (0.99 via 2  ) -- (-0.99 via 2  ) -- (242342343 via 2  ) -- (-242342343 via 2  ) -- 
NUMFORMAT_NUMBER_SEP >> (100 via 3  ) -- (-100 via 3  ) -- (100.99 via 3  ) -- (-100.99 via 3  ) -- (0.99 via 3  ) -- (-0.99 via 3  ) -- (242342343 via 3  ) -- (-242342343 via 3  ) -- 
NUMFORMAT_NUMBER_SEP_D2 >> (100 via 4  ) -- (-100 via 4  ) -- (100.99 via 4  ) -- (-100.99 via 4  ) -- (0.99 via 4  ) -- (-0.99 via 4  ) -- (242342343 via 4  ) -- (-242342343 via 4  ) -- 
NUMFORMAT_CURRENCY_NEGBRA >> (100 via 5  ) -- (-100 via 5  ) -- (100.99 via 5  ) -- (-100.99 via 5  ) -- (0.99 via 5  ) -- (-0.99 via 5  ) -- (242342343 via 5  ) -- (-242342343 via 5  ) -- 
NUMFORMAT_CURRENCY_NEGBRARED >> (100 via 6  ) -- (-100 via 6  ) -- (100.99 via 6  ) -- (-100.99 via 6  ) -- (0.99 via 6  ) -- (-0.99 via 6  ) -- (242342343 via 6  ) -- (-242342343 via 6  ) -- 
NUMFORMAT_CURRENCY_D2_NEGBRA >> (100 via 7  ) -- (-100 via 7  ) -- (100.99 via 7  ) -- (-100.99 via 7  ) -- (0.99 via 7  ) -- (-0.99 via 7  ) -- (242342343 via 7  ) -- (-242342343 via 7  ) -- 
NUMFORMAT_CURRENCY_D2_NEGBRARED >> (100 via 8  ) -- (-100 via 8  ) -- (100.99 via 8  ) -- (-100.99 via 8  ) -- (0.99 via 8  ) -- (-0.99 via 8  ) -- (242342343 via 8  ) -- (-242342343 via 8  ) -- 
NUMFORMAT_PERCENT >> (100 via 9  ) -- (-100 via 9  ) -- (100.99 via 9  ) -- (-100.99 via 9  ) -- (0.99 via 9  ) -- (-0.99 via 9  ) -- (242342343 via 9  ) -- (-242342343 via 9  ) -- 
NUMFORMAT_PERCENT_D2 >> (100 via 10  ) -- (-100 via 10  ) -- (100.99 via 10  ) -- (-100.99 via 10  ) -- (0.99 via 10  ) -- (-0.99 via 10  ) -- (242342343 via 10  ) -- (-242342343 via 10  ) -- 
NUMFORMAT_SCIENTIFIC_D2 >> (100 via 11  ) -- (-100 via 11  ) -- (100.99 via 11  ) -- (-100.99 via 11  ) -- (0.99 via 11  ) -- (-0.99 via 11  ) -- (242342343 via 11  ) -- (-242342343 via 11  ) -- 
OK
