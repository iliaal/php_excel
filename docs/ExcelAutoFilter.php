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
	* Create a AutoFilter within an Excel sheet
	*
	* @see ExcelSheet::autofilter()
	* @param ExcelSheet $sheet
	* @return ExcelAutoFilter
	*/
	public function __construct(ExcelSheet $sheet)
	{
	} // __construct

	/**
	* Gets the cell range of AutoFilter with header.
	*
	* @return array with keys "row_first"(int), "row_last"(int), "col_first"(int), "col_last"(int)
	*/
	public function getRef()
	{
	} // getRef

	/**
	* Sets the cell range of AutoFilter with header.
	*
	* @param int $row_first 0-based (optional, default = 0)
	* @param int $row_last 0-based (optional, default = 0)
	* @param int $col_first 0-based (optional, default = 0)
	* @param int $col_last 0-based (optional, default = 0)
	* @return void
	*/
	public function setRef($row_first = 0, $row_last = 0, $col_first = 0, $col_last = 0)
	{
	} // setRef

	/**
	* Returns the AutoFilter column by zero-based index. Creates it if it doesn't exist.
	*
	* @param int $colId 0-based
	* @return ExcelFilterColumn
	*/
	public function column($colId)
	{
	} // column

	/**
	* Returns the number of specified AutoFilter columns which have a filter information.
	*
	* @return int
	*/
	public function columnSize()
	{
	} // columnSize

	/**
	* Returns the specified AutoFilter column which have a filter information by index.
	*
	* @param int $index 0-based
	* @return ExcelFilterColumn
	*/
	public function columnByIndex($index)
	{
	} // columnByIndex

	/**
	* Gets the whole range of data to sort.
	*
	* @return array with keys "row_first"(int), "col_first"(int), "row_last"(int), "col_last"(int)
	*/
	public function getSortRange()
	{
	} // getSortRange

	/**
	* Gets the zero-based index of sorted column in AutoFilter and its sort order.
	*
	* @return array with keys "column_index"(int), "descending"(bool),
	*/
	public function getSort()
	{
	} // getSort

	/**
	* Sets the sorted column in AutoFilter by zero-based index and its sort order.
	*
	* @param int $columnIndex 0-based
	* @param bool $descending
	* @return bool
	*/
	public function setSort($columnIndex, $descending)
	{
	} // setSort

} // end ExcelAutoFilter
