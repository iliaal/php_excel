--TEST--
Timestamp date conversions reject unsupported workbook calendar boundaries
--EXTENSIONS--
excel
--SKIPIF--
<?php if (PHP_INT_SIZE < 8) die('skip 64-bit timestamps required for dates near 1900'); ?>
--ENV--
TZ=UTC
--INI--
date.timezone=UTC
--FILE--
<?php
function check($condition, $label) {
    if (!$condition) {
        throw new RuntimeException($label);
    }
}
foreach ([false, true] as $xlsx) {
    foreach ([false, true] as $date1904) {
        foreach (['UTC', 'America/New_York'] as $timezone) {
            date_default_timezone_set($timezone);
            $label = ($xlsx ? 'xlsx' : 'xls') . ' ' . ($date1904 ? '1904' : '1900') . ' ' . $timezone;
            $book = new ExcelBook(null, null, $xlsx);
            $book->setDate1904($date1904);
            $sheet = $book->addSheet('Dates');
            $first = strtotime($date1904 ? '1904-01-02 00:00:00' : '1900-01-01 00:00:00');
            foreach ([$first, $first + 43200, -1] as $timestamp) {
                $serial = $book->packDate($timestamp);
                check(is_float($serial) && $serial >= 1, "$label: accepted serial");
                check($book->unpackDate($serial) === $timestamp, "$label: timestamp round trip");
                check($sheet->write(1, 0, $timestamp, null, ExcelFormat::AS_DATE), "$label: accepted write");
                check($sheet->read(1, 0) === $timestamp, "$label: written date round trip");
            }
            $invalid = [$first - 1];
            foreach (['1800-01-01 00:00:00', '1899-12-28 00:00:00', '1899-12-29 00:00:00',
                '1899-12-30 00:00:00', '1899-12-31 00:00:00', '1899-12-31 12:00:00'] as $date) {
                $invalid[] = strtotime($date);
            }
            if ($date1904) {
                foreach (['1900-01-01 00:00:00', '1903-12-31 00:00:00',
                    '1904-01-01 00:00:00', '1904-01-01 12:00:00'] as $date) {
                    $invalid[] = strtotime($date);
                }
            }
            $sheet->write(2, 0, 'keep first');
            $sheet->write(2, 1, 'keep row');
            $sheet->write(3, 0, 'keep column');
            foreach ($invalid as $timestamp) {
                check($book->packDate($timestamp) === false, "$label: rejected timestamp $timestamp");
                foreach ([$timestamp, (float) $timestamp] as $value) {
                    $warnings = [];
                    set_error_handler(function ($severity, $message) use (&$warnings) {
                        $warnings[] = $message;
                        return true;
                    });
                    try {
                        check($sheet->write(2, 0, $value, null, ExcelFormat::AS_DATE) === false, "$label: single rejection");
                        check($sheet->writeRow(2, [-1, $value], 0, null, ExcelFormat::AS_DATE) === false, "$label: row rejection");
                        check($sheet->writeCol(0, [-1, $value], 2, null, ExcelFormat::AS_DATE) === false, "$label: column rejection");
                    } finally {
                        restore_error_handler();
                    }
                    check(count($warnings) === 3, "$label: rejection warnings");
                    foreach ($warnings as $warning) {
                        check(str_contains($warning, 'timestamp is outside the range Excel can store as a date'), "$label: rejection reason");
                    }
                    check($sheet->read(2, 0) === 'keep first' && $sheet->read(2, 1) === 'keep row'
                        && $sheet->read(3, 0) === 'keep column', "$label: rejected writes are atomic");
                }
            }
            // Explicit time-only values keep their existing contract.
            check($book->packDateValues(0, 0, 0, 12, 0, 0) === 0.5, "$label: time-only pack");
            check($book->unpackDate(0.0) === 0 && $book->unpackDate(0.5) === 43200, "$label: time-only unpack");
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
