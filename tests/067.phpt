--TEST--
Book::insertSheet()
--EXTENSIONS--
excel
--FILE--
<?php 
	$x = new ExcelBook();
	for ($i = 0; $i < 5; $i++) {
		$x->addSheet("Sheet " . $i);
	}
	var_dump($x->sheetCount());
	var_dump($x->insertSheet(1, "test", $x->addSheet("new")) instanceof ExcelSheet);
	var_dump($x->sheetCount());
	var_dump($x->insertSheet(1, "test2") instanceof ExcelSheet);
	var_dump($x->sheetCount());

	$other = new ExcelBook();
	$foreign = $other->addSheet("foreign");
	var_dump(@$x->insertSheet(1, "foreign", $foreign));

	$sameBook = $x->getSheet(0);
	var_dump($x->insertSheet(1, "same", $sameBook) instanceof ExcelSheet);
?>
--EXPECT--
int(5)
bool(true)
int(7)
bool(true)
int(8)
bool(false)
bool(true)
