--TEST--
Table getter reference documents live, uninitialized and stale return contracts
--EXTENSIONS--
excel
--FILE--
<?php
function check($condition, $label) {
    if (!$condition) {
        echo "FAIL: $label\n";
    }
}
$source = file_get_contents(__DIR__ . '/../docs/ExcelTable.php');
$source = str_replace('class ExcelTable', 'class DocumentedExcelTable', $source);
eval('?>' . $source);
$docs = new ReflectionClass(DocumentedExcelTable::class);
check(str_contains($docs->getDocComment(),
    'The getters below warn and return false for an uninitialized or stale table wrapper.'),
    'complete wrapper failure explanation');
$returns = [
    'name' => 'string|null|false',
    'ref' => 'string|null|false',
    'columnSize' => 'int|false',
    'autoFilter' => 'ExcelAutoFilter|false',
];
foreach ($returns as $method => $expected) {
    preg_match('/@return\s+(\S+)/', $docs->getMethod($method)->getDocComment(), $match);
    check(($match[1] ?? '') === $expected, "$method full return union");
}
check(str_contains($docs->getMethod('autoFilter')->getDocComment(),
    '@return ExcelAutoFilter|false The table autofilter, or false when the wrapper is uninitialized or stale, or libxl yields no handle.'),
    'complete autofilter failure explanation');

$book = new ExcelBook(null, null, true);
$book->addSheet('Temporary');
$sheet = $book->addSheet('Retained');
$sheet->write(1, 0, 'First');
$sheet->write(1, 1, 'Second');
$table = $sheet->addTable('Data', 1, 3, 0, 1);
check($table instanceof ExcelTable, 'table created');
check($table->name() === 'Data', 'live name');
check($table->ref() === 'A2:B4', 'live range');
check($table->columnSize() === 2, 'live column count');
$filter = $table->autoFilter();
check($filter instanceof ExcelAutoFilter || $filter === false, 'live filter');

class UninitializedTable extends ExcelTable {
    public function __construct() {}
}
check($book->deleteSheet(0), 'delete unrelated sheet');
foreach (['uninitialized' => new UninitializedTable(), 'stale' => $table] as $state => $invalid) {
    foreach (array_keys($returns) as $method) {
        $warnings = [];
        set_error_handler(function ($severity, $message) use (&$warnings) {
            $warnings[] = [$severity, $message];
            return true;
        });
        try {
            $result = $invalid->$method();
        } finally {
            restore_error_handler();
        }
        check($result === false, "$state $method returns false");
        check(count($warnings) === 1 && $warnings[0][0] === E_WARNING,
            "$state $method emits one warning");
    }
}
echo "Table getter contracts OK\n";
?>
--EXPECT--
Table getter contracts OK
