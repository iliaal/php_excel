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
/**
 * The getters below warn and return false for an uninitialized or stale table wrapper.
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
	const TABLESTYLE_MEDIUM1 = 22;
	const TABLESTYLE_MEDIUM2 = 23;
	const TABLESTYLE_MEDIUM3 = 24;
	const TABLESTYLE_DARK1 = 50;
	const TABLESTYLE_DARK2 = 51;
	const TABLESTYLE_DARK3 = 52;

	/**
	* Create a table within an Excel sheet
	*
	* All LibXL table style IDs from 0 through 60 are accepted, including IDs
	* without a registered PHP constant. See setStyle() for the style families.
	* A style outside this range throws an exception.
	*
	* @see ExcelSheet::addTable()
	* @see ExcelTable::setStyle()
	* @param ExcelSheet $sheet
	* @param string $name
	* @param int $rowFirst
	* @param int $rowLast
	* @param int $colFirst
	* @param int $colLast
	* @param bool $hasHeaders (optional)
	* @param int $style (optional) LibXL table style ID from 0 through 60 (default=0)
	*/
	public function __construct(ExcelSheet $sheet, string $name, int $rowFirst, int $rowLast, int $colFirst, int $colLast, bool $hasHeaders = true, int $style = 0)
	{
	}

	/**
	* Returns the table name
	*
	* @return string|null|false
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
	* @return string|null|false
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
	* @return ExcelAutoFilter|false The table autofilter, or false when the wrapper is uninitialized or stale, or libxl yields no handle.
	* @note Probe with isAutoFilter() before chaining: `$t->isAutoFilter() ? $t->autoFilter()->getRef() : null`.
	*/
	public function autoFilter(): mixed
	{
	}

	/**
	* Returns whether the table has an autofilter (only for xlsx files, libxl 5.2.0+)
	*
	* @since libxl 5.2.0
	* @return bool
	*/
	public function isAutoFilter(): bool
	{
	}

	/**
	* Removes the table's autofilter (only for xlsx files, libxl 5.2.0+).
	*
	* On a successful call, invalidates every existing ExcelAutoFilter and
	* ExcelFilterColumn wrapper in this workbook, including those from other
	* sheets and tables, even if this table has no filter to remove. Re-fetch wrappers for surviving filters
	* from their sheet or table before using them again. Sheet and table wrappers
	* remain usable; filters in other workbooks are unaffected.
	*
	* @since libxl 5.2.0
	* @return bool
	*/
	public function removeFilter(): bool
	{
	}

	/**
	* Returns the table style
	*
	* The returned ID need not have a corresponding ExcelTable::TABLESTYLE_*
	* constant. It can be passed directly to setStyle().
	*
	* @see ExcelTable::setStyle()
	* @return int|false LibXL table style ID, or false on failure
	*/
	public function style(): mixed
	{
	}

	/**
	* Sets the table style
	*
	* Accepts the full LibXL table style range: 0 means no style, 1-21 are
	* Light1-Light21, 22-49 are Medium1-Medium28, and 50-60 are Dark1-Dark11.
	* Only some IDs have registered ExcelTable::TABLESTYLE_* constants; use
	* the numeric ID for the others (for example, 30 is Medium9).
	* An out-of-range value emits a warning and returns false without changing
	* the current style.
	*
	* @param int $value LibXL table style ID from 0 through 60
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
	* @return int|false
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
