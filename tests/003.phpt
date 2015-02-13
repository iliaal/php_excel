--TEST--
Basic Type write tests
--SKIPIF--
<?php if (!extension_loaded("excel")) die("skip - Excel extension not found"); ?>
--FILE--
<?php 
	$x = new ExcelBook();

	$data = array(true, 1.222, 434324, "fsdfasDF", NULL, "");

	$s = $x->addSheet("Sheet 1");
	
	foreach ($data as $k => $v) {
		$s->write(($k + 1), 1, gettype($v));
		$s->write(($k + 1), 2, $v);
	}
	


	echo "OK\n";
?>
--EXPECT--
OK
