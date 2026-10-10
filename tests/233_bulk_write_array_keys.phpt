--TEST--
Bulk writers ignore array keys while keyed writes preserve sparse coordinates
--EXTENSIONS--
excel
--FILE--
<?php
foreach ([false, true] as $xlsx) {
    echo $xlsx ? "xlsx\n" : "xls\n";
    $book = new ExcelBook(null, null, $xlsx);
    $sheet = $book->addSheet('Keys');
    // Deliberately non-sorted, negative, string, and out-of-range keys.
    $values = [100000000 => 'first', -7 => 0, 'named' => false, 2 => 'last'];
    $original = $values;
    var_dump($sheet->writeRow(1, $values, 2));
    var_dump($sheet->readRow(1, 1, 6) === ['', 'first', 0.0, false, 'last', '']);
    var_dump($sheet->writeCol(1, $values, 2));
    var_dump($sheet->readCol(1, 1, 6) === ['', 'first', 0.0, false, 'last', '']);
    var_dump($values === $original);

    $source = $book->addSheet('Source');
    $target = $book->addSheet('Copy');
    $source->write(1, 1, 'A');
    $source->write(1, 3, false);
    $source->write(3, 1, 0);

    $row = $source->readSparseRow(1, 1, 3);
    var_dump($row === [1 => 'A', 3 => false]);
    var_dump($target->writeRow(1, $row, 1));
    var_dump($target->readRow(1, 1, 3) === ['A', false, '']);
    foreach ($row as $column => $value) {
        $target->write(2, $column, $value);
    }
    var_dump($target->readRow(2, 1, 3) === ['A', '', false]);

    $col = $source->readSparseCol(1, 1, 3);
    var_dump($col === [1 => 'A', 3 => 0.0]);
    var_dump($target->writeCol(4, $col, 1));
    var_dump($target->readCol(4, 1, 3) === ['A', 0.0, '']);
    foreach ($col as $rowIndex => $value) {
        $target->write($rowIndex, 5, $value);
    }
    var_dump($target->readCol(5, 1, 3) === ['A', '', 0.0]);
}
?>
--EXPECT--
xls
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
bool(true)
xlsx
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
bool(true)
