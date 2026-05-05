--TEST--
Verify serialization is blocked for all Excel classes
--EXTENSIONS--
excel
--FILE--
<?php
$classes = [
    'ExcelBook' => fn() => new ExcelBook(null, null, true),
    'ExcelSheet' => function() {
        $b = new ExcelBook(null, null, true);
        return $b->addSheet("S");
    },
    'ExcelFormat' => function() {
        $b = new ExcelBook(null, null, true);
        return $b->addFormat();
    },
    'ExcelFont' => function() {
        $b = new ExcelBook(null, null, true);
        return $b->addFont();
    },
];

foreach ($classes as $name => $factory) {
    $obj = $factory();
    try {
        serialize($obj);
        echo "$name: serialization should have thrown\n";
    } catch (\Exception $e) {
        echo "$name: blocked\n";
    }
}

echo "OK\n";
?>
--EXPECT--
ExcelBook: blocked
ExcelSheet: blocked
ExcelFormat: blocked
ExcelFont: blocked
OK
