<?php
/*
  +---------------------------------------------------------------------------+
  | ExcelTable                                                                |
  |                                                                           |
  | Reference file for NuSphere PHPEd (and possibly other IDE's) for use with |
  | php_excel interface to libxl by Ilia Alshanetsky <ilia@ilia.ws>           |
  |                                                                           |
  | php_excel "PECL" style module (http://github.com/iliaal/php_excel)        |
  | libxl library (http://www.libxl.com)                                      |
  |                                                                           |
  +---------------------------------------------------------------------------+
*/
class ExcelTable
{
	const TABLESTYLE_NONE = 0;
	const TABLESTYLE_LIGHT1 = 1;
	const TABLESTYLE_LIGHT2 = 2;
	const TABLESTYLE_LIGHT3 = 3;
	const TABLESTYLE_LIGHT4 = 4;
	const TABLESTYLE_LIGHT5 = 5;
	const TABLESTYLE_LIGHT6 = 6;
	const TABLESTYLE_LIGHT7 = 7;
	const TABLESTYLE_LIGHT8 = 8;
	const TABLESTYLE_LIGHT9 = 9;
	const TABLESTYLE_LIGHT10 = 10;
	const TABLESTYLE_MEDIUM1 = 11;
	const TABLESTYLE_MEDIUM2 = 12;
	const TABLESTYLE_MEDIUM3 = 13;
	const TABLESTYLE_DARK1 = 14;
	const TABLESTYLE_DARK2 = 15;
	const TABLESTYLE_DARK3 = 16;

	/**
	* Create a table within an Excel sheet
	*
	* @see ExcelSheet::addTable()
	* @param ExcelSheet $sheet
	* @param string $name
	* @param int $rowFirst
	* @param int $rowLast
	* @param int $colFirst
	* @param int $colLast
	* @param bool $hasHeaders (optional)
	* @param int $style (optional) One of ExcelTable::TABLESTYLE_* constants
	*/
	public function __construct(ExcelSheet $sheet, string $name, int $rowFirst, int $rowLast, int $colFirst, int $colLast, bool $hasHeaders = true, int $style = 0)
	{
	}

	/**
	* Returns the table name
	*
	* @return string|null
	*/
	public function name(): mixed
	{
	}

	/**
	* Sets the table name
	*
	* @param string $value
	* @return bool
	*/
	public function setName(string $value): bool
	{
	}

	/**
	* Returns the table reference range as a string
	*
	* @return string|null
	*/
	public function ref(): mixed
	{
	}

	/**
	* Sets the table reference range
	*
	* @param string $value
	* @return bool
	*/
	public function setRef(string $value): bool
	{
	}

	/**
	* Returns the AutoFilter for this table
	*
	* @return ExcelAutoFilter
	*/
	public function autoFilter(): mixed
	{
	}

	/**
	* Returns whether the table has an autofilter (only for xlsx files, libxl 5.2.0+)
	*
	* @return bool
	*/
	public function isAutoFilter(): bool
	{
	}

	/**
	* Removes the table's autofilter (only for xlsx files, libxl 5.2.0+)
	*
	* @return bool
	*/
	public function removeFilter(): bool
	{
	}

	/**
	* Returns the table style
	*
	* @return int One of ExcelTable::TABLESTYLE_* constants
	*/
	public function style(): mixed
	{
	}

	/**
	* Sets the table style
	*
	* @param int $value One of ExcelTable::TABLESTYLE_* constants
	* @return bool
	*/
	public function setStyle(int $value): bool
	{
	}

	/**
	* Returns whether row stripes are shown
	*
	* @return bool
	*/
	public function showRowStripes(): mixed
	{
	}

	/**
	* Sets whether row stripes are shown
	*
	* @param bool $value
	* @return bool
	*/
	public function setShowRowStripes(bool $value): bool
	{
	}

	/**
	* Returns whether column stripes are shown
	*
	* @return bool
	*/
	public function showColumnStripes(): mixed
	{
	}

	/**
	* Sets whether column stripes are shown
	*
	* @param bool $value
	* @return bool
	*/
	public function setShowColumnStripes(bool $value): bool
	{
	}

	/**
	* Returns whether the first column is highlighted
	*
	* @return bool
	*/
	public function showFirstColumn(): mixed
	{
	}

	/**
	* Sets whether the first column is highlighted
	*
	* @param bool $value
	* @return bool
	*/
	public function setShowFirstColumn(bool $value): bool
	{
	}

	/**
	* Returns whether the last column is highlighted
	*
	* @return bool
	*/
	public function showLastColumn(): mixed
	{
	}

	/**
	* Sets whether the last column is highlighted
	*
	* @param bool $value
	* @return bool
	*/
	public function setShowLastColumn(bool $value): bool
	{
	}

	/**
	* Returns the number of columns in the table
	*
	* @return int
	*/
	public function columnSize(): mixed
	{
	}

	/**
	* Returns the column name at the specified index
	*
	* @param int $index
	* @return string|null|false
	*/
	public function columnName(int $index): string|false|null
	{
	}

	/**
	* Sets the column name at the specified index
	*
	* @param int $index
	* @param string $name
	* @return bool
	*/
	public function setColumnName(int $index, string $name): bool
	{
	}

} // end ExcelTable
