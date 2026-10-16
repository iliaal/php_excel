--TEST--
Book::loadInfo()/loadInfoRaw() put the book in info-only mode until a full load
--EXTENSIONS--
excel
--SKIPIF--
<?php
if (!method_exists(ExcelBook::class, 'loadInfoRaw')) {
    print 'skip LibXL 5.0.1+ required';
}
?>
--DESCRIPTION--
After loadInfo()/loadInfoRaw() on an XLSX book, LibXL holds only the sheet
names, and its font/format APIs dereference NULL. Every ExcelBook method except
sheetCount(), getSheetName(), load*() and __construct() must warn and return
false instead, constructors taking the book must throw, and a successful full
load or __construct() must restore the API.
--FILE--
<?php
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

$src = new ExcelBook(null, null, true);
$sheet = $src->addSheet('Data');
$font = $src->addFont();
$font->bold(true);
$fmt = $src->addFormat();
$fmt->setFont($font);
$sheet->write(1, 1, 'cell', $fmt);
$src->addCustomFormat('0.000');
$src->addSheet('Second');
$path = tempnam(sys_get_temp_dir(), 'pxl241');
var_dump($src->save($path));
$bytes = file_get_contents($path);

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
    'save' => [],
];

const INFO_ONLY = 'The book was loaded with loadInfo() or loadInfoRaw(); load it fully first';

function sweep(ExcelBook $b, array $calls, array &$errors): void
{
    $guarded = 0;
    foreach ($calls as $method => $args) {
        if (!method_exists($b, $method)) {
            continue;
        }
        $r = $b->$method(...$args);
        $got = take($errors);
        if ($r !== false || $got !== "ExcelBook::$method(): " . INFO_ONLY) {
            printf("FAIL %s: %s / %s\n", $method, var_export($r, true), $got);
            continue;
        }
        $guarded++;
    }
    echo $guarded === count(array_filter(array_keys($calls), fn($m) => method_exists($b, $m)))
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

function full_api_ok(ExcelBook $b, array &$errors): void
{
    $font = $b->addFont();
    $s = $b->getSheet(0);
    printf("full: %s %s %s / %s\n", get_class($font), get_class($s), var_export($s->read(1, 1), true), take($errors));
}

$b = new ExcelBook(null, null, true);
var_dump($b->loadFile($path));
$stale = $b->getSheet(0);

echo "-- loadInfo\n";
var_dump($b->loadInfo($path));
take($errors);
sweep($b, $calls, $errors);
var_dump($stale->read(1, 1));
echo take($errors), "\n";

echo "-- failed full load keeps info-only mode\n";
var_dump($b->load('not a workbook'));
take($errors);
var_dump($b->addFont());
echo take($errors), "\n";

echo "-- loadFile restores\n";
var_dump($b->loadFile($path));
full_api_ok($b, $errors);

echo "-- loadInfoRaw\n";
var_dump($b->loadInfoRaw($bytes));
take($errors);
sweep($b, $calls, $errors);

echo "-- load restores\n";
var_dump($b->load($bytes));
full_api_ok($b, $errors);

echo "-- __construct reuse restores\n";
var_dump($b->loadInfo($path));
$b->__construct(null, null, true);
var_dump(get_class($b->addFont()), take($errors));

unlink($path);
?>
--EXPECT--
bool(true)
bool(true)
-- loadInfo
bool(true)
all present methods warn and return false
sheetCount: 2 / no warning
getSheetName(1): 'Second' / no warning
ExcelFont: throws info-only
ExcelFormat: throws info-only
ExcelRichString: throws info-only
bool(false)
ExcelSheet::read(): Underlying ExcelBook handle is stale (parent was reloaded, cleared, or reinitialized)
-- failed full load keeps info-only mode
bool(false)
bool(false)
ExcelBook::addFont(): The book was loaded with loadInfo() or loadInfoRaw(); load it fully first
-- loadFile restores
bool(true)
full: ExcelFont ExcelSheet 'cell' / no warning
-- loadInfoRaw
bool(true)
all present methods warn and return false
sheetCount: 2 / no warning
getSheetName(1): 'Second' / no warning
ExcelFont: throws info-only
ExcelFormat: throws info-only
ExcelRichString: throws info-only
-- load restores
bool(true)
full: ExcelFont ExcelSheet 'cell' / no warning
-- __construct reuse restores
bool(true)
string(9) "ExcelFont"
string(10) "no warning"
