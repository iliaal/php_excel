--TEST--
Conditional formatting removal invalidates workbook-wide wrappers only on success
--EXTENSIONS--
excel
--SKIPIF--
<?php
if (!method_exists('ExcelSheet', 'removeConditionalFormatting')) {
    die('skip LibXL 5.1.0+ required');
}
?>
--FILE--
<?php
function check(string $label, bool $ok): void {
    echo $label, ': ', $ok ? 'OK' : 'FAIL', "\n";
}

$book = new ExcelBook(null, null, true);
$sheet = $book->addSheet('Remove');
$other = $book->addSheet('Keep');
$empty = $book->addSheet('Empty');
$sheet->addConditionalFormatting(1, 1, 0, 0);
$sameSheet = new ExcelConditionalFormatting($sheet, 2, 2, 0, 0);
$other->addConditionalFormatting(1, 1, 0, 0);
$otherSheet = $other->conditionalFormatting(0);
$style = $book->addConditionalFormat();

$separate = new ExcelBook(null, null, true);
$separateSheet = $separate->addSheet('Separate');
$separateFormatting = $separateSheet->addConditionalFormatting(1, 1, 0, 0);

// Both a missing index and an empty sheet leave all existing wrappers valid.
check('missing index', $sheet->removeConditionalFormatting(999) === false);
check('empty sheet', $empty->removeConditionalFormatting(0) === false);
check('same sheet after failures', $sameSheet->addRange(3, 3, 0, 0));
check('other sheet after failures', $otherSheet->addRange(3, 3, 0, 0));

check('remove', $sheet->removeConditionalFormatting(0));
check('surviving collections', $sheet->conditionalFormattingSize() === 1
    && $other->conditionalFormattingSize() === 1);

// Only probe surviving native handles, so a missing generation bump fails
// safely rather than calling through the handle that LibXL just destroyed.
$warnings = [];
set_error_handler(function (int $severity, string $message) use (&$warnings): bool {
    $warnings[] = [$severity, $message];
    return true;
});
check('stale same sheet', $sameSheet->addRange(4, 4, 0, 0) === false);
check('stale other sheet', $otherSheet->addRange(4, 4, 0, 0) === false);
restore_error_handler();
$expectedWarning = [E_WARNING, 'ExcelConditionalFormatting::addRange(): '
    . 'Underlying ExcelBook conditional formatting state changed; wrapper must be re-fetched'];
check('exact warnings', $warnings === [$expectedWarning, $expectedWarning]);

check('sheets remain usable', $sheet->write(1, 0, 'A') && $other->write(1, 0, 'B'));
check('style remains usable', $style->setFillPattern(1));
check('other workbook remains usable', $separateFormatting->addRange(4, 4, 0, 0));

// The survivor moved from index 1 to 0 only on the sheet that lost a collection.
$freshSame = $sheet->conditionalFormatting(0);
$freshOther = $other->conditionalFormatting(0);
check('shifted index', $sheet->conditionalFormatting(1) === false);
check('re-fetched same sheet', $freshSame->addRange(5, 5, 0, 0));
check('re-fetched other sheet', $freshOther->addRange(5, 5, 0, 0));
check('retained style can be applied', $freshOther->addRule(
    ExcelConditionalFormatting::CFORMAT_CONTAINSBLANKS, $style, ''));
?>
--EXPECT--
missing index: OK
empty sheet: OK
same sheet after failures: OK
other sheet after failures: OK
remove: OK
surviving collections: OK
stale same sheet: OK
stale other sheet: OK
exact warnings: OK
sheets remain usable: OK
style remains usable: OK
other workbook remains usable: OK
shifted index: OK
re-fetched same sheet: OK
re-fetched other sheet: OK
retained style can be applied: OK
