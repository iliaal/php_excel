--TEST--
Area Clear
--INI--
date.timezone=America/Toronto
--EXTENSIONS--
excel
--FILE--
<?php 
	$x = new ExcelBook();

	$s = $x->addSheet("Sheet 1");

	for ($i = 0; $i < 10; $i++) {
		for ($j = 1; $j < 11; $j++) {
			$s->write($j, $i, "{$j} {$i}");
		}
	}

	$s->clear(3,4,4,5);
	var_dump($x->getError());


	
	echo "OK\n";
?>
--EXPECT--
bool(false)
OK
