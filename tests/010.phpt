--TEST--
Text indentation
--INI--
date.timezone=America/Toronto
--EXTENSIONS--
excel
--FILE--
<?php 
	$x = new ExcelBook();

	$s = $x->addSheet("Sheet 1");

	$data = "Test";

	for ($i = 1; $i < 18; $i++) {
		$format = $x->addFormat();
		$format->indent($i - 1);

		var_dump($s->write($i, 0, $data, $format));
		var_dump($x->getError());
	}

	for($i = 1; $i < 18; $i++) {
		$format = '';
		$s->read($i, 0, $format);
		echo (int)$format->indent() . "\n";
	}



	echo "OK\n";
?>
--EXPECTF--
bool(true)
bool(false)
bool(true)
bool(false)
bool(true)
bool(false)
bool(true)
bool(false)
bool(true)
bool(false)
bool(true)
bool(false)
bool(true)
bool(false)
bool(true)
bool(false)
bool(true)
bool(false)
bool(true)
bool(false)
bool(true)
bool(false)
bool(true)
bool(false)
bool(true)
bool(false)
bool(true)
bool(false)
bool(true)
bool(false)
bool(true)
bool(false)

Warning: ExcelFormat::indent(): Text indentation level must be less than or equal to 15 in %s on line %d
bool(true)
bool(false)
0
1
2
3
4
5
6
7
8
9
10
11
12
13
14
15
0
OK
