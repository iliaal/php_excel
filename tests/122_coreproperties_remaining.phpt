--TEST--
CoreProperties: created, setCreated, modified, setModified, lastModifiedBy, setLastModifiedBy, modifiedAsDouble, setModifiedAsDouble
--EXTENSIONS--
excel
--FILE--
<?php
$book = new ExcelBook(null, null, true);
$book->addSheet("Sheet1");

$cp = $book->coreProperties();

// lastModifiedBy
var_dump($cp->setLastModifiedBy("Test User"));
var_dump($cp->lastModifiedBy());

// created (string)
$orig = $cp->created();
var_dump(is_string($orig) || is_null($orig));
var_dump($cp->setCreated("2024-01-01T00:00:00Z"));
var_dump($cp->created());

// modified (string)
var_dump($cp->setModified("2024-06-15T12:00:00Z"));
var_dump($cp->modified());

// modifiedAsDouble
$md = $cp->modifiedAsDouble();
var_dump(is_float($md));
var_dump($cp->setModifiedAsDouble(45000.5));
var_dump($cp->modifiedAsDouble());

echo "OK\n";
?>
--EXPECT--
bool(true)
string(9) "Test User"
bool(true)
bool(true)
string(20) "2024-01-01T00:00:00Z"
bool(true)
string(20) "2024-06-15T12:00:00Z"
bool(true)
bool(true)
float(45000.5)
OK
