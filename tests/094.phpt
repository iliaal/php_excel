--TEST--
Sheet::addDataValidation()
--SKIPIF--
<?php if (!extension_loaded("excel") || !in_array('addDataValidation', get_class_methods('ExcelSheet'))) print "skip"; ?>
--FILE--
<?php
	$book = new ExcelBook(null, null, true);
	$sheet = new ExcelSheet($book, 'sheet');

	var_dump(
		$sheet->addDataValidation(
			\ExcelSheet::VALIDATION_TYPE_WHOLE,
			\ExcelSheet::VALIDATION_OP_BETWEEN,
			1,
			2,
			1,
			2,
			'1'
		)
	);

	var_dump(
		$sheet->addDataValidation(
			\ExcelSheet::VALIDATION_TYPE_WHOLE,
			\ExcelSheet::VALIDATION_OP_BETWEEN,
			1,
			2,
			1,
			2,
			'1',
			'3'
		)
	);

	var_dump(
		$sheet->addDataValidation(
			\ExcelSheet::VALIDATION_TYPE_WHOLE,
			\ExcelSheet::VALIDATION_OP_BETWEEN,
			1,
			2,
			1,
			2,
			'1',
			'100',
			1,
			0,
			1,
			1,
			'Prompt_Title',
			'Prompt',
			'Error Title',
			'Error'
		)
	);

	var_dump(
		$sheet->addDataValidationDouble(
			\ExcelSheet::VALIDATION_TYPE_WHOLE,
			\ExcelSheet::VALIDATION_OP_BETWEEN,
			1,
			2,
			1,
			2,
			'1'
		)
	);

	var_dump(
		$sheet->addDataValidationDouble(
			\ExcelSheet::VALIDATION_TYPE_WHOLE,
			\ExcelSheet::VALIDATION_OP_BETWEEN,
			1,
			2,
			1,
			2,
			1,
			2,
			1,
			0,
			1,
			3,
			'Prompt_Title',
			'Prompt',
			'Error Title',
			'Error'
		)
	);

	var_dump(
		$sheet->addDataValidation(
			\ExcelSheet::VALIDATION_TYPE_WHOLE,
			\ExcelSheet::VALIDATION_OP_EQUAL,
			1,
			2,
			1,
			2,
			'1'
		)
	);

	var_dump(
		$sheet->removeDataValidations()
	);

	echo "OK\n";

?>
--EXPECTF--
Warning: ExcelSheet::addDataValidation(): The second value can not be null when used with (not) between operator. in %s on line %d
bool(false)
bool(true)
bool(true)

Warning: ExcelSheet::addDataValidationDouble(): The second value can not be null when used with (not) between operator. in %s on line %d
bool(false)
bool(true)
bool(true)
bool(true)
OK