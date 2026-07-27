--TEST--
Diagonal Border Test
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

	foreach ($oClass->getConstants() as $c => $style) {
		if (strpos($c, 'BORDERSTYLE_') !== 0) {
			continue;
		}

		foreach ($oClass->getConstants() as $c2 => $color) {
			if (strpos($c2, 'COLOR_') !== 0) {
				continue;
			}

			if ($row > 200) {
				$x = new ExcelBook();
				$s = $x->addSheet("Sheet 1");
				$row = 1;
			}

			$format = $x->addFormat();
			$format->borderDiagonal(ExcelFormat::BORDERDIAGONAL_BOTH);
			$format->borderDiagonalStyle($style);
			$format->borderDiagonalColor($color);

			var_dump($s->write($row, 1, $data, $format));
			var_dump($x->getError());

			$fmt = null;
			$s->read($row, 1, $fmt);
			echo $fmt->borderDiagonal() . " - " . $fmt->borderDiagonalStyle() . " - " . $fmt->borderDiagonalColor() . "\n";

			$row++;
		}
	}



	echo "OK\n";
?>
--EXPECT--
bool(true)
bool(false)
3 - 0 - 8
bool(true)
bool(false)
3 - 0 - 9
bool(true)
bool(false)
3 - 0 - 10
bool(true)
bool(false)
3 - 0 - 11
bool(true)
bool(false)
3 - 0 - 12
bool(true)
bool(false)
3 - 0 - 13
bool(true)
bool(false)
3 - 0 - 14
bool(true)
bool(false)
3 - 0 - 15
bool(true)
bool(false)
3 - 0 - 16
bool(true)
bool(false)
3 - 0 - 17
bool(true)
bool(false)
3 - 0 - 18
bool(true)
bool(false)
3 - 0 - 19
bool(true)
bool(false)
3 - 0 - 20
bool(true)
bool(false)
3 - 0 - 21
bool(true)
bool(false)
3 - 0 - 22
bool(true)
bool(false)
3 - 0 - 23
bool(true)
bool(false)
3 - 0 - 24
bool(true)
bool(false)
3 - 0 - 25
bool(true)
bool(false)
3 - 0 - 26
bool(true)
bool(false)
3 - 0 - 27
bool(true)
bool(false)
3 - 0 - 28
bool(true)
bool(false)
3 - 0 - 29
bool(true)
bool(false)
3 - 0 - 30
bool(true)
bool(false)
3 - 0 - 31
bool(true)
bool(false)
3 - 0 - 32
bool(true)
bool(false)
3 - 0 - 33
bool(true)
bool(false)
3 - 0 - 34
bool(true)
bool(false)
3 - 0 - 35
bool(true)
bool(false)
3 - 0 - 36
bool(true)
bool(false)
3 - 0 - 37
bool(true)
bool(false)
3 - 0 - 38
bool(true)
bool(false)
3 - 0 - 39
bool(true)
bool(false)
3 - 0 - 40
bool(true)
bool(false)
3 - 0 - 41
bool(true)
bool(false)
3 - 0 - 42
bool(true)
bool(false)
3 - 0 - 43
bool(true)
bool(false)
3 - 0 - 44
bool(true)
bool(false)
3 - 0 - 45
bool(true)
bool(false)
3 - 0 - 46
bool(true)
bool(false)
3 - 0 - 47
bool(true)
bool(false)
3 - 0 - 48
bool(true)
bool(false)
3 - 0 - 49
bool(true)
bool(false)
3 - 0 - 50
bool(true)
bool(false)
3 - 0 - 51
bool(true)
bool(false)
3 - 0 - 52
bool(true)
bool(false)
3 - 0 - 53
bool(true)
bool(false)
3 - 0 - 54
bool(true)
bool(false)
3 - 0 - 55
bool(true)
bool(false)
3 - 0 - 56
bool(true)
bool(false)
3 - 0 - 57
bool(true)
bool(false)
3 - 0 - 58
bool(true)
bool(false)
3 - 0 - 59
bool(true)
bool(false)
3 - 0 - 60
bool(true)
bool(false)
3 - 0 - 61
bool(true)
bool(false)
3 - 0 - 62
bool(true)
bool(false)
3 - 0 - 63
bool(true)
bool(false)
3 - 0 - 64
bool(true)
bool(false)
3 - 0 - 65
bool(true)
bool(false)
3 - 1 - 8
bool(true)
bool(false)
3 - 1 - 9
bool(true)
bool(false)
3 - 1 - 10
bool(true)
bool(false)
3 - 1 - 11
bool(true)
bool(false)
3 - 1 - 12
bool(true)
bool(false)
3 - 1 - 13
bool(true)
bool(false)
3 - 1 - 14
bool(true)
bool(false)
3 - 1 - 15
bool(true)
bool(false)
3 - 1 - 16
bool(true)
bool(false)
3 - 1 - 17
bool(true)
bool(false)
3 - 1 - 18
bool(true)
bool(false)
3 - 1 - 19
bool(true)
bool(false)
3 - 1 - 20
bool(true)
bool(false)
3 - 1 - 21
bool(true)
bool(false)
3 - 1 - 22
bool(true)
bool(false)
3 - 1 - 23
bool(true)
bool(false)
3 - 1 - 24
bool(true)
bool(false)
3 - 1 - 25
bool(true)
bool(false)
3 - 1 - 26
bool(true)
bool(false)
3 - 1 - 27
bool(true)
bool(false)
3 - 1 - 28
bool(true)
bool(false)
3 - 1 - 29
bool(true)
bool(false)
3 - 1 - 30
bool(true)
bool(false)
3 - 1 - 31
bool(true)
bool(false)
3 - 1 - 32
bool(true)
bool(false)
3 - 1 - 33
bool(true)
bool(false)
3 - 1 - 34
bool(true)
bool(false)
3 - 1 - 35
bool(true)
bool(false)
3 - 1 - 36
bool(true)
bool(false)
3 - 1 - 37
bool(true)
bool(false)
3 - 1 - 38
bool(true)
bool(false)
3 - 1 - 39
bool(true)
bool(false)
3 - 1 - 40
bool(true)
bool(false)
3 - 1 - 41
bool(true)
bool(false)
3 - 1 - 42
bool(true)
bool(false)
3 - 1 - 43
bool(true)
bool(false)
3 - 1 - 44
bool(true)
bool(false)
3 - 1 - 45
bool(true)
bool(false)
3 - 1 - 46
bool(true)
bool(false)
3 - 1 - 47
bool(true)
bool(false)
3 - 1 - 48
bool(true)
bool(false)
3 - 1 - 49
bool(true)
bool(false)
3 - 1 - 50
bool(true)
bool(false)
3 - 1 - 51
bool(true)
bool(false)
3 - 1 - 52
bool(true)
bool(false)
3 - 1 - 53
bool(true)
bool(false)
3 - 1 - 54
bool(true)
bool(false)
3 - 1 - 55
bool(true)
bool(false)
3 - 1 - 56
bool(true)
bool(false)
3 - 1 - 57
bool(true)
bool(false)
3 - 1 - 58
bool(true)
bool(false)
3 - 1 - 59
bool(true)
bool(false)
3 - 1 - 60
bool(true)
bool(false)
3 - 1 - 61
bool(true)
bool(false)
3 - 1 - 62
bool(true)
bool(false)
3 - 1 - 63
bool(true)
bool(false)
3 - 1 - 64
bool(true)
bool(false)
3 - 1 - 65
bool(true)
bool(false)
3 - 2 - 8
bool(true)
bool(false)
3 - 2 - 9
bool(true)
bool(false)
3 - 2 - 10
bool(true)
bool(false)
3 - 2 - 11
bool(true)
bool(false)
3 - 2 - 12
bool(true)
bool(false)
3 - 2 - 13
bool(true)
bool(false)
3 - 2 - 14
bool(true)
bool(false)
3 - 2 - 15
bool(true)
bool(false)
3 - 2 - 16
bool(true)
bool(false)
3 - 2 - 17
bool(true)
bool(false)
3 - 2 - 18
bool(true)
bool(false)
3 - 2 - 19
bool(true)
bool(false)
3 - 2 - 20
bool(true)
bool(false)
3 - 2 - 21
bool(true)
bool(false)
3 - 2 - 22
bool(true)
bool(false)
3 - 2 - 23
bool(true)
bool(false)
3 - 2 - 24
bool(true)
bool(false)
3 - 2 - 25
bool(true)
bool(false)
3 - 2 - 26
bool(true)
bool(false)
3 - 2 - 27
bool(true)
bool(false)
3 - 2 - 28
bool(true)
bool(false)
3 - 2 - 29
bool(true)
bool(false)
3 - 2 - 30
bool(true)
bool(false)
3 - 2 - 31
bool(true)
bool(false)
3 - 2 - 32
bool(true)
bool(false)
3 - 2 - 33
bool(true)
bool(false)
3 - 2 - 34
bool(true)
bool(false)
3 - 2 - 35
bool(true)
bool(false)
3 - 2 - 36
bool(true)
bool(false)
3 - 2 - 37
bool(true)
bool(false)
3 - 2 - 38
bool(true)
bool(false)
3 - 2 - 39
bool(true)
bool(false)
3 - 2 - 40
bool(true)
bool(false)
3 - 2 - 41
bool(true)
bool(false)
3 - 2 - 42
bool(true)
bool(false)
3 - 2 - 43
bool(true)
bool(false)
3 - 2 - 44
bool(true)
bool(false)
3 - 2 - 45
bool(true)
bool(false)
3 - 2 - 46
bool(true)
bool(false)
3 - 2 - 47
bool(true)
bool(false)
3 - 2 - 48
bool(true)
bool(false)
3 - 2 - 49
bool(true)
bool(false)
3 - 2 - 50
bool(true)
bool(false)
3 - 2 - 51
bool(true)
bool(false)
3 - 2 - 52
bool(true)
bool(false)
3 - 2 - 53
bool(true)
bool(false)
3 - 2 - 54
bool(true)
bool(false)
3 - 2 - 55
bool(true)
bool(false)
3 - 2 - 56
bool(true)
bool(false)
3 - 2 - 57
bool(true)
bool(false)
3 - 2 - 58
bool(true)
bool(false)
3 - 2 - 59
bool(true)
bool(false)
3 - 2 - 60
bool(true)
bool(false)
3 - 2 - 61
bool(true)
bool(false)
3 - 2 - 62
bool(true)
bool(false)
3 - 2 - 63
bool(true)
bool(false)
3 - 2 - 64
bool(true)
bool(false)
3 - 2 - 65
bool(true)
bool(false)
3 - 3 - 8
bool(true)
bool(false)
3 - 3 - 9
bool(true)
bool(false)
3 - 3 - 10
bool(true)
bool(false)
3 - 3 - 11
bool(true)
bool(false)
3 - 3 - 12
bool(true)
bool(false)
3 - 3 - 13
bool(true)
bool(false)
3 - 3 - 14
bool(true)
bool(false)
3 - 3 - 15
bool(true)
bool(false)
3 - 3 - 16
bool(true)
bool(false)
3 - 3 - 17
bool(true)
bool(false)
3 - 3 - 18
bool(true)
bool(false)
3 - 3 - 19
bool(true)
bool(false)
3 - 3 - 20
bool(true)
bool(false)
3 - 3 - 21
bool(true)
bool(false)
3 - 3 - 22
bool(true)
bool(false)
3 - 3 - 23
bool(true)
bool(false)
3 - 3 - 24
bool(true)
bool(false)
3 - 3 - 25
bool(true)
bool(false)
3 - 3 - 26
bool(true)
bool(false)
3 - 3 - 27
bool(true)
bool(false)
3 - 3 - 28
bool(true)
bool(false)
3 - 3 - 29
bool(true)
bool(false)
3 - 3 - 30
bool(true)
bool(false)
3 - 3 - 31
bool(true)
bool(false)
3 - 3 - 32
bool(true)
bool(false)
3 - 3 - 33
bool(true)
bool(false)
3 - 3 - 34
bool(true)
bool(false)
3 - 3 - 35
bool(true)
bool(false)
3 - 3 - 36
bool(true)
bool(false)
3 - 3 - 37
bool(true)
bool(false)
3 - 3 - 38
bool(true)
bool(false)
3 - 3 - 39
bool(true)
bool(false)
3 - 3 - 40
bool(true)
bool(false)
3 - 3 - 41
bool(true)
bool(false)
3 - 3 - 42
bool(true)
bool(false)
3 - 3 - 43
bool(true)
bool(false)
3 - 3 - 44
bool(true)
bool(false)
3 - 3 - 45
bool(true)
bool(false)
3 - 3 - 46
bool(true)
bool(false)
3 - 3 - 47
bool(true)
bool(false)
3 - 3 - 48
bool(true)
bool(false)
3 - 3 - 49
bool(true)
bool(false)
3 - 3 - 50
bool(true)
bool(false)
3 - 3 - 51
bool(true)
bool(false)
3 - 3 - 52
bool(true)
bool(false)
3 - 3 - 53
bool(true)
bool(false)
3 - 3 - 54
bool(true)
bool(false)
3 - 3 - 55
bool(true)
bool(false)
3 - 3 - 56
bool(true)
bool(false)
3 - 3 - 57
bool(true)
bool(false)
3 - 3 - 58
bool(true)
bool(false)
3 - 3 - 59
bool(true)
bool(false)
3 - 3 - 60
bool(true)
bool(false)
3 - 3 - 61
bool(true)
bool(false)
3 - 3 - 62
bool(true)
bool(false)
3 - 3 - 63
bool(true)
bool(false)
3 - 3 - 64
bool(true)
bool(false)
3 - 3 - 65
bool(true)
bool(false)
3 - 4 - 8
bool(true)
bool(false)
3 - 4 - 9
bool(true)
bool(false)
3 - 4 - 10
bool(true)
bool(false)
3 - 4 - 11
bool(true)
bool(false)
3 - 4 - 12
bool(true)
bool(false)
3 - 4 - 13
bool(true)
bool(false)
3 - 4 - 14
bool(true)
bool(false)
3 - 4 - 15
bool(true)
bool(false)
3 - 4 - 16
bool(true)
bool(false)
3 - 4 - 17
bool(true)
bool(false)
3 - 4 - 18
bool(true)
bool(false)
3 - 4 - 19
bool(true)
bool(false)
3 - 4 - 20
bool(true)
bool(false)
3 - 4 - 21
bool(true)
bool(false)
3 - 4 - 22
bool(true)
bool(false)
3 - 4 - 23
bool(true)
bool(false)
3 - 4 - 24
bool(true)
bool(false)
3 - 4 - 25
bool(true)
bool(false)
3 - 4 - 26
bool(true)
bool(false)
3 - 4 - 27
bool(true)
bool(false)
3 - 4 - 28
bool(true)
bool(false)
3 - 4 - 29
bool(true)
bool(false)
3 - 4 - 30
bool(true)
bool(false)
3 - 4 - 31
bool(true)
bool(false)
3 - 4 - 32
bool(true)
bool(false)
3 - 4 - 33
bool(true)
bool(false)
3 - 4 - 34
bool(true)
bool(false)
3 - 4 - 35
bool(true)
bool(false)
3 - 4 - 36
bool(true)
bool(false)
3 - 4 - 37
bool(true)
bool(false)
3 - 4 - 38
bool(true)
bool(false)
3 - 4 - 39
bool(true)
bool(false)
3 - 4 - 40
bool(true)
bool(false)
3 - 4 - 41
bool(true)
bool(false)
3 - 4 - 42
bool(true)
bool(false)
3 - 4 - 43
bool(true)
bool(false)
3 - 4 - 44
bool(true)
bool(false)
3 - 4 - 45
bool(true)
bool(false)
3 - 4 - 46
bool(true)
bool(false)
3 - 4 - 47
bool(true)
bool(false)
3 - 4 - 48
bool(true)
bool(false)
3 - 4 - 49
bool(true)
bool(false)
3 - 4 - 50
bool(true)
bool(false)
3 - 4 - 51
bool(true)
bool(false)
3 - 4 - 52
bool(true)
bool(false)
3 - 4 - 53
bool(true)
bool(false)
3 - 4 - 54
bool(true)
bool(false)
3 - 4 - 55
bool(true)
bool(false)
3 - 4 - 56
bool(true)
bool(false)
3 - 4 - 57
bool(true)
bool(false)
3 - 4 - 58
bool(true)
bool(false)
3 - 4 - 59
bool(true)
bool(false)
3 - 4 - 60
bool(true)
bool(false)
3 - 4 - 61
bool(true)
bool(false)
3 - 4 - 62
bool(true)
bool(false)
3 - 4 - 63
bool(true)
bool(false)
3 - 4 - 64
bool(true)
bool(false)
3 - 4 - 65
bool(true)
bool(false)
3 - 5 - 8
bool(true)
bool(false)
3 - 5 - 9
bool(true)
bool(false)
3 - 5 - 10
bool(true)
bool(false)
3 - 5 - 11
bool(true)
bool(false)
3 - 5 - 12
bool(true)
bool(false)
3 - 5 - 13
bool(true)
bool(false)
3 - 5 - 14
bool(true)
bool(false)
3 - 5 - 15
bool(true)
bool(false)
3 - 5 - 16
bool(true)
bool(false)
3 - 5 - 17
bool(true)
bool(false)
3 - 5 - 18
bool(true)
bool(false)
3 - 5 - 19
bool(true)
bool(false)
3 - 5 - 20
bool(true)
bool(false)
3 - 5 - 21
bool(true)
bool(false)
3 - 5 - 22
bool(true)
bool(false)
3 - 5 - 23
bool(true)
bool(false)
3 - 5 - 24
bool(true)
bool(false)
3 - 5 - 25
bool(true)
bool(false)
3 - 5 - 26
bool(true)
bool(false)
3 - 5 - 27
bool(true)
bool(false)
3 - 5 - 28
bool(true)
bool(false)
3 - 5 - 29
bool(true)
bool(false)
3 - 5 - 30
bool(true)
bool(false)
3 - 5 - 31
bool(true)
bool(false)
3 - 5 - 32
bool(true)
bool(false)
3 - 5 - 33
bool(true)
bool(false)
3 - 5 - 34
bool(true)
bool(false)
3 - 5 - 35
bool(true)
bool(false)
3 - 5 - 36
bool(true)
bool(false)
3 - 5 - 37
bool(true)
bool(false)
3 - 5 - 38
bool(true)
bool(false)
3 - 5 - 39
bool(true)
bool(false)
3 - 5 - 40
bool(true)
bool(false)
3 - 5 - 41
bool(true)
bool(false)
3 - 5 - 42
bool(true)
bool(false)
3 - 5 - 43
bool(true)
bool(false)
3 - 5 - 44
bool(true)
bool(false)
3 - 5 - 45
bool(true)
bool(false)
3 - 5 - 46
bool(true)
bool(false)
3 - 5 - 47
bool(true)
bool(false)
3 - 5 - 48
bool(true)
bool(false)
3 - 5 - 49
bool(true)
bool(false)
3 - 5 - 50
bool(true)
bool(false)
3 - 5 - 51
bool(true)
bool(false)
3 - 5 - 52
bool(true)
bool(false)
3 - 5 - 53
bool(true)
bool(false)
3 - 5 - 54
bool(true)
bool(false)
3 - 5 - 55
bool(true)
bool(false)
3 - 5 - 56
bool(true)
bool(false)
3 - 5 - 57
bool(true)
bool(false)
3 - 5 - 58
bool(true)
bool(false)
3 - 5 - 59
bool(true)
bool(false)
3 - 5 - 60
bool(true)
bool(false)
3 - 5 - 61
bool(true)
bool(false)
3 - 5 - 62
bool(true)
bool(false)
3 - 5 - 63
bool(true)
bool(false)
3 - 5 - 64
bool(true)
bool(false)
3 - 5 - 65
bool(true)
bool(false)
3 - 6 - 8
bool(true)
bool(false)
3 - 6 - 9
bool(true)
bool(false)
3 - 6 - 10
bool(true)
bool(false)
3 - 6 - 11
bool(true)
bool(false)
3 - 6 - 12
bool(true)
bool(false)
3 - 6 - 13
bool(true)
bool(false)
3 - 6 - 14
bool(true)
bool(false)
3 - 6 - 15
bool(true)
bool(false)
3 - 6 - 16
bool(true)
bool(false)
3 - 6 - 17
bool(true)
bool(false)
3 - 6 - 18
bool(true)
bool(false)
3 - 6 - 19
bool(true)
bool(false)
3 - 6 - 20
bool(true)
bool(false)
3 - 6 - 21
bool(true)
bool(false)
3 - 6 - 22
bool(true)
bool(false)
3 - 6 - 23
bool(true)
bool(false)
3 - 6 - 24
bool(true)
bool(false)
3 - 6 - 25
bool(true)
bool(false)
3 - 6 - 26
bool(true)
bool(false)
3 - 6 - 27
bool(true)
bool(false)
3 - 6 - 28
bool(true)
bool(false)
3 - 6 - 29
bool(true)
bool(false)
3 - 6 - 30
bool(true)
bool(false)
3 - 6 - 31
bool(true)
bool(false)
3 - 6 - 32
bool(true)
bool(false)
3 - 6 - 33
bool(true)
bool(false)
3 - 6 - 34
bool(true)
bool(false)
3 - 6 - 35
bool(true)
bool(false)
3 - 6 - 36
bool(true)
bool(false)
3 - 6 - 37
bool(true)
bool(false)
3 - 6 - 38
bool(true)
bool(false)
3 - 6 - 39
bool(true)
bool(false)
3 - 6 - 40
bool(true)
bool(false)
3 - 6 - 41
bool(true)
bool(false)
3 - 6 - 42
bool(true)
bool(false)
3 - 6 - 43
bool(true)
bool(false)
3 - 6 - 44
bool(true)
bool(false)
3 - 6 - 45
bool(true)
bool(false)
3 - 6 - 46
bool(true)
bool(false)
3 - 6 - 47
bool(true)
bool(false)
3 - 6 - 48
bool(true)
bool(false)
3 - 6 - 49
bool(true)
bool(false)
3 - 6 - 50
bool(true)
bool(false)
3 - 6 - 51
bool(true)
bool(false)
3 - 6 - 52
bool(true)
bool(false)
3 - 6 - 53
bool(true)
bool(false)
3 - 6 - 54
bool(true)
bool(false)
3 - 6 - 55
bool(true)
bool(false)
3 - 6 - 56
bool(true)
bool(false)
3 - 6 - 57
bool(true)
bool(false)
3 - 6 - 58
bool(true)
bool(false)
3 - 6 - 59
bool(true)
bool(false)
3 - 6 - 60
bool(true)
bool(false)
3 - 6 - 61
bool(true)
bool(false)
3 - 6 - 62
bool(true)
bool(false)
3 - 6 - 63
bool(true)
bool(false)
3 - 6 - 64
bool(true)
bool(false)
3 - 6 - 65
bool(true)
bool(false)
3 - 7 - 8
bool(true)
bool(false)
3 - 7 - 9
bool(true)
bool(false)
3 - 7 - 10
bool(true)
bool(false)
3 - 7 - 11
bool(true)
bool(false)
3 - 7 - 12
bool(true)
bool(false)
3 - 7 - 13
bool(true)
bool(false)
3 - 7 - 14
bool(true)
bool(false)
3 - 7 - 15
bool(true)
bool(false)
3 - 7 - 16
bool(true)
bool(false)
3 - 7 - 17
bool(true)
bool(false)
3 - 7 - 18
bool(true)
bool(false)
3 - 7 - 19
bool(true)
bool(false)
3 - 7 - 20
bool(true)
bool(false)
3 - 7 - 21
bool(true)
bool(false)
3 - 7 - 22
bool(true)
bool(false)
3 - 7 - 23
bool(true)
bool(false)
3 - 7 - 24
bool(true)
bool(false)
3 - 7 - 25
bool(true)
bool(false)
3 - 7 - 26
bool(true)
bool(false)
3 - 7 - 27
bool(true)
bool(false)
3 - 7 - 28
bool(true)
bool(false)
3 - 7 - 29
bool(true)
bool(false)
3 - 7 - 30
bool(true)
bool(false)
3 - 7 - 31
bool(true)
bool(false)
3 - 7 - 32
bool(true)
bool(false)
3 - 7 - 33
bool(true)
bool(false)
3 - 7 - 34
bool(true)
bool(false)
3 - 7 - 35
bool(true)
bool(false)
3 - 7 - 36
bool(true)
bool(false)
3 - 7 - 37
bool(true)
bool(false)
3 - 7 - 38
bool(true)
bool(false)
3 - 7 - 39
bool(true)
bool(false)
3 - 7 - 40
bool(true)
bool(false)
3 - 7 - 41
bool(true)
bool(false)
3 - 7 - 42
bool(true)
bool(false)
3 - 7 - 43
bool(true)
bool(false)
3 - 7 - 44
bool(true)
bool(false)
3 - 7 - 45
bool(true)
bool(false)
3 - 7 - 46
bool(true)
bool(false)
3 - 7 - 47
bool(true)
bool(false)
3 - 7 - 48
bool(true)
bool(false)
3 - 7 - 49
bool(true)
bool(false)
3 - 7 - 50
bool(true)
bool(false)
3 - 7 - 51
bool(true)
bool(false)
3 - 7 - 52
bool(true)
bool(false)
3 - 7 - 53
bool(true)
bool(false)
3 - 7 - 54
bool(true)
bool(false)
3 - 7 - 55
bool(true)
bool(false)
3 - 7 - 56
bool(true)
bool(false)
3 - 7 - 57
bool(true)
bool(false)
3 - 7 - 58
bool(true)
bool(false)
3 - 7 - 59
bool(true)
bool(false)
3 - 7 - 60
bool(true)
bool(false)
3 - 7 - 61
bool(true)
bool(false)
3 - 7 - 62
bool(true)
bool(false)
3 - 7 - 63
bool(true)
bool(false)
3 - 7 - 64
bool(true)
bool(false)
3 - 7 - 65
bool(true)
bool(false)
3 - 8 - 8
bool(true)
bool(false)
3 - 8 - 9
bool(true)
bool(false)
3 - 8 - 10
bool(true)
bool(false)
3 - 8 - 11
bool(true)
bool(false)
3 - 8 - 12
bool(true)
bool(false)
3 - 8 - 13
bool(true)
bool(false)
3 - 8 - 14
bool(true)
bool(false)
3 - 8 - 15
bool(true)
bool(false)
3 - 8 - 16
bool(true)
bool(false)
3 - 8 - 17
bool(true)
bool(false)
3 - 8 - 18
bool(true)
bool(false)
3 - 8 - 19
bool(true)
bool(false)
3 - 8 - 20
bool(true)
bool(false)
3 - 8 - 21
bool(true)
bool(false)
3 - 8 - 22
bool(true)
bool(false)
3 - 8 - 23
bool(true)
bool(false)
3 - 8 - 24
bool(true)
bool(false)
3 - 8 - 25
bool(true)
bool(false)
3 - 8 - 26
bool(true)
bool(false)
3 - 8 - 27
bool(true)
bool(false)
3 - 8 - 28
bool(true)
bool(false)
3 - 8 - 29
bool(true)
bool(false)
3 - 8 - 30
bool(true)
bool(false)
3 - 8 - 31
bool(true)
bool(false)
3 - 8 - 32
bool(true)
bool(false)
3 - 8 - 33
bool(true)
bool(false)
3 - 8 - 34
bool(true)
bool(false)
3 - 8 - 35
bool(true)
bool(false)
3 - 8 - 36
bool(true)
bool(false)
3 - 8 - 37
bool(true)
bool(false)
3 - 8 - 38
bool(true)
bool(false)
3 - 8 - 39
bool(true)
bool(false)
3 - 8 - 40
bool(true)
bool(false)
3 - 8 - 41
bool(true)
bool(false)
3 - 8 - 42
bool(true)
bool(false)
3 - 8 - 43
bool(true)
bool(false)
3 - 8 - 44
bool(true)
bool(false)
3 - 8 - 45
bool(true)
bool(false)
3 - 8 - 46
bool(true)
bool(false)
3 - 8 - 47
bool(true)
bool(false)
3 - 8 - 48
bool(true)
bool(false)
3 - 8 - 49
bool(true)
bool(false)
3 - 8 - 50
bool(true)
bool(false)
3 - 8 - 51
bool(true)
bool(false)
3 - 8 - 52
bool(true)
bool(false)
3 - 8 - 53
bool(true)
bool(false)
3 - 8 - 54
bool(true)
bool(false)
3 - 8 - 55
bool(true)
bool(false)
3 - 8 - 56
bool(true)
bool(false)
3 - 8 - 57
bool(true)
bool(false)
3 - 8 - 58
bool(true)
bool(false)
3 - 8 - 59
bool(true)
bool(false)
3 - 8 - 60
bool(true)
bool(false)
3 - 8 - 61
bool(true)
bool(false)
3 - 8 - 62
bool(true)
bool(false)
3 - 8 - 63
bool(true)
bool(false)
3 - 8 - 64
bool(true)
bool(false)
3 - 8 - 65
bool(true)
bool(false)
3 - 9 - 8
bool(true)
bool(false)
3 - 9 - 9
bool(true)
bool(false)
3 - 9 - 10
bool(true)
bool(false)
3 - 9 - 11
bool(true)
bool(false)
3 - 9 - 12
bool(true)
bool(false)
3 - 9 - 13
bool(true)
bool(false)
3 - 9 - 14
bool(true)
bool(false)
3 - 9 - 15
bool(true)
bool(false)
3 - 9 - 16
bool(true)
bool(false)
3 - 9 - 17
bool(true)
bool(false)
3 - 9 - 18
bool(true)
bool(false)
3 - 9 - 19
bool(true)
bool(false)
3 - 9 - 20
bool(true)
bool(false)
3 - 9 - 21
bool(true)
bool(false)
3 - 9 - 22
bool(true)
bool(false)
3 - 9 - 23
bool(true)
bool(false)
3 - 9 - 24
bool(true)
bool(false)
3 - 9 - 25
bool(true)
bool(false)
3 - 9 - 26
bool(true)
bool(false)
3 - 9 - 27
bool(true)
bool(false)
3 - 9 - 28
bool(true)
bool(false)
3 - 9 - 29
bool(true)
bool(false)
3 - 9 - 30
bool(true)
bool(false)
3 - 9 - 31
bool(true)
bool(false)
3 - 9 - 32
bool(true)
bool(false)
3 - 9 - 33
bool(true)
bool(false)
3 - 9 - 34
bool(true)
bool(false)
3 - 9 - 35
bool(true)
bool(false)
3 - 9 - 36
bool(true)
bool(false)
3 - 9 - 37
bool(true)
bool(false)
3 - 9 - 38
bool(true)
bool(false)
3 - 9 - 39
bool(true)
bool(false)
3 - 9 - 40
bool(true)
bool(false)
3 - 9 - 41
bool(true)
bool(false)
3 - 9 - 42
bool(true)
bool(false)
3 - 9 - 43
bool(true)
bool(false)
3 - 9 - 44
bool(true)
bool(false)
3 - 9 - 45
bool(true)
bool(false)
3 - 9 - 46
bool(true)
bool(false)
3 - 9 - 47
bool(true)
bool(false)
3 - 9 - 48
bool(true)
bool(false)
3 - 9 - 49
bool(true)
bool(false)
3 - 9 - 50
bool(true)
bool(false)
3 - 9 - 51
bool(true)
bool(false)
3 - 9 - 52
bool(true)
bool(false)
3 - 9 - 53
bool(true)
bool(false)
3 - 9 - 54
bool(true)
bool(false)
3 - 9 - 55
bool(true)
bool(false)
3 - 9 - 56
bool(true)
bool(false)
3 - 9 - 57
bool(true)
bool(false)
3 - 9 - 58
bool(true)
bool(false)
3 - 9 - 59
bool(true)
bool(false)
3 - 9 - 60
bool(true)
bool(false)
3 - 9 - 61
bool(true)
bool(false)
3 - 9 - 62
bool(true)
bool(false)
3 - 9 - 63
bool(true)
bool(false)
3 - 9 - 64
bool(true)
bool(false)
3 - 9 - 65
bool(true)
bool(false)
3 - 10 - 8
bool(true)
bool(false)
3 - 10 - 9
bool(true)
bool(false)
3 - 10 - 10
bool(true)
bool(false)
3 - 10 - 11
bool(true)
bool(false)
3 - 10 - 12
bool(true)
bool(false)
3 - 10 - 13
bool(true)
bool(false)
3 - 10 - 14
bool(true)
bool(false)
3 - 10 - 15
bool(true)
bool(false)
3 - 10 - 16
bool(true)
bool(false)
3 - 10 - 17
bool(true)
bool(false)
3 - 10 - 18
bool(true)
bool(false)
3 - 10 - 19
bool(true)
bool(false)
3 - 10 - 20
bool(true)
bool(false)
3 - 10 - 21
bool(true)
bool(false)
3 - 10 - 22
bool(true)
bool(false)
3 - 10 - 23
bool(true)
bool(false)
3 - 10 - 24
bool(true)
bool(false)
3 - 10 - 25
bool(true)
bool(false)
3 - 10 - 26
bool(true)
bool(false)
3 - 10 - 27
bool(true)
bool(false)
3 - 10 - 28
bool(true)
bool(false)
3 - 10 - 29
bool(true)
bool(false)
3 - 10 - 30
bool(true)
bool(false)
3 - 10 - 31
bool(true)
bool(false)
3 - 10 - 32
bool(true)
bool(false)
3 - 10 - 33
bool(true)
bool(false)
3 - 10 - 34
bool(true)
bool(false)
3 - 10 - 35
bool(true)
bool(false)
3 - 10 - 36
bool(true)
bool(false)
3 - 10 - 37
bool(true)
bool(false)
3 - 10 - 38
bool(true)
bool(false)
3 - 10 - 39
bool(true)
bool(false)
3 - 10 - 40
bool(true)
bool(false)
3 - 10 - 41
bool(true)
bool(false)
3 - 10 - 42
bool(true)
bool(false)
3 - 10 - 43
bool(true)
bool(false)
3 - 10 - 44
bool(true)
bool(false)
3 - 10 - 45
bool(true)
bool(false)
3 - 10 - 46
bool(true)
bool(false)
3 - 10 - 47
bool(true)
bool(false)
3 - 10 - 48
bool(true)
bool(false)
3 - 10 - 49
bool(true)
bool(false)
3 - 10 - 50
bool(true)
bool(false)
3 - 10 - 51
bool(true)
bool(false)
3 - 10 - 52
bool(true)
bool(false)
3 - 10 - 53
bool(true)
bool(false)
3 - 10 - 54
bool(true)
bool(false)
3 - 10 - 55
bool(true)
bool(false)
3 - 10 - 56
bool(true)
bool(false)
3 - 10 - 57
bool(true)
bool(false)
3 - 10 - 58
bool(true)
bool(false)
3 - 10 - 59
bool(true)
bool(false)
3 - 10 - 60
bool(true)
bool(false)
3 - 10 - 61
bool(true)
bool(false)
3 - 10 - 62
bool(true)
bool(false)
3 - 10 - 63
bool(true)
bool(false)
3 - 10 - 64
bool(true)
bool(false)
3 - 10 - 65
bool(true)
bool(false)
3 - 11 - 8
bool(true)
bool(false)
3 - 11 - 9
bool(true)
bool(false)
3 - 11 - 10
bool(true)
bool(false)
3 - 11 - 11
bool(true)
bool(false)
3 - 11 - 12
bool(true)
bool(false)
3 - 11 - 13
bool(true)
bool(false)
3 - 11 - 14
bool(true)
bool(false)
3 - 11 - 15
bool(true)
bool(false)
3 - 11 - 16
bool(true)
bool(false)
3 - 11 - 17
bool(true)
bool(false)
3 - 11 - 18
bool(true)
bool(false)
3 - 11 - 19
bool(true)
bool(false)
3 - 11 - 20
bool(true)
bool(false)
3 - 11 - 21
bool(true)
bool(false)
3 - 11 - 22
bool(true)
bool(false)
3 - 11 - 23
bool(true)
bool(false)
3 - 11 - 24
bool(true)
bool(false)
3 - 11 - 25
bool(true)
bool(false)
3 - 11 - 26
bool(true)
bool(false)
3 - 11 - 27
bool(true)
bool(false)
3 - 11 - 28
bool(true)
bool(false)
3 - 11 - 29
bool(true)
bool(false)
3 - 11 - 30
bool(true)
bool(false)
3 - 11 - 31
bool(true)
bool(false)
3 - 11 - 32
bool(true)
bool(false)
3 - 11 - 33
bool(true)
bool(false)
3 - 11 - 34
bool(true)
bool(false)
3 - 11 - 35
bool(true)
bool(false)
3 - 11 - 36
bool(true)
bool(false)
3 - 11 - 37
bool(true)
bool(false)
3 - 11 - 38
bool(true)
bool(false)
3 - 11 - 39
bool(true)
bool(false)
3 - 11 - 40
bool(true)
bool(false)
3 - 11 - 41
bool(true)
bool(false)
3 - 11 - 42
bool(true)
bool(false)
3 - 11 - 43
bool(true)
bool(false)
3 - 11 - 44
bool(true)
bool(false)
3 - 11 - 45
bool(true)
bool(false)
3 - 11 - 46
bool(true)
bool(false)
3 - 11 - 47
bool(true)
bool(false)
3 - 11 - 48
bool(true)
bool(false)
3 - 11 - 49
bool(true)
bool(false)
3 - 11 - 50
bool(true)
bool(false)
3 - 11 - 51
bool(true)
bool(false)
3 - 11 - 52
bool(true)
bool(false)
3 - 11 - 53
bool(true)
bool(false)
3 - 11 - 54
bool(true)
bool(false)
3 - 11 - 55
bool(true)
bool(false)
3 - 11 - 56
bool(true)
bool(false)
3 - 11 - 57
bool(true)
bool(false)
3 - 11 - 58
bool(true)
bool(false)
3 - 11 - 59
bool(true)
bool(false)
3 - 11 - 60
bool(true)
bool(false)
3 - 11 - 61
bool(true)
bool(false)
3 - 11 - 62
bool(true)
bool(false)
3 - 11 - 63
bool(true)
bool(false)
3 - 11 - 64
bool(true)
bool(false)
3 - 11 - 65
bool(true)
bool(false)
3 - 12 - 8
bool(true)
bool(false)
3 - 12 - 9
bool(true)
bool(false)
3 - 12 - 10
bool(true)
bool(false)
3 - 12 - 11
bool(true)
bool(false)
3 - 12 - 12
bool(true)
bool(false)
3 - 12 - 13
bool(true)
bool(false)
3 - 12 - 14
bool(true)
bool(false)
3 - 12 - 15
bool(true)
bool(false)
3 - 12 - 16
bool(true)
bool(false)
3 - 12 - 17
bool(true)
bool(false)
3 - 12 - 18
bool(true)
bool(false)
3 - 12 - 19
bool(true)
bool(false)
3 - 12 - 20
bool(true)
bool(false)
3 - 12 - 21
bool(true)
bool(false)
3 - 12 - 22
bool(true)
bool(false)
3 - 12 - 23
bool(true)
bool(false)
3 - 12 - 24
bool(true)
bool(false)
3 - 12 - 25
bool(true)
bool(false)
3 - 12 - 26
bool(true)
bool(false)
3 - 12 - 27
bool(true)
bool(false)
3 - 12 - 28
bool(true)
bool(false)
3 - 12 - 29
bool(true)
bool(false)
3 - 12 - 30
bool(true)
bool(false)
3 - 12 - 31
bool(true)
bool(false)
3 - 12 - 32
bool(true)
bool(false)
3 - 12 - 33
bool(true)
bool(false)
3 - 12 - 34
bool(true)
bool(false)
3 - 12 - 35
bool(true)
bool(false)
3 - 12 - 36
bool(true)
bool(false)
3 - 12 - 37
bool(true)
bool(false)
3 - 12 - 38
bool(true)
bool(false)
3 - 12 - 39
bool(true)
bool(false)
3 - 12 - 40
bool(true)
bool(false)
3 - 12 - 41
bool(true)
bool(false)
3 - 12 - 42
bool(true)
bool(false)
3 - 12 - 43
bool(true)
bool(false)
3 - 12 - 44
bool(true)
bool(false)
3 - 12 - 45
bool(true)
bool(false)
3 - 12 - 46
bool(true)
bool(false)
3 - 12 - 47
bool(true)
bool(false)
3 - 12 - 48
bool(true)
bool(false)
3 - 12 - 49
bool(true)
bool(false)
3 - 12 - 50
bool(true)
bool(false)
3 - 12 - 51
bool(true)
bool(false)
3 - 12 - 52
bool(true)
bool(false)
3 - 12 - 53
bool(true)
bool(false)
3 - 12 - 54
bool(true)
bool(false)
3 - 12 - 55
bool(true)
bool(false)
3 - 12 - 56
bool(true)
bool(false)
3 - 12 - 57
bool(true)
bool(false)
3 - 12 - 58
bool(true)
bool(false)
3 - 12 - 59
bool(true)
bool(false)
3 - 12 - 60
bool(true)
bool(false)
3 - 12 - 61
bool(true)
bool(false)
3 - 12 - 62
bool(true)
bool(false)
3 - 12 - 63
bool(true)
bool(false)
3 - 12 - 64
bool(true)
bool(false)
3 - 12 - 65
bool(true)
bool(false)
3 - 13 - 8
bool(true)
bool(false)
3 - 13 - 9
bool(true)
bool(false)
3 - 13 - 10
bool(true)
bool(false)
3 - 13 - 11
bool(true)
bool(false)
3 - 13 - 12
bool(true)
bool(false)
3 - 13 - 13
bool(true)
bool(false)
3 - 13 - 14
bool(true)
bool(false)
3 - 13 - 15
bool(true)
bool(false)
3 - 13 - 16
bool(true)
bool(false)
3 - 13 - 17
bool(true)
bool(false)
3 - 13 - 18
bool(true)
bool(false)
3 - 13 - 19
bool(true)
bool(false)
3 - 13 - 20
bool(true)
bool(false)
3 - 13 - 21
bool(true)
bool(false)
3 - 13 - 22
bool(true)
bool(false)
3 - 13 - 23
bool(true)
bool(false)
3 - 13 - 24
bool(true)
bool(false)
3 - 13 - 25
bool(true)
bool(false)
3 - 13 - 26
bool(true)
bool(false)
3 - 13 - 27
bool(true)
bool(false)
3 - 13 - 28
bool(true)
bool(false)
3 - 13 - 29
bool(true)
bool(false)
3 - 13 - 30
bool(true)
bool(false)
3 - 13 - 31
bool(true)
bool(false)
3 - 13 - 32
bool(true)
bool(false)
3 - 13 - 33
bool(true)
bool(false)
3 - 13 - 34
bool(true)
bool(false)
3 - 13 - 35
bool(true)
bool(false)
3 - 13 - 36
bool(true)
bool(false)
3 - 13 - 37
bool(true)
bool(false)
3 - 13 - 38
bool(true)
bool(false)
3 - 13 - 39
bool(true)
bool(false)
3 - 13 - 40
bool(true)
bool(false)
3 - 13 - 41
bool(true)
bool(false)
3 - 13 - 42
bool(true)
bool(false)
3 - 13 - 43
bool(true)
bool(false)
3 - 13 - 44
bool(true)
bool(false)
3 - 13 - 45
bool(true)
bool(false)
3 - 13 - 46
bool(true)
bool(false)
3 - 13 - 47
bool(true)
bool(false)
3 - 13 - 48
bool(true)
bool(false)
3 - 13 - 49
bool(true)
bool(false)
3 - 13 - 50
bool(true)
bool(false)
3 - 13 - 51
bool(true)
bool(false)
3 - 13 - 52
bool(true)
bool(false)
3 - 13 - 53
bool(true)
bool(false)
3 - 13 - 54
bool(true)
bool(false)
3 - 13 - 55
bool(true)
bool(false)
3 - 13 - 56
bool(true)
bool(false)
3 - 13 - 57
bool(true)
bool(false)
3 - 13 - 58
bool(true)
bool(false)
3 - 13 - 59
bool(true)
bool(false)
3 - 13 - 60
bool(true)
bool(false)
3 - 13 - 61
bool(true)
bool(false)
3 - 13 - 62
bool(true)
bool(false)
3 - 13 - 63
bool(true)
bool(false)
3 - 13 - 64
bool(true)
bool(false)
3 - 13 - 65
OK