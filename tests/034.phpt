--TEST--
Area Clear
--INI--
date.timezone=America/Toronto
--SKIPIF--
<?php if (!extension_loaded("excel")) die("skip - Excel extension not found"); ?>
--FILE--
<?php 
	$x = new ExcelBook();

	$s = $x->addSheet("Sheet 1");

	for ($i = 0; $i < 10; $i++) {
		for ($j = 0; $j < 10; $j++) {
			$s->write($j+1, $i, "{$j} {$i}");
		}
	}

	$s->clear(2,4,3,5);
	var_dump($x->getError());


	
	echo "OK\n";
?>
--EXPECT--
bool(false)
OK
