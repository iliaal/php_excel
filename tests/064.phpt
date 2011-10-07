--TEST--
New Picture Functions
--SKIPIF--
<?php if (!extension_loaded("excel")) print "skip"; ?>
--FILE--
<?php 
	$x = new ExcelBook();

	$s = $x->addSheet("Sheet 1");

	var_dump($s->getNumPictures());
	var_dump($x->getNumPictures());

	$pic_file = dirname(__FILE__) . "/phplogo.jpg";

	$pic = $x->addPictureFromFile($pic_file);
	var_dump($pic, $x->getError());

	$pic2 = $x->addPictureFromString(file_get_contents($pic_file));
	var_dump($pic2, $x->getError());

	$s->addPictureScaled(1,1,$pic,0.1);
	var_dump($x->getError());
	$s->addPictureScaled(2,2,$pic2,0.1);
	var_dump($x->getError());

	var_dump($s->getNumPictures());
	var_dump($x->getNumPictures());

	$s->addPictureDim(10,10,$pic,578,406);
	var_dump($x->getError());
	$s->addPictureDim(20,20,$pic2,578,406);
	var_dump($x->getError());

	var_dump($s->getNumPictures());
	var_dump($x->getNumPictures());

	$s->addPictureDim(30,30,$pic,578/2,406/2);
	var_dump($x->getError());
	$s->addPictureDim(40,40,$pic2,578/2,406/2);
	var_dump($x->getError());

	var_dump($s->getNumPictures());
	var_dump($x->getNumPictures());
	
	var_dump($s->getPictureInfo(0));
	var_dump($s->getPictureInfo(10));

	$info = $s->getPictureInfo(0);
	$pic = $x->getPicture($info['picture_index']);
	var_dump($pic['type'] == $x::PICTURETYPE_JPEG, strlen($pic['data']) == filesize($pic_file));

	echo "OK\n";
?>
--EXPECTF--
int(0)
int(0)
int(%d)
bool(false)
int(%d)
bool(false)
bool(false)
bool(false)
int(2)
int(2)
bool(false)
bool(false)
int(4)
int(2)
bool(false)
bool(false)
int(6)
int(2)
array(9) {
  ["picture_index"]=>
  int(%d)
  ["row_top"]=>
  int(1)
  ["col_left"]=>
  int(1)
  ["row_bottom"]=>
  int(3)
  ["col_right"]=>
  int(1)
  ["width"]=>
  int(57)
  ["height"]=>
  int(40)
  ["offset_x"]=>
  int(0)
  ["offset_y"]=>
  int(0)
}
bool(false)
bool(true)
bool(true)
OK