--TEST--
ExcelCoreProperties: get/set title, subject, creator, tags, categories, comments, createdAsDouble, removeAll (xlsx only)
--EXTENSIONS--
excel
--FILE--
<?php
$book = new ExcelBook(null, null, true);
$sheet = $book->addSheet("Sheet1");

$cp = $book->coreProperties();
var_dump($cp instanceof ExcelCoreProperties);

var_dump($cp->setTitle("Test Title"));
var_dump($cp->title());

var_dump($cp->setSubject("Test Subject"));
var_dump($cp->subject());

var_dump($cp->setCreator("Test Creator"));
var_dump($cp->creator());

var_dump($cp->setTags("tag1, tag2"));
var_dump($cp->tags());

var_dump($cp->setCategories("cat1"));
var_dump($cp->categories());

var_dump($cp->setComments("A comment"));
var_dump($cp->comments());

$d = $cp->createdAsDouble();
var_dump(is_float($d));

var_dump($cp->setCreatedAsDouble(44000.5));
var_dump($cp->createdAsDouble());

var_dump($cp->removeAll());
var_dump($cp->title());
var_dump($cp->subject());

echo "OK\n";
?>
--EXPECT--
bool(true)
bool(true)
string(10) "Test Title"
bool(true)
string(12) "Test Subject"
bool(true)
string(12) "Test Creator"
bool(true)
string(10) "tag1, tag2"
bool(true)
string(4) "cat1"
bool(true)
string(9) "A comment"
bool(true)
bool(true)
float(44000.5)
bool(true)
NULL
NULL
OK
