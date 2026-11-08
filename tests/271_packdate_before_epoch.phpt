--TEST--
packDate() accepts negative timestamps in both workbook formats and date systems
--EXTENSIONS--
excel
--ENV--
TZ=UTC
--INI--
date.timezone=UTC
--FILE--
<?php
foreach ([false, true] as $xlsx) {
    foreach ([false, true] as $date1904) {
        $book = new ExcelBook(null, null, $xlsx);
        $book->setDate1904($date1904);
        $sheet = $book->addSheet('Dates');
        foreach (['UTC', 'America/New_York'] as $timezone) {
            date_default_timezone_set($timezone);
            $label = ($xlsx ? 'xlsx' : 'xls') . ' ' . ($date1904 ? '1904' : '1900') . ' ' . $timezone;
            foreach ([-86400, -1, 0, 86400] as $timestamp) {
                $parts = array_map('intval', explode(' ', date('Y n j G i s', $timestamp)));
                $serial = $book->packDate($timestamp);
                if (!is_float($serial) || $serial !== $book->packDateValues(...$parts)) {
                    throw new RuntimeException("$label: could not pack $timestamp");
                }
                if ($book->unpackDate($serial) !== $timestamp) {
                    throw new RuntimeException("$label: could not unpack $timestamp");
                }
                if (!$sheet->write(1, 0, $timestamp, null, ExcelFormat::AS_DATE)
                    || $sheet->read(1, 0) !== $timestamp) {
                    throw new RuntimeException("$label: date write differs for $timestamp");
                }
            }
            echo "$label: OK\n";
        }
    }
}
?>
--EXPECT--
xls 1900 UTC: OK
xls 1900 America/New_York: OK
xls 1904 UTC: OK
xls 1904 America/New_York: OK
xlsx 1900 UTC: OK
xlsx 1900 America/New_York: OK
xlsx 1904 UTC: OK
xlsx 1904 America/New_York: OK
