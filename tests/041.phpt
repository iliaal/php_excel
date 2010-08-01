--TEST--
Page Orientation 
--INI--
date.timezone=America/Toronto
--SKIPIF--
<?php if (!extension_loaded("excel")) print "skip"; ?>
--FILE--
<?php 
	$x = new ExcelBook();

	$s = $x->addSheet("Sheet 1");

	$s->write(1, 1, "Test");
	$s->write(12, 11, "Test 2");

	$oClass = new ReflectionClass('ExcelSheet');
    foreach ($oClass->getConstants() as $c => $val) {
    	if (strpos($c, 'PAPER_') !== 0) {
    		continue;
		}

		$s->setPaper($val);
		var_dump($x->getError(), $s->paper());
	}


	
	echo "OK\n";
?>
--EXPECT--
bool(false)
int(0)
bool(false)
int(1)
bool(false)
int(2)
bool(false)
int(3)
bool(false)
int(4)
bool(false)
int(5)
bool(false)
int(6)
bool(false)
int(7)
bool(false)
int(8)
bool(false)
int(9)
bool(false)
int(10)
bool(false)
int(11)
bool(false)
int(12)
bool(false)
int(13)
bool(false)
int(14)
bool(false)
int(15)
bool(false)
int(16)
bool(false)
int(17)
bool(false)
int(18)
bool(false)
int(19)
bool(false)
int(20)
bool(false)
int(21)
bool(false)
int(22)
bool(false)
int(23)
bool(false)
int(24)
bool(false)
int(25)
bool(false)
int(26)
bool(false)
int(27)
bool(false)
int(28)
bool(false)
int(29)
bool(false)
int(30)
bool(false)
int(31)
bool(false)
int(32)
bool(false)
int(33)
bool(false)
int(34)
bool(false)
int(35)
bool(false)
int(36)
bool(false)
int(37)
bool(false)
int(38)
bool(false)
int(39)
bool(false)
int(40)
bool(false)
int(41)
OK
