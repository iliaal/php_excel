--TEST--
Alignment Tests
--INI--
date.timezone=America/Toronto
--SKIPIF--
<?php if (!extension_loaded("excel")) die("skip - Excel extension not found"); ?>
--FILE--
<?php 
	$x = new ExcelBook();

	$s = $x->addSheet("Sheet 1");

	$row = 1;

	$oClass = new ReflectionClass('ExcelFormat');

	$data = "my test\ndata";

	$s->setColWidth(1, 1, 55);
	var_dump($x->getError());

	foreach ($oClass->getConstants() as $c => $val) {
		if (strpos($c, 'ALIGNH_') !== 0) {
			continue;
		}

		var_dump($s->write($row, 0, $c));
		var_dump($x->getError());

		$format = $x->addFormat();
		$format->horizontalAlign($val);

		var_dump($s->write($row, 1, $data, $format));
		var_dump($x->getError());

		++$row;
	}

	foreach ($oClass->getConstants() as $c => $val) {
		if (strpos($c, 'ALIGNV_') !== 0) {
			continue;
		}

		$s->setRowHeight($row, 50);

		var_dump($s->write($row, 0, $c));
		var_dump($x->getError());

		$format = $x->addFormat();
		$format->verticalAlign($val);

		var_dump($s->write($row, 1, $data, $format));
		var_dump($x->getError());

		++$row;
	}

	for($i = 1; $i < $row; $i++) {
		echo $s->read($i, 0) . " >> ";
		$format = '';
		$s->read($i, 1, $format);
		echo "h: " . $format->horizontalAlign() . " || v: " . $format->verticalAlign() . "\n";
	}



	echo "OK\n";
?>
--EXPECT--
bool(false)
bool(true)
bool(false)
bool(true)
bool(false)
bool(true)
bool(false)
bool(true)
bool(false)
bool(true)
bool(false)
bool(true)
bool(false)
bool(true)
bool(false)
bool(true)
bool(false)
bool(true)
bool(false)
bool(true)
bool(false)
bool(true)
bool(false)
bool(true)
bool(false)
bool(true)
bool(false)
bool(true)
bool(false)
bool(true)
bool(false)
bool(true)
bool(false)
bool(true)
bool(false)
bool(true)
bool(false)
bool(true)
bool(false)
bool(true)
bool(false)
bool(true)
bool(false)
bool(true)
bool(false)
bool(true)
bool(false)
bool(true)
bool(false)
bool(true)
bool(false)
bool(true)
bool(false)
ALIGNH_GENERAL >> h: 0 || v: 2
ALIGNH_LEFT >> h: 1 || v: 2
ALIGNH_CENTER >> h: 2 || v: 2
ALIGNH_RIGHT >> h: 3 || v: 2
ALIGNH_FILL >> h: 4 || v: 2
ALIGNH_JUSTIFY >> h: 5 || v: 2
ALIGNH_MERGE >> h: 6 || v: 2
ALIGNH_DISTRIBUTED >> h: 7 || v: 2
ALIGNV_TOP >> h: 0 || v: 0
ALIGNV_CENTER >> h: 0 || v: 1
ALIGNV_BOTTOM >> h: 0 || v: 2
ALIGNV_JUSTIFY >> h: 0 || v: 3
ALIGNV_DISTRIBUTED >> h: 0 || v: 4
OK
