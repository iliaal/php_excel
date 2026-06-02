--TEST--
Reflection-based check that ZPP-affected stubs match implementation
--EXTENSIONS--
excel
--FILE--
<?php
$cases = [
    // [class, method, expected param count, expected param types]
    [ExcelBook::class, "addPictureFromFile", 1, ["string"]],
    [ExcelBook::class, "addPictureFromString", 1, ["string"]],
    [ExcelSheet::class, "insertRow", 2, ["int", "int"]],
    [ExcelSheet::class, "insertCol", 2, ["int", "int"]],
    [ExcelSheet::class, "removeRow", 2, ["int", "int"]],
    [ExcelSheet::class, "removeCol", 2, ["int", "int"]],
    [ExcelSheet::class, "horPageBreak", 2, ["int", "bool"]],
    [ExcelSheet::class, "verPageBreak", 2, ["int", "bool"]],
    [ExcelSheet::class, "setPaper", 1, ["int"]],
    [ExcelSheet::class, "setPrintRepeatRows", 2, ["int", "int"]],
    [ExcelSheet::class, "setPrintRepeatCols", 2, ["int", "int"]],
    [ExcelTable::class, "__construct", 8, ["ExcelSheet", "string", "int", "int", "int", "int", "bool", "int"]],
];

$cases[] = [ExcelConditionalFormatting::class, "addRule", 4, ["int", "ExcelConditionalFormat", "string", "bool"]];
$cases[] = [ExcelConditionalFormatting::class, "addTopRule", 5, ["ExcelConditionalFormat", "int", "bool", "bool", "bool"]];
$cases[] = [ExcelConditionalFormatting::class, "addOpNumRule", 5, ["int", "ExcelConditionalFormat", "float", "float", "bool"]];
$cases[] = [ExcelConditionalFormatting::class, "addOpStrRule", 5, ["int", "ExcelConditionalFormat", "string", "string", "bool"]];
$cases[] = [ExcelConditionalFormatting::class, "addAboveAverageRule", 5, ["ExcelConditionalFormat", "bool", "bool", "int", "bool"]];
$cases[] = [ExcelConditionalFormatting::class, "addTimePeriodRule", 3, ["ExcelConditionalFormat", "int", "bool"]];

// Setters whose stubs were tightened from mixed to the concrete scalar the C ZPP parses.
$cases[] = [ExcelSheet::class, "setZoom", 1, ["int"]];
$cases[] = [ExcelSheet::class, "setZoomPrint", 1, ["int"]];
$cases[] = [ExcelSheet::class, "setPrintGridlines", 1, ["bool"]];
$cases[] = [ExcelSheet::class, "setLandscape", 1, ["bool"]];
$cases[] = [ExcelSheet::class, "setHCenter", 1, ["bool"]];
$cases[] = [ExcelSheet::class, "setVCenter", 1, ["bool"]];
$cases[] = [ExcelSheet::class, "setMarginLeft", 1, ["float"]];
$cases[] = [ExcelSheet::class, "setMarginBottom", 1, ["float"]];
$cases[] = [ExcelSheet::class, "setHeader", 2, ["string", "float"]];
$cases[] = [ExcelSheet::class, "setFooter", 2, ["string", "float"]];

foreach ($cases as [$cls, $m, $count, $types]) {
    $r = new ReflectionMethod($cls, $m);
    if ($r->getNumberOfParameters() !== $count) {
        echo "$cls::$m param count mismatch: got " . $r->getNumberOfParameters() . " want $count\n";
        continue;
    }
    foreach ($r->getParameters() as $i => $p) {
        $t = $p->getType()?->getName() ?? "(none)";
        if ($t !== $types[$i]) {
            echo "$cls::$m param $i type mismatch: got $t want $types[$i]\n";
            continue 2;
        }
    }
    echo "$cls::$m: OK\n";
}

// ZPP runtime: insertRow with 3 args must error
try {
    (new ExcelBook(null, null, true))->addSheet("S")->insertRow(1, 1, false);
    echo "insertRow(1,1,false): unexpected pass\n";
} catch (ArgumentCountError $e) {
    echo "insertRow(1,1,false): ArgumentCountError\n";
}

echo "OK\n";
?>
--EXPECT--
ExcelBook::addPictureFromFile: OK
ExcelBook::addPictureFromString: OK
ExcelSheet::insertRow: OK
ExcelSheet::insertCol: OK
ExcelSheet::removeRow: OK
ExcelSheet::removeCol: OK
ExcelSheet::horPageBreak: OK
ExcelSheet::verPageBreak: OK
ExcelSheet::setPaper: OK
ExcelSheet::setPrintRepeatRows: OK
ExcelSheet::setPrintRepeatCols: OK
ExcelTable::__construct: OK
ExcelConditionalFormatting::addRule: OK
ExcelConditionalFormatting::addTopRule: OK
ExcelConditionalFormatting::addOpNumRule: OK
ExcelConditionalFormatting::addOpStrRule: OK
ExcelConditionalFormatting::addAboveAverageRule: OK
ExcelConditionalFormatting::addTimePeriodRule: OK
ExcelSheet::setZoom: OK
ExcelSheet::setZoomPrint: OK
ExcelSheet::setPrintGridlines: OK
ExcelSheet::setLandscape: OK
ExcelSheet::setHCenter: OK
ExcelSheet::setVCenter: OK
ExcelSheet::setMarginLeft: OK
ExcelSheet::setMarginBottom: OK
ExcelSheet::setHeader: OK
ExcelSheet::setFooter: OK
insertRow(1,1,false): ArgumentCountError
OK
