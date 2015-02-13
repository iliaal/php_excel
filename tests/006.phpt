--TEST--
Numeric Data format tests
--INI--
date.timezone=America/Toronto
--SKIPIF--
<?php if (!extension_loaded("excel")) die("skip - Excel extension not found"); ?>
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

    // bypass LibXL trial limitations
    $storage = array();
    $storage[0]['book'] = $x;
    $storage[0]['sheet'] = $s;
    
	foreach ($oClass->getConstants() as $c => $val) {
		if (!in_array($c, $constants)) {
			continue;
		}
        
        // bypass LibXL trial limitations
        $storageIndex = floor($row / 5);
        if (0 == $row % 5) {
            $x = new ExcelBook();
            $s = $x->addSheet("Sheet 1");
            $storage[$storageIndex]['book'] = $x;
            $storage[$storageIndex]['sheet'] = $s;
        }
        $x = $storage[$storageIndex]['book'];
        $s = $storage[$storageIndex]['sheet'];
	
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
    
        // bypass LibXL trial limitations
        $storageIndex = floor($i / 5);
        $x = $storage[$storageIndex]['book'];
        $s = $storage[$storageIndex]['sheet'];
    
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
