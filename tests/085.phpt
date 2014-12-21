--TEST--
test Sheet::indexToColName() and Sheet::colNameToIndex()
--SKIPIF--
<?php if (!extension_loaded("excel")) print "skip"; ?>
--FILE--
<?php

var_dump(
    ExcelSheet::indexToColName(0),
    ExcelSheet::indexToColName(1),
    ExcelSheet::indexToColName(25),
    ExcelSheet::indexToColName(26),
    ExcelSheet::indexToColName(27),
    ExcelSheet::indexToColName(700),
    ExcelSheet::indexToColName(701),
    ExcelSheet::indexToColName(702),
    ExcelSheet::indexToColName(16383),
    
    ExcelSheet::colNameToIndex("A"),
    ExcelSheet::colNameToIndex("B"),
    ExcelSheet::colNameToIndex("Z"),
    ExcelSheet::colNameToIndex("AA"),
    ExcelSheet::colNameToIndex("AB"),
    ExcelSheet::colNameToIndex("ZY"),
    ExcelSheet::colNameToIndex("ZZ"),
    ExcelSheet::colNameToIndex("AAA"),
    ExcelSheet::colNameToIndex("XFD"),
    
    ExcelSheet::indexToColName(-1),
    ExcelSheet::indexToColName(16384),
    ExcelSheet::colNameToIndex("XFE"),
    ExcelSheet::colNameToIndex("a")
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