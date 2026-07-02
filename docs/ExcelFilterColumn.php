<?php
/*
  +---------------------------------------------------------------------------+
  | ExcelFilterColumn                                                         |
  |                                                                           |
  | Reference file for NuSphere PHPEd (and possibly other IDE's) for use with |
  | php_excel interface to libxl by Ilia Alshanetsky <ilia@ilia.ws>           |
  |                                                                           |
  | php_excel "PECL" style module (http://github.com/iliaal/php_excel)        |
  | libxl library (http://www.libxl.com)                                      |
  |                                                                           |
  +---------------------------------------------------------------------------+
*/
class ExcelFilterColumn
{
	const FILTER_VALUE = 0;
	const FILTER_TOP10 = 1;
	const FILTER_CUSTOM = 2;
	const FILTER_DYNAMIC = 3;
	const FILTER_COLOR = 4;
	const FILTER_ICON = 5;
	const FILTER_EXT = 6;
	const FILTER_NOT_SET = 7;

	const OPERATOR_EQUAL = 0;
	const OPERATOR_GREATER_THAN = 1;
	const OPERATOR_GREATER_THAN_OR_EQUAL = 2;
	const OPERATOR_LESS_THAN = 3;
	const OPERATOR_LESS_THAN_OR_EQUAL = 4;
	const OPERATOR_NOT_EQUAL = 5;

	/**
	* Create a filter column within an Excel auto filter
	*
	* @see ExcelAutoFilter::column()
	* @param ExcelAutoFilter $autoFilter
	* @param int $colId
	*/
	public function __construct(ExcelAutoFilter $autoFilter, int $colId)
	{
	}

	/**
	* Returns the zero-based index of this AutoFilter column
	*
	* @return int|false
	*/
	public function index(): int|false
	{
	}

	/**
	* Returns the filter type of this AutoFilter column
	*
	* @return int|false One of ExcelFilterColumn::FILTER_* constants
	*/
	public function filterType(): int|false
	{
	}

	/**
	* Returns the number of filter values
	*
	* @return int|false
	*/
	public function filterSize(): int|false
	{
	}

	/**
	* Returns the filter value by index
	*
	* @param int $index
	* @return string|false
	*/
	public function filter(int $index): string|false
	{
	}

	/**
	* Adds the filter value
	*
	* @param string $filterValue
	* @return bool
	*/
	public function addFilter(string $filterValue): bool
	{
	}

	/**
	* Gets the number of top or bottom items
	*
	* @return array|false Array with keys "value"(float), "top"(bool) and "percent"(bool)
	*/
	public function getTop10(): array|false
	{
	}

	/**
	* Sets the number of top or bottom items
	*
	* @param float $value Number of items
	* @param bool $top (optional, default=true) Top items if true, bottom items otherwise
	* @param bool $percent (optional, default=false) Using percent instead of number items
	* @return bool
	*/
	public function setTop10(float $value, bool $top = true, bool $percent = false): bool
	{
	}

	/**
	* Gets the custom filter criteria
	*
	* @return array|false Array with keys "operator_1"(int), "value_1"(string), "operator_2"(int), "value_2"(string) and "andOp"(bool)
	*/
	public function getCustomFilter(): array|false
	{
	}

	/**
	* Sets the custom filter criteria
	*
	* @param int $operator_1 One of ExcelFilterColumn::OPERATOR_* constants
	* @param string $value_1 Value used in the first filter criteria
	* @param int $operator_2 (optional, default=-1) One of ExcelFilterColumn::OPERATOR_* constants
	* @param string|null $value_2 (optional, default=null) Value used in the second filter criteria
	* @param bool $andOp (optional, default=false) True indicates "and", false indicates "or"
	* @return bool
	*/
	public function setCustomFilter(int $operator_1, string $value_1, int $operator_2 = -1, ?string $value_2 = null, bool $andOp = false): bool
	{
	}

	/**
	* Clear the filter criteria
	*
	* @return bool
	*/
	public function clear(): bool
	{
	}

} // end ExcelFilterColumn
