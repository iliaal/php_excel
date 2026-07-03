--TEST--
Benchmark guard: rectangular bulk reads avoid PHP per-cell dispatch overhead
--EXTENSIONS--
excel
--SKIPIF--
<?php if (!ExcelBook::requiresKey() || !ini_get("excel.license_name") || !ini_get("excel.license_key")) print "skip"; ?>
--FILE--
<?php
$book = new ExcelBook(null, null, true);
$sheet = $book->addSheet("Bench");
for ($row = 1; $row <= 50; $row++) {
    $values = [];
    for ($col = 0; $col < 20; $col++) {
        $values[] = $row * 100 + $col;
    }
    $sheet->writeRow($row, $values);
}

$iterations = 100;

$start = hrtime(true);
$perCellSum = 0;
for ($i = 0; $i < $iterations; $i++) {
    for ($row = 1; $row <= 50; $row++) {
        for ($col = 0; $col < 20; $col++) {
            $perCellSum += (int)$sheet->read($row, $col, $fmt, false);
        }
    }
}
$perCellNs = hrtime(true) - $start;

$start = hrtime(true);
$rangeSum = 0;
for ($i = 0; $i < $iterations; $i++) {
    foreach ($sheet->readRange(1, 50, 0, 19, false) as $values) {
        foreach ($values as $value) {
            $rangeSum += (int)$value;
        }
    }
}
$rangeNs = hrtime(true) - $start;

if ($perCellSum !== $rangeSum) {
    echo "sum mismatch\n";
    var_dump($perCellSum, $rangeSum);
    exit;
}

$ratio = $perCellNs / max(1, $rangeNs);
printf("ratio %.2f\n", $ratio);
echo $ratio >= 1.5 ? "bulk faster: yes\n" : "bulk faster: no\n";
?>
--EXPECTF--
ratio %f
bulk faster: yes
