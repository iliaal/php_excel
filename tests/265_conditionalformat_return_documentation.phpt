--TEST--
ConditionalFormat reference includes nullable text and wrapper failure returns
--EXTENSIONS--
excel
--FILE--
<?php
$integer = ['numFormat', 'borderLeft', 'borderRight', 'borderTop', 'borderBottom', 'borderLeftColor', 'borderRightColor', 'borderTopColor', 'borderBottomColor', 'fillPattern', 'patternForegroundColor', 'patternBackgroundColor'];
$methods = array_merge(['font', 'customNumFormat'], $integer);
$source = file_get_contents(__DIR__ . '/../docs/ExcelConditionalFormat.php');
eval('namespace ConditionalFormatDocs; ' . substr($source, strlen('<?php')));
$reference = new ReflectionClass(ConditionalFormatDocs\ExcelConditionalFormat::class);
$errors = [];
foreach ($methods as $method) {
    $claim = match ($method) {
        'font' => 'ExcelFont|false Font, or false for an uninitialized or stale wrapper or when LibXL returns no font',
        'customNumFormat' => 'string|null|false Text, null when LibXL returns no custom format string, or false for an uninitialized or stale wrapper',
        default => 'int|false Value, or false for an uninitialized or stale wrapper',
    };
    $doc = $reference->getMethod($method)->getDocComment();
    if (!preg_match('/@return ([^\r\n]+)/', $doc, $match) || trim($match[1]) !== $claim) {
        $errors[] = "incorrect complete return claim: $method";
    }
}
$native = new ReflectionClass(ExcelConditionalFormat::class);
$getters = [];
foreach ($native->getMethods() as $method) {
    if ($method->getNumberOfParameters() === 0 && (string) $method->getReturnType() === 'mixed') {
        $getters[] = $method->getName();
    }
}
sort($getters);
$sorted = $methods;
sort($sorted);
if ($getters !== $sorted) $errors[] = 'getter coverage is incomplete';
$book = new ExcelBook(null, null, true);
$format = new ExcelConditionalFormat($book);
foreach ($methods as $method) {
    $value = $format->$method();
    $valid = match ($method) {
        'font' => $value instanceof ExcelFont,
        'customNumFormat' => is_string($value) || $value === null,
        default => is_int($value),
    };
    if (!$valid) $errors[] = "unexpected live value: $method";
}
$format->setCustomNumFormat('0.00');
if ($format->customNumFormat() !== '0.00') $errors[] = 'custom format text did not round-trip';
$uninitialized = $native->newInstanceWithoutConstructor();
$book->__construct(null, null, true);
foreach (['uninitialized' => $uninitialized, 'stale' => $format] as $state => $wrapper) {
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
else echo "14 complete claims, 14 live values, custom text and 28 wrapper failures verified\n";
?>
--EXPECT--
14 complete claims, 14 live values, custom text and 28 wrapper failures verified
