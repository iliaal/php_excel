--TEST--
getSheetByName() tests
--SKIPIF--
<?php if (!extension_loaded("excel")) die("skip - Excel extension not found"); ?>
--FILE--
<?php 
	$x = new ExcelBook();
	for ($i = 0; $i < 5; $i++) {
		$x->addSheet("Sheet " . $i);
	}

	var_dump($x->getSheetByName("Sheet 3"));
	var_dump($x->getSheetByName("sheet 2"));
	var_dump($x->getSheetByName("sheet 2", true));
?>
--EXPECT--
object(ExcelSheet)#2 (0) {
}
bool(false)
object(ExcelSheet)#2 (0) {
}
