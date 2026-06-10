--TEST--
Stub signatures match the implementation: setNamedRange named args, setTop10 optionals, setCustomFilter sentinel, rowColToAddr defaults
--EXTENSIONS--
excel
--FILE--
<?php
$b = new ExcelBook(null, null, true);
$s = $b->addSheet("S1");

// named arguments follow the published rows-then-columns signature
var_dump($s->setNamedRange(name: "rng", row_first: 1, row_last: 2, col_first: 3, col_last: 4));
$r = $s->getNamedRange("rng");
var_dump($r["row_first"], $r["row_last"], $r["col_first"], $r["col_last"]);

// rowColToAddr defaults are relative, matching the declared = true
var_dump($s->rowColToAddr(1, 1));
var_dump($s->rowColToAddr(1, 1, true, true));
var_dump($s->rowColToAddr(1, 1, false, false));

// setTop10's trailing args are optional with $top defaulting to true
$s->write(1, 0, "h");
$s->write(2, 0, "a");
$af = $s->autoFilter();
$af->setRef(1, 2, 0, 0);
$fc = $af->column(0);
var_dump($fc->setTop10(5.0));
$t = $fc->getTop10();
var_dump($t["value"], $t["top"], $t["percent"]);

// setCustomFilter single-criterion form, with and without the documented defaults
var_dump($fc->setCustomFilter(2, "a"));
var_dump($fc->setCustomFilter(2, "a", -1, null));
$cf = $fc->getCustomFilter();
var_dump($cf["operator_1"], $cf["value_1"]);

// reflection sees the corrected sentinel defaults
echo (new ReflectionMethod("ExcelSheet", "setProtect"))->getParameters()[2]->getDefaultValue(), "\n";
echo (new ReflectionMethod("ExcelSheet", "setNamedRange"))->getParameters()[5]->getDefaultValue(), "\n";
var_dump((new ReflectionMethod("ExcelSheet", "addTable"))->getParameters()[5]->getDefaultValue());

echo "OK\n";
?>
--EXPECT--
bool(true)
int(1)
int(2)
int(3)
int(4)
string(2) "B2"
string(2) "B2"
string(4) "$B$2"
bool(true)
float(5)
bool(true)
bool(false)
bool(true)
bool(true)
int(2)
string(1) "a"
-1
-1
bool(true)
OK
