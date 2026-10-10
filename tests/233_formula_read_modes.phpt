--TEST--
Formula and cached-value modes agree across all readers in XLS and XLSX
--EXTENSIONS--
excel
--FILE--
<?php
function checkReaders(ExcelSheet $sheet): void {
    foreach ([true, false] as $formulas) {
        $format = null;
        $expected = $formulas ? 'B2*2' : 0.0;
        $values = [
            $sheet->read(1, 2, $format, $formulas),
            $sheet->readRow(1, 2, 2, $formulas)[0],
            $sheet->readCol(2, 1, 1, $formulas)[0],
            $sheet->readRange(1, 1, 2, 2, $formulas)[0][0],
            $sheet->readSparseRow(1, 2, 2, $formulas)[2],
            $sheet->readSparseCol(2, 1, 1, $formulas)[1],
        ];
        foreach ($values as $value) {
            if ($value !== $expected) {
                var_dump($value);
                throw new Exception('Unexpected formula read result');
            }
        }
        echo $formulas ? "formula text: all six readers\n" : "cached zero: all six readers\n";
    }
}
foreach ([false, true] as $xlsx) {
    echo $xlsx ? "XLSX\n" : "XLS\n";
    $book = new ExcelBook(null, null, $xlsx);
    $sheet = $book->addSheet('Formulas');
    $sheet->write(1, 1, 7);
    $sheet->write(1, 2, '=B2*2');
    checkReaders($sheet);
    // Neither changing an input nor saving evaluates the formula.
    $sheet->write(1, 1, 9);
    $copy = new ExcelBook(null, null, $xlsx);
    var_dump($copy->load($book->save()));
    checkReaders($copy->getSheet(0));
}
?>
--EXPECT--
XLS
formula text: all six readers
cached zero: all six readers
bool(true)
formula text: all six readers
cached zero: all six readers
XLSX
formula text: all six readers
cached zero: all six readers
bool(true)
formula text: all six readers
cached zero: all six readers
