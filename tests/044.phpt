--TEST--
Margin Tests
--INI--
date.timezone=America/Toronto
--SKIPIF--
<?php if (!extension_loaded("excel")) print "skip"; ?>
--FILE--
<?php 
	$x = new ExcelBook();

	$s = $x->addSheet("Sheet 1");

	$s->write(1, 1, "Test");

	foreach(array('Left','Right','Top','Bottom') as $k => $v) {
		$f = 'margin' . $v;
		$fs = 'setMargin' . $v;
		var_dump($s->$f());
		$s->$fs(1 + 1 / ($k + 1));
		var_dump($s->$f());
	}


	
	echo "OK\n";
?>
--EXPECT--
float(0.75)
float(2)
float(0.75)
float(1.5)
float(1)
float(1.3333333333333)
float(1)
float(1.25)
OK
