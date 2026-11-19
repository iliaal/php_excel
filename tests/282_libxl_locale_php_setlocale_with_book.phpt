--TEST--
PHP setlocale() takes effect on the calling thread while a book is open
--EXTENSIONS--
excel
--SKIPIF--
<?php
if (PHP_OS_FAMILY !== "Linux") die("skip glibc relocation patching only");
if (!PHP_ZTS) die("skip isolation is installed on thread-safe builds only");
ob_start();
phpinfo(INFO_MODULES);
if (str_contains(ob_get_clean(), 'LibXL locale isolation => disabled (unsupported platform)')) die('skip not glibc on x86_64 or aarch64');
if (!setlocale(LC_NUMERIC, 'de_DE.utf8')) die('skip de_DE.utf8 locale unavailable');
if (!setlocale(LC_CTYPE, 'de_DE.iso88591')) die('skip de_DE.iso88591 locale unavailable');
?>
--ENV--
LC_ALL=C.UTF-8
--FILE--
<?php
setlocale(LC_ALL, 'C');
foreach ([false, true] as $xlsx) {
    echo $xlsx ? "== xlsx\n" : "== xls\n";
    $book = new ExcelBook(null, null, $xlsx);
    var_dump(setlocale(LC_NUMERIC, 'de_DE.utf8'));
    printf("with book open: %.1f %s\n", 1.5, localeconv()['decimal_point']);
    var_dump(setlocale(LC_CTYPE, 'de_DE.iso88591'));
    echo "codeset with book open: ", nl_langinfo(CODESET), "\n";
    $book->addSheet('S')->write(2, 1, 1.5);
    $copy = new ExcelBook(null, null, $xlsx);
    var_dump($copy->load($book->save()), $copy->getSheet(0)->read(2, 1));
    printf("after save and load: %.1f %s\n", 1.5, nl_langinfo(CODESET));
    unset($copy, $book);
    printf("after release: %.1f %s\n", 1.5, nl_langinfo(CODESET));
    setlocale(LC_ALL, 'C');
}
?>
--EXPECT--
== xls
string(10) "de_DE.utf8"
with book open: 1,5 ,
string(14) "de_DE.iso88591"
codeset with book open: ISO-8859-1
bool(true)
float(1.5)
after save and load: 1,5 ISO-8859-1
after release: 1,5 ISO-8859-1
== xlsx
string(10) "de_DE.utf8"
with book open: 1,5 ,
string(14) "de_DE.iso88591"
codeset with book open: ISO-8859-1
bool(true)
float(1.5)
after save and load: 1,5 ISO-8859-1
after release: 1,5 ISO-8859-1
