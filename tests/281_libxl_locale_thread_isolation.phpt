--TEST--
LibXL locale changes stay on the calling thread under ZTS
--EXTENSIONS--
excel
--SKIPIF--
<?php
if (PHP_OS_FAMILY !== "Linux") die("skip glibc relocation patching only");
if (!PHP_ZTS) die("skip isolation is installed on thread-safe builds only");
if (!setlocale(LC_NUMERIC, 'de_DE.utf8')) die('skip de_DE.utf8 locale unavailable');
if (!setlocale(LC_CTYPE, 'de_DE.iso88591')) die('skip de_DE.iso88591 locale unavailable');
ob_start();
phpinfo(INFO_MODULES);
if (str_contains(ob_get_clean(), 'LibXL locale isolation => disabled (unsupported platform)')) die('skip not glibc on x86_64 or aarch64');
?>
--ENV--
LC_ALL=C.UTF-8
--FILE--
<?php
ob_start();
phpinfo(INFO_MODULES);
preg_match('/^LibXL locale isolation => (.*)$/m', ob_get_clean(), $m);
echo "isolation: ", $m[1] ?? 'missing', "\n";

/* LibXL resolves LC_CTYPE "" to C.UTF-8 from the environment, which differs
 * from the C set here, and saves under LC_NUMERIC "C", which differs from
 * the de_DE set here. */
setlocale(LC_ALL, 'C');
setlocale(LC_NUMERIC, 'de_DE.utf8');
$global = setlocale(LC_ALL, 0);

function check(string $label): void
{
    global $global;
    $all = setlocale(LC_ALL, 0);
    echo $label, ": ", $all === $global ? "unchanged" : "changed to $all", ", ", sprintf('%.1f', 1.5), "\n";
}

foreach ([false, true] as $xlsx) {
    echo $xlsx ? "== xlsx\n" : "== xls\n";
    $book = new ExcelBook(null, null, $xlsx);
    check('create');
    $sheet = $book->addSheet('S');
    var_dump($sheet->write(2, 1, 1.5));
    $raw = $book->save();
    check('save()');
    $path = tempnam(sys_get_temp_dir(), 'pxl281');
    var_dump($book->save($path));
    check('save($path)');

    $copy = new ExcelBook(null, null, $xlsx);
    var_dump($copy->load($raw), $copy->getSheet(0)->read(2, 1));
    check('load()');
    var_dump($copy->loadFile($path), $copy->getSheet(0)->read(2, 1));
    check('loadFile()');
    unlink($path);

    var_dump($book->setLocale('de_DE.iso88591'));
    check('setLocale()');
    unset($copy, $sheet, $book);
    check('release');
    /* Each release restores the LC_CTYPE its book saw at creation, and reloads
     * release books out of that order, so the thread keeps the C.UTF-8 that a
     * process-wide LibXL would have left in every thread. */
    echo "thread codeset: ", nl_langinfo(CODESET), "\n";
}
?>
--EXPECT--
isolation: enabled
== xls
create: unchanged, 1,5
bool(true)
save(): unchanged, 1,5
bool(true)
save($path): unchanged, 1,5
bool(true)
float(1.5)
load(): unchanged, 1,5
bool(true)
float(1.5)
loadFile(): unchanged, 1,5
bool(true)
setLocale(): unchanged, 1,5
release: unchanged, 1,5
thread codeset: UTF-8
== xlsx
create: unchanged, 1,5
bool(true)
save(): unchanged, 1,5
bool(true)
save($path): unchanged, 1,5
bool(true)
float(1.5)
load(): unchanged, 1,5
bool(true)
float(1.5)
loadFile(): unchanged, 1,5
bool(true)
setLocale(): unchanged, 1,5
release: unchanged, 1,5
thread codeset: UTF-8
