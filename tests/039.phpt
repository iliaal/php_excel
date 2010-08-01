--TEST--
Zoom Test
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

	var_dump(
		$s->zoom(),
		$s->setZoom(200),
		$s->zoom()
	);

	var_dump(
		$s->zoomPrint(),
		$s->setZoomPrint(500),
		$s->zoomPrint()
	);
	

	
	echo "OK\n";
?>
--EXPECT--
int(100)
NULL
int(200)
int(100)
NULL
int(500)
OK
