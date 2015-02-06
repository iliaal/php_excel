--TEST--
Excel date pack/unpack tests
--INI--
date.timezone=America/Toronto
--SKIPIF--
<?php if (!extension_loaded("excel")) print "skip"; ?>
--FILE--
<?php 
	$x = new ExcelBook();

	$t = time();
	for ($i = 0; $i < 1000; $i++) {
		$tm = rand(10000000, $t);
		
		$out = $x->unpackDate($x->packDate($tm));
		if ($out != $tm) {
			echo "source: {$tm} <> res: " . $out . " >> diff: ".($out - $tm)." packed: '".$x->packDate($tm)."'\n";
		}
	}
	echo "OK\n";
	
	for ($i = 0; $i < 1000; $i++) {
		$tm = time(); //rand(10000000, $t);
		
		$ed = $x->packDateValues(
		    date('Y', $tm),
		    date('m', $tm),
		    date('d', $tm),
		    date('H', $tm),
		    date('i', $tm),
		    date('s', $tm)
		);
		$out = $x->unpackDate($ed);
		if ($out != $tm) {
			echo "source: {$tm} <> res: " . $out . " >> diff: ".($out - $tm)." packed: '".$ed."'\n";
		}
	}
	echo "OK\n";
?>
--EXPECT--
OK
OK
