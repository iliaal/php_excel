--TEST--
New Page break functions
--SKIPIF--
<?php if (!extension_loaded("excel")) die("skip - Excel extension not found"); ?>
--FILE--
<?php 
	$x = new ExcelBook();

	$s = $x->addSheet("Sheet 1");

	var_dump($s->getHorPageBreakSize(), $s->getVerPageBreakSize());

	var_dump($s->horPageBreak(10, 1), $s->verPageBreak(10, 1));

	var_dump($s->getHorPageBreakSize(), $s->getVerPageBreakSize());

	var_dump($s->horPageBreak(20, 1), $s->verPageBreak(20, 1));

	var_dump($s->getHorPageBreakSize(), $s->getVerPageBreakSize());

	var_dump($s->getHorPageBreak(0));
	var_dump($s->getVerPageBreak(0));

	var_dump($s->getHorPageBreak(10));
	var_dump($s->getVerPageBreak(10));
	
	echo "OK\n";
?>
--EXPECT--
int(0)
int(0)
bool(true)
bool(true)
int(1)
int(1)
bool(true)
bool(true)
int(2)
int(2)
int(10)
int(10)
int(-1)
int(-1)
OK
