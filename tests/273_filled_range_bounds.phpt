--TEST--
Filled range exclusive ends convert to inclusive readRange bounds in XLS and XLSX
--EXTENSIONS--
excel
--FILE--
<?php
foreach ([false, true] as $xlsx) {
    $book = new ExcelBook(null, null, $xlsx);
    $sheet = $book->addSheet('Filled bounds');
    $readFilled = static function () use ($sheet): array {
        // Stay below trial content in row/column zero. The filled range can
        // start before our data, especially for XLS trial workbooks.
        $rowStart = max(3, $sheet->firstFilledRow());
        $rowEnd = $sheet->lastFilledRow();
        $colStart = max(5, $sheet->firstFilledCol());
        $colEnd = $sheet->lastFilledCol();
        return ($rowStart < $rowEnd && $colStart < $colEnd)
            ? $sheet->readRange($rowStart, $rowEnd - 1, $colStart, $colEnd - 1)
            : [];
    };

    var_dump($readFilled() === []);
    $sheet->write(3, 5, 'A');
    var_dump([$sheet->lastFilledRow(), $sheet->lastFilledCol()] === [4, 6]);
    var_dump($readFilled() === [['A']]);

    $sheet->write(4, 6, 0);
    $sheet->write(5, 7, false);
    var_dump([$sheet->lastFilledRow(), $sheet->lastFilledCol()] === [6, 8]);
    var_dump($readFilled() === [
        ['A', '', ''],
        ['', 0.0, ''],
        ['', '', false],
    ]);
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
