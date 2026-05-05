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

// writeError (must pass format to avoid NULL deref bug)
$fmt = $book->addFormat();
$sheet->writeError(1, 0, 1, $fmt);
var_dump($sheet->cellType(1, 0));

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
int(4)
bool(true)
bool(false)
bool(true)
OK
