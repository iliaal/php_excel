--TEST--
Print Header & Footer
--INI--
date.timezone=America/Toronto
--SKIPIF--
<?php if (!extension_loaded("excel")) print "skip"; ?>
--FILE--
<?php 
	$x = new ExcelBook();

	$s = $x->addSheet("Sheet 1");

	$s->write(1, 1, "Test");
	$s->write(12, 11, "Test 2");

	var_dump($s->header(), $s->footer());

	$s->setFooter("&L &10 &D &T &C &U &A &R &P of &N", 0.25);
	var_dump($x->getError());
	$s->setHeader("&L &I left &C &E middle &R &\"Times New Roman\" right times", 0.5);
	var_dump($x->getError());

	var_dump($s->header(), $s->footer());
	var_dump($s->headerMargin(), $s->footerMargin());


	
	echo "OK\n";
?>
--EXPECT--
NULL
NULL
bool(false)
bool(false)
string(57) "&L &I left &C &E middle &R &"Times New Roman" right times"
string(33) "&L &10 &D &T &C &U &A &R &P of &N"
float(0.5)
float(0.25)
OK
