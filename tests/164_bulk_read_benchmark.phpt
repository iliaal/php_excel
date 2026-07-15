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

$iterations = 40;
$rounds = 5;

$readPerCell = static function () use ($sheet, $iterations): array {
    $start = hrtime(true);
    $sum = 0;
    for ($i = 0; $i < $iterations; $i++) {
        for ($row = 1; $row <= 50; $row++) {
            for ($col = 0; $col < 20; $col++) {
                $sum += (int)$sheet->read($row, $col, $fmt, false);
            }
        }
    }

    return [hrtime(true) - $start, $sum];
};

$readRange = static function () use ($sheet, $iterations): array {
    $start = hrtime(true);
    $sum = 0;
    for ($i = 0; $i < $iterations; $i++) {
        foreach ($sheet->readRange(1, 50, 0, 19, false) as $values) {
            foreach ($values as $value) {
                $sum += (int)$value;
            }
        }
    }

    return [hrtime(true) - $start, $sum];
};

$ratios = [];
for ($round = 0; $round < $rounds; $round++) {
    if ($round % 2 === 0) {
        [$perCellNs, $perCellSum] = $readPerCell();
        [$rangeNs, $rangeSum] = $readRange();
    } else {
        [$rangeNs, $rangeSum] = $readRange();
        [$perCellNs, $perCellSum] = $readPerCell();
    }

    if ($perCellSum !== $rangeSum) {
        echo "sum mismatch\n";
        var_dump($perCellSum, $rangeSum);
        exit;
    }

    $ratios[] = $perCellNs / max(1, $rangeNs);
}

sort($ratios);
$median = $ratios[intdiv($rounds, 2)];
printf("median ratio %.2f\n", $median);
echo $median >= 1.1 ? "bulk faster: yes\n" : "bulk faster: no\n";
?>
--EXPECTF--
median ratio %f
bulk faster: yes
