--TEST--
Cell protection flags toggle independently and survive XLS/XLSX save/load
--EXTENSIONS--
excel
--FILE--
<?php
function check(bool $condition, string $label): void {
    if (!$condition) {
        echo "FAIL: $label\n";
    }
}

function checkFlags(ExcelFormat $format, array $flags, string $label): void {
    foreach ($flags as $method => $value) {
        check($format->$method() === $value, "$label $method getter");
        check($format->$method(null) === $value, "$label $method null getter");
        check($format->$method() === $value, "$label $method after null");
    }
}

foreach ([false, true] as $xlsx) {
    $kind = $xlsx ? 'XLSX' : 'XLS';
    $book = new ExcelBook(null, null, $xlsx);
    $sheet = $book->addSheet('Protection flags');
    $cases = [];
    $row = 1;
    foreach ([false, true] as $locked) {
        foreach ([false, true] as $hidden) {
            $flags = ['locked' => $locked, 'hidden' => $hidden];
            foreach ([['locked', 'hidden'], ['hidden', 'locked']] as $order) {
                $format = $book->addFormat();
                // Seed the opposite state so every final setter changes a bit.
                $current = ['locked' => !$locked, 'hidden' => !$hidden];
                foreach ($current as $method => $value) {
                    check($format->$method($value) === $value, "$kind seed $method");
                }
                checkFlags($format, $current, "$kind row $row seeded");
                foreach ($order as $method) {
                    check($format->$method($flags[$method]) === $flags[$method], "$kind set $method");
                    $current[$method] = $flags[$method];
                    // Updating one flag must leave the other one unchanged.
                    checkFlags($format, $current, "$kind row $row after $method");
                }
                foreach ([0, 1] as $column) {
                    $value = "flags $row,$column";
                    if ($column === 0) {
                        check($sheet->write($row, $column, $value, $format), 'write formatted cell');
                    } else {
                        check($sheet->write($row, $column, $value), 'write cell');
                        $sheet->setCellFormat($row, $column, $format);
                    }
                    $cases[] = [$row, $column, $value, $flags];
                }
                $row++;
            }
        }
    }

    $loaded = new ExcelBook(null, null, $xlsx);
    check($loaded->load($book->save()), 'reload workbook');
    foreach ([$sheet, $loaded->getSheet(0)] as $stage => $readSheet) {
        foreach ($cases as [$row, $column, $value, $flags]) {
            $format = null;
            check($readSheet->read($row, $column, $format) === $value, 'read cell');
            checkFlags($format, $flags, "$kind stage $stage read $row,$column");
            checkFlags($readSheet->cellFormat($row, $column), $flags,
                "$kind stage $stage cellFormat $row,$column");
        }
    }
    echo "$kind complete\n";
}
?>
--EXPECT--
XLS complete
XLSX complete
