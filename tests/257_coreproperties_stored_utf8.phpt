--TEST--
Saved docProps/core.xml holds core properties as UTF-8
--EXTENSIONS--
excel
zlib
--FILE--
<?php
/* Reads the member through the central directory, which carries the sizes
 * even when the local header defers them to a data descriptor. */
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

function stored_title($zip) {
    preg_match('~<dc:title>(.*?)</dc:title>~s', zip_member($zip, 'docProps/core.xml'), $m);
    return $m[1] ?? null;
}

$cafe = "Caf\xc3\xa9 \xe2\x82\xac \xc3\x81\xc3\x9d";
$book = new ExcelBook(null, null, true);
$book->addSheet('Data');
var_dump($book->coreProperties()->setTitle($cafe));
$saved = $book->save();
var_dump(bin2hex(stored_title($saved)));

$copy = new ExcelBook(null, null, true);
var_dump($copy->load($saved));
var_dump(bin2hex(stored_title($copy->save())));
?>
--EXPECT--
bool(true)
string(28) "436166c3a920e282ac20c381c39d"
bool(true)
string(28) "436166c3a920e282ac20c381c39d"
