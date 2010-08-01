--TEST--
Fill Pattern Test
--INI--
date.timezone=America/Toronto
--SKIPIF--
<?php if (!extension_loaded("excel")) print "skip"; ?>
--FILE--
<?php 
	$x = new ExcelBook();

	$s = $x->addSheet("Sheet 1");

	$data = "Test";
	$oClass = new ReflectionClass('ExcelFormat');

	$row = $col = 0;

	foreach ($oClass->getConstants() as $c => $color) {
		if (strpos($c, 'COLOR_') !== 0) {
			continue;
		}

		foreach ($oClass->getConstants() as $c => $val) {
			if (strpos($c, 'FILLPATTERN_') !== 0) {
				continue;
			}
			
			$format = $x->addFormat();
			$format->fillPattern($val);
			$format->patternBackgroundColor($color);

			var_dump($s->write($row, 1, $data, $format));
			var_dump($x->getError());

			$fmt = null;
			$s->read($row, 1, $fmt);
			echo $fmt->fillPattern() . " - " . $format->patternBackgroundColor() . "\n";

			$format = $x->addFormat();
			$format->fillPattern($val);
			$format->patternForegroundColor($color);

			var_dump($s->write($row, 2, $data, $format));
			var_dump($x->getError());

			$fmt = null;
			$s->read($row, 2, $fmt);
			echo $fmt->fillPattern() . " - " . $format->patternForegroundColor() . "\n";

			$row++;	
		}
	}



	echo "OK\n";
?>
--EXPECT--
bool(true)
bool(false)
0 - 8
bool(true)
bool(false)
0 - 8
bool(true)
bool(false)
1 - 8
bool(true)
bool(false)
1 - 8
bool(true)
bool(false)
2 - 8
bool(true)
bool(false)
2 - 8
bool(true)
bool(false)
3 - 8
bool(true)
bool(false)
3 - 8
bool(true)
bool(false)
4 - 8
bool(true)
bool(false)
4 - 8
bool(true)
bool(false)
5 - 8
bool(true)
bool(false)
5 - 8
bool(true)
bool(false)
6 - 8
bool(true)
bool(false)
6 - 8
bool(true)
bool(false)
7 - 8
bool(true)
bool(false)
7 - 8
bool(true)
bool(false)
8 - 8
bool(true)
bool(false)
8 - 8
bool(true)
bool(false)
9 - 8
bool(true)
bool(false)
9 - 8
bool(true)
bool(false)
10 - 8
bool(true)
bool(false)
10 - 8
bool(true)
bool(false)
11 - 8
bool(true)
bool(false)
11 - 8
bool(true)
bool(false)
12 - 8
bool(true)
bool(false)
12 - 8
bool(true)
bool(false)
13 - 8
bool(true)
bool(false)
13 - 8
bool(true)
bool(false)
14 - 8
bool(true)
bool(false)
14 - 8
bool(true)
bool(false)
15 - 8
bool(true)
bool(false)
15 - 8
bool(true)
bool(false)
16 - 8
bool(true)
bool(false)
16 - 8
bool(true)
bool(false)
17 - 8
bool(true)
bool(false)
17 - 8
bool(true)
bool(false)
18 - 8
bool(true)
bool(false)
18 - 8
bool(true)
bool(false)
0 - 9
bool(true)
bool(false)
0 - 9
bool(true)
bool(false)
1 - 9
bool(true)
bool(false)
1 - 9
bool(true)
bool(false)
2 - 9
bool(true)
bool(false)
2 - 9
bool(true)
bool(false)
3 - 9
bool(true)
bool(false)
3 - 9
bool(true)
bool(false)
4 - 9
bool(true)
bool(false)
4 - 9
bool(true)
bool(false)
5 - 9
bool(true)
bool(false)
5 - 9
bool(true)
bool(false)
6 - 9
bool(true)
bool(false)
6 - 9
bool(true)
bool(false)
7 - 9
bool(true)
bool(false)
7 - 9
bool(true)
bool(false)
8 - 9
bool(true)
bool(false)
8 - 9
bool(true)
bool(false)
9 - 9
bool(true)
bool(false)
9 - 9
bool(true)
bool(false)
10 - 9
bool(true)
bool(false)
10 - 9
bool(true)
bool(false)
11 - 9
bool(true)
bool(false)
11 - 9
bool(true)
bool(false)
12 - 9
bool(true)
bool(false)
12 - 9
bool(true)
bool(false)
13 - 9
bool(true)
bool(false)
13 - 9
bool(true)
bool(false)
14 - 9
bool(true)
bool(false)
14 - 9
bool(true)
bool(false)
15 - 9
bool(true)
bool(false)
15 - 9
bool(true)
bool(false)
16 - 9
bool(true)
bool(false)
16 - 9
bool(true)
bool(false)
17 - 9
bool(true)
bool(false)
17 - 9
bool(true)
bool(false)
18 - 9
bool(true)
bool(false)
18 - 9
bool(true)
bool(false)
0 - 10
bool(true)
bool(false)
0 - 10
bool(true)
bool(false)
1 - 10
bool(true)
bool(false)
1 - 10
bool(true)
bool(false)
2 - 10
bool(true)
bool(false)
2 - 10
bool(true)
bool(false)
3 - 10
bool(true)
bool(false)
3 - 10
bool(true)
bool(false)
4 - 10
bool(true)
bool(false)
4 - 10
bool(true)
bool(false)
5 - 10
bool(true)
bool(false)
5 - 10
bool(true)
bool(false)
6 - 10
bool(true)
bool(false)
6 - 10
bool(true)
bool(false)
7 - 10
bool(true)
bool(false)
7 - 10
bool(true)
bool(false)
8 - 10
bool(true)
bool(false)
8 - 10
bool(true)
bool(false)
9 - 10
bool(true)
bool(false)
9 - 10
bool(true)
bool(false)
10 - 10
bool(true)
bool(false)
10 - 10
bool(true)
bool(false)
11 - 10
bool(true)
bool(false)
11 - 10
bool(true)
bool(false)
12 - 10
bool(true)
bool(false)
12 - 10
bool(true)
bool(false)
13 - 10
bool(true)
bool(false)
13 - 10
bool(true)
bool(false)
14 - 10
bool(true)
bool(false)
14 - 10
bool(true)
bool(false)
15 - 10
bool(true)
bool(false)
15 - 10
bool(true)
bool(false)
16 - 10
bool(true)
bool(false)
16 - 10
bool(true)
bool(false)
17 - 10
bool(true)
bool(false)
17 - 10
bool(true)
bool(false)
18 - 10
bool(true)
bool(false)
18 - 10
bool(true)
bool(false)
0 - 11
bool(true)
bool(false)
0 - 11
bool(true)
bool(false)
1 - 11
bool(true)
bool(false)
1 - 11
bool(true)
bool(false)
2 - 11
bool(true)
bool(false)
2 - 11
bool(true)
bool(false)
3 - 11
bool(true)
bool(false)
3 - 11
bool(true)
bool(false)
4 - 11
bool(true)
bool(false)
4 - 11
bool(true)
bool(false)
5 - 11
bool(true)
bool(false)
5 - 11
bool(true)
bool(false)
6 - 11
bool(true)
bool(false)
6 - 11
bool(true)
bool(false)
7 - 11
bool(true)
bool(false)
7 - 11
bool(true)
bool(false)
8 - 11
bool(true)
bool(false)
8 - 11
bool(true)
bool(false)
9 - 11
bool(true)
bool(false)
9 - 11
bool(true)
bool(false)
10 - 11
bool(true)
bool(false)
10 - 11
bool(true)
bool(false)
11 - 11
bool(true)
bool(false)
11 - 11
bool(true)
bool(false)
12 - 11
bool(true)
bool(false)
12 - 11
bool(true)
bool(false)
13 - 11
bool(true)
bool(false)
13 - 11
bool(true)
bool(false)
14 - 11
bool(true)
bool(false)
14 - 11
bool(true)
bool(false)
15 - 11
bool(true)
bool(false)
15 - 11
bool(true)
bool(false)
16 - 11
bool(true)
bool(false)
16 - 11
bool(true)
bool(false)
17 - 11
bool(true)
bool(false)
17 - 11
bool(true)
bool(false)
18 - 11
bool(true)
bool(false)
18 - 11
bool(true)
bool(false)
0 - 12
bool(true)
bool(false)
0 - 12
bool(true)
bool(false)
1 - 12
bool(true)
bool(false)
1 - 12
bool(true)
bool(false)
2 - 12
bool(true)
bool(false)
2 - 12
bool(true)
bool(false)
3 - 12
bool(true)
bool(false)
3 - 12
bool(true)
bool(false)
4 - 12
bool(true)
bool(false)
4 - 12
bool(true)
bool(false)
5 - 12
bool(true)
bool(false)
5 - 12
bool(true)
bool(false)
6 - 12
bool(true)
bool(false)
6 - 12
bool(true)
bool(false)
7 - 12
bool(true)
bool(false)
7 - 12
bool(true)
bool(false)
8 - 12
bool(true)
bool(false)
8 - 12
bool(true)
bool(false)
9 - 12
bool(true)
bool(false)
9 - 12
bool(true)
bool(false)
10 - 12
bool(true)
bool(false)
10 - 12
bool(true)
bool(false)
11 - 12
bool(true)
bool(false)
11 - 12
bool(true)
bool(false)
12 - 12
bool(true)
bool(false)
12 - 12
bool(true)
bool(false)
13 - 12
bool(true)
bool(false)
13 - 12
bool(true)
bool(false)
14 - 12
bool(true)
bool(false)
14 - 12
bool(true)
bool(false)
15 - 12
bool(true)
bool(false)
15 - 12
bool(true)
bool(false)
16 - 12
bool(true)
bool(false)
16 - 12
bool(true)
bool(false)
17 - 12
bool(true)
bool(false)
17 - 12
bool(true)
bool(false)
18 - 12
bool(true)
bool(false)
18 - 12
bool(true)
bool(false)
0 - 13
bool(true)
bool(false)
0 - 13
bool(true)
bool(false)
1 - 13
bool(true)
bool(false)
1 - 13
bool(true)
bool(false)
2 - 13
bool(true)
bool(false)
2 - 13
bool(true)
bool(false)
3 - 13
bool(true)
bool(false)
3 - 13
bool(true)
bool(false)
4 - 13
bool(true)
bool(false)
4 - 13
bool(true)
bool(false)
5 - 13
bool(true)
bool(false)
5 - 13
bool(true)
bool(false)
6 - 13
bool(true)
bool(false)
6 - 13
bool(true)
bool(false)
7 - 13
bool(true)
bool(false)
7 - 13
bool(true)
bool(false)
8 - 13
bool(true)
bool(false)
8 - 13
bool(true)
bool(false)
9 - 13
bool(true)
bool(false)
9 - 13
bool(true)
bool(false)
10 - 13
bool(true)
bool(false)
10 - 13
bool(true)
bool(false)
11 - 13
bool(true)
bool(false)
11 - 13
bool(true)
bool(false)
12 - 13
bool(true)
bool(false)
12 - 13
bool(true)
bool(false)
13 - 13
bool(true)
bool(false)
13 - 13
bool(true)
bool(false)
14 - 13
bool(true)
bool(false)
14 - 13
bool(true)
bool(false)
15 - 13
bool(true)
bool(false)
15 - 13
bool(true)
bool(false)
16 - 13
bool(true)
bool(false)
16 - 13
bool(true)
bool(false)
17 - 13
bool(true)
bool(false)
17 - 13
bool(true)
bool(false)
18 - 13
bool(true)
bool(false)
18 - 13
bool(true)
bool(false)
0 - 14
bool(true)
bool(false)
0 - 14
bool(true)
bool(false)
1 - 14
bool(true)
bool(false)
1 - 14
bool(true)
bool(false)
2 - 14
bool(true)
bool(false)
2 - 14
bool(true)
bool(false)
3 - 14
bool(true)
bool(false)
3 - 14
bool(true)
bool(false)
4 - 14
bool(true)
bool(false)
4 - 14
bool(true)
bool(false)
5 - 14
bool(true)
bool(false)
5 - 14
bool(true)
bool(false)
6 - 14
bool(true)
bool(false)
6 - 14
bool(true)
bool(false)
7 - 14
bool(true)
bool(false)
7 - 14
bool(true)
bool(false)
8 - 14
bool(true)
bool(false)
8 - 14
bool(true)
bool(false)
9 - 14
bool(true)
bool(false)
9 - 14
bool(true)
bool(false)
10 - 14
bool(true)
bool(false)
10 - 14
bool(true)
bool(false)
11 - 14
bool(true)
bool(false)
11 - 14
bool(true)
bool(false)
12 - 14
bool(true)
bool(false)
12 - 14
bool(true)
bool(false)
13 - 14
bool(true)
bool(false)
13 - 14
bool(true)
bool(false)
14 - 14
bool(true)
bool(false)
14 - 14
bool(true)
bool(false)
15 - 14
bool(true)
bool(false)
15 - 14
bool(true)
bool(false)
16 - 14
bool(true)
bool(false)
16 - 14
bool(true)
bool(false)
17 - 14
bool(true)
bool(false)
17 - 14
bool(true)
bool(false)
18 - 14
bool(true)
bool(false)
18 - 14
bool(true)
bool(false)
0 - 15
bool(true)
bool(false)
0 - 15
bool(true)
bool(false)
1 - 15
bool(true)
bool(false)
1 - 15
bool(true)
bool(false)
2 - 15
bool(true)
bool(false)
2 - 15
bool(true)
bool(false)
3 - 15
bool(true)
bool(false)
3 - 15
bool(true)
bool(false)
4 - 15
bool(true)
bool(false)
4 - 15
bool(true)
bool(false)
5 - 15
bool(true)
bool(false)
5 - 15
bool(true)
bool(false)
6 - 15
bool(true)
bool(false)
6 - 15
bool(true)
bool(false)
7 - 15
bool(true)
bool(false)
7 - 15
bool(true)
bool(false)
8 - 15
bool(true)
bool(false)
8 - 15
bool(true)
bool(false)
9 - 15
bool(true)
bool(false)
9 - 15
bool(true)
bool(false)
10 - 15
bool(true)
bool(false)
10 - 15
bool(true)
bool(false)
11 - 15
bool(true)
bool(false)
11 - 15
bool(true)
bool(false)
12 - 15
bool(true)
bool(false)
12 - 15
bool(true)
bool(false)
13 - 15
bool(true)
bool(false)
13 - 15
bool(true)
bool(false)
14 - 15
bool(true)
bool(false)
14 - 15
bool(true)
bool(false)
15 - 15
bool(true)
bool(false)
15 - 15
bool(true)
bool(false)
16 - 15
bool(true)
bool(false)
16 - 15
bool(true)
bool(false)
17 - 15
bool(true)
bool(false)
17 - 15
bool(true)
bool(false)
18 - 15
bool(true)
bool(false)
18 - 15
bool(true)
bool(false)
0 - 16
bool(true)
bool(false)
0 - 16
bool(true)
bool(false)
1 - 16
bool(true)
bool(false)
1 - 16
bool(true)
bool(false)
2 - 16
bool(true)
bool(false)
2 - 16
bool(true)
bool(false)
3 - 16
bool(true)
bool(false)
3 - 16
bool(true)
bool(false)
4 - 16
bool(true)
bool(false)
4 - 16
bool(true)
bool(false)
5 - 16
bool(true)
bool(false)
5 - 16
bool(true)
bool(false)
6 - 16
bool(true)
bool(false)
6 - 16
bool(true)
bool(false)
7 - 16
bool(true)
bool(false)
7 - 16
bool(true)
bool(false)
8 - 16
bool(true)
bool(false)
8 - 16
bool(true)
bool(false)
9 - 16
bool(true)
bool(false)
9 - 16
bool(true)
bool(false)
10 - 16
bool(true)
bool(false)
10 - 16
bool(true)
bool(false)
11 - 16
bool(true)
bool(false)
11 - 16
bool(true)
bool(false)
12 - 16
bool(true)
bool(false)
12 - 16
bool(true)
bool(false)
13 - 16
bool(true)
bool(false)
13 - 16
bool(true)
bool(false)
14 - 16
bool(true)
bool(false)
14 - 16
bool(true)
bool(false)
15 - 16
bool(true)
bool(false)
15 - 16
bool(true)
bool(false)
16 - 16
bool(true)
bool(false)
16 - 16
bool(true)
bool(false)
17 - 16
bool(true)
bool(false)
17 - 16
bool(true)
bool(false)
18 - 16
bool(true)
bool(false)
18 - 16
bool(true)
bool(false)
0 - 17
bool(true)
bool(false)
0 - 17
bool(true)
bool(false)
1 - 17
bool(true)
bool(false)
1 - 17
bool(true)
bool(false)
2 - 17
bool(true)
bool(false)
2 - 17
bool(true)
bool(false)
3 - 17
bool(true)
bool(false)
3 - 17
bool(true)
bool(false)
4 - 17
bool(true)
bool(false)
4 - 17
bool(true)
bool(false)
5 - 17
bool(true)
bool(false)
5 - 17
bool(true)
bool(false)
6 - 17
bool(true)
bool(false)
6 - 17
bool(true)
bool(false)
7 - 17
bool(true)
bool(false)
7 - 17
bool(true)
bool(false)
8 - 17
bool(true)
bool(false)
8 - 17
bool(true)
bool(false)
9 - 17
bool(true)
bool(false)
9 - 17
bool(true)
bool(false)
10 - 17
bool(true)
bool(false)
10 - 17
bool(true)
bool(false)
11 - 17
bool(true)
bool(false)
11 - 17
bool(true)
bool(false)
12 - 17
bool(true)
bool(false)
12 - 17
bool(true)
bool(false)
13 - 17
bool(true)
bool(false)
13 - 17
bool(true)
bool(false)
14 - 17
bool(true)
bool(false)
14 - 17
bool(true)
bool(false)
15 - 17
bool(true)
bool(false)
15 - 17
bool(true)
bool(false)
16 - 17
bool(true)
bool(false)
16 - 17
bool(true)
bool(false)
17 - 17
bool(true)
bool(false)
17 - 17
bool(true)
bool(false)
18 - 17
bool(true)
bool(false)
18 - 17
bool(true)
bool(false)
0 - 18
bool(true)
bool(false)
0 - 18
bool(true)
bool(false)
1 - 18
bool(true)
bool(false)
1 - 18
bool(true)
bool(false)
2 - 18
bool(true)
bool(false)
2 - 18
bool(true)
bool(false)
3 - 18
bool(true)
bool(false)
3 - 18
bool(true)
bool(false)
4 - 18
bool(true)
bool(false)
4 - 18
bool(true)
bool(false)
5 - 18
bool(true)
bool(false)
5 - 18
bool(true)
bool(false)
6 - 18
bool(true)
bool(false)
6 - 18
bool(true)
bool(false)
7 - 18
bool(true)
bool(false)
7 - 18
bool(true)
bool(false)
8 - 18
bool(true)
bool(false)
8 - 18
bool(true)
bool(false)
9 - 18
bool(true)
bool(false)
9 - 18
bool(true)
bool(false)
10 - 18
bool(true)
bool(false)
10 - 18
bool(true)
bool(false)
11 - 18
bool(true)
bool(false)
11 - 18
bool(true)
bool(false)
12 - 18
bool(true)
bool(false)
12 - 18
bool(true)
bool(false)
13 - 18
bool(true)
bool(false)
13 - 18
bool(true)
bool(false)
14 - 18
bool(true)
bool(false)
14 - 18
bool(true)
bool(false)
15 - 18
bool(true)
bool(false)
15 - 18
bool(true)
bool(false)
16 - 18
bool(true)
bool(false)
16 - 18
bool(true)
bool(false)
17 - 18
bool(true)
bool(false)
17 - 18
bool(true)
bool(false)
18 - 18
bool(true)
bool(false)
18 - 18
bool(true)
bool(false)
0 - 19
bool(true)
bool(false)
0 - 19
bool(true)
bool(false)
1 - 19
bool(true)
bool(false)
1 - 19
bool(true)
bool(false)
2 - 19
bool(true)
bool(false)
2 - 19
bool(true)
bool(false)
3 - 19
bool(true)
bool(false)
3 - 19
bool(true)
bool(false)
4 - 19
bool(true)
bool(false)
4 - 19
bool(true)
bool(false)
5 - 19
bool(true)
bool(false)
5 - 19
bool(true)
bool(false)
6 - 19
bool(true)
bool(false)
6 - 19
bool(true)
bool(false)
7 - 19
bool(true)
bool(false)
7 - 19
bool(true)
bool(false)
8 - 19
bool(true)
bool(false)
8 - 19
bool(true)
bool(false)
9 - 19
bool(true)
bool(false)
9 - 19
bool(true)
bool(false)
10 - 19
bool(true)
bool(false)
10 - 19
bool(true)
bool(false)
11 - 19
bool(true)
bool(false)
11 - 19
bool(true)
bool(false)
12 - 19
bool(true)
bool(false)
12 - 19
bool(true)
bool(false)
13 - 19
bool(true)
bool(false)
13 - 19
bool(true)
bool(false)
14 - 19
bool(true)
bool(false)
14 - 19
bool(true)
bool(false)
15 - 19
bool(true)
bool(false)
15 - 19
bool(true)
bool(false)
16 - 19
bool(true)
bool(false)
16 - 19
bool(true)
bool(false)
17 - 19
bool(true)
bool(false)
17 - 19
bool(true)
bool(false)
18 - 19
bool(true)
bool(false)
18 - 19
bool(true)
bool(false)
0 - 20
bool(true)
bool(false)
0 - 20
bool(true)
bool(false)
1 - 20
bool(true)
bool(false)
1 - 20
bool(true)
bool(false)
2 - 20
bool(true)
bool(false)
2 - 20
bool(true)
bool(false)
3 - 20
bool(true)
bool(false)
3 - 20
bool(true)
bool(false)
4 - 20
bool(true)
bool(false)
4 - 20
bool(true)
bool(false)
5 - 20
bool(true)
bool(false)
5 - 20
bool(true)
bool(false)
6 - 20
bool(true)
bool(false)
6 - 20
bool(true)
bool(false)
7 - 20
bool(true)
bool(false)
7 - 20
bool(true)
bool(false)
8 - 20
bool(true)
bool(false)
8 - 20
bool(true)
bool(false)
9 - 20
bool(true)
bool(false)
9 - 20
bool(true)
bool(false)
10 - 20
bool(true)
bool(false)
10 - 20
bool(true)
bool(false)
11 - 20
bool(true)
bool(false)
11 - 20
bool(true)
bool(false)
12 - 20
bool(true)
bool(false)
12 - 20
bool(true)
bool(false)
13 - 20
bool(true)
bool(false)
13 - 20
bool(true)
bool(false)
14 - 20
bool(true)
bool(false)
14 - 20
bool(true)
bool(false)
15 - 20
bool(true)
bool(false)
15 - 20
bool(true)
bool(false)
16 - 20
bool(true)
bool(false)
16 - 20
bool(true)
bool(false)
17 - 20
bool(true)
bool(false)
17 - 20
bool(true)
bool(false)
18 - 20
bool(true)
bool(false)
18 - 20
bool(true)
bool(false)
0 - 21
bool(true)
bool(false)
0 - 21
bool(true)
bool(false)
1 - 21
bool(true)
bool(false)
1 - 21
bool(true)
bool(false)
2 - 21
bool(true)
bool(false)
2 - 21
bool(true)
bool(false)
3 - 21
bool(true)
bool(false)
3 - 21
bool(true)
bool(false)
4 - 21
bool(true)
bool(false)
4 - 21
bool(true)
bool(false)
5 - 21
bool(true)
bool(false)
5 - 21
bool(true)
bool(false)
6 - 21
bool(true)
bool(false)
6 - 21
bool(true)
bool(false)
7 - 21
bool(true)
bool(false)
7 - 21
bool(true)
bool(false)
8 - 21
bool(true)
bool(false)
8 - 21
bool(true)
bool(false)
9 - 21
bool(true)
bool(false)
9 - 21
bool(true)
bool(false)
10 - 21
bool(true)
bool(false)
10 - 21
bool(true)
bool(false)
11 - 21
bool(true)
bool(false)
11 - 21
bool(true)
bool(false)
12 - 21
bool(true)
bool(false)
12 - 21
bool(true)
bool(false)
13 - 21
bool(true)
bool(false)
13 - 21
bool(true)
bool(false)
14 - 21
bool(true)
bool(false)
14 - 21
bool(true)
bool(false)
15 - 21
bool(true)
bool(false)
15 - 21
bool(true)
bool(false)
16 - 21
bool(true)
bool(false)
16 - 21
bool(true)
bool(false)
17 - 21
bool(true)
bool(false)
17 - 21
bool(true)
bool(false)
18 - 21
bool(true)
bool(false)
18 - 21
bool(true)
bool(false)
0 - 22
bool(true)
bool(false)
0 - 22
bool(true)
bool(false)
1 - 22
bool(true)
bool(false)
1 - 22
bool(true)
bool(false)
2 - 22
bool(true)
bool(false)
2 - 22
bool(true)
bool(false)
3 - 22
bool(true)
bool(false)
3 - 22
bool(true)
bool(false)
4 - 22
bool(true)
bool(false)
4 - 22
bool(true)
bool(false)
5 - 22
bool(true)
bool(false)
5 - 22
bool(true)
bool(false)
6 - 22
bool(true)
bool(false)
6 - 22
bool(true)
bool(false)
7 - 22
bool(true)
bool(false)
7 - 22
bool(true)
bool(false)
8 - 22
bool(true)
bool(false)
8 - 22
bool(true)
bool(false)
9 - 22
bool(true)
bool(false)
9 - 22
bool(true)
bool(false)
10 - 22
bool(true)
bool(false)
10 - 22
bool(true)
bool(false)
11 - 22
bool(true)
bool(false)
11 - 22
bool(true)
bool(false)
12 - 22
bool(true)
bool(false)
12 - 22
bool(true)
bool(false)
13 - 22
bool(true)
bool(false)
13 - 22
bool(true)
bool(false)
14 - 22
bool(true)
bool(false)
14 - 22
bool(true)
bool(false)
15 - 22
bool(true)
bool(false)
15 - 22
bool(true)
bool(false)
16 - 22
bool(true)
bool(false)
16 - 22
bool(true)
bool(false)
17 - 22
bool(true)
bool(false)
17 - 22
bool(true)
bool(false)
18 - 22
bool(true)
bool(false)
18 - 22
bool(true)
bool(false)
0 - 23
bool(true)
bool(false)
0 - 23
bool(true)
bool(false)
1 - 23
bool(true)
bool(false)
1 - 23
bool(true)
bool(false)
2 - 23
bool(true)
bool(false)
2 - 23
bool(true)
bool(false)
3 - 23
bool(true)
bool(false)
3 - 23
bool(true)
bool(false)
4 - 23
bool(true)
bool(false)
4 - 23
bool(true)
bool(false)
5 - 23
bool(true)
bool(false)
5 - 23
bool(true)
bool(false)
6 - 23
bool(true)
bool(false)
6 - 23
bool(true)
bool(false)
7 - 23
bool(true)
bool(false)
7 - 23
bool(true)
bool(false)
8 - 23
bool(true)
bool(false)
8 - 23
bool(true)
bool(false)
9 - 23
bool(true)
bool(false)
9 - 23
bool(true)
bool(false)
10 - 23
bool(true)
bool(false)
10 - 23
bool(true)
bool(false)
11 - 23
bool(true)
bool(false)
11 - 23
bool(true)
bool(false)
12 - 23
bool(true)
bool(false)
12 - 23
bool(true)
bool(false)
13 - 23
bool(true)
bool(false)
13 - 23
bool(true)
bool(false)
14 - 23
bool(true)
bool(false)
14 - 23
bool(true)
bool(false)
15 - 23
bool(true)
bool(false)
15 - 23
bool(true)
bool(false)
16 - 23
bool(true)
bool(false)
16 - 23
bool(true)
bool(false)
17 - 23
bool(true)
bool(false)
17 - 23
bool(true)
bool(false)
18 - 23
bool(true)
bool(false)
18 - 23
bool(true)
bool(false)
0 - 24
bool(true)
bool(false)
0 - 24
bool(true)
bool(false)
1 - 24
bool(true)
bool(false)
1 - 24
bool(true)
bool(false)
2 - 24
bool(true)
bool(false)
2 - 24
bool(true)
bool(false)
3 - 24
bool(true)
bool(false)
3 - 24
bool(true)
bool(false)
4 - 24
bool(true)
bool(false)
4 - 24
bool(true)
bool(false)
5 - 24
bool(true)
bool(false)
5 - 24
bool(true)
bool(false)
6 - 24
bool(true)
bool(false)
6 - 24
bool(true)
bool(false)
7 - 24
bool(true)
bool(false)
7 - 24
bool(true)
bool(false)
8 - 24
bool(true)
bool(false)
8 - 24
bool(true)
bool(false)
9 - 24
bool(true)
bool(false)
9 - 24
bool(true)
bool(false)
10 - 24
bool(true)
bool(false)
10 - 24
bool(true)
bool(false)
11 - 24
bool(true)
bool(false)
11 - 24
bool(true)
bool(false)
12 - 24
bool(true)
bool(false)
12 - 24
bool(true)
bool(false)
13 - 24
bool(true)
bool(false)
13 - 24
bool(true)
bool(false)
14 - 24
bool(true)
bool(false)
14 - 24
bool(true)
bool(false)
15 - 24
bool(true)
bool(false)
15 - 24
bool(true)
bool(false)
16 - 24
bool(true)
bool(false)
16 - 24
bool(true)
bool(false)
17 - 24
bool(true)
bool(false)
17 - 24
bool(true)
bool(false)
18 - 24
bool(true)
bool(false)
18 - 24
bool(true)
bool(false)
0 - 25
bool(true)
bool(false)
0 - 25
bool(true)
bool(false)
1 - 25
bool(true)
bool(false)
1 - 25
bool(true)
bool(false)
2 - 25
bool(true)
bool(false)
2 - 25
bool(true)
bool(false)
3 - 25
bool(true)
bool(false)
3 - 25
bool(true)
bool(false)
4 - 25
bool(true)
bool(false)
4 - 25
bool(true)
bool(false)
5 - 25
bool(true)
bool(false)
5 - 25
bool(true)
bool(false)
6 - 25
bool(true)
bool(false)
6 - 25
bool(true)
bool(false)
7 - 25
bool(true)
bool(false)
7 - 25
bool(true)
bool(false)
8 - 25
bool(true)
bool(false)
8 - 25
bool(true)
bool(false)
9 - 25
bool(true)
bool(false)
9 - 25
bool(true)
bool(false)
10 - 25
bool(true)
bool(false)
10 - 25
bool(true)
bool(false)
11 - 25
bool(true)
bool(false)
11 - 25
bool(true)
bool(false)
12 - 25
bool(true)
bool(false)
12 - 25
bool(true)
bool(false)
13 - 25
bool(true)
bool(false)
13 - 25
bool(true)
bool(false)
14 - 25
bool(true)
bool(false)
14 - 25
bool(true)
bool(false)
15 - 25
bool(true)
bool(false)
15 - 25
bool(true)
bool(false)
16 - 25
bool(true)
bool(false)
16 - 25
bool(true)
bool(false)
17 - 25
bool(true)
bool(false)
17 - 25
bool(true)
bool(false)
18 - 25
bool(true)
bool(false)
18 - 25
bool(true)
bool(false)
0 - 26
bool(true)
bool(false)
0 - 26
bool(true)
bool(false)
1 - 26
bool(true)
bool(false)
1 - 26
bool(true)
bool(false)
2 - 26
bool(true)
bool(false)
2 - 26
bool(true)
bool(false)
3 - 26
bool(true)
bool(false)
3 - 26
bool(true)
bool(false)
4 - 26
bool(true)
bool(false)
4 - 26
bool(true)
bool(false)
5 - 26
bool(true)
bool(false)
5 - 26
bool(true)
bool(false)
6 - 26
bool(true)
bool(false)
6 - 26
bool(true)
bool(false)
7 - 26
bool(true)
bool(false)
7 - 26
bool(true)
bool(false)
8 - 26
bool(true)
bool(false)
8 - 26
bool(true)
bool(false)
9 - 26
bool(true)
bool(false)
9 - 26
bool(true)
bool(false)
10 - 26
bool(true)
bool(false)
10 - 26
bool(true)
bool(false)
11 - 26
bool(true)
bool(false)
11 - 26
bool(true)
bool(false)
12 - 26
bool(true)
bool(false)
12 - 26
bool(true)
bool(false)
13 - 26
bool(true)
bool(false)
13 - 26
bool(true)
bool(false)
14 - 26
bool(true)
bool(false)
14 - 26
bool(true)
bool(false)
15 - 26
bool(true)
bool(false)
15 - 26
bool(true)
bool(false)
16 - 26
bool(true)
bool(false)
16 - 26
bool(true)
bool(false)
17 - 26
bool(true)
bool(false)
17 - 26
bool(true)
bool(false)
18 - 26
bool(true)
bool(false)
18 - 26
bool(true)
bool(false)
0 - 27
bool(true)
bool(false)
0 - 27
bool(true)
bool(false)
1 - 27
bool(true)
bool(false)
1 - 27
bool(true)
bool(false)
2 - 27
bool(true)
bool(false)
2 - 27
bool(true)
bool(false)
3 - 27
bool(true)
bool(false)
3 - 27
bool(true)
bool(false)
4 - 27
bool(true)
bool(false)
4 - 27
bool(true)
bool(false)
5 - 27
bool(true)
bool(false)
5 - 27
bool(true)
bool(false)
6 - 27
bool(true)
bool(false)
6 - 27
bool(true)
bool(false)
7 - 27
bool(true)
bool(false)
7 - 27
bool(true)
bool(false)
8 - 27
bool(true)
bool(false)
8 - 27
bool(true)
bool(false)
9 - 27
bool(true)
bool(false)
9 - 27
bool(true)
bool(false)
10 - 27
bool(true)
bool(false)
10 - 27
bool(true)
bool(false)
11 - 27
bool(true)
bool(false)
11 - 27
bool(true)
bool(false)
12 - 27
bool(true)
bool(false)
12 - 27
bool(true)
bool(false)
13 - 27
bool(true)
bool(false)
13 - 27
bool(true)
bool(false)
14 - 27
bool(true)
bool(false)
14 - 27
bool(true)
bool(false)
15 - 27
bool(true)
bool(false)
15 - 27
bool(true)
bool(false)
16 - 27
bool(true)
bool(false)
16 - 27
bool(true)
bool(false)
17 - 27
bool(true)
bool(false)
17 - 27
bool(true)
bool(false)
18 - 27
bool(true)
bool(false)
18 - 27
bool(true)
bool(false)
0 - 28
bool(true)
bool(false)
0 - 28
bool(true)
bool(false)
1 - 28
bool(true)
bool(false)
1 - 28
bool(true)
bool(false)
2 - 28
bool(true)
bool(false)
2 - 28
bool(true)
bool(false)
3 - 28
bool(true)
bool(false)
3 - 28
bool(true)
bool(false)
4 - 28
bool(true)
bool(false)
4 - 28
bool(true)
bool(false)
5 - 28
bool(true)
bool(false)
5 - 28
bool(true)
bool(false)
6 - 28
bool(true)
bool(false)
6 - 28
bool(true)
bool(false)
7 - 28
bool(true)
bool(false)
7 - 28
bool(true)
bool(false)
8 - 28
bool(true)
bool(false)
8 - 28
bool(true)
bool(false)
9 - 28
bool(true)
bool(false)
9 - 28
bool(true)
bool(false)
10 - 28
bool(true)
bool(false)
10 - 28
bool(true)
bool(false)
11 - 28
bool(true)
bool(false)
11 - 28
bool(true)
bool(false)
12 - 28
bool(true)
bool(false)
12 - 28
bool(true)
bool(false)
13 - 28
bool(true)
bool(false)
13 - 28
bool(true)
bool(false)
14 - 28
bool(true)
bool(false)
14 - 28
bool(true)
bool(false)
15 - 28
bool(true)
bool(false)
15 - 28
bool(true)
bool(false)
16 - 28
bool(true)
bool(false)
16 - 28
bool(true)
bool(false)
17 - 28
bool(true)
bool(false)
17 - 28
bool(true)
bool(false)
18 - 28
bool(true)
bool(false)
18 - 28
bool(true)
bool(false)
0 - 29
bool(true)
bool(false)
0 - 29
bool(true)
bool(false)
1 - 29
bool(true)
bool(false)
1 - 29
bool(true)
bool(false)
2 - 29
bool(true)
bool(false)
2 - 29
bool(true)
bool(false)
3 - 29
bool(true)
bool(false)
3 - 29
bool(true)
bool(false)
4 - 29
bool(true)
bool(false)
4 - 29
bool(true)
bool(false)
5 - 29
bool(true)
bool(false)
5 - 29
bool(true)
bool(false)
6 - 29
bool(true)
bool(false)
6 - 29
bool(true)
bool(false)
7 - 29
bool(true)
bool(false)
7 - 29
bool(true)
bool(false)
8 - 29
bool(true)
bool(false)
8 - 29
bool(true)
bool(false)
9 - 29
bool(true)
bool(false)
9 - 29
bool(true)
bool(false)
10 - 29
bool(true)
bool(false)
10 - 29
bool(true)
bool(false)
11 - 29
bool(true)
bool(false)
11 - 29
bool(true)
bool(false)
12 - 29
bool(true)
bool(false)
12 - 29
bool(true)
bool(false)
13 - 29
bool(true)
bool(false)
13 - 29
bool(true)
bool(false)
14 - 29
bool(true)
bool(false)
14 - 29
bool(true)
bool(false)
15 - 29
bool(true)
bool(false)
15 - 29
bool(true)
bool(false)
16 - 29
bool(true)
bool(false)
16 - 29
bool(true)
bool(false)
17 - 29
bool(true)
bool(false)
17 - 29
bool(true)
bool(false)
18 - 29
bool(true)
bool(false)
18 - 29
bool(true)
bool(false)
0 - 30
bool(true)
bool(false)
0 - 30
bool(true)
bool(false)
1 - 30
bool(true)
bool(false)
1 - 30
bool(true)
bool(false)
2 - 30
bool(true)
bool(false)
2 - 30
bool(true)
bool(false)
3 - 30
bool(true)
bool(false)
3 - 30
bool(true)
bool(false)
4 - 30
bool(true)
bool(false)
4 - 30
bool(true)
bool(false)
5 - 30
bool(true)
bool(false)
5 - 30
bool(true)
bool(false)
6 - 30
bool(true)
bool(false)
6 - 30
bool(true)
bool(false)
7 - 30
bool(true)
bool(false)
7 - 30
bool(true)
bool(false)
8 - 30
bool(true)
bool(false)
8 - 30
bool(true)
bool(false)
9 - 30
bool(true)
bool(false)
9 - 30
bool(true)
bool(false)
10 - 30
bool(true)
bool(false)
10 - 30
bool(true)
bool(false)
11 - 30
bool(true)
bool(false)
11 - 30
bool(true)
bool(false)
12 - 30
bool(true)
bool(false)
12 - 30
bool(true)
bool(false)
13 - 30
bool(true)
bool(false)
13 - 30
bool(true)
bool(false)
14 - 30
bool(true)
bool(false)
14 - 30
bool(true)
bool(false)
15 - 30
bool(true)
bool(false)
15 - 30
bool(true)
bool(false)
16 - 30
bool(true)
bool(false)
16 - 30
bool(true)
bool(false)
17 - 30
bool(true)
bool(false)
17 - 30
bool(true)
bool(false)
18 - 30
bool(true)
bool(false)
18 - 30
bool(true)
bool(false)
0 - 31
bool(true)
bool(false)
0 - 31
bool(true)
bool(false)
1 - 31
bool(true)
bool(false)
1 - 31
bool(true)
bool(false)
2 - 31
bool(true)
bool(false)
2 - 31
bool(true)
bool(false)
3 - 31
bool(true)
bool(false)
3 - 31
bool(true)
bool(false)
4 - 31
bool(true)
bool(false)
4 - 31
bool(true)
bool(false)
5 - 31
bool(true)
bool(false)
5 - 31
bool(true)
bool(false)
6 - 31
bool(true)
bool(false)
6 - 31
bool(true)
bool(false)
7 - 31
bool(true)
bool(false)
7 - 31
bool(true)
bool(false)
8 - 31
bool(true)
bool(false)
8 - 31
bool(true)
bool(false)
9 - 31
bool(true)
bool(false)
9 - 31
bool(true)
bool(false)
10 - 31
bool(true)
bool(false)
10 - 31
bool(true)
bool(false)
11 - 31
bool(true)
bool(false)
11 - 31
bool(true)
bool(false)
12 - 31
bool(true)
bool(false)
12 - 31
bool(true)
bool(false)
13 - 31
bool(true)
bool(false)
13 - 31
bool(true)
bool(false)
14 - 31
bool(true)
bool(false)
14 - 31
bool(true)
bool(false)
15 - 31
bool(true)
bool(false)
15 - 31
bool(true)
bool(false)
16 - 31
bool(true)
bool(false)
16 - 31
bool(true)
bool(false)
17 - 31
bool(true)
bool(false)
17 - 31
bool(true)
bool(false)
18 - 31
bool(true)
bool(false)
18 - 31
bool(true)
bool(false)
0 - 32
bool(true)
bool(false)
0 - 32
bool(true)
bool(false)
1 - 32
bool(true)
bool(false)
1 - 32
bool(true)
bool(false)
2 - 32
bool(true)
bool(false)
2 - 32
bool(true)
bool(false)
3 - 32
bool(true)
bool(false)
3 - 32
bool(true)
bool(false)
4 - 32
bool(true)
bool(false)
4 - 32
bool(true)
bool(false)
5 - 32
bool(true)
bool(false)
5 - 32
bool(true)
bool(false)
6 - 32
bool(true)
bool(false)
6 - 32
bool(true)
bool(false)
7 - 32
bool(true)
bool(false)
7 - 32
bool(true)
bool(false)
8 - 32
bool(true)
bool(false)
8 - 32
bool(true)
bool(false)
9 - 32
bool(true)
bool(false)
9 - 32
bool(true)
bool(false)
10 - 32
bool(true)
bool(false)
10 - 32
bool(true)
bool(false)
11 - 32
bool(true)
bool(false)
11 - 32
bool(true)
bool(false)
12 - 32
bool(true)
bool(false)
12 - 32
bool(true)
bool(false)
13 - 32
bool(true)
bool(false)
13 - 32
bool(true)
bool(false)
14 - 32
bool(true)
bool(false)
14 - 32
bool(true)
bool(false)
15 - 32
bool(true)
bool(false)
15 - 32
bool(true)
bool(false)
16 - 32
bool(true)
bool(false)
16 - 32
bool(true)
bool(false)
17 - 32
bool(true)
bool(false)
17 - 32
bool(true)
bool(false)
18 - 32
bool(true)
bool(false)
18 - 32
bool(true)
bool(false)
0 - 33
bool(true)
bool(false)
0 - 33
bool(true)
bool(false)
1 - 33
bool(true)
bool(false)
1 - 33
bool(true)
bool(false)
2 - 33
bool(true)
bool(false)
2 - 33
bool(true)
bool(false)
3 - 33
bool(true)
bool(false)
3 - 33
bool(true)
bool(false)
4 - 33
bool(true)
bool(false)
4 - 33
bool(true)
bool(false)
5 - 33
bool(true)
bool(false)
5 - 33
bool(true)
bool(false)
6 - 33
bool(true)
bool(false)
6 - 33
bool(true)
bool(false)
7 - 33
bool(true)
bool(false)
7 - 33
bool(true)
bool(false)
8 - 33
bool(true)
bool(false)
8 - 33
bool(true)
bool(false)
9 - 33
bool(true)
bool(false)
9 - 33
bool(true)
bool(false)
10 - 33
bool(true)
bool(false)
10 - 33
bool(true)
bool(false)
11 - 33
bool(true)
bool(false)
11 - 33
bool(true)
bool(false)
12 - 33
bool(true)
bool(false)
12 - 33
bool(true)
bool(false)
13 - 33
bool(true)
bool(false)
13 - 33
bool(true)
bool(false)
14 - 33
bool(true)
bool(false)
14 - 33
bool(true)
bool(false)
15 - 33
bool(true)
bool(false)
15 - 33
bool(true)
bool(false)
16 - 33
bool(true)
bool(false)
16 - 33
bool(true)
bool(false)
17 - 33
bool(true)
bool(false)
17 - 33
bool(true)
bool(false)
18 - 33
bool(true)
bool(false)
18 - 33
bool(true)
bool(false)
0 - 34
bool(true)
bool(false)
0 - 34
bool(true)
bool(false)
1 - 34
bool(true)
bool(false)
1 - 34
bool(true)
bool(false)
2 - 34
bool(true)
bool(false)
2 - 34
bool(true)
bool(false)
3 - 34
bool(true)
bool(false)
3 - 34
bool(true)
bool(false)
4 - 34
bool(true)
bool(false)
4 - 34
bool(true)
bool(false)
5 - 34
bool(true)
bool(false)
5 - 34
bool(true)
bool(false)
6 - 34
bool(true)
bool(false)
6 - 34
bool(true)
bool(false)
7 - 34
bool(true)
bool(false)
7 - 34
bool(true)
bool(false)
8 - 34
bool(true)
bool(false)
8 - 34
bool(true)
bool(false)
9 - 34
bool(true)
bool(false)
9 - 34
bool(true)
bool(false)
10 - 34
bool(true)
bool(false)
10 - 34
bool(true)
bool(false)
11 - 34
bool(true)
bool(false)
11 - 34
bool(true)
bool(false)
12 - 34
bool(true)
bool(false)
12 - 34
bool(true)
bool(false)
13 - 34
bool(true)
bool(false)
13 - 34
bool(true)
bool(false)
14 - 34
bool(true)
bool(false)
14 - 34
bool(true)
bool(false)
15 - 34
bool(true)
bool(false)
15 - 34
bool(true)
bool(false)
16 - 34
bool(true)
bool(false)
16 - 34
bool(true)
bool(false)
17 - 34
bool(true)
bool(false)
17 - 34
bool(true)
bool(false)
18 - 34
bool(true)
bool(false)
18 - 34
bool(true)
bool(false)
0 - 35
bool(true)
bool(false)
0 - 35
bool(true)
bool(false)
1 - 35
bool(true)
bool(false)
1 - 35
bool(true)
bool(false)
2 - 35
bool(true)
bool(false)
2 - 35
bool(true)
bool(false)
3 - 35
bool(true)
bool(false)
3 - 35
bool(true)
bool(false)
4 - 35
bool(true)
bool(false)
4 - 35
bool(true)
bool(false)
5 - 35
bool(true)
bool(false)
5 - 35
bool(true)
bool(false)
6 - 35
bool(true)
bool(false)
6 - 35
bool(true)
bool(false)
7 - 35
bool(true)
bool(false)
7 - 35
bool(true)
bool(false)
8 - 35
bool(true)
bool(false)
8 - 35
bool(true)
bool(false)
9 - 35
bool(true)
bool(false)
9 - 35
bool(true)
bool(false)
10 - 35
bool(true)
bool(false)
10 - 35
bool(true)
bool(false)
11 - 35
bool(true)
bool(false)
11 - 35
bool(true)
bool(false)
12 - 35
bool(true)
bool(false)
12 - 35
bool(true)
bool(false)
13 - 35
bool(true)
bool(false)
13 - 35
bool(true)
bool(false)
14 - 35
bool(true)
bool(false)
14 - 35
bool(true)
bool(false)
15 - 35
bool(true)
bool(false)
15 - 35
bool(true)
bool(false)
16 - 35
bool(true)
bool(false)
16 - 35
bool(true)
bool(false)
17 - 35
bool(true)
bool(false)
17 - 35
bool(true)
bool(false)
18 - 35
bool(true)
bool(false)
18 - 35
bool(true)
bool(false)
0 - 36
bool(true)
bool(false)
0 - 36
bool(true)
bool(false)
1 - 36
bool(true)
bool(false)
1 - 36
bool(true)
bool(false)
2 - 36
bool(true)
bool(false)
2 - 36
bool(true)
bool(false)
3 - 36
bool(true)
bool(false)
3 - 36
bool(true)
bool(false)
4 - 36
bool(true)
bool(false)
4 - 36
bool(true)
bool(false)
5 - 36
bool(true)
bool(false)
5 - 36
bool(true)
bool(false)
6 - 36
bool(true)
bool(false)
6 - 36
bool(true)
bool(false)
7 - 36
bool(true)
bool(false)
7 - 36
bool(true)
bool(false)
8 - 36
bool(true)
bool(false)
8 - 36
bool(true)
bool(false)
9 - 36
bool(true)
bool(false)
9 - 36
bool(true)
bool(false)
10 - 36
bool(true)
bool(false)
10 - 36
bool(true)
bool(false)
11 - 36
bool(true)
bool(false)
11 - 36
bool(true)
bool(false)
12 - 36
bool(true)
bool(false)
12 - 36
bool(true)
bool(false)
13 - 36
bool(true)
bool(false)
13 - 36
bool(true)
bool(false)
14 - 36
bool(true)
bool(false)
14 - 36
bool(true)
bool(false)
15 - 36
bool(true)
bool(false)
15 - 36
bool(true)
bool(false)
16 - 36
bool(true)
bool(false)
16 - 36
bool(true)
bool(false)
17 - 36
bool(true)
bool(false)
17 - 36
bool(true)
bool(false)
18 - 36
bool(true)
bool(false)
18 - 36
bool(true)
bool(false)
0 - 37
bool(true)
bool(false)
0 - 37
bool(true)
bool(false)
1 - 37
bool(true)
bool(false)
1 - 37
bool(true)
bool(false)
2 - 37
bool(true)
bool(false)
2 - 37
bool(true)
bool(false)
3 - 37
bool(true)
bool(false)
3 - 37
bool(true)
bool(false)
4 - 37
bool(true)
bool(false)
4 - 37
bool(true)
bool(false)
5 - 37
bool(true)
bool(false)
5 - 37
bool(true)
bool(false)
6 - 37
bool(true)
bool(false)
6 - 37
bool(true)
bool(false)
7 - 37
bool(true)
bool(false)
7 - 37
bool(true)
bool(false)
8 - 37
bool(true)
bool(false)
8 - 37
bool(true)
bool(false)
9 - 37
bool(true)
bool(false)
9 - 37
bool(true)
bool(false)
10 - 37
bool(true)
bool(false)
10 - 37
bool(true)
bool(false)
11 - 37
bool(true)
bool(false)
11 - 37
bool(true)
bool(false)
12 - 37
bool(true)
bool(false)
12 - 37
bool(true)
bool(false)
13 - 37
bool(true)
bool(false)
13 - 37
bool(true)
bool(false)
14 - 37
bool(true)
bool(false)
14 - 37
bool(true)
bool(false)
15 - 37
bool(true)
bool(false)
15 - 37
bool(true)
bool(false)
16 - 37
bool(true)
bool(false)
16 - 37
bool(true)
bool(false)
17 - 37
bool(true)
bool(false)
17 - 37
bool(true)
bool(false)
18 - 37
bool(true)
bool(false)
18 - 37
bool(true)
bool(false)
0 - 38
bool(true)
bool(false)
0 - 38
bool(true)
bool(false)
1 - 38
bool(true)
bool(false)
1 - 38
bool(true)
bool(false)
2 - 38
bool(true)
bool(false)
2 - 38
bool(true)
bool(false)
3 - 38
bool(true)
bool(false)
3 - 38
bool(true)
bool(false)
4 - 38
bool(true)
bool(false)
4 - 38
bool(true)
bool(false)
5 - 38
bool(true)
bool(false)
5 - 38
bool(true)
bool(false)
6 - 38
bool(true)
bool(false)
6 - 38
bool(true)
bool(false)
7 - 38
bool(true)
bool(false)
7 - 38
bool(true)
bool(false)
8 - 38
bool(true)
bool(false)
8 - 38
bool(true)
bool(false)
9 - 38
bool(true)
bool(false)
9 - 38
bool(true)
bool(false)
10 - 38
bool(true)
bool(false)
10 - 38
bool(true)
bool(false)
11 - 38
bool(true)
bool(false)
11 - 38
bool(true)
bool(false)
12 - 38
bool(true)
bool(false)
12 - 38
bool(true)
bool(false)
13 - 38
bool(true)
bool(false)
13 - 38
bool(true)
bool(false)
14 - 38
bool(true)
bool(false)
14 - 38
bool(true)
bool(false)
15 - 38
bool(true)
bool(false)
15 - 38
bool(true)
bool(false)
16 - 38
bool(true)
bool(false)
16 - 38
bool(true)
bool(false)
17 - 38
bool(true)
bool(false)
17 - 38
bool(true)
bool(false)
18 - 38
bool(true)
bool(false)
18 - 38
bool(true)
bool(false)
0 - 39
bool(true)
bool(false)
0 - 39
bool(true)
bool(false)
1 - 39
bool(true)
bool(false)
1 - 39
bool(true)
bool(false)
2 - 39
bool(true)
bool(false)
2 - 39
bool(true)
bool(false)
3 - 39
bool(true)
bool(false)
3 - 39
bool(true)
bool(false)
4 - 39
bool(true)
bool(false)
4 - 39
bool(true)
bool(false)
5 - 39
bool(true)
bool(false)
5 - 39
bool(true)
bool(false)
6 - 39
bool(true)
bool(false)
6 - 39
bool(true)
bool(false)
7 - 39
bool(true)
bool(false)
7 - 39
bool(true)
bool(false)
8 - 39
bool(true)
bool(false)
8 - 39
bool(true)
bool(false)
9 - 39
bool(true)
bool(false)
9 - 39
bool(true)
bool(false)
10 - 39
bool(true)
bool(false)
10 - 39
bool(true)
bool(false)
11 - 39
bool(true)
bool(false)
11 - 39
bool(true)
bool(false)
12 - 39
bool(true)
bool(false)
12 - 39
bool(true)
bool(false)
13 - 39
bool(true)
bool(false)
13 - 39
bool(true)
bool(false)
14 - 39
bool(true)
bool(false)
14 - 39
bool(true)
bool(false)
15 - 39
bool(true)
bool(false)
15 - 39
bool(true)
bool(false)
16 - 39
bool(true)
bool(false)
16 - 39
bool(true)
bool(false)
17 - 39
bool(true)
bool(false)
17 - 39
bool(true)
bool(false)
18 - 39
bool(true)
bool(false)
18 - 39
bool(true)
bool(false)
0 - 40
bool(true)
bool(false)
0 - 40
bool(true)
bool(false)
1 - 40
bool(true)
bool(false)
1 - 40
bool(true)
bool(false)
2 - 40
bool(true)
bool(false)
2 - 40
bool(true)
bool(false)
3 - 40
bool(true)
bool(false)
3 - 40
bool(true)
bool(false)
4 - 40
bool(true)
bool(false)
4 - 40
bool(true)
bool(false)
5 - 40
bool(true)
bool(false)
5 - 40
bool(true)
bool(false)
6 - 40
bool(true)
bool(false)
6 - 40
bool(true)
bool(false)
7 - 40
bool(true)
bool(false)
7 - 40
bool(true)
bool(false)
8 - 40
bool(true)
bool(false)
8 - 40
bool(true)
bool(false)
9 - 40
bool(true)
bool(false)
9 - 40
bool(true)
bool(false)
10 - 40
bool(true)
bool(false)
10 - 40
bool(true)
bool(false)
11 - 40
bool(true)
bool(false)
11 - 40
bool(true)
bool(false)
12 - 40
bool(true)
bool(false)
12 - 40
bool(true)
bool(false)
13 - 40
bool(true)
bool(false)
13 - 40
bool(true)
bool(false)
14 - 40
bool(true)
bool(false)
14 - 40
bool(true)
bool(false)
15 - 40
bool(true)
bool(false)
15 - 40
bool(true)
bool(false)
16 - 40
bool(true)
bool(false)
16 - 40
bool(true)
bool(false)
17 - 40
bool(true)
bool(false)
17 - 40
bool(true)
bool(false)
18 - 40
bool(true)
bool(false)
18 - 40
bool(true)
bool(false)
0 - 41
bool(true)
bool(false)
0 - 41
bool(true)
bool(false)
1 - 41
bool(true)
bool(false)
1 - 41
bool(true)
bool(false)
2 - 41
bool(true)
bool(false)
2 - 41
bool(true)
bool(false)
3 - 41
bool(true)
bool(false)
3 - 41
bool(true)
bool(false)
4 - 41
bool(true)
bool(false)
4 - 41
bool(true)
bool(false)
5 - 41
bool(true)
bool(false)
5 - 41
bool(true)
bool(false)
6 - 41
bool(true)
bool(false)
6 - 41
bool(true)
bool(false)
7 - 41
bool(true)
bool(false)
7 - 41
bool(true)
bool(false)
8 - 41
bool(true)
bool(false)
8 - 41
bool(true)
bool(false)
9 - 41
bool(true)
bool(false)
9 - 41
bool(true)
bool(false)
10 - 41
bool(true)
bool(false)
10 - 41
bool(true)
bool(false)
11 - 41
bool(true)
bool(false)
11 - 41
bool(true)
bool(false)
12 - 41
bool(true)
bool(false)
12 - 41
bool(true)
bool(false)
13 - 41
bool(true)
bool(false)
13 - 41
bool(true)
bool(false)
14 - 41
bool(true)
bool(false)
14 - 41
bool(true)
bool(false)
15 - 41
bool(true)
bool(false)
15 - 41
bool(true)
bool(false)
16 - 41
bool(true)
bool(false)
16 - 41
bool(true)
bool(false)
17 - 41
bool(true)
bool(false)
17 - 41
bool(true)
bool(false)
18 - 41
bool(true)
bool(false)
18 - 41
bool(true)
bool(false)
0 - 42
bool(true)
bool(false)
0 - 42
bool(true)
bool(false)
1 - 42
bool(true)
bool(false)
1 - 42
bool(true)
bool(false)
2 - 42
bool(true)
bool(false)
2 - 42
bool(true)
bool(false)
3 - 42
bool(true)
bool(false)
3 - 42
bool(true)
bool(false)
4 - 42
bool(true)
bool(false)
4 - 42
bool(true)
bool(false)
5 - 42
bool(true)
bool(false)
5 - 42
bool(true)
bool(false)
6 - 42
bool(true)
bool(false)
6 - 42
bool(true)
bool(false)
7 - 42
bool(true)
bool(false)
7 - 42
bool(true)
bool(false)
8 - 42
bool(true)
bool(false)
8 - 42
bool(true)
bool(false)
9 - 42
bool(true)
bool(false)
9 - 42
bool(true)
bool(false)
10 - 42
bool(true)
bool(false)
10 - 42
bool(true)
bool(false)
11 - 42
bool(true)
bool(false)
11 - 42
bool(true)
bool(false)
12 - 42
bool(true)
bool(false)
12 - 42
bool(true)
bool(false)
13 - 42
bool(true)
bool(false)
13 - 42
bool(true)
bool(false)
14 - 42
bool(true)
bool(false)
14 - 42
bool(true)
bool(false)
15 - 42
bool(true)
bool(false)
15 - 42
bool(true)
bool(false)
16 - 42
bool(true)
bool(false)
16 - 42
bool(true)
bool(false)
17 - 42
bool(true)
bool(false)
17 - 42
bool(true)
bool(false)
18 - 42
bool(true)
bool(false)
18 - 42
bool(true)
bool(false)
0 - 43
bool(true)
bool(false)
0 - 43
bool(true)
bool(false)
1 - 43
bool(true)
bool(false)
1 - 43
bool(true)
bool(false)
2 - 43
bool(true)
bool(false)
2 - 43
bool(true)
bool(false)
3 - 43
bool(true)
bool(false)
3 - 43
bool(true)
bool(false)
4 - 43
bool(true)
bool(false)
4 - 43
bool(true)
bool(false)
5 - 43
bool(true)
bool(false)
5 - 43
bool(true)
bool(false)
6 - 43
bool(true)
bool(false)
6 - 43
bool(true)
bool(false)
7 - 43
bool(true)
bool(false)
7 - 43
bool(true)
bool(false)
8 - 43
bool(true)
bool(false)
8 - 43
bool(true)
bool(false)
9 - 43
bool(true)
bool(false)
9 - 43
bool(true)
bool(false)
10 - 43
bool(true)
bool(false)
10 - 43
bool(true)
bool(false)
11 - 43
bool(true)
bool(false)
11 - 43
bool(true)
bool(false)
12 - 43
bool(true)
bool(false)
12 - 43
bool(true)
bool(false)
13 - 43
bool(true)
bool(false)
13 - 43
bool(true)
bool(false)
14 - 43
bool(true)
bool(false)
14 - 43
bool(true)
bool(false)
15 - 43
bool(true)
bool(false)
15 - 43
bool(true)
bool(false)
16 - 43
bool(true)
bool(false)
16 - 43
bool(true)
bool(false)
17 - 43
bool(true)
bool(false)
17 - 43
bool(true)
bool(false)
18 - 43
bool(true)
bool(false)
18 - 43
bool(true)
bool(false)
0 - 44
bool(true)
bool(false)
0 - 44
bool(true)
bool(false)
1 - 44
bool(true)
bool(false)
1 - 44
bool(true)
bool(false)
2 - 44
bool(true)
bool(false)
2 - 44
bool(true)
bool(false)
3 - 44
bool(true)
bool(false)
3 - 44
bool(true)
bool(false)
4 - 44
bool(true)
bool(false)
4 - 44
bool(true)
bool(false)
5 - 44
bool(true)
bool(false)
5 - 44
bool(true)
bool(false)
6 - 44
bool(true)
bool(false)
6 - 44
bool(true)
bool(false)
7 - 44
bool(true)
bool(false)
7 - 44
bool(true)
bool(false)
8 - 44
bool(true)
bool(false)
8 - 44
bool(true)
bool(false)
9 - 44
bool(true)
bool(false)
9 - 44
bool(true)
bool(false)
10 - 44
bool(true)
bool(false)
10 - 44
bool(true)
bool(false)
11 - 44
bool(true)
bool(false)
11 - 44
bool(true)
bool(false)
12 - 44
bool(true)
bool(false)
12 - 44
bool(true)
bool(false)
13 - 44
bool(true)
bool(false)
13 - 44
bool(true)
bool(false)
14 - 44
bool(true)
bool(false)
14 - 44
bool(true)
bool(false)
15 - 44
bool(true)
bool(false)
15 - 44
bool(true)
bool(false)
16 - 44
bool(true)
bool(false)
16 - 44
bool(true)
bool(false)
17 - 44
bool(true)
bool(false)
17 - 44
bool(true)
bool(false)
18 - 44
bool(true)
bool(false)
18 - 44
bool(true)
bool(false)
0 - 45
bool(true)
bool(false)
0 - 45
bool(true)
bool(false)
1 - 45
bool(true)
bool(false)
1 - 45
bool(true)
bool(false)
2 - 45
bool(true)
bool(false)
2 - 45
bool(true)
bool(false)
3 - 45
bool(true)
bool(false)
3 - 45
bool(true)
bool(false)
4 - 45
bool(true)
bool(false)
4 - 45
bool(true)
bool(false)
5 - 45
bool(true)
bool(false)
5 - 45
bool(true)
bool(false)
6 - 45
bool(true)
bool(false)
6 - 45
bool(true)
bool(false)
7 - 45
bool(true)
bool(false)
7 - 45
bool(true)
bool(false)
8 - 45
bool(true)
bool(false)
8 - 45
bool(true)
bool(false)
9 - 45
bool(true)
bool(false)
9 - 45
bool(true)
bool(false)
10 - 45
bool(true)
bool(false)
10 - 45
bool(true)
bool(false)
11 - 45
bool(true)
bool(false)
11 - 45
bool(true)
bool(false)
12 - 45
bool(true)
bool(false)
12 - 45
bool(true)
bool(false)
13 - 45
bool(true)
bool(false)
13 - 45
bool(true)
bool(false)
14 - 45
bool(true)
bool(false)
14 - 45
bool(true)
bool(false)
15 - 45
bool(true)
bool(false)
15 - 45
bool(true)
bool(false)
16 - 45
bool(true)
bool(false)
16 - 45
bool(true)
bool(false)
17 - 45
bool(true)
bool(false)
17 - 45
bool(true)
bool(false)
18 - 45
bool(true)
bool(false)
18 - 45
bool(true)
bool(false)
0 - 46
bool(true)
bool(false)
0 - 46
bool(true)
bool(false)
1 - 46
bool(true)
bool(false)
1 - 46
bool(true)
bool(false)
2 - 46
bool(true)
bool(false)
2 - 46
bool(true)
bool(false)
3 - 46
bool(true)
bool(false)
3 - 46
bool(true)
bool(false)
4 - 46
bool(true)
bool(false)
4 - 46
bool(true)
bool(false)
5 - 46
bool(true)
bool(false)
5 - 46
bool(true)
bool(false)
6 - 46
bool(true)
bool(false)
6 - 46
bool(true)
bool(false)
7 - 46
bool(true)
bool(false)
7 - 46
bool(true)
bool(false)
8 - 46
bool(true)
bool(false)
8 - 46
bool(true)
bool(false)
9 - 46
bool(true)
bool(false)
9 - 46
bool(true)
bool(false)
10 - 46
bool(true)
bool(false)
10 - 46
bool(true)
bool(false)
11 - 46
bool(true)
bool(false)
11 - 46
bool(true)
bool(false)
12 - 46
bool(true)
bool(false)
12 - 46
bool(true)
bool(false)
13 - 46
bool(true)
bool(false)
13 - 46
bool(true)
bool(false)
14 - 46
bool(true)
bool(false)
14 - 46
bool(true)
bool(false)
15 - 46
bool(true)
bool(false)
15 - 46
bool(true)
bool(false)
16 - 46
bool(true)
bool(false)
16 - 46
bool(true)
bool(false)
17 - 46
bool(true)
bool(false)
17 - 46
bool(true)
bool(false)
18 - 46
bool(true)
bool(false)
18 - 46
bool(true)
bool(false)
0 - 47
bool(true)
bool(false)
0 - 47
bool(true)
bool(false)
1 - 47
bool(true)
bool(false)
1 - 47
bool(true)
bool(false)
2 - 47
bool(true)
bool(false)
2 - 47
bool(true)
bool(false)
3 - 47
bool(true)
bool(false)
3 - 47
bool(true)
bool(false)
4 - 47
bool(true)
bool(false)
4 - 47
bool(true)
bool(false)
5 - 47
bool(true)
bool(false)
5 - 47
bool(true)
bool(false)
6 - 47
bool(true)
bool(false)
6 - 47
bool(true)
bool(false)
7 - 47
bool(true)
bool(false)
7 - 47
bool(true)
bool(false)
8 - 47
bool(true)
bool(false)
8 - 47
bool(true)
bool(false)
9 - 47
bool(true)
bool(false)
9 - 47
bool(true)
bool(false)
10 - 47
bool(true)
bool(false)
10 - 47
bool(true)
bool(false)
11 - 47
bool(true)
bool(false)
11 - 47
bool(true)
bool(false)
12 - 47
bool(true)
bool(false)
12 - 47
bool(true)
bool(false)
13 - 47
bool(true)
bool(false)
13 - 47
bool(true)
bool(false)
14 - 47
bool(true)
bool(false)
14 - 47
bool(true)
bool(false)
15 - 47
bool(true)
bool(false)
15 - 47
bool(true)
bool(false)
16 - 47
bool(true)
bool(false)
16 - 47
bool(true)
bool(false)
17 - 47
bool(true)
bool(false)
17 - 47
bool(true)
bool(false)
18 - 47
bool(true)
bool(false)
18 - 47
bool(true)
bool(false)
0 - 48
bool(true)
bool(false)
0 - 48
bool(true)
bool(false)
1 - 48
bool(true)
bool(false)
1 - 48
bool(true)
bool(false)
2 - 48
bool(true)
bool(false)
2 - 48
bool(true)
bool(false)
3 - 48
bool(true)
bool(false)
3 - 48
bool(true)
bool(false)
4 - 48
bool(true)
bool(false)
4 - 48
bool(true)
bool(false)
5 - 48
bool(true)
bool(false)
5 - 48
bool(true)
bool(false)
6 - 48
bool(true)
bool(false)
6 - 48
bool(true)
bool(false)
7 - 48
bool(true)
bool(false)
7 - 48
bool(true)
bool(false)
8 - 48
bool(true)
bool(false)
8 - 48
bool(true)
bool(false)
9 - 48
bool(true)
bool(false)
9 - 48
bool(true)
bool(false)
10 - 48
bool(true)
bool(false)
10 - 48
bool(true)
bool(false)
11 - 48
bool(true)
bool(false)
11 - 48
bool(true)
bool(false)
12 - 48
bool(true)
bool(false)
12 - 48
bool(true)
bool(false)
13 - 48
bool(true)
bool(false)
13 - 48
bool(true)
bool(false)
14 - 48
bool(true)
bool(false)
14 - 48
bool(true)
bool(false)
15 - 48
bool(true)
bool(false)
15 - 48
bool(true)
bool(false)
16 - 48
bool(true)
bool(false)
16 - 48
bool(true)
bool(false)
17 - 48
bool(true)
bool(false)
17 - 48
bool(true)
bool(false)
18 - 48
bool(true)
bool(false)
18 - 48
bool(true)
bool(false)
0 - 49
bool(true)
bool(false)
0 - 49
bool(true)
bool(false)
1 - 49
bool(true)
bool(false)
1 - 49
bool(true)
bool(false)
2 - 49
bool(true)
bool(false)
2 - 49
bool(true)
bool(false)
3 - 49
bool(true)
bool(false)
3 - 49
bool(true)
bool(false)
4 - 49
bool(true)
bool(false)
4 - 49
bool(true)
bool(false)
5 - 49
bool(true)
bool(false)
5 - 49
bool(true)
bool(false)
6 - 49
bool(true)
bool(false)
6 - 49
bool(true)
bool(false)
7 - 49
bool(true)
bool(false)
7 - 49
bool(true)
bool(false)
8 - 49
bool(true)
bool(false)
8 - 49
bool(true)
bool(false)
9 - 49
bool(true)
bool(false)
9 - 49
bool(true)
bool(false)
10 - 49
bool(true)
bool(false)
10 - 49
bool(true)
bool(false)
11 - 49
bool(true)
bool(false)
11 - 49
bool(true)
bool(false)
12 - 49
bool(true)
bool(false)
12 - 49
bool(true)
bool(false)
13 - 49
bool(true)
bool(false)
13 - 49
bool(true)
bool(false)
14 - 49
bool(true)
bool(false)
14 - 49
bool(true)
bool(false)
15 - 49
bool(true)
bool(false)
15 - 49
bool(true)
bool(false)
16 - 49
bool(true)
bool(false)
16 - 49
bool(true)
bool(false)
17 - 49
bool(true)
bool(false)
17 - 49
bool(true)
bool(false)
18 - 49
bool(true)
bool(false)
18 - 49
bool(true)
bool(false)
0 - 50
bool(true)
bool(false)
0 - 50
bool(true)
bool(false)
1 - 50
bool(true)
bool(false)
1 - 50
bool(true)
bool(false)
2 - 50
bool(true)
bool(false)
2 - 50
bool(true)
bool(false)
3 - 50
bool(true)
bool(false)
3 - 50
bool(true)
bool(false)
4 - 50
bool(true)
bool(false)
4 - 50
bool(true)
bool(false)
5 - 50
bool(true)
bool(false)
5 - 50
bool(true)
bool(false)
6 - 50
bool(true)
bool(false)
6 - 50
bool(true)
bool(false)
7 - 50
bool(true)
bool(false)
7 - 50
bool(true)
bool(false)
8 - 50
bool(true)
bool(false)
8 - 50
bool(true)
bool(false)
9 - 50
bool(true)
bool(false)
9 - 50
bool(true)
bool(false)
10 - 50
bool(true)
bool(false)
10 - 50
bool(true)
bool(false)
11 - 50
bool(true)
bool(false)
11 - 50
bool(true)
bool(false)
12 - 50
bool(true)
bool(false)
12 - 50
bool(true)
bool(false)
13 - 50
bool(true)
bool(false)
13 - 50
bool(true)
bool(false)
14 - 50
bool(true)
bool(false)
14 - 50
bool(true)
bool(false)
15 - 50
bool(true)
bool(false)
15 - 50
bool(true)
bool(false)
16 - 50
bool(true)
bool(false)
16 - 50
bool(true)
bool(false)
17 - 50
bool(true)
bool(false)
17 - 50
bool(true)
bool(false)
18 - 50
bool(true)
bool(false)
18 - 50
bool(true)
bool(false)
0 - 51
bool(true)
bool(false)
0 - 51
bool(true)
bool(false)
1 - 51
bool(true)
bool(false)
1 - 51
bool(true)
bool(false)
2 - 51
bool(true)
bool(false)
2 - 51
bool(true)
bool(false)
3 - 51
bool(true)
bool(false)
3 - 51
bool(true)
bool(false)
4 - 51
bool(true)
bool(false)
4 - 51
bool(true)
bool(false)
5 - 51
bool(true)
bool(false)
5 - 51
bool(true)
bool(false)
6 - 51
bool(true)
bool(false)
6 - 51
bool(true)
bool(false)
7 - 51
bool(true)
bool(false)
7 - 51
bool(true)
bool(false)
8 - 51
bool(true)
bool(false)
8 - 51
bool(true)
bool(false)
9 - 51
bool(true)
bool(false)
9 - 51
bool(true)
bool(false)
10 - 51
bool(true)
bool(false)
10 - 51
bool(true)
bool(false)
11 - 51
bool(true)
bool(false)
11 - 51
bool(true)
bool(false)
12 - 51
bool(true)
bool(false)
12 - 51
bool(true)
bool(false)
13 - 51
bool(true)
bool(false)
13 - 51
bool(true)
bool(false)
14 - 51
bool(true)
bool(false)
14 - 51
bool(true)
bool(false)
15 - 51
bool(true)
bool(false)
15 - 51
bool(true)
bool(false)
16 - 51
bool(true)
bool(false)
16 - 51
bool(true)
bool(false)
17 - 51
bool(true)
bool(false)
17 - 51
bool(true)
bool(false)
18 - 51
bool(true)
bool(false)
18 - 51
bool(true)
bool(false)
0 - 52
bool(true)
bool(false)
0 - 52
bool(true)
bool(false)
1 - 52
bool(true)
bool(false)
1 - 52
bool(true)
bool(false)
2 - 52
bool(true)
bool(false)
2 - 52
bool(true)
bool(false)
3 - 52
bool(true)
bool(false)
3 - 52
bool(true)
bool(false)
4 - 52
bool(true)
bool(false)
4 - 52
bool(true)
bool(false)
5 - 52
bool(true)
bool(false)
5 - 52
bool(true)
bool(false)
6 - 52
bool(true)
bool(false)
6 - 52
bool(true)
bool(false)
7 - 52
bool(true)
bool(false)
7 - 52
bool(true)
bool(false)
8 - 52
bool(true)
bool(false)
8 - 52
bool(true)
bool(false)
9 - 52
bool(true)
bool(false)
9 - 52
bool(true)
bool(false)
10 - 52
bool(true)
bool(false)
10 - 52
bool(true)
bool(false)
11 - 52
bool(true)
bool(false)
11 - 52
bool(true)
bool(false)
12 - 52
bool(true)
bool(false)
12 - 52
bool(true)
bool(false)
13 - 52
bool(true)
bool(false)
13 - 52
bool(true)
bool(false)
14 - 52
bool(true)
bool(false)
14 - 52
bool(true)
bool(false)
15 - 52
bool(true)
bool(false)
15 - 52
bool(true)
bool(false)
16 - 52
bool(true)
bool(false)
16 - 52
bool(true)
bool(false)
17 - 52
bool(true)
bool(false)
17 - 52
bool(true)
bool(false)
18 - 52
bool(true)
bool(false)
18 - 52
bool(true)
bool(false)
0 - 53
bool(true)
bool(false)
0 - 53
bool(true)
bool(false)
1 - 53
bool(true)
bool(false)
1 - 53
bool(true)
bool(false)
2 - 53
bool(true)
bool(false)
2 - 53
bool(true)
bool(false)
3 - 53
bool(true)
bool(false)
3 - 53
bool(true)
bool(false)
4 - 53
bool(true)
bool(false)
4 - 53
bool(true)
bool(false)
5 - 53
bool(true)
bool(false)
5 - 53
bool(true)
bool(false)
6 - 53
bool(true)
bool(false)
6 - 53
bool(true)
bool(false)
7 - 53
bool(true)
bool(false)
7 - 53
bool(true)
bool(false)
8 - 53
bool(true)
bool(false)
8 - 53
bool(true)
bool(false)
9 - 53
bool(true)
bool(false)
9 - 53
bool(true)
bool(false)
10 - 53
bool(true)
bool(false)
10 - 53
bool(true)
bool(false)
11 - 53
bool(true)
bool(false)
11 - 53
bool(true)
bool(false)
12 - 53
bool(true)
bool(false)
12 - 53
bool(true)
bool(false)
13 - 53
bool(true)
bool(false)
13 - 53
bool(true)
bool(false)
14 - 53
bool(true)
bool(false)
14 - 53
bool(true)
bool(false)
15 - 53
bool(true)
bool(false)
15 - 53
bool(true)
bool(false)
16 - 53
bool(true)
bool(false)
16 - 53
bool(true)
bool(false)
17 - 53
bool(true)
bool(false)
17 - 53
bool(true)
bool(false)
18 - 53
bool(true)
bool(false)
18 - 53
bool(true)
bool(false)
0 - 54
bool(true)
bool(false)
0 - 54
bool(true)
bool(false)
1 - 54
bool(true)
bool(false)
1 - 54
bool(true)
bool(false)
2 - 54
bool(true)
bool(false)
2 - 54
bool(true)
bool(false)
3 - 54
bool(true)
bool(false)
3 - 54
bool(true)
bool(false)
4 - 54
bool(true)
bool(false)
4 - 54
bool(true)
bool(false)
5 - 54
bool(true)
bool(false)
5 - 54
bool(true)
bool(false)
6 - 54
bool(true)
bool(false)
6 - 54
bool(true)
bool(false)
7 - 54
bool(true)
bool(false)
7 - 54
bool(true)
bool(false)
8 - 54
bool(true)
bool(false)
8 - 54
bool(true)
bool(false)
9 - 54
bool(true)
bool(false)
9 - 54
bool(true)
bool(false)
10 - 54
bool(true)
bool(false)
10 - 54
bool(true)
bool(false)
11 - 54
bool(true)
bool(false)
11 - 54
bool(true)
bool(false)
12 - 54
bool(true)
bool(false)
12 - 54
bool(true)
bool(false)
13 - 54
bool(true)
bool(false)
13 - 54
bool(true)
bool(false)
14 - 54
bool(true)
bool(false)
14 - 54
bool(true)
bool(false)
15 - 54
bool(true)
bool(false)
15 - 54
bool(true)
bool(false)
16 - 54
bool(true)
bool(false)
16 - 54
bool(true)
bool(false)
17 - 54
bool(true)
bool(false)
17 - 54
bool(true)
bool(false)
18 - 54
bool(true)
bool(false)
18 - 54
bool(true)
bool(false)
0 - 55
bool(true)
bool(false)
0 - 55
bool(true)
bool(false)
1 - 55
bool(true)
bool(false)
1 - 55
bool(true)
bool(false)
2 - 55
bool(true)
bool(false)
2 - 55
bool(true)
bool(false)
3 - 55
bool(true)
bool(false)
3 - 55
bool(true)
bool(false)
4 - 55
bool(true)
bool(false)
4 - 55
bool(true)
bool(false)
5 - 55
bool(true)
bool(false)
5 - 55
bool(true)
bool(false)
6 - 55
bool(true)
bool(false)
6 - 55
bool(true)
bool(false)
7 - 55
bool(true)
bool(false)
7 - 55
bool(true)
bool(false)
8 - 55
bool(true)
bool(false)
8 - 55
bool(true)
bool(false)
9 - 55
bool(true)
bool(false)
9 - 55
bool(true)
bool(false)
10 - 55
bool(true)
bool(false)
10 - 55
bool(true)
bool(false)
11 - 55
bool(true)
bool(false)
11 - 55
bool(true)
bool(false)
12 - 55
bool(true)
bool(false)
12 - 55
bool(true)
bool(false)
13 - 55
bool(true)
bool(false)
13 - 55
bool(true)
bool(false)
14 - 55
bool(true)
bool(false)
14 - 55
bool(true)
bool(false)
15 - 55
bool(true)
bool(false)
15 - 55
bool(true)
bool(false)
16 - 55
bool(true)
bool(false)
16 - 55
bool(true)
bool(false)
17 - 55
bool(true)
bool(false)
17 - 55
bool(true)
bool(false)
18 - 55
bool(true)
bool(false)
18 - 55
bool(true)
bool(false)
0 - 56
bool(true)
bool(false)
0 - 56
bool(true)
bool(false)
1 - 56
bool(true)
bool(false)
1 - 56
bool(true)
bool(false)
2 - 56
bool(true)
bool(false)
2 - 56
bool(true)
bool(false)
3 - 56
bool(true)
bool(false)
3 - 56
bool(true)
bool(false)
4 - 56
bool(true)
bool(false)
4 - 56
bool(true)
bool(false)
5 - 56
bool(true)
bool(false)
5 - 56
bool(true)
bool(false)
6 - 56
bool(true)
bool(false)
6 - 56
bool(true)
bool(false)
7 - 56
bool(true)
bool(false)
7 - 56
bool(true)
bool(false)
8 - 56
bool(true)
bool(false)
8 - 56
bool(true)
bool(false)
9 - 56
bool(true)
bool(false)
9 - 56
bool(true)
bool(false)
10 - 56
bool(true)
bool(false)
10 - 56
bool(true)
bool(false)
11 - 56
bool(true)
bool(false)
11 - 56
bool(true)
bool(false)
12 - 56
bool(true)
bool(false)
12 - 56
bool(true)
bool(false)
13 - 56
bool(true)
bool(false)
13 - 56
bool(true)
bool(false)
14 - 56
bool(true)
bool(false)
14 - 56
bool(true)
bool(false)
15 - 56
bool(true)
bool(false)
15 - 56
bool(true)
bool(false)
16 - 56
bool(true)
bool(false)
16 - 56
bool(true)
bool(false)
17 - 56
bool(true)
bool(false)
17 - 56
bool(true)
bool(false)
18 - 56
bool(true)
bool(false)
18 - 56
bool(true)
bool(false)
0 - 57
bool(true)
bool(false)
0 - 57
bool(true)
bool(false)
1 - 57
bool(true)
bool(false)
1 - 57
bool(true)
bool(false)
2 - 57
bool(true)
bool(false)
2 - 57
bool(true)
bool(false)
3 - 57
bool(true)
bool(false)
3 - 57
bool(true)
bool(false)
4 - 57
bool(true)
bool(false)
4 - 57
bool(true)
bool(false)
5 - 57
bool(true)
bool(false)
5 - 57
bool(true)
bool(false)
6 - 57
bool(true)
bool(false)
6 - 57
bool(true)
bool(false)
7 - 57
bool(true)
bool(false)
7 - 57
bool(true)
bool(false)
8 - 57
bool(true)
bool(false)
8 - 57
bool(true)
bool(false)
9 - 57
bool(true)
bool(false)
9 - 57
bool(true)
bool(false)
10 - 57
bool(true)
bool(false)
10 - 57
bool(true)
bool(false)
11 - 57
bool(true)
bool(false)
11 - 57
bool(true)
bool(false)
12 - 57
bool(true)
bool(false)
12 - 57
bool(true)
bool(false)
13 - 57
bool(true)
bool(false)
13 - 57
bool(true)
bool(false)
14 - 57
bool(true)
bool(false)
14 - 57
bool(true)
bool(false)
15 - 57
bool(true)
bool(false)
15 - 57
bool(true)
bool(false)
16 - 57
bool(true)
bool(false)
16 - 57
bool(true)
bool(false)
17 - 57
bool(true)
bool(false)
17 - 57
bool(true)
bool(false)
18 - 57
bool(true)
bool(false)
18 - 57
bool(true)
bool(false)
0 - 58
bool(true)
bool(false)
0 - 58
bool(true)
bool(false)
1 - 58
bool(true)
bool(false)
1 - 58
bool(true)
bool(false)
2 - 58
bool(true)
bool(false)
2 - 58
bool(true)
bool(false)
3 - 58
bool(true)
bool(false)
3 - 58
bool(true)
bool(false)
4 - 58
bool(true)
bool(false)
4 - 58
bool(true)
bool(false)
5 - 58
bool(true)
bool(false)
5 - 58
bool(true)
bool(false)
6 - 58
bool(true)
bool(false)
6 - 58
bool(true)
bool(false)
7 - 58
bool(true)
bool(false)
7 - 58
bool(true)
bool(false)
8 - 58
bool(true)
bool(false)
8 - 58
bool(true)
bool(false)
9 - 58
bool(true)
bool(false)
9 - 58
bool(true)
bool(false)
10 - 58
bool(true)
bool(false)
10 - 58
bool(true)
bool(false)
11 - 58
bool(true)
bool(false)
11 - 58
bool(true)
bool(false)
12 - 58
bool(true)
bool(false)
12 - 58
bool(true)
bool(false)
13 - 58
bool(true)
bool(false)
13 - 58
bool(true)
bool(false)
14 - 58
bool(true)
bool(false)
14 - 58
bool(true)
bool(false)
15 - 58
bool(true)
bool(false)
15 - 58
bool(true)
bool(false)
16 - 58
bool(true)
bool(false)
16 - 58
bool(true)
bool(false)
17 - 58
bool(true)
bool(false)
17 - 58
bool(true)
bool(false)
18 - 58
bool(true)
bool(false)
18 - 58
bool(true)
bool(false)
0 - 59
bool(true)
bool(false)
0 - 59
bool(true)
bool(false)
1 - 59
bool(true)
bool(false)
1 - 59
bool(true)
bool(false)
2 - 59
bool(true)
bool(false)
2 - 59
bool(true)
bool(false)
3 - 59
bool(true)
bool(false)
3 - 59
bool(true)
bool(false)
4 - 59
bool(true)
bool(false)
4 - 59
bool(true)
bool(false)
5 - 59
bool(true)
bool(false)
5 - 59
bool(true)
bool(false)
6 - 59
bool(true)
bool(false)
6 - 59
bool(true)
bool(false)
7 - 59
bool(true)
bool(false)
7 - 59
bool(true)
bool(false)
8 - 59
bool(true)
bool(false)
8 - 59
bool(true)
bool(false)
9 - 59
bool(true)
bool(false)
9 - 59
bool(true)
bool(false)
10 - 59
bool(true)
bool(false)
10 - 59
bool(true)
bool(false)
11 - 59
bool(true)
bool(false)
11 - 59
bool(true)
bool(false)
12 - 59
bool(true)
bool(false)
12 - 59
bool(true)
bool(false)
13 - 59
bool(true)
bool(false)
13 - 59
bool(true)
bool(false)
14 - 59
bool(true)
bool(false)
14 - 59
bool(true)
bool(false)
15 - 59
bool(true)
bool(false)
15 - 59
bool(true)
bool(false)
16 - 59
bool(true)
bool(false)
16 - 59
bool(true)
bool(false)
17 - 59
bool(true)
bool(false)
17 - 59
bool(true)
bool(false)
18 - 59
bool(true)
bool(false)
18 - 59
bool(true)
bool(false)
0 - 60
bool(true)
bool(false)
0 - 60
bool(true)
bool(false)
1 - 60
bool(true)
bool(false)
1 - 60
bool(true)
bool(false)
2 - 60
bool(true)
bool(false)
2 - 60
bool(true)
bool(false)
3 - 60
bool(true)
bool(false)
3 - 60
bool(true)
bool(false)
4 - 60
bool(true)
bool(false)
4 - 60
bool(true)
bool(false)
5 - 60
bool(true)
bool(false)
5 - 60
bool(true)
bool(false)
6 - 60
bool(true)
bool(false)
6 - 60
bool(true)
bool(false)
7 - 60
bool(true)
bool(false)
7 - 60
bool(true)
bool(false)
8 - 60
bool(true)
bool(false)
8 - 60
bool(true)
bool(false)
9 - 60
bool(true)
bool(false)
9 - 60
bool(true)
bool(false)
10 - 60
bool(true)
bool(false)
10 - 60
bool(true)
bool(false)
11 - 60
bool(true)
bool(false)
11 - 60
bool(true)
bool(false)
12 - 60
bool(true)
bool(false)
12 - 60
bool(true)
bool(false)
13 - 60
bool(true)
bool(false)
13 - 60
bool(true)
bool(false)
14 - 60
bool(true)
bool(false)
14 - 60
bool(true)
bool(false)
15 - 60
bool(true)
bool(false)
15 - 60
bool(true)
bool(false)
16 - 60
bool(true)
bool(false)
16 - 60
bool(true)
bool(false)
17 - 60
bool(true)
bool(false)
17 - 60
bool(true)
bool(false)
18 - 60
bool(true)
bool(false)
18 - 60
bool(true)
bool(false)
0 - 61
bool(true)
bool(false)
0 - 61
bool(true)
bool(false)
1 - 61
bool(true)
bool(false)
1 - 61
bool(true)
bool(false)
2 - 61
bool(true)
bool(false)
2 - 61
bool(true)
bool(false)
3 - 61
bool(true)
bool(false)
3 - 61
bool(true)
bool(false)
4 - 61
bool(true)
bool(false)
4 - 61
bool(true)
bool(false)
5 - 61
bool(true)
bool(false)
5 - 61
bool(true)
bool(false)
6 - 61
bool(true)
bool(false)
6 - 61
bool(true)
bool(false)
7 - 61
bool(true)
bool(false)
7 - 61
bool(true)
bool(false)
8 - 61
bool(true)
bool(false)
8 - 61
bool(true)
bool(false)
9 - 61
bool(true)
bool(false)
9 - 61
bool(true)
bool(false)
10 - 61
bool(true)
bool(false)
10 - 61
bool(true)
bool(false)
11 - 61
bool(true)
bool(false)
11 - 61
bool(true)
bool(false)
12 - 61
bool(true)
bool(false)
12 - 61
bool(true)
bool(false)
13 - 61
bool(true)
bool(false)
13 - 61
bool(true)
bool(false)
14 - 61
bool(true)
bool(false)
14 - 61
bool(true)
bool(false)
15 - 61
bool(true)
bool(false)
15 - 61
bool(true)
bool(false)
16 - 61
bool(true)
bool(false)
16 - 61
bool(true)
bool(false)
17 - 61
bool(true)
bool(false)
17 - 61
bool(true)
bool(false)
18 - 61
bool(true)
bool(false)
18 - 61
bool(true)
bool(false)
0 - 62
bool(true)
bool(false)
0 - 62
bool(true)
bool(false)
1 - 62
bool(true)
bool(false)
1 - 62
bool(true)
bool(false)
2 - 62
bool(true)
bool(false)
2 - 62
bool(true)
bool(false)
3 - 62
bool(true)
bool(false)
3 - 62
bool(true)
bool(false)
4 - 62
bool(true)
bool(false)
4 - 62
bool(true)
bool(false)
5 - 62
bool(true)
bool(false)
5 - 62
bool(true)
bool(false)
6 - 62
bool(true)
bool(false)
6 - 62
bool(true)
bool(false)
7 - 62
bool(true)
bool(false)
7 - 62
bool(true)
bool(false)
8 - 62
bool(true)
bool(false)
8 - 62
bool(true)
bool(false)
9 - 62
bool(true)
bool(false)
9 - 62
bool(true)
bool(false)
10 - 62
bool(true)
bool(false)
10 - 62
bool(true)
bool(false)
11 - 62
bool(true)
bool(false)
11 - 62
bool(true)
bool(false)
12 - 62
bool(true)
bool(false)
12 - 62
bool(true)
bool(false)
13 - 62
bool(true)
bool(false)
13 - 62
bool(true)
bool(false)
14 - 62
bool(true)
bool(false)
14 - 62
bool(true)
bool(false)
15 - 62
bool(true)
bool(false)
15 - 62
bool(true)
bool(false)
16 - 62
bool(true)
bool(false)
16 - 62
bool(true)
bool(false)
17 - 62
bool(true)
bool(false)
17 - 62
bool(true)
bool(false)
18 - 62
bool(true)
bool(false)
18 - 62
bool(true)
bool(false)
0 - 63
bool(true)
bool(false)
0 - 63
bool(true)
bool(false)
1 - 63
bool(true)
bool(false)
1 - 63
bool(true)
bool(false)
2 - 63
bool(true)
bool(false)
2 - 63
bool(true)
bool(false)
3 - 63
bool(true)
bool(false)
3 - 63
bool(true)
bool(false)
4 - 63
bool(true)
bool(false)
4 - 63
bool(true)
bool(false)
5 - 63
bool(true)
bool(false)
5 - 63
bool(true)
bool(false)
6 - 63
bool(true)
bool(false)
6 - 63
bool(true)
bool(false)
7 - 63
bool(true)
bool(false)
7 - 63
bool(true)
bool(false)
8 - 63
bool(true)
bool(false)
8 - 63
bool(true)
bool(false)
9 - 63
bool(true)
bool(false)
9 - 63
bool(true)
bool(false)
10 - 63
bool(true)
bool(false)
10 - 63
bool(true)
bool(false)
11 - 63
bool(true)
bool(false)
11 - 63
bool(true)
bool(false)
12 - 63
bool(true)
bool(false)
12 - 63
bool(true)
bool(false)
13 - 63
bool(true)
bool(false)
13 - 63
bool(true)
bool(false)
14 - 63
bool(true)
bool(false)
14 - 63
bool(true)
bool(false)
15 - 63
bool(true)
bool(false)
15 - 63
bool(true)
bool(false)
16 - 63
bool(true)
bool(false)
16 - 63
bool(true)
bool(false)
17 - 63
bool(true)
bool(false)
17 - 63
bool(true)
bool(false)
18 - 63
bool(true)
bool(false)
18 - 63
bool(true)
bool(false)
0 - 64
bool(true)
bool(false)
0 - 64
bool(true)
bool(false)
1 - 64
bool(true)
bool(false)
1 - 64
bool(true)
bool(false)
2 - 64
bool(true)
bool(false)
2 - 64
bool(true)
bool(false)
3 - 64
bool(true)
bool(false)
3 - 64
bool(true)
bool(false)
4 - 64
bool(true)
bool(false)
4 - 64
bool(true)
bool(false)
5 - 64
bool(true)
bool(false)
5 - 64
bool(true)
bool(false)
6 - 64
bool(true)
bool(false)
6 - 64
bool(true)
bool(false)
7 - 64
bool(true)
bool(false)
7 - 64
bool(true)
bool(false)
8 - 64
bool(true)
bool(false)
8 - 64
bool(true)
bool(false)
9 - 64
bool(true)
bool(false)
9 - 64
bool(true)
bool(false)
10 - 64
bool(true)
bool(false)
10 - 64
bool(true)
bool(false)
11 - 64
bool(true)
bool(false)
11 - 64
bool(true)
bool(false)
12 - 64
bool(true)
bool(false)
12 - 64
bool(true)
bool(false)
13 - 64
bool(true)
bool(false)
13 - 64
bool(true)
bool(false)
14 - 64
bool(true)
bool(false)
14 - 64
bool(true)
bool(false)
15 - 64
bool(true)
bool(false)
15 - 64
bool(true)
bool(false)
16 - 64
bool(true)
bool(false)
16 - 64
bool(true)
bool(false)
17 - 64
bool(true)
bool(false)
17 - 64
bool(true)
bool(false)
18 - 64
bool(true)
bool(false)
18 - 64
bool(true)
bool(false)
0 - 65
bool(true)
bool(false)
0 - 65
bool(true)
bool(false)
1 - 65
bool(true)
bool(false)
1 - 65
bool(true)
bool(false)
2 - 65
bool(true)
bool(false)
2 - 65
bool(true)
bool(false)
3 - 65
bool(true)
bool(false)
3 - 65
bool(true)
bool(false)
4 - 65
bool(true)
bool(false)
4 - 65
bool(true)
bool(false)
5 - 65
bool(true)
bool(false)
5 - 65
bool(true)
bool(false)
6 - 65
bool(true)
bool(false)
6 - 65
bool(true)
bool(false)
7 - 65
bool(true)
bool(false)
7 - 65
bool(true)
bool(false)
8 - 65
bool(true)
bool(false)
8 - 65
bool(true)
bool(false)
9 - 65
bool(true)
bool(false)
9 - 65
bool(true)
bool(false)
10 - 65
bool(true)
bool(false)
10 - 65
bool(true)
bool(false)
11 - 65
bool(true)
bool(false)
11 - 65
bool(true)
bool(false)
12 - 65
bool(true)
bool(false)
12 - 65
bool(true)
bool(false)
13 - 65
bool(true)
bool(false)
13 - 65
bool(true)
bool(false)
14 - 65
bool(true)
bool(false)
14 - 65
bool(true)
bool(false)
15 - 65
bool(true)
bool(false)
15 - 65
bool(true)
bool(false)
16 - 65
bool(true)
bool(false)
16 - 65
bool(true)
bool(false)
17 - 65
bool(true)
bool(false)
17 - 65
bool(true)
bool(false)
18 - 65
bool(true)
bool(false)
18 - 65
OK
