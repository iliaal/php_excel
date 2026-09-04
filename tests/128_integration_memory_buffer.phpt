--TEST--
Integration: save to memory buffer, load from memory buffer
--EXTENSIONS--
excel
--FILE--
<?php
$book = new ExcelBook(null, null, true);
$sheet = $book->addSheet("MemTest");
$sheet->write(1, 0, "from memory");
$sheet->write(1, 1, 42);

// Save to memory (save without filename returns raw data)
$tmp = tempnam(sys_get_temp_dir(), "xl") . ".xlsx";
$book->save($tmp);
$raw = file_get_contents($tmp);
unlink($tmp);

echo "raw size > 0: " . var_export(strlen($raw) > 0, true) . "\n";

// Load from memory
$book2 = new ExcelBook(null, null, true);
var_dump($book2->load($raw));
$s = $book2->getSheet(0);
echo "name: " . $s->name() . "\n";
echo "val: " . $s->read(1, 0) . "\n";
echo "num: " . $s->read(1, 1) . "\n";

echo "OK\n";
?>
--EXPECT--
raw size > 0: true
bool(true)
name: MemTest
val: from memory
num: 42
OK
