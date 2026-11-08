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
$filterDoc = $docs->getMethod('autoFilter')->getDocComment();
preg_match('/@note [^`]*`([^`]+)`/', $filterDoc, $example);
$example = $example[1] ?? '';
preg_match('/@example [^`]*`([^`]+)`/', $filterDoc, $readOnly);
$readOnly = $readOnly[1] ?? '';
check($example !== '', 'get-or-create example exists');
check($readOnly !== '', 'read-only example exists');
function documentedFilterRange($t, $example) {
    $range = 'not assigned';
    eval($example . ';');
    return $range;
}
// Model the older API without isAutoFilter(), including the no-handle result.
foreach ([false, new class {
    public function getRef() { return ['row_first' => 1, 'row_last' => 3, 'col_first' => 0, 'col_last' => 1]; }
}] as $handle) {
    $olderTable = new class($handle) {
        public $calls = 0;
        private $handle;
        public function __construct($handle) { $this->handle = $handle; }
        public function autoFilter() { ++$this->calls; return $this->handle; }
    };
    try {
        check(documentedFilterRange($olderTable, $readOnly) === null && $olderTable->calls === 0,
            'read-only example does not call get-or-create on older API');
        check(documentedFilterRange($olderTable, $example) === ($handle === false ? null : $handle->getRef()),
            'get-or-create example handles older API and no-handle failure');
    } catch (Throwable $error) {
        check(false, 'example threw ' . get_class($error));
    }
}

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
check(documentedFilterRange($table, $example) === ($filter === false ? null : $filter->getRef()),
    'example works with native table');

// Exercise the actual native mutation, not only an already-filtered table.
if (method_exists($table, 'removeFilter')) {
    check($table->removeFilter(), 'remove filter');
    check(!$table->isAutoFilter(), 'native table starts filterless');
    check(documentedFilterRange($table, $readOnly) === null, 'read-only absent range');
    check(!$table->isAutoFilter(), 'read-only example preserves filter absence');
    check(documentedFilterRange($table, $example) === false, 'new filter has unset range');
    check($table->isAutoFilter(), 'get-or-create example creates absent filter');
    $filter = $table->autoFilter();
    $filter->setRef(1, 3, 0, 1);
    check(documentedFilterRange($table, $readOnly) === $filter->getRef(),
        'read-only example reads existing native filter');
}
// Even a positive existence probe does not justify chaining a failed handle.
$failedHandle = new class {
    public function isAutoFilter() { return true; }
    public function autoFilter() { return false; }
};
check(documentedFilterRange($failedHandle, $readOnly) === null, 'read-only checks returned handle');

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
