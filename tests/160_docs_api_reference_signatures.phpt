--TEST--
Docs API reference exhaustively matches the runtime public API
--EXTENSIONS--
excel
--FILE--
<?php
function typeShape(?ReflectionType $type): array
{
    if ($type === null) {
        return [];
    }
    if ($type instanceof ReflectionNamedType) {
        $types = [$type->getName()];
        if ($type->allowsNull() && $type->getName() !== "null" && $type->getName() !== "mixed") {
            $types[] = "null";
        }
        sort($types);
        return $types;
    }
    $types = [];
    foreach ($type->getTypes() as $member) {
        foreach (typeShape($member) as $name) {
            $types[$name] = true;
        }
    }
    $types = array_keys($types);
    sort($types);
    return $types;
}

function methodShape(ReflectionMethod $method): array
{
    $parameters = [];
    foreach ($method->getParameters() as $parameter) {
        $parameters[] = [
            "name" => $parameter->getName(),
            "type" => typeShape($parameter->getType()),
            "reference" => $parameter->isPassedByReference(),
            "variadic" => $parameter->isVariadic(),
            "optional" => $parameter->isOptional(),
            "default" => $parameter->isDefaultValueAvailable()
                ? serialize($parameter->getDefaultValue())
                : null,
        ];
    }
    return ["parameters" => $parameters, "return" => typeShape($method->getReturnType())];
}

$errors = [];
$documentedVersionGates = [];
if (!method_exists(ExcelBook::class, "loadPartially")) {
    $documentedVersionGates += array_fill_keys([
        "ExcelBook::loadPartially",
        "ExcelBook::loadFilePartially",
        "ExcelBook::loadFileWithoutEmptyCells",
        "ExcelBook::setPassword",
        "ExcelBook::dpiAwareness",
        "ExcelBook::setDpiAwareness",
    ], true);
}
if (!method_exists(ExcelBook::class, "loadInfoRaw")) {
    $documentedVersionGates["ExcelBook::loadInfoRaw"] = true;
}
if (!method_exists(ExcelBook::class, "errorCode")) {
    $documentedVersionGates += array_fill_keys([
        "ExcelBook::errorCode",
        "ExcelBook::conditionalFormat",
        "ExcelBook::conditionalFormatSize",
        "ExcelBook::clear",
        "ExcelSheet::addConditionalFormatting",
        "ExcelSheet::conditionalFormatting",
        "ExcelSheet::removeConditionalFormatting",
        "ExcelSheet::conditionalFormattingSize",
        "ExcelConditionalFormatting::__construct",
    ], true);
}
if (!method_exists(ExcelSheet::class, "dataValidationSize")) {
    $documentedVersionGates += array_fill_keys([
        "ExcelSheet::dataValidation",
        "ExcelSheet::dataValidationSize",
        "ExcelTable::isAutoFilter",
        "ExcelTable::removeFilter",
    ], true);
}
foreach (glob(__DIR__ . "/../docs/Excel*.php") as $file) {
    $class = basename($file, ".php");
    $docClass = "Documentation" . $class;
    $source = file_get_contents($file);
    $source = preg_replace(
        "/\\bclass\\s+" . preg_quote($class, "/") . "\\b/",
        "class " . $docClass,
        $source,
        1,
        $replacements,
    );
    if ($replacements !== 1) {
        $errors[] = "$class: class declaration not found";
        continue;
    }
    eval("?>" . $source);

    $runtime = new ReflectionClass($class);
    $documented = new ReflectionClass($docClass);
    $runtimeMethods = [];
    $documentedMethods = [];
    foreach ($runtime->getMethods(ReflectionMethod::IS_PUBLIC) as $method) {
        if ($method->getDeclaringClass()->getName() === $class) {
            $runtimeMethods[$method->getName()] = $method;
        }
    }
    foreach ($documented->getMethods(ReflectionMethod::IS_PUBLIC) as $method) {
        if ($method->getDeclaringClass()->getName() === $docClass) {
            $documentedMethods[$method->getName()] = $method;
        }
    }

    foreach (array_diff_key($runtimeMethods, $documentedMethods) as $name => $_) {
        $errors[] = "$class::$name is undocumented";
    }
    foreach (array_diff_key($documentedMethods, $runtimeMethods) as $name => $_) {
        if (!isset($documentedVersionGates["$class::$name"])) {
            $errors[] = "$class::$name is documented but unavailable";
        }
    }
    foreach (array_intersect_key($runtimeMethods, $documentedMethods) as $name => $method) {
        if (!isset($documentedVersionGates["$class::$name"])
            && methodShape($method) !== methodShape($documentedMethods[$name])) {
            $errors[] = "$class::$name signature mismatch";
        }
    }
}

if ($errors) {
    sort($errors);
    echo implode("\n", $errors), "\n";
} else {
    echo "OK\n";
}
?>
--EXPECT--
OK
