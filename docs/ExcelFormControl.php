<?php
/*
  +---------------------------------------------------------------------------+
  | ExcelFormControl                                                          |
  |                                                                           |
  | Reference file for NuSphere PHPEd (and possibly other IDE's) for use with |
  | php_excel interface to libxl by Ilia Alshanetsky <ilia@ilia.ws>           |
  |                                                                           |
  | php_excel "PECL" style module (http://github.com/iliaal/php_excel)        |
  | libxl library (http://www.libxl.com)                                      |
  |                                                                           |
  +---------------------------------------------------------------------------+
*/
class ExcelFormControl
{
	const CHECKEDTYPE_UNCHECKED = 0;
	const CHECKEDTYPE_CHECKED = 1;
	const CHECKEDTYPE_MIXED = 2;

	const OBJECT_UNKNOWN = 0;
	const OBJECT_BUTTON = 1;
	const OBJECT_CHECKBOX = 2;
	const OBJECT_DROP = 3;
	const OBJECT_GBOX = 4;
	const OBJECT_LABEL = 5;
	const OBJECT_LIST = 6;
	const OBJECT_RADIO = 7;
	const OBJECT_SCROLL = 8;
	const OBJECT_SPIN = 9;
	const OBJECT_EDITBOX = 10;
	const OBJECT_DIALOG = 11;

	/**
	* Create a form control from a sheet and index
	*
	* @see ExcelSheet::formControl()
	* @param ExcelSheet $sheet
	* @param int $index
	*/
	public function __construct(ExcelSheet $sheet, int $index)
	{
	}

	/**
	* Returns the object type of the form control
	*
	* @return int One of ExcelFormControl::OBJECT_* constants
	*/
	public function objectType(): mixed
	{
	}

	/**
	* Returns the checked state
	*
	* @return int One of ExcelFormControl::CHECKEDTYPE_* constants
	*/
	public function checked(): mixed
	{
	}

	/**
	* Sets the checked state
	*
	* @param int $value One of ExcelFormControl::CHECKEDTYPE_* constants
	* @return bool
	*/
	public function setChecked(int $value): bool
	{
	}

	/**
	* Returns the group box formula
	*
	* @return string|null
	*/
	public function fmlaGroup(): mixed
	{
	}

	/**
	* Sets the group box formula
	*
	* @param string $value
	* @return bool
	*/
	public function setFmlaGroup(string $value): bool
	{
	}

	/**
	* Returns the cell link formula
	*
	* @return string|null
	*/
	public function fmlaLink(): mixed
	{
	}

	/**
	* Sets the cell link formula
	*
	* @param string $value
	* @return bool
	*/
	public function setFmlaLink(string $value): bool
	{
	}

	/**
	* Returns the source range formula
	*
	* @return string|null
	*/
	public function fmlaRange(): mixed
	{
	}

	/**
	* Sets the source range formula
	*
	* @param string $value
	* @return bool
	*/
	public function setFmlaRange(string $value): bool
	{
	}

	/**
	* Returns the text box formula
	*
	* @return string|null
	*/
	public function fmlaTxbx(): mixed
	{
	}

	/**
	* Sets the text box formula
	*
	* @param string $value
	* @return bool
	*/
	public function setFmlaTxbx(string $value): bool
	{
	}

	/**
	* Returns the name of the form control
	*
	* @return string|null
	*/
	public function name(): mixed
	{
	}

	/**
	* Returns the linked cell reference
	*
	* @return string|null
	*/
	public function linkedCell(): mixed
	{
	}

	/**
	* Returns the list fill range reference
	*
	* @return string|null
	*/
	public function listFillRange(): mixed
	{
	}

	/**
	* Returns the macro name
	*
	* @return string|null
	*/
	public function macro(): mixed
	{
	}

	/**
	* Returns the alternative text
	*
	* @return string|null
	*/
	public function altText(): mixed
	{
	}

	/**
	* Returns whether the form control is locked
	*
	* @return bool
	*/
	public function locked(): mixed
	{
	}

	/**
	* Returns whether the form control has default size
	*
	* @return bool
	*/
	public function defaultSize(): mixed
	{
	}

	/**
	* Returns whether the form control is printed
	*
	* @return bool
	*/
	public function print(): mixed
	{
	}

	/**
	* Returns whether the form control is disabled
	*
	* @return bool
	*/
	public function disabled(): mixed
	{
	}

	/**
	* Returns the list item at the specified index
	*
	* @param int $index
	* @return string|null|false
	*/
	public function item(int $index): string|false|null
	{
	}

	/**
	* Returns the number of items in the list
	*
	* @return int
	*/
	public function itemSize(): mixed
	{
	}

	/**
	* Adds an item to the list
	*
	* @param string $value
	* @return bool
	*/
	public function addItem(string $value): bool
	{
	}

	/**
	* Inserts an item at the specified index
	*
	* @param int $index
	* @param string $value
	* @return bool
	*/
	public function insertItem(int $index, string $value): bool
	{
	}

	/**
	* Clears all items from the list
	*
	* @return void
	*/
	public function clearItems(): mixed
	{
	}

	/**
	* Returns the number of drop lines
	*
	* @return int
	*/
	public function dropLines(): mixed
	{
	}

	/**
	* Sets the number of drop lines
	*
	* @param int $value
	* @return bool
	*/
	public function setDropLines(int $value): bool
	{
	}

	/**
	* Returns the scroll bar width
	*
	* @return int
	*/
	public function dx(): mixed
	{
	}

	/**
	* Sets the scroll bar width
	*
	* @param int $value
	* @return bool
	*/
	public function setDx(int $value): bool
	{
	}

	/**
	* Returns whether the first button is selected
	*
	* @return bool
	*/
	public function firstButton(): mixed
	{
	}

	/**
	* Sets whether the first button is selected
	*
	* @param bool $value
	* @return bool
	*/
	public function setFirstButton(bool $value): bool
	{
	}

	/**
	* Returns whether the scroll bar is horizontal
	*
	* @return bool
	*/
	public function horiz(): mixed
	{
	}

	/**
	* Sets whether the scroll bar is horizontal
	*
	* @param bool $value
	* @return bool
	*/
	public function setHoriz(bool $value): bool
	{
	}

	/**
	* Returns the increment value
	*
	* @return int
	*/
	public function inc(): mixed
	{
	}

	/**
	* Sets the increment value
	*
	* @param int $value
	* @return bool
	*/
	public function setInc(int $value): bool
	{
	}

	/**
	* Returns the maximum value
	*
	* @return int
	*/
	public function getMax(): mixed
	{
	}

	/**
	* Sets the maximum value
	*
	* @param int $value
	* @return bool
	*/
	public function setMax(int $value): bool
	{
	}

	/**
	* Returns the minimum value
	*
	* @return int
	*/
	public function getMin(): mixed
	{
	}

	/**
	* Sets the minimum value
	*
	* @param int $value
	* @return bool
	*/
	public function setMin(int $value): bool
	{
	}

	/**
	* Returns the multi-selection mode string
	*
	* @return string|null
	*/
	public function multiSel(): mixed
	{
	}

	/**
	* Sets the multi-selection mode string
	*
	* @param string $value
	* @return bool
	*/
	public function setMultiSel(string $value): bool
	{
	}

	/**
	* Returns the selected index
	*
	* @return int
	*/
	public function sel(): mixed
	{
	}

	/**
	* Sets the selected index
	*
	* @param int $value
	* @return bool
	*/
	public function setSel(int $value): bool
	{
	}

	/**
	* Returns the from-anchor position
	*
	* @return array
	*/
	public function fromAnchor(): mixed
	{
	}

	/**
	* Returns the to-anchor position
	*
	* @return array
	*/
	public function toAnchor(): mixed
	{
	}

} // end ExcelFormControl
