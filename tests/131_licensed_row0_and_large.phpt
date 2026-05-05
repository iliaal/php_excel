--TEST--
Licensed: row 0 access and large cell count (>300)
--EXTENSIONS--
excel
--SKIPIF--
<?php if (!ExcelBook::requiresKey() || !ini_get("excel.license_name") || !ini_get("excel.license_key")) print "skip"; ?>
--FILE--
<?php
$book = new ExcelBook(null, null, true);
$sheet = $book->addSheet("Licensed");

// Row 0 write/read -- blocked in trial
$sheet->write(0, 0, "header1");
$sheet->write(0, 1, "header2");
$sheet->write(0, 2, 42);
echo "row0 read: " . $sheet->read(0, 0) . "\n";
echo "row0 col1: " . $sheet->read(0, 1) . "\n";
echo "row0 col2: " . $sheet->read(0, 2) . "\n";

// Row 0 writeRow/readRow
$sheet->writeRow(0, ["rh1", "rh2", "rh3"], 3);
$row0 = $sheet->readRow(0, 3, 5);
echo "row0 readRow: " . implode(",", $row0) . "\n";

// Write >300 cells in a single book -- trial caps reads at ~300
for ($r = 1; $r <= 50; $r++) {
    for ($c = 0; $c < 10; $c++) {
        $sheet->write($r, $c, $r * 10 + $c);
    }
}
echo "wrote 500 cells\n";

// Read them all back -- trial would fail partway through
$count = 0;
for ($r = 1; $r <= 50; $r++) {
    for ($c = 0; $c < 10; $c++) {
        $val = $sheet->read($r, $c);
        if ($val !== (float)($r * 10 + $c)) {
            echo "MISMATCH at ($r,$c): expected " . ($r * 10 + $c) . " got " . var_export($val, true) . "\n";
        }
        $count++;
    }
}
echo "read back $count cells\n";

// Save/reload and verify row 0 persists
$tmp = tempnam("/tmp", "xl") . ".xlsx";
$book->save($tmp);

$book2 = new ExcelBook(null, null, true);
$book2->loadFile($tmp);
$s2 = $book2->getSheet(0);
echo "reload row0: " . $s2->read(0, 0) . "\n";
echo "reload cell(50,9): " . (int)$s2->read(50, 9) . "\n";

unlink($tmp);
echo "OK\n";
?>
--EXPECT--
row0 read: header1
row0 col1: header2
row0 col2: 42
row0 readRow: rh1,rh2,rh3
wrote 500 cells
read back 500 cells
reload row0: header1
reload cell(50,9): 509
OK
