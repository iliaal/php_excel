--TEST--
Font script modes survive cell formatting and XLS/XLSX save/load
--EXTENSIONS--
excel
--FILE--
<?php
foreach ([false, true] as $xlsx) {
    echo $xlsx ? "XLSX\n" : "XLS\n";
    $book = new ExcelBook(null, null, $xlsx);
    $sheet = $book->addSheet('Script modes');
    $modes = [ExcelFont::NORMAL, ExcelFont::SUPERSCRIPT, ExcelFont::SUBSCRIPT];
    foreach ($modes as $column => $mode) {
        $font = $book->addFont();
        // Start from another mode so NORMAL also exercises a real change.
        var_dump($font->mode(ExcelFont::SUPERSCRIPT) === ExcelFont::SUPERSCRIPT);
        var_dump($font->mode($mode) === $mode);
        $format = $book->addFormat();
        var_dump($format->setFont($font));
        var_dump($sheet->write(1, $column, 'mode ' . $mode, $format));
    }

    $loaded = new ExcelBook(null, null, $xlsx);
    var_dump($loaded->load($book->save()));
    $restored = $loaded->getSheet(0);
    foreach ($modes as $column => $mode) {
        $format = null;
        var_dump($restored->read(1, $column, $format) === 'mode ' . $mode);
        var_dump($format->getFont()->mode() === $mode);
    }
}
?>
--EXPECT--
XLS
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
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
XLSX
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
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
