--TEST--
Conditional formatting time-period rules reject unsupported enum values
--EXTENSIONS--
excel
--FILE--
<?php
$book = new ExcelBook(null, null, true);
$sheet = $book->addSheet('Dates');
$format = $book->addConditionalFormat();
$format->setFillPattern(1);
if ((new ReflectionMethod($sheet, 'addConditionalFormatting'))->getNumberOfParameters() === 0) {
    $formatting = $sheet->addConditionalFormatting();
    $formatting->addRange(1, 3, 0, 0);
} else {
    $formatting = $sheet->addConditionalFormatting(1, 3, 0, 0);
}

foreach ([
    ExcelConditionalFormatting::CFTP_LAST7DAYS,
    ExcelConditionalFormatting::CFTP_LASTMONTH,
    ExcelConditionalFormatting::CFTP_LASTWEEK,
    ExcelConditionalFormatting::CFTP_NEXTMONTH,
    ExcelConditionalFormatting::CFTP_NEXTWEEK,
    ExcelConditionalFormatting::CFTP_THISMONTH,
    ExcelConditionalFormatting::CFTP_THISWEEK,
    ExcelConditionalFormatting::CFTP_TODAY,
    ExcelConditionalFormatting::CFTP_TOMORROW,
    ExcelConditionalFormatting::CFTP_YESTERDAY,
] as $period) {
    foreach ([false, true] as $stopIfTrue) {
        if ($formatting->addTimePeriodRule($format, $period, $stopIfTrue) !== true) {
            throw new Exception("Valid time period $period rejected");
        }
    }
}
echo "All valid periods accepted\n";

$warnings = [];
set_error_handler(static function (int $severity, string $message) use (&$warnings): bool {
    $warnings[] = [$severity, $message];
    return true;
});
foreach ([-1, 10, 11, 999, 2147483647, PHP_INT_MAX] as $period) {
    foreach ([false, true] as $stopIfTrue) {
        $warnings = [];
        $result = $formatting->addTimePeriodRule($format, $period, $stopIfTrue);
        $reason = $period < 0 || $period > 2147483647
            ? 'Argument out of int range'
            : 'Invalid conditional formatting time period';
        if ($result !== false || $warnings !== [[E_WARNING,
            "ExcelConditionalFormatting::addTimePeriodRule(): $reason"]]) {
            throw new Exception("Invalid time period $period not rejected correctly");
        }
    }
}
restore_error_handler();
echo "All invalid periods rejected with a warning\n";

// Throwing handlers must also stop before LibXL sees the invalid period.
set_error_handler(static function (int $severity, string $message): never {
    throw new ErrorException($message, 0, $severity);
});
try {
    $formatting->addTimePeriodRule($format, 10);
    echo "FAIL: expected exception\n";
} catch (ErrorException $e) {
    echo $e->getMessage(), "\n";
}
restore_error_handler();

// A rejection does not prevent adding a subsequent valid rule or saving.
var_dump($formatting->addTimePeriodRule($format, ExcelConditionalFormatting::CFTP_TODAY));
$copy = new ExcelBook(null, null, true);
var_dump($copy->load($book->save()));
?>
--EXPECT--
All valid periods accepted
All invalid periods rejected with a warning
ExcelConditionalFormatting::addTimePeriodRule(): Invalid conditional formatting time period
bool(true)
bool(true)
