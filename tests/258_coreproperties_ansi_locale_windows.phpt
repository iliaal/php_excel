--TEST--
Windows: core properties of a book set to an ANSI locale round-trip their bytes
--EXTENSIONS--
excel
zlib
--SKIPIF--
<?php
if (PHP_OS_FAMILY !== 'Windows') die('skip Windows ANSI code page behaviour');
if (sapi_windows_cp_get('ansi') !== 1252) die('skip assumes the 1252 ANSI code page');
?>
--FILE--
<?php
function zip_member($zip, $want) {
    $eocd = strrpos($zip, "PK\x05\x06");
    $end = unpack('Vsig/vdisk/vcddisk/vdiskentries/ventries/Vcdsize/Vcdoffset', substr($zip, $eocd, 20));
    $p = $end['cdoffset'];
    for ($i = 0; $i < $end['entries']; $i++) {
        $c = unpack('Vsig/vmade/vneed/vflag/vmethod/vtime/vdate/Vcrc/Vcsize/Vusize/vnlen/vxlen/vclen/vdisk/vint/Vext/Voffset', substr($zip, $p, 46));
        if (substr($zip, $p + 46, $c['nlen']) === $want) {
            $l = unpack('Vsig/vneed/vflag/vmethod/vtime/vdate/Vcrc/Vcsize/Vusize/vnlen/vxlen', substr($zip, $c['offset'], 30));
            $body = substr($zip, $c['offset'] + 30 + $l['nlen'] + $l['xlen'], $c['csize']);
            return $c['method'] === 8 ? gzinflate($body) : $body;
        }
        $p += 46 + $c['nlen'] + $c['xlen'] + $c['clen'];
    }
    return null;
}

/* The values are code page 1252 bytes, stored as UTF-8 in core.xml. */
$cases = ["x\x80y" => '78e282ac79', "caf\xe9" => '636166c3a9'];
foreach (['.1252', 'C', 'English_United States.1252'] as $locale) {
    echo "== $locale\n";
    foreach ($cases as $value => $xml) {
        $book = new ExcelBook(null, null, true);
        $book->addSheet('Data');
        var_dump($book->setLocale($locale));
        $props = $book->coreProperties();
        var_dump($props->setTitle($value), $props->setComments($value));
        var_dump($props->title() === $value, $props->comments() === $value);
        $saved = $book->save();
        preg_match('~<dc:title>(.*?)</dc:title>~s', zip_member($saved, 'docProps/core.xml'), $m);
        var_dump(bin2hex($m[1]) === $xml);
        $copy = new ExcelBook(null, null, true);
        $copy->setLocale($locale);
        $copy->load($saved);
        var_dump($copy->coreProperties()->title() === $value);
    }
}
?>
--EXPECT--
== .1252
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
== C
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
== English_United States.1252
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
