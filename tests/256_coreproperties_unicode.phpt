--TEST--
Core properties keep non-ASCII text through set, get, save, and load
--EXTENSIONS--
excel
--SKIPIF--
<?php
if (PHP_OS_FAMILY === 'Windows' && sapi_windows_cp_get('ansi') !== 1252) {
    die('skip Windows assertions assume the 1252 ANSI code page');
}
?>
--FILE--
<?php
/* Runs on every platform. On Windows title, subject, creator, and comments
 * pass through the ANSI code page; the UTF-8 forms of the letters below
 * contain the bytes 0x81, 0x8D, 0x8F, 0x90, and 0x9D, which code page 1252
 * leaves undefined. */
function stored_zip(array $files) {
    $data = '';
    $central = '';
    foreach ($files as $name => $body) {
        $crc = crc32($body);
        $len = strlen($body);
        $offset = strlen($data);
        $data .= pack('VvvvvvVVVvv', 0x04034b50, 20, 0, 0, 0, 0x21, $crc, $len, $len, strlen($name), 0) . $name . $body;
        $central .= pack('VvvvvvvVVVvvvvvVV', 0x02014b50, 20, 20, 0, 0, 0, 0x21, $crc, $len, $len, strlen($name), 0, 0, 0, 0, 0, $offset) . $name;
    }
    return $data . $central . pack('VvvvvVVv', 0x06054b50, 0, 0, count($files), count($files), strlen($central), strlen($data), 0);
}

function core_xlsx($title) {
    $head = '<?xml version="1.0" encoding="UTF-8" standalone="yes"?>' . "\n";
    $main = 'http://schemas.openxmlformats.org/spreadsheetml/2006/main';
    $rel = 'http://schemas.openxmlformats.org/officeDocument/2006/relationships';
    $pkg = 'http://schemas.openxmlformats.org/package/2006/relationships';
    $type = 'application/vnd.openxmlformats-officedocument.spreadsheetml';
    return stored_zip([
        '[Content_Types].xml' => $head . '<Types xmlns="http://schemas.openxmlformats.org/package/2006/content-types"><Default Extension="rels" ContentType="application/vnd.openxmlformats-package.relationships+xml"/><Default Extension="xml" ContentType="application/xml"/><Override PartName="/xl/workbook.xml" ContentType="' . $type . '.sheet.main+xml"/><Override PartName="/xl/worksheets/sheet1.xml" ContentType="' . $type . '.worksheet+xml"/><Override PartName="/docProps/core.xml" ContentType="application/vnd.openxmlformats-package.core-properties+xml"/></Types>',
        '_rels/.rels' => $head . '<Relationships xmlns="' . $pkg . '"><Relationship Id="rId1" Type="' . $rel . '/officeDocument" Target="xl/workbook.xml"/><Relationship Id="rId2" Type="' . $pkg . '/metadata/core-properties" Target="docProps/core.xml"/></Relationships>',
        'docProps/core.xml' => $head . '<cp:coreProperties xmlns:cp="http://schemas.openxmlformats.org/package/2006/metadata/core-properties" xmlns:dc="http://purl.org/dc/elements/1.1/"><dc:title>' . $title . '</dc:title></cp:coreProperties>',
        'xl/workbook.xml' => $head . '<workbook xmlns="' . $main . '" xmlns:r="' . $rel . '"><sheets><sheet name="Data" sheetId="1" r:id="rId1"/></sheets></workbook>',
        'xl/_rels/workbook.xml.rels' => $head . '<Relationships xmlns="' . $pkg . '"><Relationship Id="rId1" Type="' . $rel . '/worksheet" Target="worksheets/sheet1.xml"/></Relationships>',
        'xl/worksheets/sheet1.xml' => $head . '<worksheet xmlns="' . $main . '"><sheetData><row r="2"><c r="A2"><v>1</v></c></row></sheetData></worksheet>',
    ]);
}

$latin = "Caf\xc3\xa9 \xe2\x82\xac \xc3\x81\xc3\x8d\xc3\x8f\xc3\x90\xc3\x9d";
$wide = "\xc5\x81\xc5\x8d\xc5\x90 \xe4\xb8\xad\xe6\x96\x87";
$windows = PHP_OS_FAMILY === 'Windows';

echo "== set\n";
$book = new ExcelBook(null, null, true);
$book->addSheet('Data');
$props = $book->coreProperties();
$setters = ['setTitle', 'setSubject', 'setCreator', 'setLastModifiedBy', 'setTags', 'setCategories', 'setComments'];
foreach ($setters as $i => $setter) {
    var_dump($props->$setter("$i $latin"));
}
foreach (['title', 'subject', 'creator', 'lastModifiedBy', 'tags', 'categories', 'comments'] as $i => $getter) {
    var_dump($props->$getter() === "$i $latin");
}
var_dump($props->setCreated('2026-01-02T03:04:05Z'), $props->created());

/* Code page 1252 cannot represent these characters, and LibXL on Windows
 * stores the subject only through it. */
$result = @$props->setSubject($wide);
if ($windows) {
    var_dump($result === false);
    var_dump(str_contains(error_get_last()['message'], 'Windows ANSI code page'));
    var_dump($props->subject() === "1 $latin");
} else {
    var_dump($result, true, $props->subject() === $wide);
    $props->setSubject("1 $latin");
}

$reloaded = new ExcelBook(null, null, true);
var_dump($reloaded->load($book->save()));
var_dump($reloaded->coreProperties()->title() === "0 $latin");
var_dump($reloaded->coreProperties()->comments() === "6 $latin");

echo "== load\n";
$book = new ExcelBook(null, null, true);
var_dump($book->load(core_xlsx("$wide $latin")));
var_dump($book->coreProperties()->title() === "$wide $latin");
$reloaded = new ExcelBook(null, null, true);
var_dump($reloaded->load($book->save()));
var_dump($reloaded->coreProperties()->title() === "$wide $latin");
?>
--EXPECT--
== set
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
string(20) "2026-01-02T03:04:05Z"
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
== load
bool(true)
bool(true)
bool(true)
bool(true)
