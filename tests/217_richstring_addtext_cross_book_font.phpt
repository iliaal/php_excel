--TEST--
RichString::addText() rejects a font from a different ExcelBook
--EXTENSIONS--
excel
--DESCRIPTION--
Every other site that consumes a child handle guards against cross-book input.
RichString::addText() was the only gap. addFont() keeps its template-copy
semantics (xlRichStringAddFont copies, like Book::addFont), so a foreign font
remains valid there and the font it returns is same-book for addText().
--FILE--
<?php
$bookA = new ExcelBook(null, null, true);
$bookB = new ExcelBook(null, null, true);

$foreignFont = $bookB->addFont();
$foreignFont->bold(true);

// addFont() copies: a foreign font is a legitimate template source.
$rs = $bookA->addRichString();
$copied = $rs->addFont($foreignFont);
var_dump($copied instanceof ExcelFont);
$copied->italics(true);
$rs->addText('copied run', $copied);

// The documented workflow: addFont() result feeds addText() on the same book.
var_dump($rs->addText('same-book run', $rs->addFont()));
// A null font is still accepted (default font).
var_dump($rs->addText('default-font run'));

// A foreign font passed straight to addText() is rejected with a warning.
$rs2 = $bookA->addRichString();
var_dump($rs2->addText('hello', $foreignFont));

// Same-book font still works end to end.
$sheet = $bookA->addSheet('S1');
$rs3 = $bookA->addRichString();
$rs3->addFont()->bold(true);
$rs3->addText('ok');
var_dump($sheet->writeRichStr(1, 1, $rs3));
$tmp217 = sys_get_temp_dir() . "/217_rs_" . getmypid() . ".xlsx";
var_dump($bookA->save($tmp217));
unset($bookB);

$read = new ExcelBook(null, null, true);
var_dump($read->loadFile($tmp217));
$s = $read->getSheet(0);
$back = $s->readRichStr(1, 1);
var_dump($back->textSize());
var_dump($back->getText(0)['text']);
@unlink($tmp217);
?>
--EXPECTF--
bool(true)
bool(true)
bool(true)

Warning: ExcelRichString::addText(): Argument belongs to a different ExcelBook in %s on line %d
bool(false)
bool(true)
bool(true)
bool(true)
int(1)
string(2) "ok"
