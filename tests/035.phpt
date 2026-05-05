--TEST--
Column/Row insertion/removal
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

	var_dump($s->insertRow(3,3), $x->getError());
	var_dump($s->insertRow(6,8), $x->getError());

	var_dump($s->insertCol(4,4), $x->getError());
	var_dump($s->insertCol(6,9), $x->getError());

	var_dump($s->removeRow(7,7), $x->getError());
	var_dump($s->removeRow(7,8), $x->getError());

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
