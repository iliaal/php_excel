--TEST--
Relative file names resolve against the request working directory after chdir() under ZTS
--EXTENSIONS--
excel
--SKIPIF--
<?php
if (!PHP_ZTS) die("skip ZTS only: NTS chdir() moves the process working directory LibXL uses");
if (!method_exists(ExcelBook::class, "loadFilePartially")) die("skip LibXL 5.0+ required");
?>
--FILE--
<?php
$origin = getcwd();
$work = sys_get_temp_dir() . '/excel-251-' . bin2hex(random_bytes(6));
mkdir($work);
$png = base64_decode('iVBORw0KGgoAAAANSUhEUgAAAAEAAAABCAYAAAAfFcSJAAAADUlEQVR42mNkYPhfDwAChwGA60e6kgAAAABJRU5ErkJggg==');
$jpeg = file_get_contents(__DIR__ . '/phplogo.jpg');

function excel_251_book(bool $xlsx, string $sheet, string $text): ExcelBook
{
    $book = new ExcelBook(null, null, $xlsx);
    $book->addSheet($sheet)->write(1, 1, $text);
    return $book;
}

excel_251_book(true, 'Decoy', 'decoy')->save("$origin/excel-251.xlsx");
excel_251_book(false, 'Decoy', 'decoy')->save("$origin/excel-251.xls");
file_put_contents("$origin/excel-251.jpg", $png);
file_put_contents("$work/excel-251.jpg", $jpeg);

chdir($work);

var_dump(excel_251_book(true, 'Real', 'real')->save('excel-251.xlsx'));
var_dump(excel_251_book(false, 'Real', 'real')->save('excel-251.xls'));
clearstatcache();
foreach (['xlsx', 'xls'] as $ext) {
    echo "$ext saved in work dir: ", var_export(filesize("$work/excel-251.$ext") > 0, true), "\n";
}

$book = new ExcelBook(null, null, true);
var_dump($book->loadFile('excel-251.xlsx'), $book->getSheet(0)->read(1, 1));

$book = new ExcelBook(null, null, true);
var_dump($book->loadFilePartially('excel-251.xlsx', 0, 1, 1), $book->getSheet(0)->read(1, 1));

$book = new ExcelBook(null, null, false);
var_dump($book->loadFileWithoutEmptyCells('excel-251.xls'), $book->getSheet(0)->read(1, 1));

$book = new ExcelBook(null, null, true);
var_dump($book->loadInfo('excel-251.xlsx'), $book->getSheetName(0));

$book = new ExcelBook(null, null, true);
$id = $book->addPictureFromFile('excel-251.jpg');
var_dump($id !== false && $book->getPicture($id)['data'] === $jpeg);

$book = new ExcelBook(null, null, true);
$id = $book->addPictureAsLink('excel-251.jpg', true);
var_dump($id !== false && $book->getPicture($id)['data'] === $jpeg);

chdir($origin);

$stray = array_merge(glob("$origin/excel-251*.tmp"), glob("$work/excel-251*.tmp"));
echo "stray temporary files: ", count($stray), "\n";
$decoy = new ExcelBook(null, null, true);
$decoy->loadFile("$origin/excel-251.xlsx");
echo "origin decoy intact: ", $decoy->getSheet(0)->read(1, 1), "\n";
?>
--CLEAN--
<?php
foreach (glob(getcwd() . '/excel-251*') as $f) {
    @unlink($f);
}
foreach (glob(sys_get_temp_dir() . '/excel-251-*', GLOB_ONLYDIR) as $dir) {
    foreach (glob("$dir/*") as $f) {
        @unlink($f);
    }
    @rmdir($dir);
}
?>
--EXPECT--
bool(true)
bool(true)
xlsx saved in work dir: true
xls saved in work dir: true
bool(true)
string(4) "real"
bool(true)
string(4) "real"
bool(true)
string(4) "real"
bool(true)
string(4) "Real"
bool(true)
bool(true)
stray temporary files: 0
origin decoy intact: decoy
