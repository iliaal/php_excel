--TEST--
colorPack()/colorUnpack() tests
--INI--
date.timezone=America/Toronto
--EXTENSIONS--
excel
--SKIPIF--
<?php
	if (!method_exists(new ExcelBook(), "colorPack")) print "skip";
?>
--FILE--
<?php 
	$x = new ExcelBook(null,null,1);

	$val = range(1,255);
	
	for ($i = 0; $i < 50; $i++) {
		$r = $val[array_rand($val)];
		$g = $val[array_rand($val)];
		$b = $val[array_rand($val)];

		$c = $x->colorPack($r,$g,$b);

		$out = $x->colorUnpack($c);
		if ($out['red'] != $r || $out['blue'] != $b || $out['green'] != $g) {
			var_dump($r,$g,$b,$c,$out);
			exit;
		}
	}

	$black = $x->colorPack(0, 0, 0);
	if ($x->colorUnpack($black) !== ['red' => 0, 'green' => 0, 'blue' => 0]) {
		var_dump($black, $x->colorUnpack($black));
		exit;
	}

	echo "OK\n";
?>
--EXPECT--
OK
