--TEST--
Book::loadInfo() puts XLSX and XLS books in info-only mode until a full load
--EXTENSIONS--
excel
--DESCRIPTION--
After loadInfo() on an XLSX book, LibXL holds only the sheet names, and its
font/format APIs dereference NULL. Every ExcelBook method except the
info-safe ones must warn and return false instead, constructors taking the
book must throw, a repeated or failed load must keep info-only mode, and a
successful full load or __construct() must restore the API. XLS books follow
the same rules.
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
    $path = tempnam(sys_get_temp_dir(), 'pxl243');
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
    $present = $guarded = 0;
    foreach ($calls as $method => $args) {
        if (!method_exists($b, $method)) {
            continue;
        }
        $present++;
        $r = $b->$method(...$args);
        $got = take($errors);
        if ($r !== false || $got !== "ExcelBook::$method(): " . INFO_ONLY) {
            printf("FAIL %s: %s / %s\n", $method, var_export($r, true), $got);
            continue;
        }
        $guarded++;
    }
    echo $present > 0 && $guarded === $present
        ? "all present methods warn and return false\n" : "guard sweep failed\n";
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
$b = new ExcelBook(null, null, true);
var_dump($b->loadFile($path));
$stale = $b->getSheet(0);

echo "-- xlsx loadInfo\n";
var_dump($b->loadInfo($path));
take($errors);
sweep($b, $calls, $errors);
var_dump($stale->read(1, 1));
echo take($errors), "\n";

echo "-- loadInfo again stays info-only\n";
var_dump($b->loadInfo($path));
take($errors);
echo info_only($b, $errors), "\n";

echo "-- failed full load keeps info-only mode\n";
var_dump($b->load('not a workbook'));
take($errors);
echo info_only($b, $errors), "\n";

echo "-- loadFile restores\n";
var_dump($b->loadFile($path));
full_api_ok($b, $errors);

echo "-- __construct reuse restores\n";
var_dump($b->loadInfo($path));
$b->__construct(null, null, true);
var_dump(get_class($b->addFont()), take($errors));

$xls = make_book(false);
$x = new ExcelBook();
echo "-- xls loadInfo\n";
var_dump($x->loadInfo($xls));
take($errors);
sweep($x, $calls, $errors);

echo "-- xls loadFile restores\n";
var_dump($x->loadFile($xls));
full_api_ok($x, $errors);

unlink($path);
unlink($xls);
?>
--EXPECT--
bool(true)
-- xlsx loadInfo
bool(true)
all present methods warn and return false
sheetCount: 2 / no warning
getSheetName(1): 'Second' / no warning
ExcelFont: throws info-only
ExcelFormat: throws info-only
ExcelRichString: throws info-only
bool(false)
ExcelSheet::read(): Underlying ExcelBook handle is stale (parent was reloaded, cleared, or reinitialized)
-- loadInfo again stays info-only
bool(true)
info-only
-- failed full load keeps info-only mode
bool(false)
info-only
-- loadFile restores
bool(true)
full: ExcelFont ExcelSheet 'cell' / no warning
-- __construct reuse restores
bool(true)
string(9) "ExcelFont"
string(10) "no warning"
-- xls loadInfo
bool(true)
all present methods warn and return false
sheetCount: 2 / no warning
getSheetName(1): 'Second' / no warning
ExcelFont: throws info-only
ExcelFormat: throws info-only
ExcelRichString: throws info-only
-- xls loadFile restores
bool(true)
full: ExcelFont ExcelSheet 'cell' / no warning
