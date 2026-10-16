--TEST--
Text holding an unpaired UTF-16 surrogate reads back with '?' instead of failing
--EXTENSIONS--
excel
--ENV--
LC_ALL=C
LANG=C
--FILE--
<?php
/* Minimal XLSX with uncompressed members; the _xD800_ escape in the shared
 * strings is how third-party writers store an unpaired surrogate. */
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

function surrogate_xlsx(array $shared) {
    $head = '<?xml version="1.0" encoding="UTF-8" standalone="yes"?>' . "\n";
    $main = 'http://schemas.openxmlformats.org/spreadsheetml/2006/main';
    $rel = 'http://schemas.openxmlformats.org/officeDocument/2006/relationships';
    $pkg = 'http://schemas.openxmlformats.org/package/2006/relationships';
    $type = 'application/vnd.openxmlformats-officedocument.spreadsheetml';
    $cells = '';
    $strings = '';
    foreach ($shared as $i => $text) {
        $cells .= '<c r="' . chr(65 + $i) . '2" t="s"><v>' . $i . '</v></c>';
        $strings .= '<si><t>' . $text . '</t></si>';
    }
    return stored_zip([
        '[Content_Types].xml' => $head . '<Types xmlns="http://schemas.openxmlformats.org/package/2006/content-types"><Default Extension="rels" ContentType="application/vnd.openxmlformats-package.relationships+xml"/><Default Extension="xml" ContentType="application/xml"/><Override PartName="/xl/workbook.xml" ContentType="' . $type . '.sheet.main+xml"/><Override PartName="/xl/worksheets/sheet1.xml" ContentType="' . $type . '.worksheet+xml"/><Override PartName="/xl/sharedStrings.xml" ContentType="' . $type . '.sharedStrings+xml"/></Types>',
        '_rels/.rels' => $head . '<Relationships xmlns="' . $pkg . '"><Relationship Id="rId1" Type="' . $rel . '/officeDocument" Target="xl/workbook.xml"/></Relationships>',
        'xl/workbook.xml' => $head . '<workbook xmlns="' . $main . '" xmlns:r="' . $rel . '"><sheets><sheet name="Data" sheetId="1" r:id="rId1"/></sheets></workbook>',
        'xl/_rels/workbook.xml.rels' => $head . '<Relationships xmlns="' . $pkg . '"><Relationship Id="rId1" Type="' . $rel . '/worksheet" Target="worksheets/sheet1.xml"/><Relationship Id="rId2" Type="' . $rel . '/sharedStrings" Target="sharedStrings.xml"/></Relationships>',
        'xl/worksheets/sheet1.xml' => $head . '<worksheet xmlns="' . $main . '"><sheetData><row r="2">' . $cells . '</row></sheetData></worksheet>',
        'xl/sharedStrings.xml' => $head . '<sst xmlns="' . $main . '" count="' . count($shared) . '" uniqueCount="' . count($shared) . '">' . $strings . '</sst>',
    ]);
}

/* LibXL writes "A\u{100}B" as UTF-16LE in XLS; patching U+0100 to U+D800
 * leaves an unpaired surrogate in the sheet name, the shared string, and
 * the rich-text run. */
function surrogate_xls() {
    $marker = "A\xc4\x80B";
    $book = new ExcelBook(null, null, false);
    $sheet = $book->addSheet($marker);
    $sheet->write(1, 0, $marker);
    $sheet->write(1, 1, 'ok');
    $rich = $book->addRichString();
    $rich->addText('x');
    $rich->addText($marker);
    $sheet->writeRichStr(1, 2, $rich);
    return str_replace("A\x00\x00\x01B\x00", "A\x00\x00\xd8B\x00", $book->save());
}

echo "== xlsx\n";
$book = new ExcelBook(null, null, true);
var_dump($book->load(surrogate_xlsx(['A_xD800_B', 'ok', 'lead_xD83D__xDE00_'])));
$sheet = $book->getSheet(0);
var_dump($sheet->read(1, 0));
var_dump($sheet->readRow(1, 0, 1));
var_dump($sheet->readCol(0, 1, 1));
var_dump($sheet->readRange(1, 1, 0, 1));
var_dump($sheet->readSparseRow(1, 0, 1));
/* LibXL 4.6 decodes an escaped valid pair; 5.1 and 5.2 reject it. */
var_dump($sheet->read(1, 2) === (hexdec($book->getLibXlVersion()) < 0x05010000 ? "lead\xf0\x9f\x98\x80" : 'lead??'));
var_dump($sheet->write(3, 0, "caf\xc3\xa9"), $sheet->read(3, 0) === "caf\xc3\xa9");
$copy = new ExcelBook(null, null, true);
var_dump($copy->load($book->save()), $copy->getSheet(0)->read(1, 1));

echo "== xls\n";
$book = new ExcelBook(null, null, false);
var_dump($book->load(surrogate_xls()));
$sheet = $book->getSheet(0);
var_dump($sheet->name(), $book->getSheetName(0));
var_dump($sheet->read(1, 0));
var_dump($sheet->readRow(1, 0, 2));
var_dump($sheet->readRichStr(1, 2)->getText(1)['text']);
var_dump($sheet->write(3, 0, "caf\xc3\xa9"), $sheet->read(3, 0) === "caf\xc3\xa9");
?>
--EXPECT--
== xlsx
bool(true)
string(3) "A?B"
array(2) {
  [0]=>
  string(3) "A?B"
  [1]=>
  string(2) "ok"
}
array(1) {
  [0]=>
  string(3) "A?B"
}
array(1) {
  [0]=>
  array(2) {
    [0]=>
    string(3) "A?B"
    [1]=>
    string(2) "ok"
  }
}
array(2) {
  [0]=>
  string(3) "A?B"
  [1]=>
  string(2) "ok"
}
bool(true)
bool(true)
bool(true)
bool(true)
string(2) "ok"
== xls
bool(true)
string(3) "A?B"
string(3) "A?B"
string(3) "A?B"
array(3) {
  [0]=>
  string(3) "A?B"
  [1]=>
  string(2) "ok"
  [2]=>
  string(4) "xA?B"
}
string(3) "A?B"
bool(true)
bool(true)
