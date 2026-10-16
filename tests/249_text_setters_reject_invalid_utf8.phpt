--TEST--
Text setters reject invalid UTF-8 with a warning before LibXL sees it, and the workbook stays saveable
--EXTENSIONS--
excel
--FILE--
<?php
$bad = "bad \xff\xfe";
$failures = [];
$checked = 0;

function rejects($label, callable $call, $pattern = '/ contains invalid UTF-8$/') {
    global $failures, $checked;
    $warnings = [];
    set_error_handler(function ($errno, $message) use (&$warnings) {
        $warnings[] = $message;
        return true;
    });
    try {
        $result = $call();
    } finally {
        restore_error_handler();
    }
    $checked++;
    if ($result !== false || count($warnings) !== 1 || !preg_match($pattern, $warnings[0])) {
        $failures[] = $label . ': ' . var_export($result, true) . ' ' . json_encode($warnings);
    }
}

function throws($label, callable $call) {
    global $failures, $checked;
    $checked++;
    try {
        $call();
        $failures[] = "$label: no exception";
    } catch (Exception $e) {
        if (!preg_match('/ contains invalid UTF-8$/', $e->getMessage())) {
            $failures[] = "$label: " . $e->getMessage();
        }
    }
}

$hasCfRanges = method_exists('ExcelBook', 'conditionalFormatSize');

foreach ([false, true] as $xlsx) {
    $tag = $xlsx ? 'xlsx' : 'xls';
    $book = new ExcelBook(null, null, $xlsx);
    $sheet = $book->addSheet('Data');
    $sheet->write(1, 1, 'header');

    rejects("$tag addSheet", fn() => $book->addSheet($bad));
    rejects("$tag copySheet", fn() => $book->copySheet($bad, 0));
    rejects("$tag insertSheet", fn() => $book->insertSheet(0, $bad));
    throws("$tag new ExcelSheet", fn() => new ExcelSheet($book, $bad));
    rejects("$tag addCustomFormat", fn() => $book->addCustomFormat($bad));
    rejects("$tag setDefaultFont", fn() => $book->setDefaultFont($bad, 11));
    if (method_exists($book, 'setPassword')) {
        rejects("$tag setPassword", fn() => $book->setPassword($bad));
    }
    rejects("$tag addPictureAsLink", fn() => $book->addPictureAsLink($bad));
    $font = $book->addFont();
    rejects("$tag Font::name", fn() => $font->name($bad));
    $rich = $book->addRichString();
    rejects("$tag RichString::addText", fn() => $rich->addText($bad));

    rejects("$tag setName", fn() => $sheet->setName($bad));
    rejects("$tag write", fn() => $sheet->write(2, 1, $bad), '/: string contains invalid UTF-8$/');
    rejects("$tag write AS_TEXT", fn() => $sheet->write(2, 1, $bad, null, ExcelFormat::AS_TEXT), '/: string contains invalid UTF-8$/');
    rejects("$tag write quoted", fn() => $sheet->write(2, 1, "'" . $bad), '/: string contains invalid UTF-8$/');
    rejects("$tag write formula", fn() => $sheet->write(2, 1, '="' . $bad . '"'), '/: string contains invalid UTF-8$/');
    rejects("$tag write AS_FORMULA", fn() => $sheet->write(2, 1, '"' . $bad . '"', null, ExcelFormat::AS_FORMULA), '/: string contains invalid UTF-8$/');
    rejects("$tag writeRow", fn() => $sheet->writeRow(3, ['kept out', $bad], 1), '/column 2 cannot be written: string contains invalid UTF-8; no cells were modified$/');
    rejects("$tag writeCol", fn() => $sheet->writeCol(1, ['kept out', $bad], 4), '/row 5 cannot be written: string contains invalid UTF-8; no cells were modified$/');
    rejects("$tag writeComment text", fn() => $sheet->writeComment(2, 1, $bad, 'author', 100, 50));
    rejects("$tag writeComment author", fn() => $sheet->writeComment(2, 1, 'text', $bad, 100, 50));
    rejects("$tag setHeader", fn() => $sheet->setHeader($bad, 0.5));
    rejects("$tag setFooter", fn() => $sheet->setFooter($bad, 0.5));
    rejects("$tag setNamedRange", fn() => $sheet->setNamedRange($bad, 1, 2, 1, 2));
    rejects("$tag addHyperlink", fn() => $sheet->addHyperlink($bad, 2, 2, 1, 1));
    rejects("$tag setProtect", fn() => $sheet->setProtect(true, $bad));
    rejects("$tag addDataValidation value", fn() => $sheet->addDataValidation(ExcelSheet::VALIDATION_TYPE_LIST, ExcelSheet::VALIDATION_OP_EQUAL, 2, 2, 1, 1, $bad));
    rejects("$tag addDataValidation second value", fn() => $sheet->addDataValidation(ExcelSheet::VALIDATION_TYPE_TEXTLENGTH, ExcelSheet::VALIDATION_OP_BETWEEN, 2, 2, 1, 1, '1', $bad));
    rejects("$tag addDataValidation prompt title", fn() => $sheet->addDataValidation(ExcelSheet::VALIDATION_TYPE_LIST, ExcelSheet::VALIDATION_OP_EQUAL, 2, 2, 1, 1, '"a,b"', null, true, false, true, true, $bad, 'p', 'et', 'e'));
    rejects("$tag addDataValidation error", fn() => $sheet->addDataValidation(ExcelSheet::VALIDATION_TYPE_LIST, ExcelSheet::VALIDATION_OP_EQUAL, 2, 2, 1, 1, '"a,b"', null, true, false, true, true, 'pt', 'p', 'et', $bad));
    rejects("$tag addDataValidationDouble prompt", fn() => $sheet->addDataValidationDouble(ExcelSheet::VALIDATION_TYPE_DECIMAL, ExcelSheet::VALIDATION_OP_GREATERTHAN, 2, 2, 1, 1, 1.0, null, true, false, true, true, 'pt', $bad, 'et', 'e'));
    rejects("$tag addSelectionRange", fn() => $sheet->addSelectionRange($bad));

    if ($xlsx) {
        $filter = $sheet->autoFilter();
        $filter->setRef(1, 2, 1, 1);
        $column = $filter->column(0);
        rejects("$tag addFilter", fn() => $column->addFilter($bad));
        rejects("$tag setCustomFilter", fn() => $column->setCustomFilter(ExcelFilterColumn::OPERATOR_EQUAL, $bad));
        rejects("$tag setCustomFilter second", fn() => $column->setCustomFilter(ExcelFilterColumn::OPERATOR_EQUAL, 'a', ExcelFilterColumn::OPERATOR_EQUAL, $bad));

        rejects("$tag addTable", fn() => $sheet->addTable($bad, 1, 2, 1, 1));
        throws("$tag new ExcelTable", fn() => new ExcelTable($sheet, $bad, 1, 2, 1, 1));
        $table = $sheet->addTable('Valid', 1, 2, 1, 1);
        rejects("$tag Table::setName", fn() => $table->setName($bad));
        rejects("$tag Table::setRef", fn() => $table->setRef($bad));
        rejects("$tag Table::setColumnName", fn() => $table->setColumnName(0, $bad));

        $props = $book->coreProperties();
        foreach (['setTitle', 'setSubject', 'setCreator', 'setLastModifiedBy', 'setCreated',
                  'setModified', 'setTags', 'setCategories', 'setComments'] as $setter) {
            rejects("$tag CoreProperties::$setter", fn() => $props->$setter($bad));
        }
    }

    if ($xlsx && $hasCfRanges) {
        $cf = $book->addConditionalFormat();
        rejects("$tag ConditionalFormat::setCustomNumFormat", fn() => $cf->setCustomNumFormat($bad));
        $cfing = $sheet->addConditionalFormatting(1, 2, 1, 1);
        rejects("$tag addRule", fn() => $cfing->addRule(ExcelConditionalFormatting::CFORMAT_EXPRESSION, $cf, $bad));
        rejects("$tag addOpStrRule", fn() => $cfing->addOpStrRule(ExcelConditionalFormatting::CFOPERATOR_EQUAL, $cf, $bad, 'x'));
        rejects("$tag addOpStrRule second", fn() => $cfing->addOpStrRule(ExcelConditionalFormatting::CFOPERATOR_BETWEEN, $cf, 'x', $bad));
        rejects("$tag add2ColorScaleFormulaRule", fn() => $cfing->add2ColorScaleFormulaRule(0xFFFFFF, 0xFF0000, ExcelConditionalFormatting::CFVO_FORMULA, $bad, ExcelConditionalFormatting::CFVO_FORMULA, 'A2'));
        rejects("$tag add3ColorScaleFormulaRule", fn() => $cfing->add3ColorScaleFormulaRule(0xFFFFFF, 0xCCCCCC, 0xFF0000, ExcelConditionalFormatting::CFVO_FORMULA, 'A1', ExcelConditionalFormatting::CFVO_FORMULA, $bad, ExcelConditionalFormatting::CFVO_FORMULA, 'A3'));
        /* A conditional formatting range without any rule saves into an XLSX
         * file that LibXL cannot load back. */
        $cfing->addRule(ExcelConditionalFormatting::CFORMAT_EXPRESSION, $cf, 'B2>0');
    }

    $saved = $book->save();
    $loaded = new ExcelBook(null, null, $xlsx);
    if (!is_string($saved) || !$loaded->load($saved)) {
        $failures[] = "$tag: save or reload failed";
        continue;
    }
    $copy = $loaded->getSheet(0);
    if ($loaded->sheetCount() !== 1 || $copy->name() !== 'Data' || $copy->read(1, 1) !== 'header') {
        $failures[] = "$tag: reloaded workbook changed";
    }
    foreach ([[2, 1], [3, 1], [4, 1]] as [$row, $col]) {
        if (!in_array($copy->read($row, $col), [null, ''], true)) {
            $failures[] = "$tag: cell $row,$col was written: " . var_export($copy->read($row, $col), true);
        }
    }
    if ($xlsx && $loaded->coreProperties()->title() !== null && $loaded->coreProperties()->title() !== '') {
        $failures[] = "$tag: title was stored";
    }
}

$controls = new ExcelBook(null, null, true);
if ($controls->loadFile(__DIR__ . '/formcontrols.xlsx') && $controls->getSheet(0)->formControlSize() > 0) {
    $control = $controls->getSheet(0)->formControl(0);
    rejects('addItem', fn() => $control->addItem($bad));
    rejects('insertItem', fn() => $control->insertItem(0, $bad));
    foreach (['setFmlaGroup', 'setFmlaLink', 'setFmlaRange', 'setFmlaTxbx', 'setMultiSel'] as $setter) {
        rejects("FormControl::$setter", fn() => $control->$setter($bad));
    }
}

var_dump($checked >= 80);
echo $failures ? implode("\n", $failures) . "\n" : "no failures\n";
?>
--EXPECT--
bool(true)
no failures
