--TEST--
Sheet: setTabColor, writeError, addIgnoredError, isRichStr
--EXTENSIONS--
excel
--FILE--
<?php
$book = new ExcelBook(null, null, true);
$sheet = $book->addSheet("Sheet1");

// setTabColor
var_dump($sheet->setTabColor(8));

// writeError wraps a void libxl op: null on success, false on failure.
// (A format is passed explicitly, as in the original coverage.)
$fmt = $book->addFormat();
var_dump($sheet->writeError(1, 0, ExcelSheet::ERRORTYPE_DIV_0, $fmt));
var_dump($sheet->cellType(1, 0));
echo "div0 read: ";
var_dump($sheet->read(1, 0));

// Every writable iError code round-trips through read() as its libxl code.
// ERRORTYPE_NOERROR (0xFF) means "no error", so it is not writable data.
foreach (["ERRORTYPE_NULL", "ERRORTYPE_DIV_0", "ERRORTYPE_VALUE", "ERRORTYPE_REF", "ERRORTYPE_NAME", "ERRORTYPE_NUM", "ERRORTYPE_NA"] as $i => $name) {
    $code = constant("ExcelSheet::$name");
    $sheet->writeError($i + 2, 0, $code, $fmt);
    echo "$name: ";
    var_dump($sheet->read($i + 2, 0));
}

// Failure side: an out-of-int-range iError is rejected and the cell is untouched.
var_dump(@$sheet->writeError(9, 0, PHP_INT_MAX, $fmt));
echo "rejected cell: ";
var_dump($sheet->read(9, 0));

// addIgnoredError
$sheet->write(2, 0, "=1+1");
var_dump($sheet->addIgnoredError(1, 2, 0, 2, 0));

// isRichStr on plain text
$sheet->write(3, 0, "plain");
var_dump($sheet->isRichStr(3, 0));

// isRichStr on rich string
$rs = $book->addRichString();
$rs->addFont();
$rs->addText("Hello ");
$rs->addText("World");
$sheet->writeRichStr(4, 0, $rs);
var_dump($sheet->isRichStr(4, 0));

echo "OK\n";
?>
--EXPECT--
bool(true)
NULL
int(5)
div0 read: int(7)
ERRORTYPE_NULL: int(0)
ERRORTYPE_DIV_0: int(7)
ERRORTYPE_VALUE: int(15)
ERRORTYPE_REF: int(23)
ERRORTYPE_NAME: int(29)
ERRORTYPE_NUM: int(36)
ERRORTYPE_NA: int(42)
bool(false)
rejected cell: string(0) ""
bool(true)
bool(false)
bool(true)
OK
