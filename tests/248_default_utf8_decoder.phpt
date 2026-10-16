--TEST--
Books decode and encode text as UTF-8 by default, independent of the process locale
--EXTENSIONS--
excel
--ENV--
LC_ALL=C
LANG=C
--FILE--
<?php
/* Runs on every platform, Windows included: the default decoder must not
 * depend on the C runtime's locale handling. */
$cafe = "Caf\xc3\xa9 \xe2\x82\xac";
$sheetName = "Lohn \xc3\xbc";
$link = "https://example.com/caf\xc3\xa9-\xe2\x82\xac";

foreach ([false, true] as $xlsx) {
    echo $xlsx ? "== xlsx\n" : "== xls\n";
    $book = new ExcelBook(null, null, $xlsx);
    $sheet = $book->addSheet($sheetName);
    var_dump($sheet->name() === $sheetName);

    var_dump($sheet->write(2, 1, $cafe));
    var_dump($sheet->writeRow(3, [$cafe, 'ascii'], 1));
    var_dump($sheet->read(2, 1) === $cafe);

    $font = $book->addFont();
    var_dump($font->name($cafe) === $cafe);
    $format = $book->addFormat();
    $format->setFont($font);
    var_dump($sheet->write(4, 1, 'styled', $format));

    $rich = $book->addRichString();
    var_dump($rich->addText($cafe, $rich->addFont()));
    var_dump($sheet->writeRichStr(5, 1, $rich));

    /* LibXL reads comments back from XLS only, and saves an XLS comment
     * holding non-ASCII text into a file it cannot load again ("record id
     * can't be 0"), with or without setLocale(). The XLS comment therefore
     * goes on a book that is never saved; the XLSX one only has to survive
     * the save and reload below. */
    if ($xlsx) {
        $sheet->writeComment(6, 1, $cafe, $cafe, 100, 50);
    } else {
        $notes = (new ExcelBook(null, null, false))->addSheet('Notes');
        $notes->writeComment(6, 1, $cafe, $cafe, 100, 50);
        var_dump($notes->readComment(6, 1) === $cafe);
    }

    var_dump($sheet->addHyperlink($link, 7, 7, 1, 1));
    var_dump($sheet->hyperlink(0)['hyperlink'] === $link);

    if ($xlsx) {
        $props = $book->coreProperties();
        var_dump($props->setTitle($cafe));
        var_dump($props->title() === $cafe);
    }

    $saved = $book->save();
    $loaded = new ExcelBook(null, null, $xlsx);
    var_dump(is_string($saved) && $loaded->load($saved));
    $copy = $loaded->getSheet(0);
    var_dump($copy->name() === $sheetName);
    var_dump($copy->read(2, 1) === $cafe);
    var_dump($copy->read(3, 1) === $cafe);
    var_dump($copy->read(5, 1) === $cafe);
    $cellFormat = null;
    $copy->read(4, 1, $cellFormat);
    var_dump($cellFormat->getFont()->name() === $cafe);
    var_dump($copy->hyperlink(0)['hyperlink'] === $link);
    if ($xlsx) {
        var_dump($loaded->coreProperties()->title() === $cafe);
    }
}
?>
--EXPECT--
== xls
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
== xlsx
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
bool(true)
