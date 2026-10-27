--TEST--
FormControl reference includes nullable values and wrapper failure returns
--EXTENSIONS--
excel
--FILE--
<?php
$integer = ['objectType', 'checked', 'itemSize', 'dropLines', 'dx', 'inc', 'getMax', 'getMin', 'sel'];
$text = ['fmlaGroup', 'fmlaLink', 'fmlaRange', 'fmlaTxbx', 'name', 'linkedCell', 'listFillRange', 'macro', 'altText', 'multiSel'];
$anchors = ['fromAnchor', 'toAnchor'];
$methods = array_merge($integer, $text, $anchors);
$source = file_get_contents(__DIR__ . '/../docs/ExcelFormControl.php');
eval('namespace FormControlDocs; ' . substr($source, strlen('<?php')));
$reference = new ReflectionClass(FormControlDocs\ExcelFormControl::class);
$docs = [];
foreach ($reference->getMethods() as $method) {
    $docs[$method->getName()] = $method->getDocComment();
}
$errors = [];
foreach ($methods as $method) {
    if (in_array($method, $integer, true)) {
        $value = match ($method) {
            'objectType' => 'One of ExcelFormControl::OBJECT_* constants',
            'checked' => 'One of ExcelFormControl::CHECKEDTYPE_* constants',
            default => 'Value',
        };
        $claim = 'int|false ' . $value . ', or false for an uninitialized or stale wrapper';
    } elseif (in_array($method, $text, true)) {
        $claim = 'string|null|false Text or null, or false for an uninitialized or stale wrapper';
    } else {
        $claim = 'array|false Anchor position, or false for an uninitialized or stale wrapper or when LibXL cannot retrieve the anchor';
    }
    if (!preg_match('/@return ([^\r\n]+)/', $docs[$method] ?? '', $match) || trim($match[1]) !== $claim) {
        $errors[] = "incorrect complete return claim: $method";
    }
}
$book = new ExcelBook(null, null, true);
$book->loadFile(__DIR__ . '/formcontrols.xlsx');
$control = $book->getSheet(0)->formControl(0);
foreach ($methods as $method) {
    $value = $control->$method();
    $valid = in_array($method, $integer, true) ? is_int($value)
        : (in_array($method, $text, true) ? (is_string($value) || $value === null)
        : (is_array($value) && array_keys($value) === ['col', 'colOff', 'row', 'rowOff']
            && count(array_filter($value, 'is_int')) === 4));
    if (!$valid) $errors[] = "unexpected live value: $method";
}
$uninitialized = (new ReflectionClass(ExcelFormControl::class))->newInstanceWithoutConstructor();
$book->__construct(null, null, true);
foreach (['uninitialized' => $uninitialized, 'stale' => $control] as $state => $wrapper) {
    foreach ($methods as $method) {
        $warnings = [];
        set_error_handler(function ($severity, $message) use (&$warnings) {
            $warnings[] = $severity;
            return true;
        });
        try {
            $value = $wrapper->$method();
        } finally {
            restore_error_handler();
        }
        if ($value !== false || $warnings !== [E_WARNING]) $errors[] = "incorrect $state failure: $method";
    }
}
if ($errors) echo implode("\n", $errors), "\n";
else echo "21 complete claims, 21 live values and 42 wrapper failures verified\n";
?>
--EXPECT--
21 complete claims, 21 live values and 42 wrapper failures verified
