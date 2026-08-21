--TEST--
RichString::addFont copies a foreign font template without retaining its book
--EXTENSIONS--
excel
--DESCRIPTION--
addFont() is a template-copy method: xlRichStringAddFont() duplicates the
source font into the rich string's book, so the source book can be freed
before the string is written. addText() itself consumes the run's font by
reference and therefore rejects foreign-book fonts (see test 217).
--FILE--
<?php
$sourceBook = new ExcelBook(null, null, true);
$font = $sourceBook->addFont();
$font->bold(true);

$targetBook = new ExcelBook(null, null, true);
$sheet = $targetBook->addSheet('S');
$richString = $targetBook->addRichString();

// Copy the foreign template into the target book...
$copied = $richString->addFont($font);
var_dump($copied instanceof ExcelFont);

// ...after which the source book can go away.
unset($font, $sourceBook);
gc_collect_cycles();
var_dump($richString->addText('copied', $copied));
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
bool(true)
string(6) "copied"
bool(true)
OK
