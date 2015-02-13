--TEST--
Column/Row insertion/removal
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

	var_dump($s->insertRow(2,2), $x->getError());
	var_dump($s->insertRow(5,7), $x->getError());

	var_dump($s->insertCol(4,4), $x->getError());
	var_dump($s->insertCol(6,9), $x->getError());

	var_dump($s->removeRow(6,6), $x->getError());
	var_dump($s->removeRow(6,7), $x->getError());

	var_dump($s->removeCol(7,7), $x->getError());
	var_dump($s->removeCol(7,8), $x->getError());


	
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
OK
