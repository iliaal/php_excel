--TEST--
ExcelRichString::addText() follows the POSIX LC_CTYPE decoder when no UTF-8 locale is active
--EXTENSIONS--
excel
--SKIPIF--
<?php if (PHP_OS_FAMILY === "Windows") die("skip POSIX LC_CTYPE semantics; the MSVC CRT ignores LC_ALL"); ?>
--ENV--
LC_ALL=C
--FILE--
<?php
function newBook($xlsx, $locale) {
    $book = new ExcelBook(null, null, $xlsx);
    if ($locale !== null) {
        $book->setLocale($locale);
    }
    return $book;
}

function saveAndReload($book, $xlsx) {
    $saved = $book->save();
    $loaded = newBook($xlsx, 'UTF-8');
    $ok = is_string($saved) && $loaded->load($saved);
    $path = tempnam(sys_get_temp_dir(), 'pxl246');
    $fromFile = newBook($xlsx, 'UTF-8');
    $ok = $ok && $book->save($path) && $fromFile->loadFile($path);
    unlink($path);
    return $ok ? $loaded : null;
}

$cafe = "caf\xc3\xa9 \xe2\x82\xac";
foreach ([false, true] as $xlsx) {
    echo $xlsx ? "== xlsx\n" : "== xls\n";

    echo "-- setLocale('UTF-8'), invalid UTF-8\n";
    foreach (["\xff\xfe", "\xc3\x28", "ok \xed\xa0\x80", "cut \xe2\x82"] as $bad) {
        $book = newBook($xlsx, 'UTF-8');
        $sheet = $book->addSheet('Rich');
        $written = $book->addRichString();
        $unwritten = $book->addRichString();
        var_dump($written->addText('lead '));
        var_dump($written->addText($bad));
        var_dump($unwritten->addText($bad));
        var_dump($written->textSize(), $unwritten->textSize());
        $sheet->writeRichStr(2, 1, $written);
        $loaded = saveAndReload($book, $xlsx);
        var_dump($loaded ? $loaded->getSheet(0)->read(2, 1) : 'save failed');
    }

    echo "-- setLocale('UTF-8'), valid text\n";
    foreach (['plain ascii', $cafe, ''] as $good) {
        $book = newBook($xlsx, 'UTF-8');
        $sheet = $book->addSheet('Rich');
        $rich = $book->addRichString();
        var_dump($rich->addText($good, $rich->addFont()));
        $sheet->writeRichStr(2, 1, $rich);
        $loaded = saveAndReload($book, $xlsx);
        var_dump($loaded && $loaded->getSheet(0)->read(2, 1) === $good);
    }

    echo "-- default decoder under LC_ALL=C\n";
    $book = newBook($xlsx, null);
    $sheet = $book->addSheet('Rich');
    $rich = $book->addRichString();
    var_dump($rich->addText($cafe));
    var_dump($rich->addText('ascii'));
    $sheet->writeRichStr(2, 1, $rich);
    $loaded = saveAndReload($book, $xlsx);
    var_dump($loaded ? $loaded->getSheet(0)->read(2, 1) : 'save failed');

    echo "-- failed setLocale() after setLocale('UTF-8')\n";
    $book = newBook($xlsx, 'UTF-8');
    $book->setLocale('xx_NONE.UTF-8');
    $sheet = $book->addSheet('Rich');
    $rich = $book->addRichString();
    var_dump($rich->addText($cafe));
    $book->setLocale('UTF-8');
    var_dump($rich->addText($cafe));
    $sheet->writeRichStr(2, 1, $rich);
    $loaded = saveAndReload($book, $xlsx);
    var_dump($loaded && $loaded->getSheet(0)->read(2, 1) === $cafe);
}
?>
--EXPECTF--
== xls
-- setLocale('UTF-8'), invalid UTF-8
bool(true)

Warning: ExcelRichString::addText(): Rich string text contains invalid UTF-8 in %s on line %d
bool(false)

Warning: ExcelRichString::addText(): Rich string text contains invalid UTF-8 in %s on line %d
bool(false)
int(1)
int(0)
string(5) "lead "
bool(true)

Warning: ExcelRichString::addText(): Rich string text contains invalid UTF-8 in %s on line %d
bool(false)

Warning: ExcelRichString::addText(): Rich string text contains invalid UTF-8 in %s on line %d
bool(false)
int(1)
int(0)
string(5) "lead "
bool(true)

Warning: ExcelRichString::addText(): Rich string text contains invalid UTF-8 in %s on line %d
bool(false)

Warning: ExcelRichString::addText(): Rich string text contains invalid UTF-8 in %s on line %d
bool(false)
int(1)
int(0)
string(5) "lead "
bool(true)

Warning: ExcelRichString::addText(): Rich string text contains invalid UTF-8 in %s on line %d
bool(false)

Warning: ExcelRichString::addText(): Rich string text contains invalid UTF-8 in %s on line %d
bool(false)
int(1)
int(0)
string(5) "lead "
-- setLocale('UTF-8'), valid text
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
-- default decoder under LC_ALL=C

Warning: ExcelRichString::addText(): Rich string text cannot be converted with the current LC_CTYPE locale in %s on line %d
bool(false)
bool(true)
string(5) "ascii"
-- failed setLocale() after setLocale('UTF-8')

Warning: ExcelRichString::addText(): Rich string text cannot be converted with the current LC_CTYPE locale in %s on line %d
bool(false)
bool(true)
bool(true)
== xlsx
-- setLocale('UTF-8'), invalid UTF-8
bool(true)

Warning: ExcelRichString::addText(): Rich string text contains invalid UTF-8 in %s on line %d
bool(false)

Warning: ExcelRichString::addText(): Rich string text contains invalid UTF-8 in %s on line %d
bool(false)
int(1)
int(0)
string(5) "lead "
bool(true)

Warning: ExcelRichString::addText(): Rich string text contains invalid UTF-8 in %s on line %d
bool(false)

Warning: ExcelRichString::addText(): Rich string text contains invalid UTF-8 in %s on line %d
bool(false)
int(1)
int(0)
string(5) "lead "
bool(true)

Warning: ExcelRichString::addText(): Rich string text contains invalid UTF-8 in %s on line %d
bool(false)

Warning: ExcelRichString::addText(): Rich string text contains invalid UTF-8 in %s on line %d
bool(false)
int(1)
int(0)
string(5) "lead "
bool(true)

Warning: ExcelRichString::addText(): Rich string text contains invalid UTF-8 in %s on line %d
bool(false)

Warning: ExcelRichString::addText(): Rich string text contains invalid UTF-8 in %s on line %d
bool(false)
int(1)
int(0)
string(5) "lead "
-- setLocale('UTF-8'), valid text
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
-- default decoder under LC_ALL=C

Warning: ExcelRichString::addText(): Rich string text cannot be converted with the current LC_CTYPE locale in %s on line %d
bool(false)
bool(true)
string(5) "ascii"
-- failed setLocale() after setLocale('UTF-8')

Warning: ExcelRichString::addText(): Rich string text cannot be converted with the current LC_CTYPE locale in %s on line %d
bool(false)
bool(true)
bool(true)
