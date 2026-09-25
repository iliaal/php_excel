--TEST--
ExcelFilterColumn custom filters accept empty criteria
--EXTENSIONS--
excel
--FILE--
<?php
$book = new ExcelBook(null, null, true);
$sheet = $book->addSheet('Filter');
$sheet->write(1, 0, 'First');
$sheet->write(1, 1, 'Second');
$sheet->write(1, 2, 'Third');
$sheet->write(1, 3, 'Control');
$filter = $sheet->autoFilter();
$filter->setRef(1, 2, 0, 3);

$first = $filter->column(0);
var_dump($first->setCustomFilter(ExcelFilterColumn::OPERATOR_EQUAL, ''));
var_dump($first->filterType() === ExcelFilterColumn::FILTER_CUSTOM);
var_dump($first->getCustomFilter());

$second = $filter->column(1);
var_dump($second->setCustomFilter(
    ExcelFilterColumn::OPERATOR_EQUAL,
    'Alice',
    ExcelFilterColumn::OPERATOR_EQUAL,
    ''
));
var_dump($second->filterType() === ExcelFilterColumn::FILTER_CUSTOM);
var_dump($second->getCustomFilter());

$control = $filter->column(2);
var_dump($control->setCustomFilter(
    ExcelFilterColumn::OPERATOR_EQUAL,
    'Bob',
    ExcelFilterColumn::OPERATOR_EQUAL,
    'Alice',
    true
));
var_dump($control->getCustomFilter());
echo "OK\n";
?>
--EXPECT--
bool(true)
bool(true)
array(5) {
  ["operator_1"]=>
  int(0)
  ["value_1"]=>
  string(0) ""
  ["operator_2"]=>
  int(0)
  ["value_2"]=>
  string(0) ""
  ["and_operator"]=>
  bool(false)
}
bool(true)
bool(true)
array(5) {
  ["operator_1"]=>
  int(0)
  ["value_1"]=>
  string(5) "Alice"
  ["operator_2"]=>
  int(0)
  ["value_2"]=>
  string(0) ""
  ["and_operator"]=>
  bool(false)
}
bool(true)
array(5) {
  ["operator_1"]=>
  int(0)
  ["value_1"]=>
  string(3) "Bob"
  ["operator_2"]=>
  int(0)
  ["value_2"]=>
  string(5) "Alice"
  ["and_operator"]=>
  bool(true)
}
OK
