--TEST--
unpackDate() accepts the valid timestamp -1 at the pre-epoch boundary
--EXTENSIONS--
excel
--ENV--
TZ=UTC
--DESCRIPTION--
_php_excel_date_unpack() used mktime()'s -1 return as its failure sentinel, so
unpackDate() rejected the real instant 1969-12-31T23:59:59 UTC, whose unix
timestamp is exactly -1. Failure is now signalled separately (errno check) and
the value is returned via an out-parameter.
--FILE--
<?php
$b = new ExcelBook();

$dt = $b->packDateValues(1969, 12, 31, 23, 59, 59);
var_dump($dt > 0);
// The whole point of the fix: -1 is a valid timestamp, not an error.
var_dump($b->unpackDate($dt));

var_dump($b->unpackDate($b->packDateValues(1970, 1, 1, 0, 0, 0)));

// Ordinary dates still round-trip through the reworked helper.
$ts = strtotime('2024-06-15 12:30:45');
var_dump($b->unpackDate($b->packDate($ts)) === $ts);

// A cell holding the timestamp -1 reads back instead of failing the read.
$sheet = $b->addSheet('S1');
var_dump($sheet->write(1, 1, -1, null, ExcelFormat::AS_DATE));
var_dump($sheet->read(1, 1));

// Genuine failures still return false.
var_dump($b->unpackDate(-5.0));
?>
--EXPECT--
bool(true)
int(-1)
int(0)
bool(true)
bool(true)
int(-1)
bool(false)
