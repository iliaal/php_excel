--TEST--
NUL-safe string and int-range guards on libxl boundary
--EXTENSIONS--
excel
--FILE--
<?php
$book = new ExcelBook(null, null, true);

// addSheet rejects embedded NUL
var_dump(@$book->addSheet("safe\0hidden"));

// addSheet accepts a clean name
$sheet = $book->addSheet("legit");
var_dump($sheet instanceof ExcelSheet);

// setLocale rejects embedded NUL
var_dump(@$book->setLocale("en_US\0EVIL"));

// addCustomFormat rejects embedded NUL
var_dump(@$book->addCustomFormat("0.00\0EVIL"));

// loadInfo rejects NUL in filename
var_dump(@$book->loadInfo("/etc/passwd\0.xls"));

// setName on Sheet rejects NUL
var_dump(@$sheet->setName("public\0private"));

// setNamedRange rejects NUL in name
var_dump(@$sheet->setNamedRange("public\0private", 0, 0, 1, 1));

// addHyperlink rejects NUL
var_dump(@$sheet->addHyperlink("https://safe\0https://evil", 0, 0, 1, 1));

// writeComment rejects NUL in value or author
var_dump(@$sheet->writeComment(0, 0, "value\0evil", "author", 100, 50));
var_dump(@$sheet->writeComment(0, 0, "value", "author\0evil", 100, 50));

// writeError now requires row/col/iError (was previously all optional)
try { $sheet->writeError(); echo "MISSING_ARGS_NOT_REJECTED\n"; }
catch (\ArgumentCountError $e) { echo "ARGS_REQUIRED\n"; }

// writeError rejects out-of-int-range row
var_dump(@$sheet->writeError(PHP_INT_MAX, 0, 1));

// addHyperlink rejects out-of-int-range row
var_dump(@$sheet->addHyperlink("https://example.com", PHP_INT_MAX, 0, 1, 1));

// readRow caps the column range
$rows = @$sheet->readRow(0, 0, PHP_INT_MAX);
var_dump($rows);

// Happy path: writeError with valid args
$fmt = $book->addFormat();
$sheet->writeError(0, 0, 1, $fmt);
echo "writeError ok\n";

echo "OK\n";
?>
--EXPECTF--
bool(false)
bool(true)
bool(false)
bool(false)
bool(false)
bool(false)
bool(false)
bool(false)
bool(false)
bool(false)
ARGS_REQUIRED
bool(false)
bool(false)
bool(false)
writeError ok
OK
