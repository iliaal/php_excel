--TEST--
Width/Height Checks
--INI--
date.timezone=America/Toronto
--SKIPIF--
<?php if (!extension_loaded("excel")) die("skip - Excel extension not found"); ?>
--FILE--
<?php 
	$x = new ExcelBook();

	$s = $x->addSheet("Sheet 1");

	$s->setColWidth(1, 1, 55);
	$s->setRowHeight(4, 50);

	var_dump($s->colWidth(1));
	var_dump($s->rowHeight(4));

    $s->setColWidth(2, 2, -1);
    var_dump($s->colWidth(2));
	
    $s->write(1, 3, 'TestTestTestTestTest');
    $colWidthBefore = $s->colWidth(3);
    $s->setColWidth(3, 3, -1);
    $colWidthAfter = $s->colWidth(3);
    
    var_dump($colWidthBefore < $colWidthAfter);
    
    $s->setColWidth(4, 4, -2);
    
	echo "OK\n";
?>
--EXPECTF--
float(55)
float(50)
float(8)
bool(true)

Warning: ExcelSheet::setColWidth(): Width cannot be less then -1 in %s on line %d
OK
	