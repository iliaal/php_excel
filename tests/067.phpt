--TEST--
Book::insertSheet()
--SKIPIF--
<?php if (!extension_loaded("excel")) die("skip - Excel extension not found"); ?>
--FILE--
<?php 
	$x = new ExcelBook();
	for ($i = 0; $i < 5; $i++) {
		$x->addSheet("Sheet " . $i);
	}
	var_dump($x->sheetCount());
	var_dump($x->insertSheet(1, "test", $x->addSheet("new")));
	var_dump($x->sheetCount());
	var_dump($x->insertSheet(1, "test2"));
	var_dump($x->sheetCount());
?>
--EXPECT--
int(5)
object(ExcelSheet)#3 (0) {
}
int(7)
object(ExcelSheet)#3 (0) {
}
int(8)
