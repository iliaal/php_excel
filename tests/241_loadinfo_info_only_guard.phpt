--TEST--
Book::loadInfoRaw() info-only mode ends on a full load or clear()
--EXTENSIONS--
excel
--SKIPIF--
<?php
if (!method_exists(ExcelBook::class, 'loadInfoRaw') || !method_exists(ExcelBook::class, 'clear')) {
    print 'skip LibXL 5.1.0+ required';
}
?>
--DESCRIPTION--
After loadInfoRaw() on an XLSX book, LibXL holds only the sheet names, and its
font/format APIs dereference NULL. Every ExcelBook method except the
info-safe ones must warn and return false instead, and constructors taking
the book must throw. load(), loadFilePartially(), loadFileWithoutEmptyCells()
and clear() must each restore the full API.
--FILE--
<?php
const INFO_ONLY = 'The book was loaded with loadInfo() or loadInfoRaw(); load it fully first';

$errors = [];
set_error_handler(function ($no, $msg) use (&$errors) {
    $errors[] = $msg;
    return true;
});
function take(array &$errors): string
{
    $out = $errors ? implode(' | ', $errors) : 'no warning';
    $errors = [];
    return $out;
}

function make_book(bool $xlsx): string
{
    $src = new ExcelBook(null, null, $xlsx);
    $sheet = $src->addSheet('Data');
    $font = $src->addFont();
    $font->bold(true);
    $fmt = $src->addFormat();
    $fmt->setFont($font);
    $sheet->write(1, 1, 'cell', $fmt);
    $src->addCustomFormat('0.000');
    $src->addSheet('Second');
    $path = tempnam(sys_get_temp_dir(), 'pxl241');
    if (!$src->save($path)) {
        echo "save failed\n";
    }
    return $path;
}

$calls = [
    'addFont' => [],
    'addFormat' => [],
    'getAllFormats' => [],
    'addCustomFormat' => ['0.00'],
    'getCustomFormat' => [164],
    'setDefaultFont' => ['Arial', 10],
    'rgbMode' => [],
    'setRGBMode' => [true],
    'addRichString' => [],
    'addFormatFromStyle' => [1],
    'conditionalFormat' => [0],
    'conditionalFormatSize' => [],
    'getSheet' => [0],
    'addSheet' => ['New'],
    'save' => [],
];

function sweep(ExcelBook $b, array $calls, array &$errors): void
{
    $guarded = 0;
    foreach ($calls as $method => $args) {
        $r = $b->$method(...$args);
        $got = take($errors);
        if ($r !== false || $got !== "ExcelBook::$method(): " . INFO_ONLY) {
            printf("FAIL %s: %s / %s\n", $method, var_export($r, true), $got);
            continue;
        }
        $guarded++;
    }
    echo $guarded === count($calls) ? "all methods warn and return false\n" : "guard sweep failed\n";
    printf("sheetCount: %s / %s\n", var_export($b->sheetCount(), true), take($errors));
    printf("getSheetName(1): %s / %s\n", var_export($b->getSheetName(1), true), take($errors));
    foreach (['ExcelFont', 'ExcelFormat', 'ExcelRichString'] as $cls) {
        try {
            new $cls($b);
            echo "$cls: constructed\n";
        } catch (Exception $e) {
            echo "$cls: ", $e->getMessage() === INFO_ONLY ? 'throws info-only' : $e->getMessage(), "\n";
        }
    }
}

function info_only(ExcelBook $b, array &$errors): string
{
    $r = $b->addFont();
    $got = take($errors);
    return $r === false && $got === 'ExcelBook::addFont(): ' . INFO_ONLY
        ? 'info-only' : 'not info-only: ' . var_export($r === false ? false : get_class($r), true) . " / $got";
}

function full_api_ok(ExcelBook $b, array &$errors): void
{
    $font = $b->addFont();
    $s = $b->getSheet(0);
    printf("full: %s %s %s / %s\n", get_class($font), get_class($s), var_export($s->read(1, 1), true), take($errors));
}

$path = make_book(true);
$bytes = file_get_contents($path);
$b = new ExcelBook(null, null, true);

echo "-- xlsx loadInfoRaw\n";
var_dump($b->loadInfoRaw($bytes));
take($errors);
sweep($b, $calls, $errors);

echo "-- load restores\n";
var_dump($b->load($bytes));
full_api_ok($b, $errors);

echo "-- clear restores\n";
var_dump($b->loadInfoRaw($bytes), info_only($b, $errors));
var_dump($b->clear(), take($errors));
$font = $b->addFont();
$s = $b->addSheet('Fresh');
var_dump(get_class($font), $s->write(1, 1, 'z', $b->addFormat()), $s->read(1, 1), $b->sheetCount());
$saved = $b->save();
var_dump(is_string($saved) && $saved !== '', take($errors));

$xls = make_book(false);
$xbytes = file_get_contents($xls);
$x = new ExcelBook();

echo "-- xls loadFilePartially restores\n";
var_dump($x->loadInfoRaw($xbytes), info_only($x, $errors));
var_dump($x->loadFilePartially($xls, 0, 1, 1));
full_api_ok($x, $errors);

echo "-- xls loadFileWithoutEmptyCells restores\n";
var_dump($x->loadInfoRaw($xbytes), info_only($x, $errors));
var_dump($x->loadFileWithoutEmptyCells($xls));
full_api_ok($x, $errors);

unlink($path);
unlink($xls);
?>
--EXPECT--
-- xlsx loadInfoRaw
bool(true)
all methods warn and return false
sheetCount: 2 / no warning
getSheetName(1): 'Second' / no warning
ExcelFont: throws info-only
ExcelFormat: throws info-only
ExcelRichString: throws info-only
-- load restores
bool(true)
full: ExcelFont ExcelSheet 'cell' / no warning
-- clear restores
bool(true)
string(9) "info-only"
bool(true)
string(10) "no warning"
string(9) "ExcelFont"
bool(true)
string(1) "z"
int(1)
bool(true)
string(10) "no warning"
-- xls loadFilePartially restores
bool(true)
string(9) "info-only"
bool(true)
full: ExcelFont ExcelSheet 'cell' / no warning
-- xls loadFileWithoutEmptyCells restores
bool(true)
string(9) "info-only"
bool(true)
full: ExcelFont ExcelSheet 'cell' / no warning
