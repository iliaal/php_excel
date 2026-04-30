--TEST--
No-argument sheet methods reject extra arguments with ArgumentCountError
--SKIPIF--
<?php if (!extension_loaded("excel")) print "skip"; ?>
--FILE--
<?php
$b = new ExcelBook(null, null, true);
$s = $b->addSheet("S");

foreach (["autoFilter", "applyFilter", "removeFilter", "splitInfo"] as $method) {
    try {
        $s->$method(1);
        echo "$method: no error?\n";
    } catch (ArgumentCountError $e) {
        echo "$method: ArgumentCountError\n";
    }
}

echo "OK\n";
?>
--EXPECT--
autoFilter: ArgumentCountError
applyFilter: ArgumentCountError
removeFilter: ArgumentCountError
splitInfo: ArgumentCountError
OK
