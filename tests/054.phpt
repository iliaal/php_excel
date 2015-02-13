--TEST--
colorPack()/colorUnpack() tests
--INI--
date.timezone=America/Toronto
--SKIPIF--
<?php
	if (!extension_loaded("excel")) die("skip - Excel extension not found");
	if (!method_exists(new ExcelBook(), "colorPack")) die("skip - ExcelBook::colorPack() missing");
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

	echo "OK\n";
?>
--EXPECT--
OK
