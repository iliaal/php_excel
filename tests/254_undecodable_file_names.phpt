--TEST--
File names LibXL's decoder rejects go through PHP streams, or are refused where no stream route exists
--EXTENSIONS--
excel
--SKIPIF--
<?php
if (PHP_OS_FAMILY === "Windows") die("skip Windows file names must be valid UTF-8");
if (!method_exists(ExcelBook::class, "loadInfoRaw")) die("skip LibXL 5.0.1+ required");
?>
--ENV--
LC_ALL=C
--FILE--
<?php
$root = sys_get_temp_dir() . '/excel-254-' . bin2hex(random_bytes(6));
mkdir($root);
$jpeg = file_get_contents(__DIR__ . '/phplogo.jpg');
$warnings = [];
set_error_handler(function ($no, $message) use (&$warnings) {
    $warnings[] = $message;
    return true;
});

function excel_254_book(bool $xlsx): ExcelBook
{
    $book = new ExcelBook(null, null, $xlsx);
    $book->addSheet('S')->write(1, 1, 'v');
    return $book;
}

function excel_254_cell(ExcelBook $book, bool $ok): string
{
    return $ok ? (string) $book->getSheet(0)->read(1, 1) : 'failed';
}

function excel_254_warned(array &$warnings): string
{
    $out = $warnings ? implode(' | ', array_map(fn($m) => preg_replace('/^\S+\(\): /', '', $m), $warnings)) : 'none';
    $warnings = [];
    return $out;
}

function excel_254_round_trip(string $dir, string $stem, array &$warnings): void
{
    $ok = excel_254_book(true)->save("$stem.xlsx");
    clearstatcache();
    echo "save xlsx: ", var_export($ok, true), " ", var_export(@filesize("$dir/$stem.xlsx") > 0, true), "\n";
    $ok = excel_254_book(false)->save("$stem.xls");
    clearstatcache();
    echo "save xls: ", var_export($ok, true), " ", var_export(@filesize("$dir/$stem.xls") > 0, true), "\n";

    $book = new ExcelBook(null, null, true);
    echo "loadFile xlsx: ", excel_254_cell($book, $book->loadFile("$stem.xlsx")), "\n";
    $book = new ExcelBook(null, null, false);
    echo "loadFile xls: ", excel_254_cell($book, $book->loadFile("$stem.xls")), "\n";
    $book = new ExcelBook(null, null, true);
    echo "loadFilePartially: ", excel_254_cell($book, $book->loadFilePartially("$stem.xlsx", 0, 1, 1)), "\n";
    $book = new ExcelBook(null, null, true);
    echo "loadInfo: ", $book->loadInfo("$stem.xlsx") ? $book->getSheetName(0) : 'failed', "\n";

    file_put_contents("$dir/$stem.jpg", $GLOBALS['jpeg']);
    $book = new ExcelBook(null, null, true);
    $id = $book->addPictureFromFile("$stem.jpg");
    echo "addPictureFromFile: ", var_export($id !== false && $book->getPicture($id)['data'] === $GLOBALS['jpeg'], true), "\n";
    echo "warnings: ", excel_254_warned($warnings), "\n";
}

$dir = "$root/plain";
mkdir($dir);
chdir($dir);

echo "== Latin-1 name\n";
excel_254_round_trip($dir, "caf\xe9", $warnings);
$book = new ExcelBook(null, null, false);
echo "loadFileWithoutEmptyCells xls: ", excel_254_cell($book, $book->loadFileWithoutEmptyCells("caf\xe9.xls")), " ", excel_254_warned($warnings), "\n";
$book = new ExcelBook(null, null, true);
echo "loadFileWithoutEmptyCells xlsx: ", var_export($book->loadFileWithoutEmptyCells("caf\xe9.xlsx"), true), " ", excel_254_warned($warnings), "\n";
$book = new ExcelBook(null, null, true);
echo "addPictureAsLink: ", var_export($book->addPictureAsLink("caf\xe9.jpg"), true), " ", excel_254_warned($warnings), "\n";

echo "== UTF-8 name\n";
excel_254_round_trip($dir, "caf\xc3\xa9", $warnings);
$book = new ExcelBook(null, null, false);
echo "loadFileWithoutEmptyCells xls: ", excel_254_cell($book, $book->loadFileWithoutEmptyCells("caf\xc3\xa9.xls")), " ", excel_254_warned($warnings), "\n";
$book = new ExcelBook(null, null, true);
echo "addPictureAsLink: ", var_export($book->addPictureAsLink("caf\xc3\xa9.jpg") !== false, true), " ", excel_254_warned($warnings), "\n";

/* Relative names stay ASCII here, but under ZTS they resolve against a
 * working directory LibXL's decoder rejects. */
echo "== ASCII name in a Latin-1 directory\n";
$latin = "$root/dir\xe9";
mkdir($latin);
chdir($latin);
excel_254_round_trip($latin, 'plain', $warnings);
$book = new ExcelBook(null, null, false);
echo "loadFileWithoutEmptyCells xls: ", excel_254_cell($book, $book->loadFileWithoutEmptyCells('plain.xls')), " ", excel_254_warned($warnings), "\n";
$book = new ExcelBook(null, null, true);
$link = $book->addPictureAsLink('plain.jpg');
$linkWarned = excel_254_warned($warnings);
if (PHP_ZTS) {
    $book = new ExcelBook(null, null, true);
    $woec = $book->loadFileWithoutEmptyCells('plain.xlsx');
    $woecWarned = excel_254_warned($warnings);
    $expected = $link === false && $linkWarned === 'Picture path contains invalid UTF-8'
        && $woec === false && $woecWarned === "Failed to load workbook: file name cannot be passed to LibXL in this workbook's encoding";
} else {
    $expected = $link !== false && $linkWarned === 'none';
}
echo "working-directory dependent refusals: ", $expected ? 'expected' : "unexpected $linkWarned", "\n";

chdir($root);
$stray = 0;
foreach ([$dir, $latin, getcwd()] as $d) {
    $stray += count(glob("$d/*.tmp"));
}
echo "stray temporary files: $stray\n";
?>
--CLEAN--
<?php
foreach (glob(sys_get_temp_dir() . '/excel-254-*') as $root) {
    foreach (glob("$root/*", GLOB_ONLYDIR) as $dir) {
        array_map('unlink', glob("$dir/*"));
        rmdir($dir);
    }
    rmdir($root);
}
foreach (['caf', 'plain'] as $stem) {
    foreach (glob(getcwd() . "/$stem*.xls*.tmp") as $f) {
        unlink($f);
    }
}
?>
--EXPECT--
== Latin-1 name
save xlsx: true true
save xls: true true
loadFile xlsx: v
loadFile xls: v
loadFilePartially: v
loadInfo: S
addPictureFromFile: true
warnings: none
loadFileWithoutEmptyCells xls: v none
loadFileWithoutEmptyCells xlsx: false Failed to load workbook: file name cannot be passed to LibXL in this workbook's encoding
addPictureAsLink: false Picture path contains invalid UTF-8
== UTF-8 name
save xlsx: true true
save xls: true true
loadFile xlsx: v
loadFile xls: v
loadFilePartially: v
loadInfo: S
addPictureFromFile: true
warnings: none
loadFileWithoutEmptyCells xls: v none
addPictureAsLink: true none
== ASCII name in a Latin-1 directory
save xlsx: true true
save xls: true true
loadFile xlsx: v
loadFile xls: v
loadFilePartially: v
loadInfo: S
addPictureFromFile: true
warnings: none
loadFileWithoutEmptyCells xls: v none
working-directory dependent refusals: expected
stray temporary files: 0
