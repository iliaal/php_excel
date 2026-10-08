--TEST--
Documented bulk-read example preserves inclusive bounds and dense/sparse indexes
--EXTENSIONS--
excel
--FILE--
<?php
foreach ([false, true] as $xlsx) {
    $book = new ExcelBook(null, null, $xlsx);
    $sheet = $book->addSheet('Read example');
    $sheet->write(1, 1, 'A');
    $sheet->write(1, 3, false);
    $sheet->write(3, 1, 0);

    var_dump($sheet->readRow(1, 1, 3) === ['A', '', false]);
    var_dump($sheet->readSparseRow(1, 1, 3) === [1 => 'A', 3 => false]);
    var_dump($sheet->readCol(1, 1, 3) === ['A', '', 0.0]);
    var_dump($sheet->readSparseCol(1, 1, 3) === [1 => 'A', 3 => 0.0]);

    $rowEnd = $sheet->lastRow();
    $colEnd = $sheet->lastCol();
    // LibXL may extend the used range beyond our data (including in trial mode).
    var_dump($rowEnd >= 4 && $colEnd >= 4);
    $rows = ($rowEnd > 1 && $colEnd > 1)
        ? $sheet->readRange(1, $rowEnd - 1, 1, $colEnd - 1)
        : [];
    var_dump(count($rows) === $rowEnd - 1 && count($rows[0]) === $colEnd - 1
        && array_slice($rows[0], 0, 3) === ['A', '', false]
        && array_slice($rows[1], 0, 3) === ['', '', '']
        && array_slice($rows[2], 0, 3) === [0.0, '', '']);
}
?>
--EXPECT--
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
