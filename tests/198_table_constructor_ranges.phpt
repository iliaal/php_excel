--TEST--
ExcelTable constructor rejects inverted ranges before calling LibXL
--EXTENSIONS--
excel
--FILE--
<?php
$book = new ExcelBook(null, null, true);
$sheet = $book->addSheet('Sheet1');

try {
    new ExcelTable($sheet, 'BadRows', 3, 1, 0, 1);
    echo "inverted rows accepted\n";
} catch (Throwable $e) {
    echo "inverted rows rejected\n";
}

try {
    new ExcelTable($sheet, 'BadCols', 1, 2, 2, 0);
    echo "inverted columns accepted\n";
} catch (Throwable $e) {
    echo "inverted columns rejected\n";
}

$single = new ExcelTable($sheet, 'SingleCell', 5, 5, 3, 3, false);
var_dump($single instanceof ExcelTable);
var_dump(str_starts_with($book->save(), "PK\x03\x04"));
?>
--EXPECT--
inverted rows rejected
inverted columns rejected
bool(true)
bool(true)
