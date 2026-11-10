--TEST--
Border Styles
--INI--
date.timezone=America/Toronto
--EXTENSIONS--
excel
--FILE--
<?php 
	$x = new ExcelBook();

	$s = $x->addSheet("Sheet 1");

	$data = "Test";
	$oClass = new ReflectionClass('ExcelFormat');

	$row = 1; $col = 0;

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
			if ($b === '') {
				// The all-side method is write-only; inspect each real getter.
				$values = [];
				foreach (['Left', 'Right', 'Top', 'Bottom'] as $side) {
					$getter = 'border' . $side . 'Style';
					$values[] = $fmt->$getter();
				}
				echo implode(',', $values) . "\n";
			} else {
				echo $fmt->$f() . "\n";
			}

			++$row;
			++$col;			
		}
	}



	echo "OK\n";
?>
--EXPECT--
bool(true)
bool(false)
0,0,0,0
bool(true)
bool(false)
1,1,1,1
bool(true)
bool(false)
2,2,2,2
bool(true)
bool(false)
3,3,3,3
bool(true)
bool(false)
4,4,4,4
bool(true)
bool(false)
5,5,5,5
bool(true)
bool(false)
6,6,6,6
bool(true)
bool(false)
7,7,7,7
bool(true)
bool(false)
8,8,8,8
bool(true)
bool(false)
9,9,9,9
bool(true)
bool(false)
10,10,10,10
bool(true)
bool(false)
11,11,11,11
bool(true)
bool(false)
12,12,12,12
bool(true)
bool(false)
13,13,13,13
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
