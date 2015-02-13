--TEST--
Custom Number Formats
--INI--
date.timezone=America/Toronto
--SKIPIF--
<?php if (!extension_loaded("excel")) die("skip - Excel extension not found"); ?>
--FILE--
<?php 
	$x = new ExcelBook();

	$s = $x->addSheet("Sheet 1");

	$fmt = $x->addCustomFormat("[Red][<=100];[Blue][>100]");
	var_dump($fmt, $x->getError());

	var_dump($x->getCustomFormat($fmt));

	$f = $x->addFormat();
	$f->numberFormat($fmt);

	$s->write(1,1,50,$f);
	var_dump($x->getError());
	
	$s->write(2,2,150,$f);
	var_dump($x->getError());



	echo "OK\n";
?>
--EXPECT--
int(164)
bool(false)
string(25) "[Red][<=100];[Blue][>100]"
bool(false)
bool(false)
OK
