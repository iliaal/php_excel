--TEST--
ExcelBook::setLocale() survives a reload, and a rejected locale warns and keeps the previous one
--EXTENSIONS--
excel
--SKIPIF--
<?php
if (PHP_OS_FAMILY === "Windows") die("skip POSIX locale names; the MSVC CRT ignores LC_ALL");
if (!setlocale(LC_CTYPE, 'de_DE.iso88591')) die('skip de_DE.iso88591 locale unavailable');
?>
--ENV--
LC_ALL=C
LANG=C
--FILE--
<?php
$latin1 = "Caf\xe9";
foreach ([false, true] as $xlsx) {
    echo $xlsx ? "== xlsx\n" : "== xls\n";
    $book = new ExcelBook(null, null, $xlsx);
    var_dump($book->setLocale('de_DE.iso88591'));
    $sheet = $book->addSheet('Data');
    var_dump($sheet->write(2, 1, $latin1));
    $saved = $book->save();

    var_dump($book->load($saved));
    $sheet = $book->getSheet(0);
    var_dump(bin2hex($sheet->read(2, 1)));
    var_dump($sheet->write(3, 1, $latin1));
    var_dump(bin2hex($sheet->read(3, 1)));

    $path = tempnam(sys_get_temp_dir(), 'pxl251');
    var_dump($book->save($path), $book->loadFile($path));
    unlink($path);
    var_dump(bin2hex($book->getSheet(0)->read(3, 1)));

    var_dump($book->setLocale('xx_NONE.UTF-8'));
    $sheet = $book->getSheet(0);
    var_dump($sheet->write(4, 1, $latin1));
    var_dump($sheet->setHeader($latin1, 0.5));
    var_dump(bin2hex($sheet->header()));

    $utf8 = new ExcelBook(null, null, $xlsx);
    var_dump($utf8->load($book->save()));
    var_dump($utf8->getSheet(0)->read(4, 1));
}
?>
--EXPECTF--
== xls
bool(true)
bool(true)
bool(true)
string(8) "436166e9"
bool(true)
string(8) "436166e9"
bool(true)
bool(true)
string(8) "436166e9"

Warning: ExcelBook::setLocale(): LibXL rejected locale "xx_NONE.UTF-8" in %s on line %d
bool(false)
bool(true)
bool(true)
string(8) "436166e9"
bool(true)
string(5) "Café"
== xlsx
bool(true)
bool(true)
bool(true)
string(8) "436166e9"
bool(true)
string(8) "436166e9"
bool(true)
bool(true)
string(8) "436166e9"

Warning: ExcelBook::setLocale(): LibXL rejected locale "xx_NONE.UTF-8" in %s on line %d
bool(false)
bool(true)
bool(true)
string(8) "436166e9"
bool(true)
string(5) "Café"
