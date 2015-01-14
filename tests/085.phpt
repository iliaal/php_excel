--TEST--
test Sheet::indexToColName() and Sheet::colNameToIndex()
--SKIPIF--
<?php if (!extension_loaded("excel")) print "skip"; ?>
--FILE--
<?php

// check all valid columns [0..16383]
// and produce no output in
$out = 'OK';
for($i=0; $i < 16384; $i++){
    $colName = ExcelSheet::indexToColName($i);
    $index = ExcelSheet::colNameToIndex($colName);
    
    if ($index !== $i) {
        $out = sprintf('ERROR %d %s %d', $i, $colName, $index);
    }
}

var_dump($out);

var_dump(
    ExcelSheet::indexToColName(0),
    ExcelSheet::indexToColName(1),
    ExcelSheet::indexToColName(24),
    ExcelSheet::indexToColName(25),
    ExcelSheet::indexToColName(26),
    ExcelSheet::indexToColName(27),
    ExcelSheet::indexToColName(28),
    ExcelSheet::indexToColName(50),
    ExcelSheet::indexToColName(51),
    ExcelSheet::indexToColName(52),
    ExcelSheet::indexToColName(53),
    ExcelSheet::indexToColName(700),
    ExcelSheet::indexToColName(701),
    ExcelSheet::indexToColName(702),
    ExcelSheet::indexToColName(16383),
    
    ExcelSheet::colNameToIndex("A"),
    ExcelSheet::colNameToIndex("B"),
    ExcelSheet::colNameToIndex("Y"),
    ExcelSheet::colNameToIndex("Z"),
    ExcelSheet::colNameToIndex("AA"),
    ExcelSheet::colNameToIndex("AB"),
    ExcelSheet::colNameToIndex("AC"),
    ExcelSheet::colNameToIndex("AY"),
    ExcelSheet::colNameToIndex("AZ"),
    ExcelSheet::colNameToIndex("BA"),
    ExcelSheet::colNameToIndex("BB"),
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
string(2) "OK"
string(1) "A"
string(1) "B"
string(1) "Y"
string(1) "Z"
string(2) "AA"
string(2) "AB"
string(2) "AC"
string(2) "AY"
string(2) "AZ"
string(2) "BA"
string(2) "BB"
string(2) "ZY"
string(2) "ZZ"
string(3) "AAA"
string(3) "XFD"
int(0)
int(1)
int(24)
int(25)
int(26)
int(27)
int(28)
int(50)
int(51)
int(52)
int(53)
int(700)
int(701)
int(702)
int(16383)
bool(false)
bool(false)
bool(false)
bool(false)