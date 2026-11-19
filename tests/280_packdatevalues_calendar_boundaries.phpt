--TEST--
Component date packing rejects unsupported calendar boundaries while preserving explicit times
--EXTENSIONS--
excel
--INI--
date.timezone=UTC
--FILE--
<?php
function check($condition, $label) {
    if (!$condition) throw new RuntimeException($label);
}
function packWithoutWarnings($book, $parts, &$warnings) {
    $warnings = [];
    set_error_handler(function ($severity, $message) use (&$warnings) {
        $warnings[] = $message;
        return true;
    });
    try {
        return $book->packDateValues(...$parts);
    } finally {
        restore_error_handler();
    }
}
foreach ([false, true] as $xlsx) {
    foreach ([false, true] as $date1904) {
        $book = new ExcelBook(null, null, $xlsx);
        $book->setDate1904($date1904);
        $first = $date1904 ? [1904, 1, 2] : [1900, 1, 1];
        foreach ([0, 12, 23] as $hour) {
            $parts = [...$first, $hour, 0, 0];
            $serial = $book->packDateValues(...$parts);
            check($serial === 1.0 + $hour / 24, 'first supported calendar serial');
            // Dates near 1900 exceed the 32-bit PHP timestamp range; component
            // serial checks and every rejection below still run on 32-bit PHP.
            if (PHP_INT_SIZE >= 8) {
                $timestamp = strtotime(sprintf('%04d-%02d-%02d %02d:00:00', ...[...$first, $hour]));
                check($book->unpackDate($serial) === $timestamp, 'calendar round trip');
                check($book->packDate($timestamp) === $serial, 'timestamp/component agreement');
            }
        }
        $invalidDates = [[1, 1, 1], [1800, 1, 1], [1899, 12, 30], [1899, 12, 31]];
        if ($date1904) {
            $invalidDates = array_merge($invalidDates, [[1900, 1, 1], [1903, 12, 31], [1904, 1, 1]]);
        }
        foreach ($invalidDates as $date) {
            foreach ([[0, 0, 0], [12, 0, 0], [23, 59, 59]] as $time) {
                check(packWithoutWarnings($book, [...$date, ...$time], $warnings) === false,
                    'unsupported calendar date rejected');
            }
        }
        foreach ([[0, 0, 0], [12, 0, 0], [23, 59, 59]] as $time) {
            $seconds = $time[0] * 3600 + $time[1] * 60 + $time[2];
            $serial = $book->packDateValues(0, 0, 0, ...$time);
            check(abs($serial - $seconds / 86400) < 1e-12, 'explicit time-only serial');
            check($book->unpackDate($serial) === $seconds, 'time-only round trip');
        }
        // Invalid time validation still precedes calendar-year validation.
        foreach ([[24, 0, 0], [0, 60, 0], [0, 0, 60]] as $time) {
            check(packWithoutWarnings($book, [2024, 1, 1, ...$time], $validYearWarnings) === false,
                'invalid time rejected');
            check(packWithoutWarnings($book, [1899, 1, 1, ...$time], $oldYearWarnings) === false,
                'invalid time with old year rejected');
            check(count($validYearWarnings) === 1 && $validYearWarnings === $oldYearWarnings,
                'time validation precedence unchanged');
        }
        // This change does not replace LibXL calendar normalization.
        check($book->packDateValues(2024, 2, 30, 12, 0, 0) ===
            $book->packDateValues(2024, 3, 1, 12, 0, 0), 'month-day normalization preserved');
        if (!$date1904) {
            check($book->packDateValues(1900, 2, 29, 12, 0, 0) === 60.5, 'Excel leap-day quirk preserved');
        }
        echo ($xlsx ? 'xlsx' : 'xls'), ' ', ($date1904 ? '1904' : '1900'), ": OK\n";
    }
}
?>
--EXPECT--
xls 1900: OK
xls 1904: OK
xlsx 1900: OK
xlsx 1904: OK
