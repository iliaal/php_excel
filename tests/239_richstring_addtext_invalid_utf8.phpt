--TEST--
ExcelRichString::addText() rejects invalid UTF-8 and keeps the workbook saveable
--EXTENSIONS--
excel
--FILE--
<?php
foreach ([false, true] as $xlsx) {
    $label = $xlsx ? 'xlsx' : 'xls';
    foreach (["\xff\xfe", "\xc3\x28", "ok \xed\xa0\x80", "cut \xe2\x82"] as $bad) {
        $book = new ExcelBook(null, null, $xlsx);
        $sheet = $book->addSheet('Rich');
        $written = $book->addRichString();
        $unwritten = $book->addRichString();
        var_dump($written->addText('lead '));
        var_dump($written->addText($bad));
        var_dump($unwritten->addText($bad));
        var_dump($written->textSize(), $unwritten->textSize());
        var_dump($sheet->writeRichStr(2, 1, $written));

        $saved = $book->save();
        $loaded = new ExcelBook(null, null, $xlsx);
        var_dump(is_string($saved) && $loaded->load($saved));
        var_dump($loaded->getSheet(0)->read(2, 1));

        $path = tempnam(sys_get_temp_dir(), 'pxl239');
        var_dump($book->save($path));
        $fromFile = new ExcelBook(null, null, $xlsx);
        var_dump($fromFile->loadFile($path));
        unlink($path);
    }

    foreach (['plain ascii', "caf\xc3\xa9 \xe2\x82\xac \xf0\x9f\x98\x80", ''] as $good) {
        $book = new ExcelBook(null, null, $xlsx);
        $sheet = $book->addSheet('Rich');
        $rich = $book->addRichString();
        var_dump($rich->addText($good, $rich->addFont()));
        var_dump($rich->textSize());
        var_dump($sheet->writeRichStr(2, 1, $rich));
        $loaded = new ExcelBook(null, null, $xlsx);
        var_dump($loaded->load($book->save()));
        echo $label, ' ', bin2hex($good), ' ', $loaded->getSheet(0)->read(2, 1) === $good ? 'round-trip' : 'mismatch', "\n";
    }
}
?>
--EXPECTF--
bool(true)

Warning: ExcelRichString::addText(): Rich string text contains invalid UTF-8 in %s on line %d
bool(false)

Warning: ExcelRichString::addText(): Rich string text contains invalid UTF-8 in %s on line %d
bool(false)
int(1)
int(0)
bool(true)
bool(true)
string(5) "lead "
bool(true)
bool(true)
bool(true)

Warning: ExcelRichString::addText(): Rich string text contains invalid UTF-8 in %s on line %d
bool(false)

Warning: ExcelRichString::addText(): Rich string text contains invalid UTF-8 in %s on line %d
bool(false)
int(1)
int(0)
bool(true)
bool(true)
string(5) "lead "
bool(true)
bool(true)
bool(true)

Warning: ExcelRichString::addText(): Rich string text contains invalid UTF-8 in %s on line %d
bool(false)

Warning: ExcelRichString::addText(): Rich string text contains invalid UTF-8 in %s on line %d
bool(false)
int(1)
int(0)
bool(true)
bool(true)
string(5) "lead "
bool(true)
bool(true)
bool(true)

Warning: ExcelRichString::addText(): Rich string text contains invalid UTF-8 in %s on line %d
bool(false)

Warning: ExcelRichString::addText(): Rich string text contains invalid UTF-8 in %s on line %d
bool(false)
int(1)
int(0)
bool(true)
bool(true)
string(5) "lead "
bool(true)
bool(true)
bool(true)
int(1)
bool(true)
bool(true)
xls 706c61696e206173636969 round-trip
bool(true)
int(1)
bool(true)
bool(true)
xls 636166c3a920e282ac20f09f9880 round-trip
bool(true)
int(1)
bool(true)
bool(true)
xls  round-trip
bool(true)

Warning: ExcelRichString::addText(): Rich string text contains invalid UTF-8 in %s on line %d
bool(false)

Warning: ExcelRichString::addText(): Rich string text contains invalid UTF-8 in %s on line %d
bool(false)
int(1)
int(0)
bool(true)
bool(true)
string(5) "lead "
bool(true)
bool(true)
bool(true)

Warning: ExcelRichString::addText(): Rich string text contains invalid UTF-8 in %s on line %d
bool(false)

Warning: ExcelRichString::addText(): Rich string text contains invalid UTF-8 in %s on line %d
bool(false)
int(1)
int(0)
bool(true)
bool(true)
string(5) "lead "
bool(true)
bool(true)
bool(true)

Warning: ExcelRichString::addText(): Rich string text contains invalid UTF-8 in %s on line %d
bool(false)

Warning: ExcelRichString::addText(): Rich string text contains invalid UTF-8 in %s on line %d
bool(false)
int(1)
int(0)
bool(true)
bool(true)
string(5) "lead "
bool(true)
bool(true)
bool(true)

Warning: ExcelRichString::addText(): Rich string text contains invalid UTF-8 in %s on line %d
bool(false)

Warning: ExcelRichString::addText(): Rich string text contains invalid UTF-8 in %s on line %d
bool(false)
int(1)
int(0)
bool(true)
bool(true)
string(5) "lead "
bool(true)
bool(true)
bool(true)
int(1)
bool(true)
bool(true)
xlsx 706c61696e206173636969 round-trip
bool(true)
int(1)
bool(true)
bool(true)
xlsx 636166c3a920e282ac20f09f9880 round-trip
bool(true)
int(1)
bool(true)
bool(true)
xlsx  round-trip
