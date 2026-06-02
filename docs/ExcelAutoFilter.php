<?php
/*
  +---------------------------------------------------------------------------+
  | ExcelAutoFilter                                                           |
  |                                                                           |
  | Reference file for NuSphere PHPEd (and possibly other IDE's) for use with |
  | php_excel interface to libxl by Ilia Alshanetsky <ilia@ilia.ws>           |
  |                                                                           |
  | php_excel "PECL" style module (http://github.com/iliaal/php_excel)        |
  | libxl library (http://www.libxl.com)                                      |
  |                                                                           |
  +---------------------------------------------------------------------------+
*/
class ExcelAutoFilter
{
	/**
	* Create an AutoFilter within an Excel sheet
	*
	* @see ExcelSheet::autoFilter()
	* @param ExcelSheet $sheet
	*/
	public function __construct(ExcelSheet $sheet)
	{
	}

	/**
	* Gets the cell range of AutoFilter with header
	*
	* @return array|false Array with keys "row_first"(int), "row_last"(int), "col_first"(int), "col_last"(int)
	*/
	public function getRef(): array|false
	{
	}

	/**
	* Sets the cell range of AutoFilter with header
	*
	* @param int $row_first 0-based
	* @param int $col_first 0-based
	* @param int $row_last 0-based
	* @param int $col_last 0-based
	* @return void
	*/
	public function setRef(int $row_first, int $col_first, int $row_last, int $col_last)
	{
	}

	/**
	* Returns the AutoFilter column by zero-based column ID. Creates it if it doesn't exist.
	*
	* @param int $colId 0-based
	* @return ExcelFilterColumn|false
	*/
	public function column(int $colId): ExcelFilterColumn|false
	{
	}

	/**
	* Returns the number of specified AutoFilter columns which have filter information
	*
	* @return int|false
	*/
	public function columnSize(): int|false
	{
	}

	/**
	* Returns the specified AutoFilter column which has filter information by index
	*
	* @param int $index 0-based
	* @return ExcelFilterColumn|false
	*/
	public function columnByIndex(int $index): ExcelFilterColumn|false
	{
	}

	/**
	* Gets the whole range of data to sort
	*
	* @return array|false Array with keys "row_first"(int), "col_first"(int), "row_last"(int), "col_last"(int)
	*/
	public function getSortRange(): array|false
	{
	}

	/**
	* Gets the zero-based index of sorted column in AutoFilter and its sort order
	*
	* @param int $level Zero-based sort level for multi-level sorts (libxl 5.2.0+; ignored on older libxl)
	* @return array|false Array with keys "column_index"(int), "descending"(bool)
	*/
	public function getSort(int $level = 0): array|false
	{
	}

	/**
	* Sets the sorted column in AutoFilter by zero-based index and its sort order
	*
	* @param int $columnIndex 0-based
	* @param bool $descending
	* @return bool
	*/
	public function setSort(int $columnIndex, bool $descending): bool
	{
	}

	/**
	* Adds a sorted column to the AutoFilter by zero-based index with sort order
	*
	* @param int $columnIndex 0-based
	* @param bool $descending
	* @return bool
	*/
	public function addSort(int $columnIndex, bool $descending): bool
	{
	}

} // end ExcelAutoFilter
