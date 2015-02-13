--TEST--
Picture Handling
--INI--
date.timezone=America/Toronto
--SKIPIF--
<?php if (!extension_loaded("excel")) die("skip - Excel extension not found"); ?>
--FILE--
<?php 
	$x = new ExcelBook();

	$s = $x->addSheet("Sheet 1");

	$pic_file = dirname(__FILE__) . "/phplogo.jpg";

	$pic = $x->addPictureFromFile($pic_file);
	var_dump($pic, $x->getError());

	$pic2 = $x->addPictureFromString(file_get_contents($pic_file));
	var_dump($pic2, $x->getError());

	$s->addPictureScaled(1,1,$pic,0.1);
	var_dump($x->getError());
	$s->addPictureScaled(2,2,$pic2,0.1);
	var_dump($x->getError());

	$s->addPictureDim(10,10,$pic,578,406);
	var_dump($x->getError());
	$s->addPictureDim(20,20,$pic2,578,406);
	var_dump($x->getError());

	$s->addPictureDim(30,30,$pic,578/2,406/2);
	var_dump($x->getError());
	$s->addPictureDim(40,40,$pic2,578/2,406/2);
	var_dump($x->getError());



	echo "OK\n";
?>
--EXPECTF--
int(%d)
bool(false)
int(%d)
bool(false)
bool(false)
bool(false)
bool(false)
bool(false)
bool(false)
bool(false)
OK
