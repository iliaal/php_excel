<?php
/*
  +---------------------------------------------------------------------------+
  | ExcelSheet                                                                |
  |                                                                           |
  | Reference file for NuSphere PHPEd (and possibly other IDE's) for use with |
  | php_excel interface to libxl by Ilia Alshanetsky <ilia@ilia.ws>           |
  |                                                                           |
  | php_excel "PECL" style module (http://github.com/iliaal/php_excel)        |
  | libxl library (http://www.libxl.com)                                      |
  |                                                                           |
  | Rob Gagnon <rgagnon24@gmail.com>                                          |
  +---------------------------------------------------------------------------+
*/
class ExcelSheet
{
	const PAPER_DEFAULT = 0;
	const PAPER_LETTER = 1;
	const PAPER_LETTERSMALL = 2;
	const PAPER_TABLOID = 3;
	const PAPER_LEDGER = 4;
	const PAPER_LEGAL = 5;
	const PAPER_STATEMENT = 6;
	const PAPER_EXECUTIVE = 7;
	const PAPER_A3 = 8;
	const PAPER_A4 = 9;
	const PAPER_A4SMALL = 10;
	const PAPER_A5 = 11;
	const PAPER_B4 = 12;
	const PAPER_B5 = 13;
	const PAPER_FOLIO = 14;
	const PAPER_QUATRO = 15;
	const PAPER_10x14 = 16;
	const PAPER_10x17 = 17;
	const PAPER_NOTE = 18;
	const PAPER_ENVELOPE_9 = 19;
	const PAPER_ENVELOPE_10 = 20;
	const PAPER_ENVELOPE_11 = 21;
	const PAPER_ENVELOPE_12 = 22;
	const PAPER_ENVELOPE_14 = 23;
	const PAPER_C_SIZE = 24;
	const PAPER_D_SIZE = 25;
	const PAPER_E_SIZE = 26;
	const PAPER_ENVELOPE_DL = 27;
	const PAPER_ENVELOPE_C5 = 28;
	const PAPER_ENVELOPE_C3 = 29;
	const PAPER_ENVELOPE_C4 = 30;
	const PAPER_ENVELOPE_C6 = 31;
	const PAPER_ENVELOPE_C65 = 32;
	const PAPER_ENVELOPE_B4 = 33;
	const PAPER_ENVELOPE_B5 = 34;
	const PAPER_ENVELOPE_B6 = 35;
	const PAPER_ENVELOPE = 36;
	const PAPER_ENVELOPE_MONARCH = 37;
	const PAPER_US_ENVELOPE = 38;
	const PAPER_FANFOLD = 39;
	const PAPER_GERMAN_STD_FANFOLD = 40;
	const PAPER_GERMAN_LEGAL_FANFOLD = 41;

	const CELLTYPE_EMPTY = 0;
	const CELLTYPE_NUMBER = 1;
	const CELLTYPE_STRING = 2;
	const CELLTYPE_BOOLEAN = 3;
	const CELLTYPE_BLANK = 4;
	const CELLTYPE_ERROR = 5;

	const ERRORTYPE_NULL = 0;
	const ERRORTYPE_DIV_0 = 7;
	const ERRORTYPE_VALUE = 15;
	const ERRORTYPE_REF = 23;
	const ERRORTYPE_NAME = 29;
	const ERRORTYPE_NUM = 36;
	const ERRORTYPE_NA = 42;

	const LEFT_TO_RIGHT = 0;
	const RIGHT_TO_LEFT = 1;

	/**
	* Create an ExcelSheet in given Workbook
	*
	* @param ExcelBook $book
	* @param string $name The name for the new worksheet
	* @return ExcelSheet The worksheet created
	*/
	public function __construct(ExcelBook $book, $name)
	{
	} // __construct

	/**
	* Adds the new hyperlink.
	*
	* @param string $hyperlink
	* @param int $row_first 0-based
	* @param int $row_last 0-based
	* @param int $col_first 0-based
	* @param int $col_last 0-based
	* @return void
	*/
	public function addHyperlink($hyperlink, $row_first, $row_last, $col_first, $col_last)
	{
	} // addHyperlink

	/**
	* Insert a picture into a cell with given dimensions
	*
	* @see ExcelBook::addPictureFromString()
	* @see ExcelBook::addPictureFromFile()
	* @see ExcelSheet::addPictureScaled()
	* @param int $row 0-based row number
	* @param int $column 0-based column number
	* @param int $picture_id Value returned by ExcelBook::addPictureFrom*() methods
	* @param int $width
	* @param int $height
	* @param int $x_offset (optional, default=0)
	* @param int $y_offset (optional, default=0)
	* @return void
	*/
	public function addPictureDim($row, $column, $picture_id, $width, $height, $x_offset = 0, $y_offset = 0)
	{
	} // addPictureDim

	/**
	* Insert a picture into a cell with a set scale
	*
	* @see ExcelBook::addPictureFromString()
	* @see ExcelBook::addPictureFromFile()
	* @see ExcelSheet::addPictureDim()
	* @param int $row 0-based row number
	* @param int $column 0-based column number
	* @param int $picture_id Value returned by ExcelBook::addPictureFrom*() methods
	* @param float $scale
	* @param int $x_offset (optional, default = 0)
	* @param int $y_offset (optional, default = 0)
	* @return void
	*/
	public function addPictureScaled($row, $column, $picture_id, $scale, $x_offset = 0, $y_offset = 0)
	{
	} // addPictureScaled

	/**
	* Converts a cell reference to row and column.
	*
	* @param string $cell_reference
	* @return array with keys "row"(int), "column"(int), "col_relative"(bool), "row_relative"(bool)
	*/
	public function addrToRowCol($cell_reference)
	{
	} // addrToRowCol

	/**
	* Get the cell format
	*
	* @param int $row 0-based row number
	* @param int $column 0-based column number
	* @return ExcelFormat
	*/
	public function cellFormat($row, $column)
	{
	} // cellFormat

	/**
	* Get the cell type
	*
	* @param int $row 0-based row number
	* @param int $column 0-based column number
	* @return int One of ExcelSheet:CELLTYPE_* constants
	*/
	public function cellType($row, $column)
	{
	} // cellType

	/**
	* Clear cells in the specified area
	*
	* @param int $row_start 0-based row number
	* @param int $row_end 0-based row number
	* @param int $column_start 0-based column number
	* @param int $column_end 0-based column number
	* @return void
	*/
	public function clear($row_start, $row_end, $column_start, $column_end)
	{
	} // clear

	/**
	* Sets the print area.
	*
	* @param int $row_start 0-based row number
	* @param int $row_end 0-based row number
	* @param int $column_start 0-based column number
	* @param int $column_end 0-based column number
	* @return bool
	*/
	public function setPrintArea($row_start, $row_end, $column_start, $column_end)
	{
	} // setPrintArea

	/**
	* Clears the print area
	*
	* @return bool
	*/
	public function clearPrintArea()
	{
	} // clearPrintArea

	/**
	* Clears repeated rows and columns on each page
	*
	* @see ExcelSheet::setPrintRepeatRows()
	* @see ExcelSheet::setPrintRepeatCols()
	* @return bool
	*/
	public function clearPrintRepeats()
	{
	} // clearPrintRepeats

	/**
	* Returns whether column is hidden.
	*
	* @param int $column 0-based column number
	* @return bool
	*/
	public function colHidden($column)
	{
	} // colHidden

	/**
	* Returns the cell width
	*
	* @see ExcelSheet::rowHeight()
	* @see ExcelSheet::setColWidth()
	* @see ExcelSheet::setRowHeight()
	* @param int $column 0-based column number
	* @return float
	*/
	public function colWidth($column)
	{
	} // colWidth

	/**
	* Copy a cell from one location to another
	*
	* @param int $row_from 0-based row number
	* @param int $column_from 0-based column number
	* @param int $row_to 0-based row number
	* @param int $column_to 0-based column number
	* @return void
	*/
	public function copy($row_from, $column_from, $row_to, $column_to)
	{
	} // copy

	/**
	* Removes hyperlink by index.
	*
	* @param int $index
	* @return bool
	*/
	public function delHyperlink($index)
	{
	} // delHyperlink

	/**
	* Delete a named range
	*
	* @see ExcelSheet::setNamedRange()
	* @param string $name
	* @param int $scope_id
	* @return bool
	*/
	public function delNamedRange($name, $scope_id = null)
	{
	} // delNamedRange

	/**
	* Delete cell merge
	*
	* @param int $row 0-based row number
	* @param int $column 0-based column number
	* @return bool
	*/
	public function deleteMerge($row, $column)
	{
	} // deleteMerge

	/**
	* Removes merged cells by index.
	*
	* @param int $index
	* @return bool
	*/
	public function delMergeByIndex($index)
	{
	} // delMergeByIndex

	/**
	* Returns whether the gridlines are displayed
	*
	* @see ExcelSheet::setDisplayGridlines()
	* @return bool
	*/
	public function displayGridlines()
	{
	} // displayGridlines

	/**
	* Returns the 0-based first column in a sheet that contains a used cell
	*
	* @see ExcelSheet::firstRow()
	* @see ExcelSheet::lastRow()
	* @see ExcelSheet::lastCol()
	* @return int
	*/
	public function firstCol()
	{
	} // firstCol

	/**
	* Returns the 0-based first row in a sheet that contains a used cell
	*
	* @see ExcelSheet::lastRow()
	* @see ExcelSheet::firstCol()
	* @see ExcelSheet::lastCol()
	* @return int
	*/
	public function firstRow()
	{
	} // firstRow

	/**
	* Returns the footer text of the sheet when printed
	*
	* @see ExcelSheet::header()
	* @see ExcelSheet::setFooter()
	* @see ExcelSheet::setHeader()
	* @return string
	*/
	public function footer()
	{
	} // footer

	/**
	* Returns the footer margin (in inches)
	*
	* @see ExcelSheet::setFooter()
	* @see ExcelSheet::headerMargin()
	* @return float
	*/
	public function footerMargin()
	{
	} // footerMargin

	/**
	* Returns whether grouping rows summary is below, or above
	*
	* @see ExcelSheet::setGroupSummaryBelow()
	* @see ExcelSheet::getGroupSummaryRight()
	* @see ExcelSheet::setGroupSummaryRight()
	* @return bool true=below, false=above
	*/
	public function getGroupSummaryBelow()
	{
	} // getGroupSummaryBelow

	/**
	* Returns whether grouping columns summary is right, or left
	*
	* @see ExcelSheet::getGroupSummaryBelow()
	* @see ExcelSheet::setGroupSummaryBelow()
	* @see ExcelSheet::setGroupSummaryRight()
	* @return bool true=right, false=left
	*/
	public function getGroupSummaryRight()
	{
	} // getGroupSummaryRight

	/**
	* Returns column with horizontal page break at position index.
	*
	* @param int $index
	* @return int
	*/
	public function getHorPageBreak($index)
	{
	} // getHorPageBreak

	/**
	* Returns a number of horizontal page breaks in the sheet.
	*
	* @return int
	*/
	public function getHorPageBreakSize()
	{
	} // getHorPageBreakSize

	/**
	* Gets the named range coordinates by index.
	*
	* @param int $index
	* @param int $scope_id (optional, default = null) index of sheet or -1 for Workbook
	* @return array with keys "row_first"(int), "row_last"(int), "col_first"(int), "col_last"(int), "hidden"(bool), "scope"(int)
	*/
	public function getIndexRange($index, $scope_id = null)
	{
	} // getIndexRange

	/**
	* Get cell merge range
	*
	* @param int $row 0-based row number
	* @param int $column 0-based column number
	* @return array Four integers as keys "row_first", "row_last", "col_first", and "col_last"
	*/
	public function getMerge($row, $column)
	{
	} // getMerge

	/**
	* Gets the named range coordinates by name, returns false if range is not found.
	*
	* @param string $name
	* @param int $scope_id (optional, default=null)
	* @return array with keys "row_first"(int), "row_last"(int), "col_first"(int), "col_last"(int), "hidden"(bool)
	*/
	public function getNamedRange($name, $scope_id = null)
	{
	} // getNamedRange

	/**
	* Returns a number of pictures in this worksheet.
	*
	* @return int
	*/
	public function getNumPictures()
	{
	} // getNumPictures

	/**
	* Returns a information about a workbook picture at position index in worksheet.
	*
	* @param int $index
	* @return array with keys "picture_index"(int), "row_top"(int), "col_left"(int), "row_bottom"(int), "col_right"(int), "width"(int), "height"(int), "offset_x"(int), "offset_y"(int)
	*/
	public function getPictureInfo($index)
	{
	} // getPictureInfo

	/**
	* Returns whether fit to page option is enabled, and if so to what width & height
	*
	* @return array with keys "width"(int), "height"(int)
	*/
	public function getPrintFit()
	{
	} // getPrintFit

	/**
	* Returns whether the text is displayed in right-to-left mode: 1 - yes, 0 - no.
	*
	* @return int
	*/
	public function getRightToLeft()
	{
	} // getRightToLeft

	/**
	* Extracts the first visible row and the leftmost visible column of the sheet.
	*
	* @return array with keys "row"(int), "column"(int)
	*/
	public function getTopLeftView()
	{
	} // getTopLeftView

	/**
	* Returns column with vertical page break at position index.
	*
	* @param int $index
	* @return int
	*/
	public function getVerPageBreak($index)
	{
	} // getVerPageBreak

	/**
	* Returns a number of vertical page breaks in the sheet.
	*
	* @return int
	*/
	public function getVerPageBreakSize()
	{
	} // getVerPageBreakSize

	/**
	* Group columns from $column_start to $column_end
	*
	* @param int $column_start 0-based column number
	* @param int $column_end 0-based column number
	* @param bool $collapse (optional, default = false)
	* @return bool
	*/
	public function groupCols($column_start, $column_end, $collapse = false)
	{
	} // groupCols

	/**
	* Group rows from $row_start to $row_end
	*
	* @param int $row_start 0-based row number
	* @param int $row_end 0-based row number
	* @param bool $collapse (optional, default = false)
	* @return bool
	*/
	public function groupRows($row_start, $row_end, $collapse = false)
	{
	} // groupRows

	/**
	* Returns whether the sheet is centered horizontally when printed
	*
	* @see ExcelSheet::vcenter()
	* @see ExcelSheet::setHCenter()
	* @see ExcelSheet::setVCenter()
	* @return bool
	*/
	public function hcenter()
	{
	} // hcenter

	/**
	* Returns the header text of the sheet when printed
	*
	* @see ExcelSheet::setHeader()
	* @see ExcelSheet::footer()
	* @see ExcelSheet::setFooter()
	* @return string
	*/
	public function header()
	{
	} // header

	/**
	* Hides/unhides the sheet
	*
	* @deprecated
	* @param bool $hide
	* @return bool
	*/
	public function hidden($hide)
	{
	} // hidden

	/**
	* Gets the hyperlink and its coordinates by index.
	*
	* @param int $index
	* @return array
	*/
	public function hyperlink($index)
	{
	} // hyperlink

	/**
	* Returns the number of hyperlinks in the sheet.
	*
	* @return int
	*/
	public function hyperlinkSize()
	{
	} // hyperlinkSize

	/**
	* Returns whether sheet is hidden
	*
	* @see ExcelSheet::hidden()
	* @return bool
	*/
	public function isHidden()
	{
	} //isHidden

	/**
	* Returns the header margin (in inches)
	*
	* @see ExcelSheet::footerMargin()
	* @see ExcelSheet::setHeader()
	* @return float
	*/
	public function headerMargin()
	{
	} // headerMargin

	/**
	* Set/Remove horizontal page break
	*
	* @param int $row 0-based row number
	* @param bool $break
	* @return bool
	*/
	public function horPageBreak($row, $break)
	{
	} // horPageBreak

	/**
	* Insert columns from column_start to column_end
	*
	* @param int $column_start 0-based column number
	* @param int $column_end 0-based column number
	* @return bool
	*/
	public function insertCol($column_start, $column_end)
	{
	} // insertCol

	/**
	* Insert rows from row_start to row_end
	*
	* @param int $row_start 0-based row number
	* @param int $row_end 0-based row number
	* @return bool
	*/
	public function insertRow($row_start, $row_end)
	{
	} // insertRow

	/**
	* Determine if a cell contains a date
	*
	* @param int $row 0-based row number
	* @param int $column 0-based column number
	* @return bool
	*/
	public function isDate($row, $column)
	{
	} // isDate

	/**
	* Determine if a cell contains a formula
	*
	* @param int $row 0-based row number
	* @param int $column 0-based column number
	* @return bool
	*/
	public function isFormula($row, $column)
	{
	} // isFormula

	/**
	* Returns the page orientation mode
	*
	* @see ExcelSheet::setLandscape()
	* @return bool true for landscape, false for portrait
	*/
	public function landscape()
	{
	} // landscape

	/**
	* Returns the 0-based last column in a sheet that contains a used cell
	*
	* @see ExcelSheet::firstRow()
	* @see ExcelSheet::lastRow()
	* @see ExcelSheet::firstCol()
	* @return int
	*/
	public function lastCol()
	{
	} // lastCol

	/**
	* Returns the 0-based last row in a sheet that contains a used cell
	*
	* @see ExcelSheet::firstRow()
	* @see ExcelSheet::firstCol()
	* @see ExcelSheet::lastCol()
	* @return int
	*/
	public function lastRow()
	{
	} // lastRow

	/**
	* Returns the bottom margin of the sheet (in inches)
	*
	* @see ExcelSheet::marginTop()
	* @see ExcelSheet::marginRight()
	* @see ExcelSheet::marginLeft()
	* @see ExcelSheet::setMarginBottom()
	* @return float
	*/
	public function marginBottom()
	{
	} // marginBottom

	/**
	* Returns the left margin of the sheet (in inches)
	*
	* @see ExcelSheet::marginTop()
	* @see ExcelSheet::marginRight()
	* @see ExcelSheet::marginBottom()
	* @see ExcelSheet::setMarginLeft()
	* @return float
	*/
	public function marginLeft()
	{
	} // marginLeft

	/**
	* Returns the right margin of the sheet (in inches)
	*
	* @see ExcelSheet::marginTop()
	* @see ExcelSheet::marginLeft()
	* @see ExcelSheet::marginBottom()
	* @see ExcelSheet::setMarginRight()
	* @return float
	*/
	public function marginRight()
	{
	} // marginRight

	/**
	* Returns the top margin of the sheet (in inches)
	*
	* @see ExcelSheet::marginRight()
	* @see ExcelSheet::marginLeft()
	* @see ExcelSheet::marginBottom()
	* @see ExcelSheet::setMarginTop()
	* @return float
	*/
	public function marginTop()
	{
	} // marginTop

	/**
	* Gets the merged cells by index.
	*
	* @param int $index
	* @return array
	*/
	public function merge($index)
	{
	} // merge

	/**
	* Returns a number of merged cells in this worksheet.
	*
	* @return int
	*/
	public function mergeSize()
	{
	} // mergeSize

	/**
	* Returns the name of the worksheet
	*
	* @see ExcelSheet::setName()
	* @return string
	*/
	public function name()
	{
	} // name

	/**
	* Returns the number of named ranges in the sheet.
	*
	* @return int
	*/
	public function namedRangeSize()
	{
	} // namedRangeSize

	/**
	* Returns the paper size
	*
	* @see ExcelSheet::setPaper()
	* @return int One of ExcelSheet::PAPER_* constants
	*/
	public function paper()
	{
	} // paper

	/**
	* Returns whether the gridlines are printed
	*
	* @see ExcelSheet::setPrintGridlines()
	* @return bool
	*/
	public function printGridlines()
	{
	} // printGridlines

	/**
	* Returns whether the row and column headers are printed
	*
	* @see ExcelSheet::setPrintHeaders()
	* @return bool
	*/
	public function printHeaders()
	{
	} // printHeaders

	/**
	* Returns whether the sheet is protected
	*
	* @see ExcelSheet::setProtect()
	* @return bool
	*/
	public function protect()
	{
	} // protect

	/**
	* Read data from a specific cell
	*
	* @param int $row 0-based row number
	* @param int $column 0-based column number
	* @param ExcelFormat $format (optional, default=null)
	* @param bool $read_formula (optional, default=true)
	* @return mixed
	*/
	public function read($row, $column, &$format = null, $read_formula = true)
	{
	} // read

	/**
	* Read an entire column worth of data
	*
	* @param int $column 0-based column number
	* @param int $row_start (optional, default=0)
	* @param int $row_end (optional, default=null)
	* @param bool $read_formula (optional, default=true)
	* @return array or false if invalid row/column positions
	*/
	public function readCol($column, $row_start = 0, $row_end = null, $read_formula = true)
	{
	} // readCol

	/**
	* Read comment from a cell
	*
	* @param int $row 0-based row number
	* @param int $column 0-based column number
	* @return string
	*/
	public function readComment($row, $column)
	{
	} // readComment

	/**
	* Read an entire row worth of data
	*
	* @param int $row 0-based row number
	* @param int $column_start (optional, default=0)
	* @param int $column_end (optional, default=null)
	* @param bool $read_formula (optional, default=true)
	* @return array or false if invalid row/column positions
	*/
	public function readRow($row, $column_start = 0, $column_end = null, $read_formula = true)
	{
	} // readRow

	/**
	* Remove columns from column_start to column_end
	*
	* @param int $column_start 0-based column number
	* @param int $column_end 0-based column number
	* @return bool
	*/
	public function removeCol($column_start, $column_end)
	{
	} // removeCol

	/**
	* Remove rows from row_start to row_end
	*
	* @param int $row_start 0-based row number
	* @param int $row_end 0-based row number
	* @return bool
	*/
	public function removeRow($row_start, $row_end)
	{
	} // removeRow

	/**
	* Converts row and column to a cell reference.
	*
	* @param int $row
	* @param int $column
	* @param bool $row_relative (optional, default=true)
	* @param bool $col_relative (optional, default=true)
	* @return string
	*/
	public function rowColToAddr($row, $column, $row_relative = true, $col_relative = true)
	{
	} // rowColToAddr

	/**
	* Returns the row height
	*
	* @see ExcelSheet::colWidth()
	* @see ExcelSheet::setColWidth()
	* @see ExcelSheet::setRowHeight()
	* @param int $row 0-based row number
	* @return float
	*/
	public function rowHeight($row)
	{
	} // rowHeight

	/**
	* Returns whether row is hidden.
	*
	* @param int $row 0-based row number
	* @return bool
	*/
	public function rowHidden($row)
	{
	} // rowHidden

	/**
	* Set cell format
	*
	* @param int $row 0-based row number
	* @param int $column 0-based column number
	* @param ExcelFormat $format
	* @return void
	*/
	public function setCellFormat($row, $column, $format)
	{
	} // setCellFormat

	/**
	* Hides column.
	*
	* @param int $column 0-based column number
	* @param bool $hidden
	* @return bool
	*/
	public function setColHidden($column, $hidden)
	{
	} // setColHidden

	/**
	* Set the width of cells in a column
	*
	* @see ExcelSheet::colWidth()
	* @see ExcelSheet::rowHeight()
	* @see ExcelSheet::setRowHeight()
	* @param int $column_start 0-based column number
	* @param int $column_end 0-based column number
	* @param float $width (-1: autofit)
	* @param bool $hidden (optional, default=false)
	* @param ExcelFormat $format (optional, default=null)
	* @return bool
	*/
	public function setColWidth($column_start, $column_end, $width, $hidden = false, $format = null)
	{
	} // setColWidth

	/**
	* Sets gridlines for displaying
	*
	* @see ExcelSheet::displayGridlines()
	* @param bool $value
	* @return void
	*/
	public function setDisplayGridlines($value)
	{
	} // setDisplayGridlines

	/**
	* Sets the footer text of the sheet when printed
	*
	* @see ExcelSheet::footer()
	* @see ExcelSheet::header()
	* @see ExcelSheet::setHeader()
	* @param string $footer
	* @param float $margin
	* @return bool
	*/
	public function setFooter($footer, $margin)
	{
	} // setFooter

	/**
	* Sets a flag of grouping rows summary
	*
	* @see ExcelSheet::getGroupSummaryBelow()
	* @see ExcelSheet::getGroupSummaryRight()
	* @see ExcelSheet::setGroupSummaryRight()
	* @param bool $direction true=below, false=above
	* @return bool
	*/
	public function setGroupSummaryBelow($direction)
	{
	} // setGroupSummaryBelow

	/**
	* Sets a flag of grouping columns summary
	*
	* @see ExcelSheet::getGroupSummaryBelow()
	* @see ExcelSheet::setGroupSummaryBelow()
	* @see ExcelSheet::getGroupSummaryRight()
	* @param bool $direction true=right, false=left
	* @return bool
	*/
	public function setGroupSummaryRight($direction)
	{
	} // setGroupSummaryRight

	/**
	* Sets a flag that the shhet is centered horizontally when printed
	*
	* @see ExcelSheet::setVCenter()
	* @param bool $value
	* @return void
	*/
	public function setHCenter($value)
	{
	} // setHCenter

	/**
	* Hides/unhides the sheet.
	*
	* @param bool $value
	* @return bool
	*/
	public function setHidden($value)
	{
	} // setHidden

	/**
	* Set the header text of the sheet when printed
	*
	* @see ExcelSheet::setFooter()
	* @see ExcelSheet::header()
	* @see ExcelSheet::footer()
	* @param string $header
	* @param float $margin
	* @return bool
	*/
	public function setHeader($header, $margin)
	{
	} // setHeader

	/**
	* Sets landscape, or portrait mode for printing
	*
	* @see ExcelSheet::landscape()
	* @param bool $value true for landscape, false for portrait
	* @return void
	*/
	public function setLandscape($value)
	{
	} // setLandscape

	/**
	* Set the bottom margin of the sheet (in inches)
	*
	* @see ExcelSheet::setMargingTop()
	* @see ExcelSheet::setMargingLeft()
	* @see ExcelSheet::setMargingRight()
	* @param float $margin
	* @return void
	*/
	public function setMarginBottom($margin)
	{
	} // setMarginBottom

	/**
	* Set the left margin of the sheet (in inches)
	*
	* @see ExcelSheet::setMargingTop()
	* @see ExcelSheet::setMargingRight()
	* @see ExcelSheet::setMargingBottom()
	* @param float $margin
	* @return void
	*/
	public function setMarginLeft($margin)
	{
	} // setMarginLeft

	/**
	* Set the right margin of the sheet (in inches)
	*
	* @see ExcelSheet::setMargingTop()
	* @see ExcelSheet::setMargingLeft()
	* @see ExcelSheet::setMargingBottom()
	* @param float $margin
	* @return void
	*/
	public function setMarginRight($margin)
	{
	} // setMarginRight

	/**
	* Set the top margin of the sheet (in inches)
	*
	* @see ExcelSheet::setMargingLeft()
	* @see ExcelSheet::setMargingRight()
	* @see ExcelSheet::setMargingBottom()
	* @param float $margin
	* @return void
	*/
	public function setMarginTop($margin)
	{
	} // setMarginTop

	/**
	* Set cell merge range
	*
	* @param int $row_start 0-based row number
	* @param int $row_end 0-based row number
	* @param int $column_start 0-based column number
	* @param int $column_end 0-based column number
	* @return bool
	*/
	public function setMerge($row_start, $row_end, $column_start, $column_end)
	{
	} // setMerge

	/**
	* Sets the name of the worksheet
	*
	* @see ExcelSheet::name()
	* @param string $name
	* @return void
	*/
	public function setName($name)
	{
	} // setName

	/**
	* Create a named range
	*
	* @see ExcelSheet::delNamedRange()
	* @param string $name
	* @param int $row_from 0-based row number
	* @param int $column_from 0-based column number
	* @param int $row_to 0-based row number
	* @param int $column_to 0-based column number
	* @param int $scope_id
	* @return bool
	*/
	public function setNamedRange($name, $row_from, $column_from, $row_to, $column_to, $scope_id = null)
	{
	} // setNamedRange

	/**
	* Sets the paper size
	*
	* @see ExcelSheet::paper()
	* @param int $paper One of ExcelSheet::PAPER_* constants
	* @return void
	*/
	public function setPaper($paper)
	{
	} // setPaper

	/**
	* Fits sheet width and sheet height to wPages and hPages respectively.
	*
	* @param int $wPages
	* @param int $hPages
	* @return bool
	*/
	public function setPrintFit($wPages, $hPages)
	{
	} // setPrintFit

	/**
	* Sets gridlines for printing
	*
	* @see ExcelSheet::printGridlines()
	* @param bool $value
	* @return void
	*/
	public function setPrintGridlines($value)
	{
	} // setPrintGridlines

	/**
	* Sets a flag to indicate row and column headers should be printed
	*
	* @see ExcelSheet::printHeaders()
	* @param bool $value
	* @return void
	*/
	public function setPrintHeaders($value)
	{
	} // setPrintHeaders

	/**
	* Sets repeated columns on each page from column_start to column_end
	*
	* @see ExcelSheet::setPrintRepeatRows()
	* @see ExcelSheet::clearPrintRepeats()
	* @param int $column_start 0-based column number
	* @param int $column_end 0-based column number
	* @return bool
	*/
	public function setPrintRepeatCols($column_start, $column_end)
	{
	} // setPrintRepeatCols

	/**
	* Sets repeated rows on each page from row_start to row_end
	*
	* @see ExcelSheet::setPrintRepeatCols()
	* @see ExcelSheet::clearPrintRepeats()
	* @param int $row_start 0-based row number
	* @param int $row_end 0-based row number
	* @return bool
	*/
	public function setPrintRepeatRows($row_start, $row_end)
	{
	} // setPrintRepeatRows

	/**
	* Protects or unprotects the worksheet
	*
	* @see ExcelSheet::protect()
	* @param bool $value
	* @return void
	*/
	public function setProtect($value)
	{
	} // setProtect

	/**
	* Sets the right-to-left mode:
	* 1 - the text is displayed in right-to-left mode,
	* 0 - the text is displayed in left-to-right mode.
	*
	* @param int $mode
	* @return void
	*/
	public function setRightToLeft($mode)
	{
	} // setRightToLeft

	/**
	* Set the height of cells in a row
	*
	* @see ExcelSheet::rowHeight()
	* @see ExcelSheet::colWidth()
	* @see ExcelSheet::setColWidth()
	* @param int $row 0-based row number
	* @param float $height
	* @param ExcelFormat $format (optional, default=null)
	* @param bool $hidden (optional, default=false)
	* @return bool
	*/
	public function setRowHeight($row, $height, $format = null, $hidden = false)
	{
	} // setRowHeight

	/**
	* Hides row.
	*
	* @param int $row 0-based row number
	* @param bool $hidden
	* @return bool
	*/
	public function setRowHidden($row, $hidden)
	{
	} // setRowHidden

	/**
	* Sets the first visible row and the leftmost visible column of the sheet.
	*
	* @param int $row
	* @param int $column
	* @return bool
	*/
	public function setTopLeftView($row, $column)
	{
	} // setTopLeftView

	/**
	* Sets a flag that the sheet is centered vertically when printed
	*
	* @see ExcelSheet::setHCenter()
	* @param bool $value
	* @return void
	*/
	public function setVCenter($value)
	{
	} // setVCenter

	/**
	* Sets the zoom level of the current view. 100 is the usual view
	*
	* @param int $value
	* @return void
	*/
	public function setZoom($value)
	{
	} // setZoom

	/**
	* Sets the scaling factor for printing (as a percentage)
	*
	* @param int $value
	* @return void
	*/
	public function setZoomPrint($value)
	{
	} // setZoomPrint

	/**
	* Gets the split information (position of frozen pane) in the sheet:
	* row - vertical position of the split;
	* col - horizontal position of the split.
	*
	* @return array
	*/
	public function splitInfo()
	{
	} // splitInfo

	/**
	* Split sheet at indicated position
	*
	* @param int $row 0-based row number
	* @param int $column 0-based column number
	* @return void
	*/
	public function splitSheet($row, $column)
	{
	} // splitSheet

	/**
	* Returns whether the sheet is centered vertically when printed
	*
	* @see ExcelSheet::hcenter()
	* @see ExcelSheet::setVCenter()
	* @see ExcelSheet::setHCenter()
	* @return bool
	*/
	public function vcenter()
	{
	} // vcenter

	/**
	* Set/Remove vertical page break
	*
	* @param int $column 0-based column number
	* @param bool $break
	* @return bool
	*/
	public function verPageBreak($column, $break)
	{
	} // verPageBreak

	/**
	* Write data into a cell
	*
	* @param int $row 0-based row number
	* @param int $column 0-based column number
	* @param mixed $data
	* @param ExcelFormat $format (optional, default=null)
	* @param int $data_type (optional, default=-1) One of ExcelFormat::AS_* constants
	* @return bool
	*/
	public function write($row, $column, $data, $format = null, $data_type = -1)
	{
	} // write

	/**
	* Write an array of values into a column
	*
	* @param int $column 0-based column number
	* @param array $data
	* @param int $row_start (optional, default=0)
	* @param ExcelFormat $format (optional, default=null)
	* @param int $data_type (optional, default=-1) One of ExcelFormat::AS_* constants
	* @return bool
	*/
	public function writeCol($column, $data, $row_start = 0, $format = null, $data_type = -1)
	{
	} // writeCol

	/**
	* Write comment to a cell
	*
	* @param int $row 0-based row number
	* @param int $column 0-based column number
	* @param string $comment
	* @param string $author
	* @param int $width
	* @param int $height
	* @return void
	*/
	public function writeComment($row, $column, $comment, $author, $width, $height)
	{
	} // writeComment

	/**
	* Write an array of values into a row
	*
	* @param int $row 0-based row number
	* @param array $data
	* @param int $column_start (optional, default=0)
	* @param ExcelFormat $format (optional, default=null)
	* @return bool
	*/
	public function writeRow($row, $data, $column_start = 0, $format = null)
	{
	} // writeRow

	/**
	* Returns the zoom level of the current view as a percentage
	*
	* @return int
	*/
	public function zoom()
	{
	} // zoom

	/**
	* Returns the scaling factor for printing as a percentage
	*
	* @return int
	*/
	public function zoomPrint()
	{
	} // zoomPrint

} // end ExcelSheet
