--TEST--
New Sheet methods: firstFilledRow/lastFilledRow/Col, tabColor, setTabRgbColor/getTabRgbColor, activeCell, selectionRange, colWidthPx, rowHeightPx, setBorder, hyperlinkIndex
--EXTENSIONS--
excel
--FILE--
<?php
$book = new ExcelBook(null, null, true);
$book->setRGBMode(true);
$sheet = $book->addSheet("Sheet1");

$sheet->write(1, 0, "A");
$sheet->write(2, 1, "B");
$sheet->write(3, 2, "C");

var_dump($sheet->firstFilledRow());
var_dump($sheet->lastFilledRow());
var_dump($sheet->firstFilledCol());
var_dump($sheet->lastFilledCol());

$tc = $sheet->tabColor();
var_dump(is_int($tc));

$sheet->setTabRgbColor(100, 150, 200);
$rgb = $sheet->getTabRgbColor();
var_dump($rgb['red']);
var_dump($rgb['green']);
var_dump($rgb['blue']);

var_dump($sheet->setActiveCell(2, 1));
$ac = $sheet->getActiveCell();
var_dump($ac['row']);
var_dump($ac['col']);

$sr = $sheet->selectionRange();
var_dump(is_string($sr) || is_null($sr));

var_dump($sheet->addSelectionRange("A1:C3"));
var_dump($sheet->removeSelection());

$wpx = $sheet->colWidthPx(0);
var_dump($wpx > 0);
$hpx = $sheet->rowHeightPx(1);
var_dump($hpx > 0);

var_dump($sheet->setBorder(1, 3, 0, 2, 1, 8));

var_dump($sheet->hyperlinkIndex(1, 0));

$fmt = $book->addFormat();
$sheet->setColWidth(0, 0, 20, false, $fmt);
$cf = $sheet->colFormat(0);
var_dump($cf === false || $cf instanceof ExcelFormat);

$sheet->setRowHeight(1, 25, $fmt, false);
$rf = $sheet->rowFormat(1);
var_dump($rf === false || $rf instanceof ExcelFormat);

echo "OK\n";
?>
--EXPECT--
int(1)
int(4)
int(0)
int(3)
bool(true)
int(100)
int(150)
int(200)
bool(true)
int(2)
int(1)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
int(-1)
bool(true)
bool(true)
OK
