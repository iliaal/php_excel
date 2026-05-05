--TEST--
Summary Direction Tests
--EXTENSIONS--
excel
--FILE--
<?php 
	$x = new ExcelBook();
	$s = $x->addSheet("Sheet 1");

	var_dump(
		$s->getGroupSummaryBelow(),
		$s->getGroupSummaryRight()
	);

	$s->setGroupSummaryBelow(0);
	$s->setGroupSummaryRight(0);

	var_dump(
		$s->getGroupSummaryBelow(),
		$s->getGroupSummaryRight()
	);

	echo "OK\n";
?>
--EXPECT--
bool(true)
bool(true)
bool(false)
bool(false)
OK
