--TEST--
test Sheet::colNameFromIndex() and Sheet::indexFromColName()
--SKIPIF--
<?php if (!extension_loaded("excel")) print "skip"; ?>
--FILE--
<?php

var_dump(
    ExcelSheet::colNameFromIndex(0),
    ExcelSheet::colNameFromIndex(1),
    ExcelSheet::colNameFromIndex(25),
    ExcelSheet::colNameFromIndex(26),
    ExcelSheet::colNameFromIndex(27),
    ExcelSheet::colNameFromIndex(700),
    ExcelSheet::colNameFromIndex(701),
    ExcelSheet::colNameFromIndex(702),
    ExcelSheet::colNameFromIndex(16383),
    
    ExcelSheet::indexFromColName("A"),
    ExcelSheet::indexFromColName("B"),
    ExcelSheet::indexFromColName("Z"),
    ExcelSheet::indexFromColName("AA"),
    ExcelSheet::indexFromColName("AB"),
    ExcelSheet::indexFromColName("ZY"),
    ExcelSheet::indexFromColName("ZZ"),
    ExcelSheet::indexFromColName("AAA"),
    ExcelSheet::indexFromColName("XFD"),
    
    ExcelSheet::colNameFromIndex(-1),
    ExcelSheet::colNameFromIndex(16384),
    ExcelSheet::indexFromColName("XFE"),
    ExcelSheet::indexFromColName("a")
);

?>
--EXPECT--
string(1) "A"
string(1) "B"
string(1) "Z"
string(2) "AA"
string(2) "AB"
string(2) "ZY"
string(2) "ZZ"
string(3) "AAA"
string(3) "XFD"
int(0)
int(1)
int(25)
int(26)
int(27)
int(700)
int(701)
int(702)
int(16383)
bool(false)
bool(false)
bool(false)
bool(false)