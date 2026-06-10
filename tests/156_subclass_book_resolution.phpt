--TEST--
Subclasses of Excel classes resolve their owning book through the parent chain
--EXTENSIONS--
excel
--FILE--
<?php
class MyBook extends ExcelBook {}
class MySheet extends ExcelSheet {}

// child wrappers created from a subclassed book must resolve their owning book
$book = new MyBook(null, null, true);
$sheet = $book->addSheet("Sheet1");
var_dump($sheet->write(1, 0, "hello"));
var_dump($sheet->read(1, 0));

// same-book guard accepts a font owned by the subclassed book
$font = $book->addFont();
$font->name("Arial");
$format = $book->addFormat();
var_dump($format->setFont($font));

// mid-chain subclass: autofilter created from a subclassed sheet
$sheet2 = new MySheet($book, "Sheet2");
$sheet2->write(1, 0, "Name");
$sheet2->write(2, 0, "A");
$af = $sheet2->autoFilter();
$af->setRef(1, 2, 0, 0);
$ref = $af->getRef();
var_dump($ref["row_first"], $ref["row_last"], $ref["col_first"], $ref["col_last"]);

// cross-book guard still rejects a foreign font
$other = new ExcelBook(null, null, true);
$otherFont = $other->addFont();
var_dump(@$format->setFont($otherFont));

echo "OK\n";
?>
--EXPECT--
bool(true)
string(5) "hello"
bool(true)
int(1)
int(2)
int(0)
int(0)
bool(false)
OK
