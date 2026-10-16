--TEST--
ExcelRichString::addText() under a UTF-8 LC_CTYPE accepts UTF-8 and rejects malformed input
--EXTENSIONS--
excel
--SKIPIF--
<?php if (!setlocale(LC_CTYPE, 'C.UTF-8')) die('skip C.UTF-8 locale unavailable'); ?>
--ENV--
LC_ALL=C.UTF-8
--FILE--
<?php
$cafe = "caf\xc3\xa9 \xe2\x82\xac";
foreach ([false, true] as $xlsx) {
    echo $xlsx ? "== xlsx\n" : "== xls\n";
    $book = new ExcelBook(null, null, $xlsx);
    $sheet = $book->addSheet('Rich');
    $rich = $book->addRichString();
    $rejected = $book->addRichString();
    var_dump($rich->addText($cafe, $rich->addFont()));
    var_dump($rejected->addText("\xff\xfe"));
    var_dump($rejected->addText("ok \xed\xa0\x80"));
    var_dump($rich->textSize(), $rejected->textSize());
    $sheet->writeRichStr(2, 1, $rich);
    $saved = $book->save();
    $loaded = new ExcelBook(null, null, $xlsx);
    var_dump(is_string($saved) && $loaded->load($saved));
    var_dump($loaded->getSheet(0)->read(2, 1) === $cafe);
}
?>
--EXPECTF--
== xls
bool(true)

Warning: ExcelRichString::addText(): Rich string text %s in %s on line %d
bool(false)

Warning: ExcelRichString::addText(): Rich string text %s in %s on line %d
bool(false)
int(1)
int(0)
bool(true)
bool(true)
== xlsx
bool(true)

Warning: ExcelRichString::addText(): Rich string text %s in %s on line %d
bool(false)

Warning: ExcelRichString::addText(): Rich string text %s in %s on line %d
bool(false)
int(1)
int(0)
bool(true)
bool(true)
