--TEST--
Default Font
--INI--
date.timezone=America/Toronto
--SKIPIF--
<?php if (!extension_loaded("excel")) die("skip - Excel extension not found"); ?>
--FILE--
<?php 
	$x = new ExcelBook();

	$s = $x->addSheet("Sheet 1");

	$x->setDefaultFont('Times New Roman', 20);
	var_dump($x->getError());
 
	$s->write(2, 2, 'Hello World!');
	var_dump($x->getError());

	var_dump($x->getDefaultFont());



	echo "OK\n";
?>
--EXPECT--
bool(false)
bool(false)
array(2) {
  ["font"]=>
  string(15) "Times New Roman"
  ["font_size"]=>
  int(20)
}
OK
