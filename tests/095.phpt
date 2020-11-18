--TEST--
Book::moveSheet()
--SKIPIF--
<?php if (!extension_loaded("excel") || !in_array('moveSheet', get_class_methods('ExcelBook'))) print "skip"; ?>
--FILE--
<?php
	$book = new ExcelBook(null, null, true);

	$sheet1 = new ExcelSheet($book, 'sheet1');
	$sheet2 = new ExcelSheet($book, 'sheet2');
	$sheet3 = new ExcelSheet($book, 'sheet3');
	$sheet4 = new ExcelSheet($book, 'sheet4');
	$sheet5 = new ExcelSheet($book, 'sheet5');

	var_dump(
		$book->getSheet(0)->name(),
		$book->getSheet(1)->name(),
		$book->getSheet(2)->name(),
		$book->getSheet(3)->name(),
		$book->getSheet(4)->name()
	);

	var_dump(
		$book->moveSheet(0,2)
	);

	var_dump(
		$book->getSheet(0)->name(),
		$book->getSheet(1)->name(),
		$book->getSheet(2)->name(),
		$book->getSheet(3)->name(),
		$book->getSheet(4)->name()
	);

	var_dump(
		$book->moveSheet(0,3)
	);

	var_dump(
		$book->getSheet(0)->name(),
		$book->getSheet(1)->name(),
		$book->getSheet(2)->name(),
		$book->getSheet(3)->name(),
		$book->getSheet(4)->name()
	);

	echo "OK\n";

?>
--EXPECT--
string(6) "sheet1"
string(6) "sheet2"
string(6) "sheet3"
string(6) "sheet4"
string(6) "sheet5"
bool(true)
string(6) "sheet2"
string(6) "sheet1"
string(6) "sheet3"
string(6) "sheet4"
string(6) "sheet5"
bool(true)
string(6) "sheet1"
string(6) "sheet3"
string(6) "sheet2"
string(6) "sheet4"
string(6) "sheet5"
OK
