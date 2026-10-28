--TEST--
CoreProperties double getter reference includes wrapper failure returns
--EXTENSIONS--
excel
--FILE--
<?php
$source = file_get_contents(__DIR__ . '/../docs/ExcelCoreProperties.php');
eval('namespace CorePropertiesDocs; ' . substr($source, strlen('<?php')));
$reference = new ReflectionClass(CorePropertiesDocs\ExcelCoreProperties::class);
$methods = ['createdAsDouble', 'modifiedAsDouble'];
$errors = [];
foreach ($methods as $method) {
    $doc = $reference->getMethod($method)->getDocComment();
    $date = $method === 'createdAsDouble' ? 'created' : 'modified';
    $claim = 'Returns the ' . $date . ' date as a double (Excel timestamp) @return float|false Excel timestamp, or false with a warning for an uninitialized or stale wrapper';
    $rendered = preg_replace('/\s+/', ' ', trim(preg_replace('/(?:^\s*\/\*\*|\*\/\s*$|^\s*\*\s?)/m', '', substr(str_replace("\r\n", "\n", $doc), 3, -2))));
    if ($rendered !== $claim) {
        $errors[] = "incorrect complete return claim: $method";
    }
}
$native = new ReflectionClass(ExcelCoreProperties::class);
$getters = [];
foreach ($native->getMethods() as $method) {
    if (str_ends_with($method->getName(), 'AsDouble') && $method->getNumberOfParameters() === 0) {
        $getters[] = $method->getName();
    }
}
sort($getters);
if ($getters !== $methods) $errors[] = 'double getter coverage is incomplete';
$book = new ExcelBook(null, null, true);
$properties = new ExcelCoreProperties($book);
foreach ($methods as $index => $method) {
    if (!is_float($properties->$method())) $errors[] = "unexpected live value: $method";
    $value = 44000.5 + $index;
    $setter = 'set' . ucfirst($method);
    if (!$properties->$setter($value) || $properties->$method() !== $value) {
        $errors[] = "timestamp did not round-trip: $method";
    }
}
$uninitialized = $native->newInstanceWithoutConstructor();
$book->__construct(null, null, true);
foreach (['uninitialized' => $uninitialized, 'stale' => $properties] as $state => $wrapper) {
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
else echo "2 complete claims, 2 live values, 2 round-trips and 4 wrapper failures verified\n";
?>
--EXPECT--
2 complete claims, 2 live values, 2 round-trips and 4 wrapper failures verified
