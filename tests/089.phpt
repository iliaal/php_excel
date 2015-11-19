--TEST--
force string behaviout with single quoted strings 
--INI--
date.timezone=America/Toronto
--SKIPIF--
<?php if (!extension_loaded("excel")) print "skip"; ?>
--FILE--
<?php 
    $xb = new ExcelBook(null, null, true);
    $xb->setLocale('UTF-8');
    $xs = new ExcelSheet($xb, 'test');
    
    var_dump($xs->write(1, 1, "'=3+4"));
    var_dump($xs->write(1, 2, "'"));
    var_dump($xs->write(1, 3, "'3"));
    
    var_dump($xs->isFormula(1, 1));
    var_dump($xs->isFormula(1, 2));
    var_dump($xs->isFormula(1, 3));
    
    var_dump($xs->read(1, 1));
    var_dump($xs->read(1, 2));
    var_dump($xs->read(1, 3));
    
	echo "OK\n";
?>
--EXPECT--
bool(true)
bool(true)
bool(true)
bool(false)
bool(false)
bool(false)
string(4) "=3+4"
string(0) ""
string(1) "3"
OK
