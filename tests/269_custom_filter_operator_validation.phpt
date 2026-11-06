--TEST--
Custom filters reject unsupported operators without changing existing criteria
--EXTENSIONS--
excel
--FILE--
<?php
$book = new ExcelBook(null, null, true);
$sheet = $book->addSheet('Filter');
$sheet->write(1, 0, 'Header');
$sheet->write(2, 0, 'Value');
$filter = $sheet->autoFilter();
$filter->setRef(1, 2, 0, 0);
$column = $filter->column(0);
$column->setCustomFilter(
    ExcelFilterColumn::OPERATOR_GREATER_THAN, 'a',
    ExcelFilterColumn::OPERATOR_LESS_THAN, 'z', true
);
$before = $column->getCustomFilter();

foreach (['first', 'second', 'second with null value'] as $position) {
    $warnings = [];
    set_error_handler(function ($level, $message) use (&$warnings) {
        $warnings[] = preg_replace('/^.*\(\): /', '', $message);
        return true;
    });
    $rejected = $unchanged = 0;
    foreach ([$position === 'first' ? -1 : -2, 6, 99, 2147483647] as $op) {
        $result = $position === 'first'
            ? $column->setCustomFilter($op, 'bad')
            : $column->setCustomFilter(
                ExcelFilterColumn::OPERATOR_EQUAL, 'bad', $op,
                $position === 'second' ? 'bad' : null
            );
        $rejected += $result === false;
        $unchanged += $column->getCustomFilter() === $before;
    }
    restore_error_handler();
    echo "$position: rejected=$rejected unchanged=$unchanged\n";
    echo implode("\n", $warnings), "\n";
}

// Exercise every valid pair, and the single-criterion fallback forms.
$pairs = $single = 0;
for ($first = 0; $first <= 5; $first++) {
    for ($second = 0; $second <= 5; $second++) {
        $column->clear();
        $ok = $column->setCustomFilter($first, 'a', $second, 'z', true);
        $criteria = $column->getCustomFilter();
        $pairs += $ok === true
            && $criteria['operator_1'] === $first
            && $criteria['operator_2'] === $second
            && $criteria['value_1'] === 'a'
            && $criteria['value_2'] === 'z'
            && $criteria['and_operator'] === true;
    }
    foreach ([[$first, ''], [$first, '', -1, null], [$first, '', -1, 'ignored'], [$first, '', 5, null]] as $args) {
        $column->clear();
        $ok = $column->setCustomFilter(...$args);
        $criteria = $column->getCustomFilter();
        $single += $ok === true
            && $criteria['operator_1'] === $first
            && $criteria['value_1'] === '';
    }
}
echo "valid pairs=$pairs single-criterion forms=$single\n";
?>
--EXPECT--
first: rejected=4 unchanged=4
Argument out of int range
Invalid custom filter operator
Invalid custom filter operator
Invalid custom filter operator
second: rejected=4 unchanged=4
Argument out of int range
Invalid custom filter operator
Invalid custom filter operator
Invalid custom filter operator
second with null value: rejected=4 unchanged=4
Argument out of int range
Invalid custom filter operator
Invalid custom filter operator
Invalid custom filter operator
valid pairs=36 single-criterion forms=24
