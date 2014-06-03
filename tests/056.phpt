--TEST--
Excel 2007/2010 read/write tests
--SKIPIF--
<?php if (!extension_loaded("excel")) print "skip"; ?>
--FILE--
<?php 
	$x = new ExcelBook(null,null,1);

	$data = array(true, 1.222, 434324, "fsdfasDF", NULL, "");

	$s = $x->addSheet("Sheet 1");
	
	foreach ($data as $k => $v) {
		$s->write(($k + 1), 1, gettype($v));
		$s->write(($k + 1), 2, $v);
	}
	
	foreach ($data as $k => $v) {
		var_dump(
			$s->read(($k + 1), 1),
			$s->read(($k + 1), 2)
		);
	}

	echo "OK\n";
?>
--EXPECT--
string(7) "boolean"
bool(true)
string(6) "double"
float(1.222)
string(7) "integer"
float(434324)
string(6) "string"
string(8) "fsdfasDF"
string(4) "NULL"
NULL
string(6) "string"
string(0) ""
OK