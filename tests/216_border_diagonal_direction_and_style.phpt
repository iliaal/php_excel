--TEST--
Format: borderDiagonal() and borderDiagonalStyle() address independent properties
--EXTENSIONS--
excel
--DESCRIPTION--
borderDiagonalStyle() used to be wired to xlFormatSetBorderDiagonal, so it set
the diagonal direction and silently clamped every BORDERSTYLE_ value above
BORDERDIAGONAL_BOTH. The direction now lives on borderDiagonal().
--FILE--
<?php
$book = new ExcelBook(null, null, true);
$fmt = $book->addFormat();

// Each property holds its own value.
$fmt->borderDiagonal(ExcelFormat::BORDERDIAGONAL_UP);
$fmt->borderDiagonalStyle(ExcelFormat::BORDERSTYLE_DOUBLE);
$fmt->borderDiagonalColor(ExcelFormat::COLOR_RED);
var_dump($fmt->borderDiagonal() === ExcelFormat::BORDERDIAGONAL_UP);
var_dump($fmt->borderDiagonalStyle() === ExcelFormat::BORDERSTYLE_DOUBLE);
var_dump($fmt->borderDiagonalColor() === ExcelFormat::COLOR_RED);

// Setting one leaves the other alone.
$fmt->borderDiagonal(ExcelFormat::BORDERDIAGONAL_BOTH);
var_dump($fmt->borderDiagonalStyle() === ExcelFormat::BORDERSTYLE_DOUBLE);
$fmt->borderDiagonalStyle(ExcelFormat::BORDERSTYLE_HAIR);
var_dump($fmt->borderDiagonal() === ExcelFormat::BORDERDIAGONAL_BOTH);

// A style beyond the BORDERDIAGONAL_ range is no longer clamped away.
$fmt->borderDiagonalStyle(ExcelFormat::BORDERSTYLE_MEDIUMDASHDOTDOT);
var_dump($fmt->borderDiagonalStyle() === ExcelFormat::BORDERSTYLE_MEDIUMDASHDOTDOT);

// Omitted argument and null both read.
var_dump($fmt->borderDiagonal(null) === ExcelFormat::BORDERDIAGONAL_BOTH);

// Documented named argument.
$named = $book->addFormat();
try {
    $named->borderDiagonal(border: ExcelFormat::BORDERDIAGONAL_DOWN);
    echo "named-arg: ok\n";
} catch (\Throwable $e) {
    echo "named-arg: " . get_class($e) . ": " . $e->getMessage() . "\n";
}
var_dump($named->borderDiagonal() === ExcelFormat::BORDERDIAGONAL_DOWN);

// Both survive a write/read roundtrip.
$sheet = $book->addSheet("S");
$sheet->write(1, 1, "x", $fmt);
$read = null;
$sheet->read(1, 1, $read);
var_dump($read->borderDiagonal() === ExcelFormat::BORDERDIAGONAL_BOTH);
var_dump($read->borderDiagonalStyle() === ExcelFormat::BORDERSTYLE_MEDIUMDASHDOTDOT);
?>
--EXPECT--
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
named-arg: ok
bool(true)
bool(true)
bool(true)
