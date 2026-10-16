--TEST--
A book set to a Latin-1 locale still hands Latin-1 file names to LibXL directly
--EXTENSIONS--
excel
--SKIPIF--
<?php
if (PHP_OS_FAMILY === "Windows") die("skip POSIX locale names; the MSVC CRT ignores LC_ALL");
if (hexdec((new ExcelBook())->getLibXlVersion()) < 0x05020000) die("skip LibXL 5.2+ required to read XLSX without empty cells");
if (!setlocale(LC_CTYPE, 'de_DE.iso88591')) die('skip de_DE.iso88591 locale unavailable');
?>
--ENV--
LC_ALL=C
--FILE--
<?php
$dir = sys_get_temp_dir() . '/excel-255-' . bin2hex(random_bytes(6));
mkdir($dir);
chdir($dir);

function excel_255_book(bool $xlsx): ExcelBook
{
    $book = new ExcelBook(null, null, $xlsx);
    var_dump($book->setLocale('de_DE.iso88591'));
    return $book;
}

$book = excel_255_book(true);
$book->addSheet('S')->write(1, 1, 'v');
var_dump($book->save("caf\xe9.xlsx"));
clearstatcache();
var_dump(filesize("$dir/caf\xe9.xlsx") > 0);

/* Only the by-name reader exists for this call, so success shows the name
 * reached LibXL. The book is configured before the previous one is released,
 * which moves the process LC_CTYPE away from this book's locale. */
$book = excel_255_book(true);
var_dump($book->loadFileWithoutEmptyCells("caf\xe9.xlsx"), $book->getSheet(0)->read(1, 1));

chdir(sys_get_temp_dir());
echo "stray temporary files: ", count(glob("$dir/*.tmp")), "\n";
?>
--CLEAN--
<?php
foreach (glob(sys_get_temp_dir() . '/excel-255-*') as $dir) {
    array_map('unlink', glob("$dir/*"));
    rmdir($dir);
}
foreach (glob(getcwd() . '/caf*.xlsx.*.tmp') as $f) {
    unlink($f);
}
?>
--EXPECT--
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
string(1) "v"
stray temporary files: 0
