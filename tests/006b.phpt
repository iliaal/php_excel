--TEST--
Numeric Data format tests (part 2)
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
		"NUMFORMAT_FRACTION_ONEDIG",
		"NUMFORMAT_FRACTION_TWODIG",
		"NUMFORMAT_NUMBER_SEP_NEGBRA",
		"NUMFORMAT_NUMBER_SEP_NEGBRARED",
		"NUMFORMAT_NUMBER_D2_SEP_NEGBRA",
		"NUMFORMAT_NUMBER_D2_SEP_NEGBRARED",
		"NUMFORMAT_ACCOUNT",
		"NUMFORMAT_ACCOUNTCUR",
		"NUMFORMAT_ACCOUNT_D2",
		"NUMFORMAT_ACCOUNT_D2_CUR",
		"NUMFORMAT_CUSTOM_000P0E_PLUS0",
		"NUMFORMAT_TEXT"
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
NUMFORMAT_FRACTION_ONEDIG >> (100 via 12  ) -- (-100 via 12  ) -- (100.99 via 12  ) -- (-100.99 via 12  ) -- (0.99 via 12  ) -- (-0.99 via 12  ) -- (242342343 via 12  ) -- (-242342343 via 12  ) -- 
NUMFORMAT_FRACTION_TWODIG >> (100 via 13  ) -- (-100 via 13  ) -- (100.99 via 13  ) -- (-100.99 via 13  ) -- (0.99 via 13  ) -- (-0.99 via 13  ) -- (242342343 via 13  ) -- (-242342343 via 13  ) -- 
NUMFORMAT_NUMBER_SEP_NEGBRA >> (100 via 37  ) -- (-100 via 37  ) -- (100.99 via 37  ) -- (-100.99 via 37  ) -- (0.99 via 37  ) -- (-0.99 via 37  ) -- (242342343 via 37  ) -- (-242342343 via 37  ) -- 
NUMFORMAT_NUMBER_SEP_NEGBRARED >> (100 via 38  ) -- (-100 via 38  ) -- (100.99 via 38  ) -- (-100.99 via 38  ) -- (0.99 via 38  ) -- (-0.99 via 38  ) -- (242342343 via 38  ) -- (-242342343 via 38  ) -- 
NUMFORMAT_NUMBER_D2_SEP_NEGBRA >> (100 via 39  ) -- (-100 via 39  ) -- (100.99 via 39  ) -- (-100.99 via 39  ) -- (0.99 via 39  ) -- (-0.99 via 39  ) -- (242342343 via 39  ) -- (-242342343 via 39  ) -- 
NUMFORMAT_NUMBER_D2_SEP_NEGBRARED >> (100 via 40  ) -- (-100 via 40  ) -- (100.99 via 40  ) -- (-100.99 via 40  ) -- (0.99 via 40  ) -- (-0.99 via 40  ) -- (242342343 via 40  ) -- (-242342343 via 40  ) -- 
NUMFORMAT_ACCOUNT >> (100 via 41  ) -- (-100 via 41  ) -- (100.99 via 41  ) -- (-100.99 via 41  ) -- (0.99 via 41  ) -- (-0.99 via 41  ) -- (242342343 via 41  ) -- (-242342343 via 41  ) -- 
NUMFORMAT_ACCOUNTCUR >> (100 via 42  ) -- (-100 via 42  ) -- (100.99 via 42  ) -- (-100.99 via 42  ) -- (0.99 via 42  ) -- (-0.99 via 42  ) -- (242342343 via 42  ) -- (-242342343 via 42  ) -- 
NUMFORMAT_ACCOUNT_D2 >> (100 via 43  ) -- (-100 via 43  ) -- (100.99 via 43  ) -- (-100.99 via 43  ) -- (0.99 via 43  ) -- (-0.99 via 43  ) -- (242342343 via 43  ) -- (-242342343 via 43  ) -- 
NUMFORMAT_ACCOUNT_D2_CUR >> (100 via 44  ) -- (-100 via 44  ) -- (100.99 via 44  ) -- (-100.99 via 44  ) -- (0.99 via 44  ) -- (-0.99 via 44  ) -- (242342343 via 44  ) -- (-242342343 via 44  ) -- 
NUMFORMAT_CUSTOM_000P0E_PLUS0 >> (100 via 48  ) -- (-100 via 48  ) -- (100.99 via 48  ) -- (-100.99 via 48  ) -- (0.99 via 48  ) -- (-0.99 via 48  ) -- (242342343 via 48  ) -- (-242342343 via 48  ) -- 
NUMFORMAT_TEXT >> (100 via 49  ) -- (-100 via 49  ) -- (100.99 via 49  ) -- (-100.99 via 49  ) -- (0.99 via 49  ) -- (-0.99 via 49  ) -- (242342343 via 49  ) -- (-242342343 via 49  ) -- 
OK
