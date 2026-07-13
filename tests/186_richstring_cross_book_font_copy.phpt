--TEST--
RichString::addText copies a foreign font template without retaining its book
--EXTENSIONS--
excel
--FILE--
<?php
$sourceBook = new ExcelBook(null, null, true);
$font = $sourceBook->addFont();
$font->bold(true);

$targetBook = new ExcelBook(null, null, true);
$sheet = $targetBook->addSheet('S');
$richString = $targetBook->addRichString();
$added = $richString->addText('copied', $font);
var_dump($added);
if (!$added) {
    echo "stopped\n";
    return;
}

unset($font, $sourceBook);
gc_collect_cycles();
var_dump($sheet->writeRichStr(1, 0, $richString));
$data = $targetBook->save();

$loaded = new ExcelBook(null, null, true);
var_dump($loaded->load($data));
$text = $loaded->getSheet()->readRichStr(1, 0)->getText(0);
var_dump($text['text']);
var_dump($text['font']->bold());
echo "OK\n";
?>
--EXPECT--
bool(true)
bool(true)
bool(true)
string(6) "copied"
bool(true)
OK
