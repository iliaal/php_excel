--TEST--
Comment Write Test
--INI--
date.timezone=America/Toronto
--SKIPIF--
<?php if (!extension_loaded("excel")) print "skip"; ?>
--FILE--
<?php 
	$x = new ExcelBook();

	$s = $x->addSheet("Sheet 1");

	$s->writeComment(10, 10, "My Sample Comment", "Ilia A.", 100, 200);
	var_dump($x->getError(), $s->readComment(10, 10));


	
	echo "OK\n";
?>
--EXPECT--
bool(false)
string(17) "My Sample Comment"
OK
