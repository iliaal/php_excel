--TEST--
Diagonal Border Test
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

	foreach ($oClass->getConstants() as $c => $style) {
		if (strpos($c, 'BORDERSTYLE_') !== 0) {
			continue;
		}
        
        // bypass LibXL trial limitations
        $x = new ExcelBook();
        $s = $x->addSheet("Sheet 1");
		
		foreach ($oClass->getConstants() as $c2 => $color) {
			if (strpos($c2, 'COLOR_') !== 0) {
				continue;
			}
			
			$format = $x->addFormat();
			$format->borderDiagonalStyle($style);
			$format->borderDiagonalColor($color);

			var_dump($s->write($row, 1, $data, $format));
			var_dump($x->getError());

			$fmt = null;
			$s->read($row, 1, $fmt);
			echo $fmt->borderDiagonalStyle() . " - " . $fmt->borderDiagonalColor() . "\n";

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
0 - 9
bool(true)
bool(false)
0 - 10
bool(true)
bool(false)
0 - 11
bool(true)
bool(false)
0 - 12
bool(true)
bool(false)
0 - 13
bool(true)
bool(false)
0 - 14
bool(true)
bool(false)
0 - 15
bool(true)
bool(false)
0 - 16
bool(true)
bool(false)
0 - 17
bool(true)
bool(false)
0 - 18
bool(true)
bool(false)
0 - 19
bool(true)
bool(false)
0 - 20
bool(true)
bool(false)
0 - 21
bool(true)
bool(false)
0 - 22
bool(true)
bool(false)
0 - 23
bool(true)
bool(false)
0 - 24
bool(true)
bool(false)
0 - 25
bool(true)
bool(false)
0 - 26
bool(true)
bool(false)
0 - 27
bool(true)
bool(false)
0 - 28
bool(true)
bool(false)
0 - 29
bool(true)
bool(false)
0 - 30
bool(true)
bool(false)
0 - 31
bool(true)
bool(false)
0 - 32
bool(true)
bool(false)
0 - 33
bool(true)
bool(false)
0 - 34
bool(true)
bool(false)
0 - 35
bool(true)
bool(false)
0 - 36
bool(true)
bool(false)
0 - 37
bool(true)
bool(false)
0 - 38
bool(true)
bool(false)
0 - 39
bool(true)
bool(false)
0 - 40
bool(true)
bool(false)
0 - 41
bool(true)
bool(false)
0 - 42
bool(true)
bool(false)
0 - 43
bool(true)
bool(false)
0 - 44
bool(true)
bool(false)
0 - 45
bool(true)
bool(false)
0 - 46
bool(true)
bool(false)
0 - 47
bool(true)
bool(false)
0 - 48
bool(true)
bool(false)
0 - 49
bool(true)
bool(false)
0 - 50
bool(true)
bool(false)
0 - 51
bool(true)
bool(false)
0 - 52
bool(true)
bool(false)
0 - 53
bool(true)
bool(false)
0 - 54
bool(true)
bool(false)
0 - 55
bool(true)
bool(false)
0 - 56
bool(true)
bool(false)
0 - 57
bool(true)
bool(false)
0 - 58
bool(true)
bool(false)
0 - 59
bool(true)
bool(false)
0 - 60
bool(true)
bool(false)
0 - 61
bool(true)
bool(false)
0 - 62
bool(true)
bool(false)
0 - 63
bool(true)
bool(false)
0 - 64
bool(true)
bool(false)
0 - 65
bool(true)
bool(false)
1 - 8
bool(true)
bool(false)
1 - 9
bool(true)
bool(false)
1 - 10
bool(true)
bool(false)
1 - 11
bool(true)
bool(false)
1 - 12
bool(true)
bool(false)
1 - 13
bool(true)
bool(false)
1 - 14
bool(true)
bool(false)
1 - 15
bool(true)
bool(false)
1 - 16
bool(true)
bool(false)
1 - 17
bool(true)
bool(false)
1 - 18
bool(true)
bool(false)
1 - 19
bool(true)
bool(false)
1 - 20
bool(true)
bool(false)
1 - 21
bool(true)
bool(false)
1 - 22
bool(true)
bool(false)
1 - 23
bool(true)
bool(false)
1 - 24
bool(true)
bool(false)
1 - 25
bool(true)
bool(false)
1 - 26
bool(true)
bool(false)
1 - 27
bool(true)
bool(false)
1 - 28
bool(true)
bool(false)
1 - 29
bool(true)
bool(false)
1 - 30
bool(true)
bool(false)
1 - 31
bool(true)
bool(false)
1 - 32
bool(true)
bool(false)
1 - 33
bool(true)
bool(false)
1 - 34
bool(true)
bool(false)
1 - 35
bool(true)
bool(false)
1 - 36
bool(true)
bool(false)
1 - 37
bool(true)
bool(false)
1 - 38
bool(true)
bool(false)
1 - 39
bool(true)
bool(false)
1 - 40
bool(true)
bool(false)
1 - 41
bool(true)
bool(false)
1 - 42
bool(true)
bool(false)
1 - 43
bool(true)
bool(false)
1 - 44
bool(true)
bool(false)
1 - 45
bool(true)
bool(false)
1 - 46
bool(true)
bool(false)
1 - 47
bool(true)
bool(false)
1 - 48
bool(true)
bool(false)
1 - 49
bool(true)
bool(false)
1 - 50
bool(true)
bool(false)
1 - 51
bool(true)
bool(false)
1 - 52
bool(true)
bool(false)
1 - 53
bool(true)
bool(false)
1 - 54
bool(true)
bool(false)
1 - 55
bool(true)
bool(false)
1 - 56
bool(true)
bool(false)
1 - 57
bool(true)
bool(false)
1 - 58
bool(true)
bool(false)
1 - 59
bool(true)
bool(false)
1 - 60
bool(true)
bool(false)
1 - 61
bool(true)
bool(false)
1 - 62
bool(true)
bool(false)
1 - 63
bool(true)
bool(false)
1 - 64
bool(true)
bool(false)
1 - 65
bool(true)
bool(false)
2 - 8
bool(true)
bool(false)
2 - 9
bool(true)
bool(false)
2 - 10
bool(true)
bool(false)
2 - 11
bool(true)
bool(false)
2 - 12
bool(true)
bool(false)
2 - 13
bool(true)
bool(false)
2 - 14
bool(true)
bool(false)
2 - 15
bool(true)
bool(false)
2 - 16
bool(true)
bool(false)
2 - 17
bool(true)
bool(false)
2 - 18
bool(true)
bool(false)
2 - 19
bool(true)
bool(false)
2 - 20
bool(true)
bool(false)
2 - 21
bool(true)
bool(false)
2 - 22
bool(true)
bool(false)
2 - 23
bool(true)
bool(false)
2 - 24
bool(true)
bool(false)
2 - 25
bool(true)
bool(false)
2 - 26
bool(true)
bool(false)
2 - 27
bool(true)
bool(false)
2 - 28
bool(true)
bool(false)
2 - 29
bool(true)
bool(false)
2 - 30
bool(true)
bool(false)
2 - 31
bool(true)
bool(false)
2 - 32
bool(true)
bool(false)
2 - 33
bool(true)
bool(false)
2 - 34
bool(true)
bool(false)
2 - 35
bool(true)
bool(false)
2 - 36
bool(true)
bool(false)
2 - 37
bool(true)
bool(false)
2 - 38
bool(true)
bool(false)
2 - 39
bool(true)
bool(false)
2 - 40
bool(true)
bool(false)
2 - 41
bool(true)
bool(false)
2 - 42
bool(true)
bool(false)
2 - 43
bool(true)
bool(false)
2 - 44
bool(true)
bool(false)
2 - 45
bool(true)
bool(false)
2 - 46
bool(true)
bool(false)
2 - 47
bool(true)
bool(false)
2 - 48
bool(true)
bool(false)
2 - 49
bool(true)
bool(false)
2 - 50
bool(true)
bool(false)
2 - 51
bool(true)
bool(false)
2 - 52
bool(true)
bool(false)
2 - 53
bool(true)
bool(false)
2 - 54
bool(true)
bool(false)
2 - 55
bool(true)
bool(false)
2 - 56
bool(true)
bool(false)
2 - 57
bool(true)
bool(false)
2 - 58
bool(true)
bool(false)
2 - 59
bool(true)
bool(false)
2 - 60
bool(true)
bool(false)
2 - 61
bool(true)
bool(false)
2 - 62
bool(true)
bool(false)
2 - 63
bool(true)
bool(false)
2 - 64
bool(true)
bool(false)
2 - 65
bool(true)
bool(false)
3 - 8
bool(true)
bool(false)
3 - 9
bool(true)
bool(false)
3 - 10
bool(true)
bool(false)
3 - 11
bool(true)
bool(false)
3 - 12
bool(true)
bool(false)
3 - 13
bool(true)
bool(false)
3 - 14
bool(true)
bool(false)
3 - 15
bool(true)
bool(false)
3 - 16
bool(true)
bool(false)
3 - 17
bool(true)
bool(false)
3 - 18
bool(true)
bool(false)
3 - 19
bool(true)
bool(false)
3 - 20
bool(true)
bool(false)
3 - 21
bool(true)
bool(false)
3 - 22
bool(true)
bool(false)
3 - 23
bool(true)
bool(false)
3 - 24
bool(true)
bool(false)
3 - 25
bool(true)
bool(false)
3 - 26
bool(true)
bool(false)
3 - 27
bool(true)
bool(false)
3 - 28
bool(true)
bool(false)
3 - 29
bool(true)
bool(false)
3 - 30
bool(true)
bool(false)
3 - 31
bool(true)
bool(false)
3 - 32
bool(true)
bool(false)
3 - 33
bool(true)
bool(false)
3 - 34
bool(true)
bool(false)
3 - 35
bool(true)
bool(false)
3 - 36
bool(true)
bool(false)
3 - 37
bool(true)
bool(false)
3 - 38
bool(true)
bool(false)
3 - 39
bool(true)
bool(false)
3 - 40
bool(true)
bool(false)
3 - 41
bool(true)
bool(false)
3 - 42
bool(true)
bool(false)
3 - 43
bool(true)
bool(false)
3 - 44
bool(true)
bool(false)
3 - 45
bool(true)
bool(false)
3 - 46
bool(true)
bool(false)
3 - 47
bool(true)
bool(false)
3 - 48
bool(true)
bool(false)
3 - 49
bool(true)
bool(false)
3 - 50
bool(true)
bool(false)
3 - 51
bool(true)
bool(false)
3 - 52
bool(true)
bool(false)
3 - 53
bool(true)
bool(false)
3 - 54
bool(true)
bool(false)
3 - 55
bool(true)
bool(false)
3 - 56
bool(true)
bool(false)
3 - 57
bool(true)
bool(false)
3 - 58
bool(true)
bool(false)
3 - 59
bool(true)
bool(false)
3 - 60
bool(true)
bool(false)
3 - 61
bool(true)
bool(false)
3 - 62
bool(true)
bool(false)
3 - 63
bool(true)
bool(false)
3 - 64
bool(true)
bool(false)
3 - 65
bool(true)
bool(false)
0 - 8
bool(true)
bool(false)
0 - 9
bool(true)
bool(false)
0 - 10
bool(true)
bool(false)
0 - 11
bool(true)
bool(false)
0 - 12
bool(true)
bool(false)
0 - 13
bool(true)
bool(false)
0 - 14
bool(true)
bool(false)
0 - 15
bool(true)
bool(false)
0 - 16
bool(true)
bool(false)
0 - 17
bool(true)
bool(false)
0 - 18
bool(true)
bool(false)
0 - 19
bool(true)
bool(false)
0 - 20
bool(true)
bool(false)
0 - 21
bool(true)
bool(false)
0 - 22
bool(true)
bool(false)
0 - 23
bool(true)
bool(false)
0 - 24
bool(true)
bool(false)
0 - 25
bool(true)
bool(false)
0 - 26
bool(true)
bool(false)
0 - 27
bool(true)
bool(false)
0 - 28
bool(true)
bool(false)
0 - 29
bool(true)
bool(false)
0 - 30
bool(true)
bool(false)
0 - 31
bool(true)
bool(false)
0 - 32
bool(true)
bool(false)
0 - 33
bool(true)
bool(false)
0 - 34
bool(true)
bool(false)
0 - 35
bool(true)
bool(false)
0 - 36
bool(true)
bool(false)
0 - 37
bool(true)
bool(false)
0 - 38
bool(true)
bool(false)
0 - 39
bool(true)
bool(false)
0 - 40
bool(true)
bool(false)
0 - 41
bool(true)
bool(false)
0 - 42
bool(true)
bool(false)
0 - 43
bool(true)
bool(false)
0 - 44
bool(true)
bool(false)
0 - 45
bool(true)
bool(false)
0 - 46
bool(true)
bool(false)
0 - 47
bool(true)
bool(false)
0 - 48
bool(true)
bool(false)
0 - 49
bool(true)
bool(false)
0 - 50
bool(true)
bool(false)
0 - 51
bool(true)
bool(false)
0 - 52
bool(true)
bool(false)
0 - 53
bool(true)
bool(false)
0 - 54
bool(true)
bool(false)
0 - 55
bool(true)
bool(false)
0 - 56
bool(true)
bool(false)
0 - 57
bool(true)
bool(false)
0 - 58
bool(true)
bool(false)
0 - 59
bool(true)
bool(false)
0 - 60
bool(true)
bool(false)
0 - 61
bool(true)
bool(false)
0 - 62
bool(true)
bool(false)
0 - 63
bool(true)
bool(false)
0 - 64
bool(true)
bool(false)
0 - 65
bool(true)
bool(false)
1 - 8
bool(true)
bool(false)
1 - 9
bool(true)
bool(false)
1 - 10
bool(true)
bool(false)
1 - 11
bool(true)
bool(false)
1 - 12
bool(true)
bool(false)
1 - 13
bool(true)
bool(false)
1 - 14
bool(true)
bool(false)
1 - 15
bool(true)
bool(false)
1 - 16
bool(true)
bool(false)
1 - 17
bool(true)
bool(false)
1 - 18
bool(true)
bool(false)
1 - 19
bool(true)
bool(false)
1 - 20
bool(true)
bool(false)
1 - 21
bool(true)
bool(false)
1 - 22
bool(true)
bool(false)
1 - 23
bool(true)
bool(false)
1 - 24
bool(true)
bool(false)
1 - 25
bool(true)
bool(false)
1 - 26
bool(true)
bool(false)
1 - 27
bool(true)
bool(false)
1 - 28
bool(true)
bool(false)
1 - 29
bool(true)
bool(false)
1 - 30
bool(true)
bool(false)
1 - 31
bool(true)
bool(false)
1 - 32
bool(true)
bool(false)
1 - 33
bool(true)
bool(false)
1 - 34
bool(true)
bool(false)
1 - 35
bool(true)
bool(false)
1 - 36
bool(true)
bool(false)
1 - 37
bool(true)
bool(false)
1 - 38
bool(true)
bool(false)
1 - 39
bool(true)
bool(false)
1 - 40
bool(true)
bool(false)
1 - 41
bool(true)
bool(false)
1 - 42
bool(true)
bool(false)
1 - 43
bool(true)
bool(false)
1 - 44
bool(true)
bool(false)
1 - 45
bool(true)
bool(false)
1 - 46
bool(true)
bool(false)
1 - 47
bool(true)
bool(false)
1 - 48
bool(true)
bool(false)
1 - 49
bool(true)
bool(false)
1 - 50
bool(true)
bool(false)
1 - 51
bool(true)
bool(false)
1 - 52
bool(true)
bool(false)
1 - 53
bool(true)
bool(false)
1 - 54
bool(true)
bool(false)
1 - 55
bool(true)
bool(false)
1 - 56
bool(true)
bool(false)
1 - 57
bool(true)
bool(false)
1 - 58
bool(true)
bool(false)
1 - 59
bool(true)
bool(false)
1 - 60
bool(true)
bool(false)
1 - 61
bool(true)
bool(false)
1 - 62
bool(true)
bool(false)
1 - 63
bool(true)
bool(false)
1 - 64
bool(true)
bool(false)
1 - 65
bool(true)
bool(false)
2 - 8
bool(true)
bool(false)
2 - 9
bool(true)
bool(false)
2 - 10
bool(true)
bool(false)
2 - 11
bool(true)
bool(false)
2 - 12
bool(true)
bool(false)
2 - 13
bool(true)
bool(false)
2 - 14
bool(true)
bool(false)
2 - 15
bool(true)
bool(false)
2 - 16
bool(true)
bool(false)
2 - 17
bool(true)
bool(false)
2 - 18
bool(true)
bool(false)
2 - 19
bool(true)
bool(false)
2 - 20
bool(true)
bool(false)
2 - 21
bool(true)
bool(false)
2 - 22
bool(true)
bool(false)
2 - 23
bool(true)
bool(false)
2 - 24
bool(true)
bool(false)
2 - 25
bool(true)
bool(false)
2 - 26
bool(true)
bool(false)
2 - 27
bool(true)
bool(false)
2 - 28
bool(true)
bool(false)
2 - 29
bool(true)
bool(false)
2 - 30
bool(true)
bool(false)
2 - 31
bool(true)
bool(false)
2 - 32
bool(true)
bool(false)
2 - 33
bool(true)
bool(false)
2 - 34
bool(true)
bool(false)
2 - 35
bool(true)
bool(false)
2 - 36
bool(true)
bool(false)
2 - 37
bool(true)
bool(false)
2 - 38
bool(true)
bool(false)
2 - 39
bool(true)
bool(false)
2 - 40
bool(true)
bool(false)
2 - 41
bool(true)
bool(false)
2 - 42
bool(true)
bool(false)
2 - 43
bool(true)
bool(false)
2 - 44
bool(true)
bool(false)
2 - 45
bool(true)
bool(false)
2 - 46
bool(true)
bool(false)
2 - 47
bool(true)
bool(false)
2 - 48
bool(true)
bool(false)
2 - 49
bool(true)
bool(false)
2 - 50
bool(true)
bool(false)
2 - 51
bool(true)
bool(false)
2 - 52
bool(true)
bool(false)
2 - 53
bool(true)
bool(false)
2 - 54
bool(true)
bool(false)
2 - 55
bool(true)
bool(false)
2 - 56
bool(true)
bool(false)
2 - 57
bool(true)
bool(false)
2 - 58
bool(true)
bool(false)
2 - 59
bool(true)
bool(false)
2 - 60
bool(true)
bool(false)
2 - 61
bool(true)
bool(false)
2 - 62
bool(true)
bool(false)
2 - 63
bool(true)
bool(false)
2 - 64
bool(true)
bool(false)
2 - 65
bool(true)
bool(false)
3 - 8
bool(true)
bool(false)
3 - 9
bool(true)
bool(false)
3 - 10
bool(true)
bool(false)
3 - 11
bool(true)
bool(false)
3 - 12
bool(true)
bool(false)
3 - 13
bool(true)
bool(false)
3 - 14
bool(true)
bool(false)
3 - 15
bool(true)
bool(false)
3 - 16
bool(true)
bool(false)
3 - 17
bool(true)
bool(false)
3 - 18
bool(true)
bool(false)
3 - 19
bool(true)
bool(false)
3 - 20
bool(true)
bool(false)
3 - 21
bool(true)
bool(false)
3 - 22
bool(true)
bool(false)
3 - 23
bool(true)
bool(false)
3 - 24
bool(true)
bool(false)
3 - 25
bool(true)
bool(false)
3 - 26
bool(true)
bool(false)
3 - 27
bool(true)
bool(false)
3 - 28
bool(true)
bool(false)
3 - 29
bool(true)
bool(false)
3 - 30
bool(true)
bool(false)
3 - 31
bool(true)
bool(false)
3 - 32
bool(true)
bool(false)
3 - 33
bool(true)
bool(false)
3 - 34
bool(true)
bool(false)
3 - 35
bool(true)
bool(false)
3 - 36
bool(true)
bool(false)
3 - 37
bool(true)
bool(false)
3 - 38
bool(true)
bool(false)
3 - 39
bool(true)
bool(false)
3 - 40
bool(true)
bool(false)
3 - 41
bool(true)
bool(false)
3 - 42
bool(true)
bool(false)
3 - 43
bool(true)
bool(false)
3 - 44
bool(true)
bool(false)
3 - 45
bool(true)
bool(false)
3 - 46
bool(true)
bool(false)
3 - 47
bool(true)
bool(false)
3 - 48
bool(true)
bool(false)
3 - 49
bool(true)
bool(false)
3 - 50
bool(true)
bool(false)
3 - 51
bool(true)
bool(false)
3 - 52
bool(true)
bool(false)
3 - 53
bool(true)
bool(false)
3 - 54
bool(true)
bool(false)
3 - 55
bool(true)
bool(false)
3 - 56
bool(true)
bool(false)
3 - 57
bool(true)
bool(false)
3 - 58
bool(true)
bool(false)
3 - 59
bool(true)
bool(false)
3 - 60
bool(true)
bool(false)
3 - 61
bool(true)
bool(false)
3 - 62
bool(true)
bool(false)
3 - 63
bool(true)
bool(false)
3 - 64
bool(true)
bool(false)
3 - 65
bool(true)
bool(false)
0 - 8
bool(true)
bool(false)
0 - 9
bool(true)
bool(false)
0 - 10
bool(true)
bool(false)
0 - 11
bool(true)
bool(false)
0 - 12
bool(true)
bool(false)
0 - 13
bool(true)
bool(false)
0 - 14
bool(true)
bool(false)
0 - 15
bool(true)
bool(false)
0 - 16
bool(true)
bool(false)
0 - 17
bool(true)
bool(false)
0 - 18
bool(true)
bool(false)
0 - 19
bool(true)
bool(false)
0 - 20
bool(true)
bool(false)
0 - 21
bool(true)
bool(false)
0 - 22
bool(true)
bool(false)
0 - 23
bool(true)
bool(false)
0 - 24
bool(true)
bool(false)
0 - 25
bool(true)
bool(false)
0 - 26
bool(true)
bool(false)
0 - 27
bool(true)
bool(false)
0 - 28
bool(true)
bool(false)
0 - 29
bool(true)
bool(false)
0 - 30
bool(true)
bool(false)
0 - 31
bool(true)
bool(false)
0 - 32
bool(true)
bool(false)
0 - 33
bool(true)
bool(false)
0 - 34
bool(true)
bool(false)
0 - 35
bool(true)
bool(false)
0 - 36
bool(true)
bool(false)
0 - 37
bool(true)
bool(false)
0 - 38
bool(true)
bool(false)
0 - 39
bool(true)
bool(false)
0 - 40
bool(true)
bool(false)
0 - 41
bool(true)
bool(false)
0 - 42
bool(true)
bool(false)
0 - 43
bool(true)
bool(false)
0 - 44
bool(true)
bool(false)
0 - 45
bool(true)
bool(false)
0 - 46
bool(true)
bool(false)
0 - 47
bool(true)
bool(false)
0 - 48
bool(true)
bool(false)
0 - 49
bool(true)
bool(false)
0 - 50
bool(true)
bool(false)
0 - 51
bool(true)
bool(false)
0 - 52
bool(true)
bool(false)
0 - 53
bool(true)
bool(false)
0 - 54
bool(true)
bool(false)
0 - 55
bool(true)
bool(false)
0 - 56
bool(true)
bool(false)
0 - 57
bool(true)
bool(false)
0 - 58
bool(true)
bool(false)
0 - 59
bool(true)
bool(false)
0 - 60
bool(true)
bool(false)
0 - 61
bool(true)
bool(false)
0 - 62
bool(true)
bool(false)
0 - 63
bool(true)
bool(false)
0 - 64
bool(true)
bool(false)
0 - 65
bool(true)
bool(false)
1 - 8
bool(true)
bool(false)
1 - 9
bool(true)
bool(false)
1 - 10
bool(true)
bool(false)
1 - 11
bool(true)
bool(false)
1 - 12
bool(true)
bool(false)
1 - 13
bool(true)
bool(false)
1 - 14
bool(true)
bool(false)
1 - 15
bool(true)
bool(false)
1 - 16
bool(true)
bool(false)
1 - 17
bool(true)
bool(false)
1 - 18
bool(true)
bool(false)
1 - 19
bool(true)
bool(false)
1 - 20
bool(true)
bool(false)
1 - 21
bool(true)
bool(false)
1 - 22
bool(true)
bool(false)
1 - 23
bool(true)
bool(false)
1 - 24
bool(true)
bool(false)
1 - 25
bool(true)
bool(false)
1 - 26
bool(true)
bool(false)
1 - 27
bool(true)
bool(false)
1 - 28
bool(true)
bool(false)
1 - 29
bool(true)
bool(false)
1 - 30
bool(true)
bool(false)
1 - 31
bool(true)
bool(false)
1 - 32
bool(true)
bool(false)
1 - 33
bool(true)
bool(false)
1 - 34
bool(true)
bool(false)
1 - 35
bool(true)
bool(false)
1 - 36
bool(true)
bool(false)
1 - 37
bool(true)
bool(false)
1 - 38
bool(true)
bool(false)
1 - 39
bool(true)
bool(false)
1 - 40
bool(true)
bool(false)
1 - 41
bool(true)
bool(false)
1 - 42
bool(true)
bool(false)
1 - 43
bool(true)
bool(false)
1 - 44
bool(true)
bool(false)
1 - 45
bool(true)
bool(false)
1 - 46
bool(true)
bool(false)
1 - 47
bool(true)
bool(false)
1 - 48
bool(true)
bool(false)
1 - 49
bool(true)
bool(false)
1 - 50
bool(true)
bool(false)
1 - 51
bool(true)
bool(false)
1 - 52
bool(true)
bool(false)
1 - 53
bool(true)
bool(false)
1 - 54
bool(true)
bool(false)
1 - 55
bool(true)
bool(false)
1 - 56
bool(true)
bool(false)
1 - 57
bool(true)
bool(false)
1 - 58
bool(true)
bool(false)
1 - 59
bool(true)
bool(false)
1 - 60
bool(true)
bool(false)
1 - 61
bool(true)
bool(false)
1 - 62
bool(true)
bool(false)
1 - 63
bool(true)
bool(false)
1 - 64
bool(true)
bool(false)
1 - 65
bool(true)
bool(false)
2 - 8
bool(true)
bool(false)
2 - 9
bool(true)
bool(false)
2 - 10
bool(true)
bool(false)
2 - 11
bool(true)
bool(false)
2 - 12
bool(true)
bool(false)
2 - 13
bool(true)
bool(false)
2 - 14
bool(true)
bool(false)
2 - 15
bool(true)
bool(false)
2 - 16
bool(true)
bool(false)
2 - 17
bool(true)
bool(false)
2 - 18
bool(true)
bool(false)
2 - 19
bool(true)
bool(false)
2 - 20
bool(true)
bool(false)
2 - 21
bool(true)
bool(false)
2 - 22
bool(true)
bool(false)
2 - 23
bool(true)
bool(false)
2 - 24
bool(true)
bool(false)
2 - 25
bool(true)
bool(false)
2 - 26
bool(true)
bool(false)
2 - 27
bool(true)
bool(false)
2 - 28
bool(true)
bool(false)
2 - 29
bool(true)
bool(false)
2 - 30
bool(true)
bool(false)
2 - 31
bool(true)
bool(false)
2 - 32
bool(true)
bool(false)
2 - 33
bool(true)
bool(false)
2 - 34
bool(true)
bool(false)
2 - 35
bool(true)
bool(false)
2 - 36
bool(true)
bool(false)
2 - 37
bool(true)
bool(false)
2 - 38
bool(true)
bool(false)
2 - 39
bool(true)
bool(false)
2 - 40
bool(true)
bool(false)
2 - 41
bool(true)
bool(false)
2 - 42
bool(true)
bool(false)
2 - 43
bool(true)
bool(false)
2 - 44
bool(true)
bool(false)
2 - 45
bool(true)
bool(false)
2 - 46
bool(true)
bool(false)
2 - 47
bool(true)
bool(false)
2 - 48
bool(true)
bool(false)
2 - 49
bool(true)
bool(false)
2 - 50
bool(true)
bool(false)
2 - 51
bool(true)
bool(false)
2 - 52
bool(true)
bool(false)
2 - 53
bool(true)
bool(false)
2 - 54
bool(true)
bool(false)
2 - 55
bool(true)
bool(false)
2 - 56
bool(true)
bool(false)
2 - 57
bool(true)
bool(false)
2 - 58
bool(true)
bool(false)
2 - 59
bool(true)
bool(false)
2 - 60
bool(true)
bool(false)
2 - 61
bool(true)
bool(false)
2 - 62
bool(true)
bool(false)
2 - 63
bool(true)
bool(false)
2 - 64
bool(true)
bool(false)
2 - 65
bool(true)
bool(false)
3 - 8
bool(true)
bool(false)
3 - 9
bool(true)
bool(false)
3 - 10
bool(true)
bool(false)
3 - 11
bool(true)
bool(false)
3 - 12
bool(true)
bool(false)
3 - 13
bool(true)
bool(false)
3 - 14
bool(true)
bool(false)
3 - 15
bool(true)
bool(false)
3 - 16
bool(true)
bool(false)
3 - 17
bool(true)
bool(false)
3 - 18
bool(true)
bool(false)
3 - 19
bool(true)
bool(false)
3 - 20
bool(true)
bool(false)
3 - 21
bool(true)
bool(false)
3 - 22
bool(true)
bool(false)
3 - 23
bool(true)
bool(false)
3 - 24
bool(true)
bool(false)
3 - 25
bool(true)
bool(false)
3 - 26
bool(true)
bool(false)
3 - 27
bool(true)
bool(false)
3 - 28
bool(true)
bool(false)
3 - 29
bool(true)
bool(false)
3 - 30
bool(true)
bool(false)
3 - 31
bool(true)
bool(false)
3 - 32
bool(true)
bool(false)
3 - 33
bool(true)
bool(false)
3 - 34
bool(true)
bool(false)
3 - 35
bool(true)
bool(false)
3 - 36
bool(true)
bool(false)
3 - 37
bool(true)
bool(false)
3 - 38
bool(true)
bool(false)
3 - 39
bool(true)
bool(false)
3 - 40
bool(true)
bool(false)
3 - 41
bool(true)
bool(false)
3 - 42
bool(true)
bool(false)
3 - 43
bool(true)
bool(false)
3 - 44
bool(true)
bool(false)
3 - 45
bool(true)
bool(false)
3 - 46
bool(true)
bool(false)
3 - 47
bool(true)
bool(false)
3 - 48
bool(true)
bool(false)
3 - 49
bool(true)
bool(false)
3 - 50
bool(true)
bool(false)
3 - 51
bool(true)
bool(false)
3 - 52
bool(true)
bool(false)
3 - 53
bool(true)
bool(false)
3 - 54
bool(true)
bool(false)
3 - 55
bool(true)
bool(false)
3 - 56
bool(true)
bool(false)
3 - 57
bool(true)
bool(false)
3 - 58
bool(true)
bool(false)
3 - 59
bool(true)
bool(false)
3 - 60
bool(true)
bool(false)
3 - 61
bool(true)
bool(false)
3 - 62
bool(true)
bool(false)
3 - 63
bool(true)
bool(false)
3 - 64
bool(true)
bool(false)
3 - 65
bool(true)
bool(false)
0 - 8
bool(true)
bool(false)
0 - 9
bool(true)
bool(false)
0 - 10
bool(true)
bool(false)
0 - 11
bool(true)
bool(false)
0 - 12
bool(true)
bool(false)
0 - 13
bool(true)
bool(false)
0 - 14
bool(true)
bool(false)
0 - 15
bool(true)
bool(false)
0 - 16
bool(true)
bool(false)
0 - 17
bool(true)
bool(false)
0 - 18
bool(true)
bool(false)
0 - 19
bool(true)
bool(false)
0 - 20
bool(true)
bool(false)
0 - 21
bool(true)
bool(false)
0 - 22
bool(true)
bool(false)
0 - 23
bool(true)
bool(false)
0 - 24
bool(true)
bool(false)
0 - 25
bool(true)
bool(false)
0 - 26
bool(true)
bool(false)
0 - 27
bool(true)
bool(false)
0 - 28
bool(true)
bool(false)
0 - 29
bool(true)
bool(false)
0 - 30
bool(true)
bool(false)
0 - 31
bool(true)
bool(false)
0 - 32
bool(true)
bool(false)
0 - 33
bool(true)
bool(false)
0 - 34
bool(true)
bool(false)
0 - 35
bool(true)
bool(false)
0 - 36
bool(true)
bool(false)
0 - 37
bool(true)
bool(false)
0 - 38
bool(true)
bool(false)
0 - 39
bool(true)
bool(false)
0 - 40
bool(true)
bool(false)
0 - 41
bool(true)
bool(false)
0 - 42
bool(true)
bool(false)
0 - 43
bool(true)
bool(false)
0 - 44
bool(true)
bool(false)
0 - 45
bool(true)
bool(false)
0 - 46
bool(true)
bool(false)
0 - 47
bool(true)
bool(false)
0 - 48
bool(true)
bool(false)
0 - 49
bool(true)
bool(false)
0 - 50
bool(true)
bool(false)
0 - 51
bool(true)
bool(false)
0 - 52
bool(true)
bool(false)
0 - 53
bool(true)
bool(false)
0 - 54
bool(true)
bool(false)
0 - 55
bool(true)
bool(false)
0 - 56
bool(true)
bool(false)
0 - 57
bool(true)
bool(false)
0 - 58
bool(true)
bool(false)
0 - 59
bool(true)
bool(false)
0 - 60
bool(true)
bool(false)
0 - 61
bool(true)
bool(false)
0 - 62
bool(true)
bool(false)
0 - 63
bool(true)
bool(false)
0 - 64
bool(true)
bool(false)
0 - 65
bool(true)
bool(false)
1 - 8
bool(true)
bool(false)
1 - 9
bool(true)
bool(false)
1 - 10
bool(true)
bool(false)
1 - 11
bool(true)
bool(false)
1 - 12
bool(true)
bool(false)
1 - 13
bool(true)
bool(false)
1 - 14
bool(true)
bool(false)
1 - 15
bool(true)
bool(false)
1 - 16
bool(true)
bool(false)
1 - 17
bool(true)
bool(false)
1 - 18
bool(true)
bool(false)
1 - 19
bool(true)
bool(false)
1 - 20
bool(true)
bool(false)
1 - 21
bool(true)
bool(false)
1 - 22
bool(true)
bool(false)
1 - 23
bool(true)
bool(false)
1 - 24
bool(true)
bool(false)
1 - 25
bool(true)
bool(false)
1 - 26
bool(true)
bool(false)
1 - 27
bool(true)
bool(false)
1 - 28
bool(true)
bool(false)
1 - 29
bool(true)
bool(false)
1 - 30
bool(true)
bool(false)
1 - 31
bool(true)
bool(false)
1 - 32
bool(true)
bool(false)
1 - 33
bool(true)
bool(false)
1 - 34
bool(true)
bool(false)
1 - 35
bool(true)
bool(false)
1 - 36
bool(true)
bool(false)
1 - 37
bool(true)
bool(false)
1 - 38
bool(true)
bool(false)
1 - 39
bool(true)
bool(false)
1 - 40
bool(true)
bool(false)
1 - 41
bool(true)
bool(false)
1 - 42
bool(true)
bool(false)
1 - 43
bool(true)
bool(false)
1 - 44
bool(true)
bool(false)
1 - 45
bool(true)
bool(false)
1 - 46
bool(true)
bool(false)
1 - 47
bool(true)
bool(false)
1 - 48
bool(true)
bool(false)
1 - 49
bool(true)
bool(false)
1 - 50
bool(true)
bool(false)
1 - 51
bool(true)
bool(false)
1 - 52
bool(true)
bool(false)
1 - 53
bool(true)
bool(false)
1 - 54
bool(true)
bool(false)
1 - 55
bool(true)
bool(false)
1 - 56
bool(true)
bool(false)
1 - 57
bool(true)
bool(false)
1 - 58
bool(true)
bool(false)
1 - 59
bool(true)
bool(false)
1 - 60
bool(true)
bool(false)
1 - 61
bool(true)
bool(false)
1 - 62
bool(true)
bool(false)
1 - 63
bool(true)
bool(false)
1 - 64
bool(true)
bool(false)
1 - 65
OK
