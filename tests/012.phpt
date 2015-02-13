--TEST--
Border Styles
--INI--
date.timezone=America/Toronto
--SKIPIF--
<?php if (!extension_loaded("excel")) die("skip - Excel extension not found"); ?>
--FILE--
<?php 
	$x = new ExcelBook();

	$s = $x->addSheet("Sheet 1");

	$data = "Test";
	$oClass = new ReflectionClass('ExcelFormat');

	$row = 1;
    $col = 0;

	foreach (array('', 'Left','Right','Top','Bottom') as $b) {
		foreach ($oClass->getConstants() as $c => $val) {
			if (strpos($c, 'BORDERSTYLE_') !== 0) {
				continue;
			}

			$f = 'border' . $b . 'Style';

			$format = $x->addFormat();
			$format->$f($val);

			var_dump($s->write($row, $col, $data, $format));
			var_dump($x->getError());

			$fmt = null;
			$s->read($row, $col, $fmt);
			echo $fmt->$f() . "\n";

			++$row;
			++$col;			
		}
	}



	echo "OK\n";
?>
--EXPECT--
bool(true)
bool(false)
1
bool(true)
bool(false)
1
bool(true)
bool(false)
1
bool(true)
bool(false)
1
bool(true)
bool(false)
1
bool(true)
bool(false)
1
bool(true)
bool(false)
1
bool(true)
bool(false)
1
bool(true)
bool(false)
1
bool(true)
bool(false)
1
bool(true)
bool(false)
1
bool(true)
bool(false)
1
bool(true)
bool(false)
1
bool(true)
bool(false)
1
bool(true)
bool(false)
0
bool(true)
bool(false)
1
bool(true)
bool(false)
2
bool(true)
bool(false)
3
bool(true)
bool(false)
4
bool(true)
bool(false)
5
bool(true)
bool(false)
6
bool(true)
bool(false)
7
bool(true)
bool(false)
8
bool(true)
bool(false)
9
bool(true)
bool(false)
10
bool(true)
bool(false)
11
bool(true)
bool(false)
12
bool(true)
bool(false)
13
bool(true)
bool(false)
0
bool(true)
bool(false)
1
bool(true)
bool(false)
2
bool(true)
bool(false)
3
bool(true)
bool(false)
4
bool(true)
bool(false)
5
bool(true)
bool(false)
6
bool(true)
bool(false)
7
bool(true)
bool(false)
8
bool(true)
bool(false)
9
bool(true)
bool(false)
10
bool(true)
bool(false)
11
bool(true)
bool(false)
12
bool(true)
bool(false)
13
bool(true)
bool(false)
0
bool(true)
bool(false)
1
bool(true)
bool(false)
2
bool(true)
bool(false)
3
bool(true)
bool(false)
4
bool(true)
bool(false)
5
bool(true)
bool(false)
6
bool(true)
bool(false)
7
bool(true)
bool(false)
8
bool(true)
bool(false)
9
bool(true)
bool(false)
10
bool(true)
bool(false)
11
bool(true)
bool(false)
12
bool(true)
bool(false)
13
bool(true)
bool(false)
0
bool(true)
bool(false)
1
bool(true)
bool(false)
2
bool(true)
bool(false)
3
bool(true)
bool(false)
4
bool(true)
bool(false)
5
bool(true)
bool(false)
6
bool(true)
bool(false)
7
bool(true)
bool(false)
8
bool(true)
bool(false)
9
bool(true)
bool(false)
10
bool(true)
bool(false)
11
bool(true)
bool(false)
12
bool(true)
bool(false)
13
OK
