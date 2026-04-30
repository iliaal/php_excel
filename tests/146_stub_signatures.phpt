--TEST--
Reflection-based check that ZPP-affected stubs match implementation
--SKIPIF--
<?php if (!extension_loaded("excel")) print "skip"; ?>
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
];

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
insertRow(1,1,false): ArgumentCountError
OK
