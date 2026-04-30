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
	const CELLTYPE_STRICTDATE = 6;

	const ERRORTYPE_NULL = 0;
	const ERRORTYPE_DIV_0 = 7;
	const ERRORTYPE_VALUE = 15;
	const ERRORTYPE_REF = 23;
	const ERRORTYPE_NAME = 29;
	const ERRORTYPE_NUM = 36;
	const ERRORTYPE_NA = 42;
	const ERRORTYPE_NOERROR = 255;

	const LEFT_TO_RIGHT = 0;
	const RIGHT_TO_LEFT = 1;

	const IERR_EVAL_ERROR = 1;
	const IERR_EMPTY_CELLREF = 2;
	const IERR_NUMBER_STORED_AS_TEXT = 4;
	const IERR_INCONSIST_RANGE = 8;
	const IERR_INCONSIST_FMLA = 16;
	const IERR_TWODIG_TEXTYEAR = 32;
	const IERR_UNLOCK_FMLA = 64;
	const IERR_DATA_VALIDATION = 128;

	const PROT_DEFAULT = -1;
	const PROT_ALL = 0;
	const PROT_OBJECTS = 1;
	const PROT_SCENARIOS = 2;
	const PROT_FORMAT_CELLS = 4;
	const PROT_FORMAT_COLUMNS = 8;
	const PROT_FORMAT_ROWS = 16;
	const PROT_INSERT_COLUMNS = 32;
	const PROT_INSERT_ROWS = 64;
	const PROT_INSERT_HYPERLINKS = 128;
	const PROT_DELETE_COLUMNS = 256;
	const PROT_DELETE_ROWS = 512;
	const PROT_SEL_LOCKED_CELLS = 1024;
	const PROT_SORT = 2048;
	const PROT_AUTOFILTER = 4096;
	const PROT_PIVOTTABLES = 8192;
	const PROT_SEL_UNLOCKED_CELLS = 16384;

	const SHEETSTATE_VISIBLE = 0;
	const SHEETSTATE_HIDDEN = 1;
	const SHEETSTATE_VERYHIDDEN = 2;

	const VALIDATION_TYPE_NONE = 0;
	const VALIDATION_TYPE_WHOLE = 1;
	const VALIDATION_TYPE_DECIMAL = 2;
	const VALIDATION_TYPE_LIST = 3;
	const VALIDATION_TYPE_DATE = 4;
	const VALIDATION_TYPE_TIME = 5;
	const VALIDATION_TYPE_TEXTLENGTH = 6;
	const VALIDATION_TYPE_CUSTOM = 7;

	const VALIDATION_OP_BETWEEN = 0;
	const VALIDATION_OP_NOTBETWEEN = 1;
	const VALIDATION_OP_EQUAL = 2;
	const VALIDATION_OP_NOTEQUAL = 3;
	const VALIDATION_OP_LESSTHAN = 4;
	const VALIDATION_OP_LESSTHANOREQUAL = 5;
	const VALIDATION_OP_GREATERTHAN = 6;
	const VALIDATION_OP_GREATERTHANOREQUAL = 7;

	const VALIDATION_ERRSTYLE_STOP = 0;
	const VALIDATION_ERRSTYLE_WARNING = 1;
	const VALIDATION_ERRSTYLE_INFORMATION = 2;

	/**
	* Create an ExcelSheet in given Workbook
	*
	* @param ExcelBook $book
	* @param string $name The name for the new worksheet
	*/
	public function __construct(ExcelBook $book, $name)
	{
	}

	/**
	* Get the cell type
	*
	* @param int $row 0-based row number
	* @param int $column 0-based column number
	* @return int|false One of ExcelSheet::CELLTYPE_* constants
	*/
	public function cellType(int $row, int $column): int|false
	{
	}

	/**
	* Get the cell format
	*
	* @param int $row 0-based row number
	* @param int $column 0-based column number
	* @return ExcelFormat|false
	*/
	public function cellFormat(int $row, int $column): ExcelFormat|false
	{
	}

	/**
	* Set cell format
	*
	* @param int $row 0-based row number
	* @param int $column 0-based column number
	* @param ExcelFormat $format
	* @return void
	*/
	public function setCellFormat(int $row, int $column, ExcelFormat $format)
	{
	}

	/**
	* Read data from a specific cell
	*
	* @param int $row 0-based row number
	* @param int $column 0-based column number
	* @param mixed &$format (optional, default=null) ExcelFormat object will be assigned here
	* @param bool $read_formula (optional, default=true)
	* @return mixed
	*/
	public function read(int $row, int $column, &$format = null, bool $read_formula = true)
	{
	}

	/**
	* Read an entire row worth of data
	*
	* @param int $row 0-based row number
	* @param int $start_col (optional, default=0)
	* @param int $end_column (optional, default=-1)
	* @param bool $read_formula (optional, default=true)
	* @return array|false
	*/
	public function readRow(int $row, int $start_col = 0, int $end_column = -1, bool $read_formula = true): array|false
	{
	}

	/**
	* Read an entire column worth of data
	*
	* @param int $column 0-based column number
	* @param int $start_row (optional, default=0)
	* @param int $end_row (optional, default=-1)
	* @param bool $read_formula (optional, default=true)
	* @return array|false
	*/
	public function readCol(int $column, int $start_row = 0, int $end_row = -1, bool $read_formula = true): array|false
	{
	}

	/**
	* Write data into a cell
	*
	* @param int $row 0-based row number
	* @param int $column 0-based column number
	* @param mixed $data
	* @param ExcelFormat|null $format (optional, default=null)
	* @param int $datatype (optional, default=-1) One of ExcelFormat::AS_* constants
	* @return bool
	*/
	public function write(int $row, int $column, mixed $data, ?ExcelFormat $format = null, $datatype = -1): bool
	{
	}

	/**
	* Write an array of values into a row
	*
	* @param int $row 0-based row number
	* @param array $data
	* @param int $start_column (optional, default=0)
	* @param ExcelFormat|null $format (optional, default=null)
	* @return bool
	*/
	public function writeRow(int $row, array $data, int $start_column = 0, ?ExcelFormat $format = null): bool
	{
	}

	/**
	* Write an array of values into a column
	*
	* @param int $row 0-based column number
	* @param array $data
	* @param int $start_row (optional, default=0)
	* @param ExcelFormat|null $format (optional, default=null)
	* @param int $data_type (optional)
	* @return bool
	*/
	public function writeCol(int $row, array $data, int $start_row = 0, ?ExcelFormat $format = null, $data_type = null): bool
	{
	}

	/**
	* Determine if a cell contains a formula
	*
	* @param int $row 0-based row number
	* @param int $column 0-based column number
	* @return bool
	*/
	public function isFormula(int $row, int $column): bool
	{
	}

	/**
	* Determine if a cell contains a date
	*
	* @param int $row 0-based row number
	* @param int $column 0-based column number
	* @return bool
	*/
	public function isDate(int $row, int $column): bool
	{
	}

	/**
	* Insert rows from row_first to row_last
	*
	* @param int $row_first 0-based row number
	* @param int $row_last 0-based row number
	* @param bool $update_named_ranges (optional, default=true)
	* @return bool
	*/
	public function insertRow($row_first, $row_last, $update_named_ranges = true): bool
	{
	}

	/**
	* Insert columns from col_first to col_last
	*
	* @param int $col_first 0-based column number
	* @param int $col_last 0-based column number
	* @param bool $update_named_ranges (optional, default=true)
	* @return bool
	*/
	public function insertCol(int $col_first, $col_last, $update_named_ranges = true): bool
	{
	}

	/**
	* Remove rows from row_first to row_last
	*
	* @param int $row_first 0-based row number
	* @param int $row_last 0-based row number
	* @param bool $update_named_ranges (optional, default=true)
	* @return bool
	*/
	public function removeRow(int $row_first, $row_last, $update_named_ranges = true): bool
	{
	}

	/**
	* Remove columns from col_first to col_last
	*
	* @param int $col_first 0-based column number
	* @param int $col_last 0-based column number
	* @param bool $update_named_ranges (optional, default=true)
	* @return bool
	*/
	public function removeCol(int $col_first, $col_last, $update_named_ranges = true): bool
	{
	}

	/**
	* Returns the cell width
	*
	* @param int $column 0-based column number
	* @return float|false
	*/
	public function colWidth(int $column): float|false
	{
	}

	/**
	* Returns the row height
	*
	* @param int $row 0-based row number
	* @return float|false
	*/
	public function rowHeight(int $row): float|false
	{
	}

	/**
	* Read comment from a cell
	*
	* @param int $row 0-based row number
	* @param int $column 0-based column number
	* @return string|false
	*/
	public function readComment(int $row, int $column): string|false
	{
	}

	/**
	* Write comment to a cell
	*
	* @param int $row 0-based row number
	* @param int $column 0-based column number
	* @param string $value
	* @param string $author
	* @param int $width
	* @param int $height
	* @return void
	*/
	public function writeComment(int $row, int $column, string $value, string $author, int $width, int $height)
	{
	}

	/**
	* Set the width of cells in a column
	*
	* @param int $column_start 0-based column number
	* @param int $column_end 0-based column number
	* @param float $width (-1: autofit)
	* @param bool $hidden (optional, default=false)
	* @param ExcelFormat|null $format (optional, default=null)
	* @return bool
	*/
	public function setColWidth(int $column_start, int $column_end, float $width, bool $hidden = false, ?ExcelFormat $format = null): bool
	{
	}

	/**
	* Set the height of cells in a row
	*
	* @param int $row 0-based row number
	* @param float $height
	* @param ExcelFormat|null $format (optional, default=null)
	* @param mixed $hidden (optional, default=false)
	* @return bool
	*/
	public function setRowHeight(int $row, float $height, ?ExcelFormat $format = null, $hidden = false): bool
	{
	}

	/**
	* Get cell merge range
	*
	* @param int $row 0-based row number
	* @param int $column 0-based column number
	* @return array|false Array with keys "row_first", "row_last", "col_first", "col_last"
	*/
	public function getMerge(int $row, int $column): array|false
	{
	}

	/**
	* Set cell merge range
	*
	* @param int $row_start 0-based row number
	* @param int $row_end 0-based row number
	* @param int $col_start 0-based column number
	* @param int $col_end 0-based column number
	* @return bool
	*/
	public function setMerge(int $row_start, int $row_end, int $col_start, int $col_end): bool
	{
	}

	/**
	* Delete cell merge
	*
	* @param int $row 0-based row number
	* @param int $column 0-based column number
	* @return bool
	*/
	public function deleteMerge(int $row, int $column): bool
	{
	}

	/**
	* Insert a picture into a cell with a set scale
	*
	* @param int $row 0-based row number
	* @param int $column 0-based column number
	* @param int $pic_id Value returned by ExcelBook::addPictureFrom*() methods
	* @param float $scale
	* @param int $x_offset (optional, default=0)
	* @param int $y_offset (optional, default=0)
	* @param int $pos (optional) One of ExcelBook::POSITION_* constants
	* @return void
	*/
	public function addPictureScaled(int $row, int $column, int $pic_id, float $scale, int $x_offset = 0, int $y_offset = 0, int $pos = null)
	{
	}

	/**
	* Insert a picture into a cell with given dimensions
	*
	* @param int $row 0-based row number
	* @param int $column 0-based column number
	* @param int $pic_id Value returned by ExcelBook::addPictureFrom*() methods
	* @param int $width
	* @param int $height
	* @param int $x_offset (optional, default=0)
	* @param int $y_offset (optional, default=0)
	* @param int $pos (optional) One of ExcelBook::POSITION_* constants
	* @return void
	*/
	public function addPictureDim(int $row, int $column, int $pic_id, int $width, int $height, int $x_offset = 0, int $y_offset = 0, int $pos = null)
	{
	}

	/**
	* Set/Remove horizontal page break
	*
	* @param int $row 0-based row number
	* @param int $break
	* @return bool
	*/
	public function horPageBreak(int $row, int $break): bool
	{
	}

	/**
	* Set/Remove vertical page break
	*
	* @param int $col 0-based column number
	* @param int $break
	* @return bool
	*/
	public function verPageBreak(int $col, int $break): bool
	{
	}

	/**
	* Split sheet at indicated position
	*
	* @param int $row 0-based row number
	* @param int $column 0-based column number
	* @return void
	*/
	public function splitSheet(int $row, int $column)
	{
	}

	/**
	* Group rows from start_row to end_row
	*
	* @param int $start_row 0-based row number
	* @param int $end_row 0-based row number
	* @param int $collapse (optional, default=false)
	* @return bool
	*/
	public function groupRows(int $start_row, int $end_row, int $collapse = 0): bool
	{
	}

	/**
	* Group columns from start_column to end_column
	*
	* @param int $start_column 0-based column number
	* @param int $end_column 0-based column number
	* @param int $collapse (optional, default=false)
	* @return bool
	*/
	public function groupCols(int $start_column, int $end_column, int $collapse = 0): bool
	{
	}

	/**
	* Clear cells in the specified area
	*
	* @param int $row_s 0-based row number
	* @param int $row_e 0-based row number
	* @param int $col_s 0-based column number
	* @param int $col_e 0-based column number
	* @return void
	*/
	public function clear(int $row_s, int $row_e, int $col_s, int $col_e)
	{
	}

	/**
	* Copy a cell from one location to another
	*
	* @param int $row 0-based row number
	* @param int $col 0-based column number
	* @param int $to_row 0-based row number
	* @param int $to_col 0-based column number
	* @return bool
	*/
	public function copy(int $row, int $col, int $to_row, int $to_col): bool
	{
	}

	/**
	* Returns the 0-based first row in a sheet that contains a used cell
	*
	* @return int|false
	*/
	public function firstRow(): int|false
	{
	}

	/**
	* Returns the 0-based last row in a sheet that contains a used cell
	*
	* @return int|false
	*/
	public function lastRow(): int|false
	{
	}

	/**
	* Returns the 0-based first column in a sheet that contains a used cell
	*
	* @return int|false
	*/
	public function firstCol(): int|false
	{
	}

	/**
	* Returns the 0-based last column in a sheet that contains a used cell
	*
	* @return int|false
	*/
	public function lastCol(): int|false
	{
	}

	/**
	* Returns whether the gridlines are displayed
	*
	* @return bool
	*/
	public function displayGridlines(): bool
	{
	}

	/**
	* Returns whether the gridlines are printed
	*
	* @return bool
	*/
	public function printGridlines(): bool
	{
	}

	/**
	* Sets gridlines for displaying
	*
	* @param bool $value
	* @return void
	*/
	public function setDisplayGridlines(bool $value)
	{
	}

	/**
	* Sets gridlines for printing
	*
	* @param mixed $value
	* @return void
	*/
	public function setPrintGridlines($value)
	{
	}

	/**
	* Returns the zoom level of the current view as a percentage
	*
	* @return int|false
	*/
	public function zoom(): int|false
	{
	}

	/**
	* Returns the scaling factor for printing as a percentage
	*
	* @return int|false
	*/
	public function zoomPrint(): int|false
	{
	}

	/**
	* Sets the zoom level of the current view
	*
	* @param mixed $value
	* @return void
	*/
	public function setZoom($value)
	{
	}

	/**
	* Sets the scaling factor for printing (as a percentage)
	*
	* @param mixed $value
	* @return void
	*/
	public function setZoomPrint($value)
	{
	}

	/**
	* Sets landscape, or portrait mode for printing
	*
	* @param mixed $value true for landscape, false for portrait
	* @return void
	*/
	public function setLandscape($value)
	{
	}

	/**
	* Returns the page orientation mode
	*
	* @return bool true for landscape, false for portrait
	*/
	public function landscape(): bool
	{
	}

	/**
	* Returns the paper size
	*
	* @return int|false One of ExcelSheet::PAPER_* constants
	*/
	public function paper(): int|false
	{
	}

	/**
	* Sets the paper size
	*
	* @param string $value One of ExcelSheet::PAPER_* constants
	* @return void
	*/
	public function setPaper(string $value)
	{
	}

	/**
	* Returns the header text of the sheet when printed
	*
	* @return string|null|false
	*/
	public function header(): string|null|false
	{
	}

	/**
	* Returns the footer text of the sheet when printed
	*
	* @return string|null|false
	*/
	public function footer(): string|null|false
	{
	}

	/**
	* Set the header text of the sheet when printed
	*
	* @param mixed $header
	* @param mixed $margin
	* @return bool
	*/
	public function setHeader($header, $margin): bool
	{
	}

	/**
	* Sets the footer text of the sheet when printed
	*
	* @param mixed $footer
	* @param mixed $margin
	* @return bool
	*/
	public function setFooter($footer, $margin): bool
	{
	}

	/**
	* Returns the header margin (in inches)
	*
	* @return float|false
	*/
	public function headerMargin(): float|false
	{
	}

	/**
	* Returns the footer margin (in inches)
	*
	* @return float|false
	*/
	public function footerMargin(): float|false
	{
	}

	/**
	* Returns whether the sheet is centered horizontally when printed
	*
	* @return bool
	*/
	public function hcenter(): bool
	{
	}

	/**
	* Returns whether the sheet is centered vertically when printed
	*
	* @return bool
	*/
	public function vcenter(): bool
	{
	}

	/**
	* Sets a flag that the sheet is centered horizontally when printed
	*
	* @param mixed $value
	* @return void
	*/
	public function setHCenter($value)
	{
	}

	/**
	* Sets a flag that the sheet is centered vertically when printed
	*
	* @param mixed $value
	* @return void
	*/
	public function setVCenter($value)
	{
	}

	/**
	* Returns the left margin of the sheet (in inches)
	*
	* @return float|false
	*/
	public function marginLeft(): float|false
	{
	}

	/**
	* Returns the right margin of the sheet (in inches)
	*
	* @return float|false
	*/
	public function marginRight(): float|false
	{
	}

	/**
	* Returns the top margin of the sheet (in inches)
	*
	* @return float|false
	*/
	public function marginTop(): float|false
	{
	}

	/**
	* Returns the bottom margin of the sheet (in inches)
	*
	* @return float|false
	*/
	public function marginBottom(): float|false
	{
	}

	/**
	* Set the left margin of the sheet (in inches)
	*
	* @param mixed $value
	* @return void
	*/
	public function setMarginLeft($value)
	{
	}

	/**
	* Set the right margin of the sheet (in inches)
	*
	* @param mixed $value
	* @return void
	*/
	public function setMarginRight($value)
	{
	}

	/**
	* Set the top margin of the sheet (in inches)
	*
	* @param mixed $value
	* @return void
	*/
	public function setMarginTop($value)
	{
	}

	/**
	* Set the bottom margin of the sheet (in inches)
	*
	* @param mixed $value
	* @return void
	*/
	public function setMarginBottom($value)
	{
	}

	/**
	* Returns whether the row and column headers are printed
	*
	* @return bool
	*/
	public function printHeaders(): bool
	{
	}

	/**
	* Sets a flag to indicate row and column headers should be printed
	*
	* @param string $value
	* @return void
	*/
	public function setPrintHeaders(string $value)
	{
	}

	/**
	* Returns the name of the worksheet
	*
	* @return string|null|false
	*/
	public function name(): string|null|false
	{
	}

	/**
	* Sets the name of the worksheet
	*
	* @param string $name
	* @return void
	*/
	public function setName(string $name)
	{
	}

	/**
	* Returns whether the sheet is protected
	*
	* @return bool
	*/
	public function protect(): bool
	{
	}

	/**
	* Protects or unprotects the worksheet
	*
	* @param bool $value
	* @param string $password (optional, default="")
	* @param int $enhancedProtection (optional, default=ExcelSheet::PROT_DEFAULT)
	* @return bool
	*/
	public function setProtect(bool $value, string $password = '', int $enhancedProtection = -1): bool
	{
	}

	/**
	* Create a named range
	*
	* @param string $name
	* @param int $row 0-based row number
	* @param int $col 0-based column number
	* @param int $to_row 0-based row number
	* @param int $to_col 0-based column number
	* @param int $scope_id (optional)
	* @return bool
	*/
	public function setNamedRange(string $name, int $row, int $col, int $to_row, int $to_col, int $scope_id = null): bool
	{
	}

	/**
	* Delete a named range
	*
	* @param string $name
	* @param int $scope_id (optional)
	* @return bool
	*/
	public function delNamedRange(string $name, int $scope_id = null): bool
	{
	}

	/**
	* Sets repeated rows on each page from row_start to row_end
	*
	* @param mixed $row_start 0-based row number
	* @param mixed $row_end 0-based row number
	* @return bool
	*/
	public function setPrintRepeatRows($row_start, $row_end): bool
	{
	}

	/**
	* Sets repeated columns on each page from col_start to col_end
	*
	* @param bool $col_start 0-based column number
	* @param mixed $col_end 0-based column number
	* @return bool
	*/
	public function setPrintRepeatCols($col_start, $col_end): bool
	{
	}

	/**
	* Sets the print area
	*
	* @param int $row_first 0-based row number
	* @param int $row_last 0-based row number
	* @param int $col_first 0-based column number
	* @param int $col_last 0-based column number
	* @return bool
	*/
	public function setPrintArea(int $row_first, int $row_last, int $col_first, int $col_last): bool
	{
	}

	/**
	* Clears repeated rows and columns on each page
	*
	* @return bool
	*/
	public function clearPrintRepeats(): bool
	{
	}

	/**
	* Clears the print area
	*
	* @return bool
	*/
	public function clearPrintArea(): bool
	{
	}

	/**
	* Returns whether grouping columns summary is right, or left
	*
	* @return bool true=right, false=left
	*/
	public function getGroupSummaryRight(): bool
	{
	}

	/**
	* Sets a flag of grouping rows summary
	*
	* @param bool $direction true=below, false=above
	* @return bool
	*/
	public function setGroupSummaryBelow(bool $direction): bool
	{
	}

	/**
	* Returns whether grouping rows summary is below, or above
	*
	* @return bool true=below, false=above
	*/
	public function getGroupSummaryBelow(): bool
	{
	}

	/**
	* Sets a flag of grouping columns summary
	*
	* @param bool $direction true=right, false=left
	* @return bool
	*/
	public function setGroupSummaryRight(bool $direction): bool
	{
	}

	/**
	* Fits sheet width and sheet height to wPages and hPages respectively
	*
	* @param int $wPages
	* @param int $hPages
	* @return bool
	*/
	public function setPrintFit(int $wPages, int $hPages): bool
	{
	}

	/**
	* Returns whether fit to page option is enabled, and if so to what width and height
	*
	* @return array|false Array with keys "width"(int), "height"(int)
	*/
	public function getPrintFit(): array|false
	{
	}

	/**
	* Gets the named range coordinates by name
	*
	* @param string $name
	* @param int $scope_id (optional)
	* @return array|false Array with keys "row_first"(int), "row_last"(int), "col_first"(int), "col_last"(int), "hidden"(bool)
	*/
	public function getNamedRange(string $name, int $scope_id = null): array|false
	{
	}

	/**
	* Gets the named range coordinates by index
	*
	* @param int $index
	* @return array|false Array with keys "row_first"(int), "row_last"(int), "col_first"(int), "col_last"(int), "hidden"(bool), "scope"(int)
	*/
	public function getIndexRange(int $index): array|false
	{
	}

	/**
	* Returns the number of named ranges in the sheet
	*
	* @return int|false
	*/
	public function namedRangeSize(): int|false
	{
	}

	/**
	* Returns a number of vertical page breaks in the sheet
	*
	* @return int|false
	*/
	public function getVerPageBreakSize(): int|false
	{
	}

	/**
	* Returns column with vertical page break at position index
	*
	* @param int $index
	* @return int|false
	*/
	public function getVerPageBreak(int $index): int|false
	{
	}

	/**
	* Returns column with horizontal page break at position index
	*
	* @param int $index
	* @return int|false
	*/
	public function getHorPageBreak(int $index): int|false
	{
	}

	/**
	* Returns a number of horizontal page breaks in the sheet
	*
	* @return int|false
	*/
	public function getHorPageBreakSize(): int|false
	{
	}

	/**
	* Returns a number of pictures in this worksheet
	*
	* @return int|false
	*/
	public function getNumPictures(): int|false
	{
	}

	/**
	* Returns information about a workbook picture at position index
	*
	* @param int $index
	* @return array|false
	*/
	public function getPictureInfo(int $index): array|false
	{
	}

	/**
	* Hides/unhides the sheet
	*
	* @param bool $value
	* @return bool
	*/
	public function setHidden(bool $value): bool
	{
	}

	/**
	* Returns whether sheet is hidden
	*
	* @return bool
	*/
	public function isHidden(): bool
	{
	}

	/**
	* Sets the first visible row and the leftmost visible column of the sheet
	*
	* @param int $row
	* @param int $column
	* @return bool
	*/
	public function setTopLeftView(int $row, int $column): bool
	{
	}

	/**
	* Extracts the first visible row and the leftmost visible column of the sheet
	*
	* @return array|false Array with keys "row"(int), "column"(int)
	*/
	public function getTopLeftView(): array|false
	{
	}

	/**
	* Converts row and column to a cell reference
	*
	* @param int $row
	* @param int $column
	* @param bool $row_relative (optional, default=true)
	* @param bool $col_relative (optional, default=true)
	* @return string|false
	*/
	public function rowColToAddr(int $row, int $column, bool $row_relative = true, bool $col_relative = true): string|false
	{
	}

	/**
	* Converts a cell reference to row and column
	*
	* @param string $cell_reference
	* @return array|false Array with keys "row"(int), "column"(int), "col_relative"(bool), "row_relative"(bool)
	*/
	public function addrToRowCol(string $cell_reference): array|false
	{
	}

	/**
	* Returns whether the text is displayed in right-to-left mode
	*
	* @return int|false 1 = yes, 0 = no
	*/
	public function getRightToLeft(): int|false
	{
	}

	/**
	* Sets the right-to-left mode
	*
	* @param int $mode
	* @return void
	*/
	public function setRightToLeft(int $mode)
	{
	}

	/**
	* Returns the number of hyperlinks in the sheet
	*
	* @return int|false
	*/
	public function hyperlinkSize(): int|false
	{
	}

	/**
	* Gets the hyperlink and its coordinates by index
	*
	* @param int $index
	* @return array|false
	*/
	public function hyperlink(int $index): array|false
	{
	}

	/**
	* Removes hyperlink by index
	*
	* @param int $index
	* @return bool
	*/
	public function delHyperlink(int $index): bool
	{
	}

	/**
	* Adds the new hyperlink
	*
	* @param string $hyperlink
	* @param int $row_first 0-based
	* @param int $row_last 0-based
	* @param int $col_first 0-based
	* @param int $col_last 0-based
	* @return bool
	*/
	public function addHyperlink(string $hyperlink, int $row_first, int $row_last, int $col_first, int $col_last): bool
	{
	}

	/**
	* Returns a number of merged cells in this worksheet
	*
	* @return int|false
	*/
	public function mergeSize(): int|false
	{
	}

	/**
	* Gets the merged cells by index
	*
	* @param int $index
	* @return array|false
	*/
	public function merge(int $index): array|false
	{
	}

	/**
	* Removes merged cells by index
	*
	* @param int $index
	* @return bool
	*/
	public function delMergeByIndex(int $index): bool
	{
	}

	/**
	* Gets the split information (position of frozen pane)
	*
	* @return array|false
	*/
	public function splitInfo(): array|false
	{
	}

	/**
	* Returns whether column is hidden
	*
	* @param int $col 0-based column number
	* @return bool
	*/
	public function colHidden(int $col): bool
	{
	}

	/**
	* Returns whether row is hidden
	*
	* @param int $row 0-based row number
	* @return bool
	*/
	public function rowHidden(int $row): bool
	{
	}

	/**
	* Hides column
	*
	* @param int $col 0-based column number
	* @param bool $hidden
	* @return bool
	*/
	public function setColHidden(int $col, bool $hidden): bool
	{
	}

	/**
	* Hides row
	*
	* @param int $row 0-based row number
	* @param bool $hidden
	* @return bool
	*/
	public function setRowHidden(int $row, bool $hidden): bool
	{
	}

	/**
	* Returns whether LibXL runs in trial or licensed mode
	*
	* @return bool
	*/
	public function isLicensed(): bool
	{
	}

	/**
	* Sets the borders for autofit column widths feature
	*
	* @param int $row_start (optional, default=0)
	* @param int $row_end (optional, default=-1)
	* @param int $col_start (optional, default=0)
	* @param int $col_end (optional, default=-1)
	* @return bool
	*/
	public function setAutoFitArea(int $row_start = 0, int $row_end = -1, int $col_start = 0, int $col_end = -1): bool
	{
	}

	/**
	* Gets repeated rows on each page. Returns false if repeated rows aren't found.
	*
	* @return array|false Array with keys "row_start"(int) and "row_end"(int)
	*/
	public function printRepeatRows(): array|false
	{
	}

	/**
	* Gets repeated columns on each page. Returns false if repeated columns aren't found.
	*
	* @return array|false Array with keys "col_start"(int) and "col_end"(int)
	*/
	public function printRepeatCols(): array|false
	{
	}

	/**
	* Gets the print area. Returns false if print area isn't found.
	*
	* @return array|false Array with keys "row_start"(int), "row_end"(int), "col_start"(int) and "col_end"(int)
	*/
	public function printArea(): array|false
	{
	}

	/**
	* Sets the color for the sheet's tab
	*
	* @param int $color One of the ExcelFormat::COLOR_* constants
	* @return bool
	*/
	public function setTabColor(int $color = 0): bool
	{
	}

	/**
	* Applies the AutoFilter to the sheet
	*
	* @return bool
	*/
	public function applyFilter(): bool
	{
	}

	/**
	* Returns the AutoFilter. Creates it if it doesn't exist.
	*
	* @return ExcelAutoFilter|false
	*/
	public function autoFilter(): ExcelAutoFilter|false
	{
	}

	/**
	* Removes the AutoFilter from the sheet
	*
	* @return bool
	*/
	public function removeFilter(): bool
	{
	}

	/**
	* Gets the table parameters by index
	*
	* @param int $index (optional, default=0)
	* @return array|false Array with keys "name", "row_first", "col_first", "row_last", "col_last", "header_row_count", "totals_row_count"
	*/
	public function table(int $index = 0): array|false
	{
	}

	/**
	* Writes error into the cell with specified format
	*
	* @param int $row (optional, default=0)
	* @param int $col (optional, default=0)
	* @param int $iError (optional, default=0) One of ExcelSheet::ERRORTYPE_* constants
	* @param ExcelFormat|null $format (optional, default=null)
	* @return void
	*/
	public function writeError(int $row, int $col, int $iError, ?ExcelFormat $format = null)
	{
	}

	/**
	* Adds the ignored error for specified range
	*
	* @param int $iError One of the ExcelSheet::IERR_* constants
	* @param int $rowFirst (optional, default=0)
	* @param int $colFirst (optional, default=0)
	* @param int $rowLast (optional, default=0)
	* @param int $colLast (optional, default=0)
	* @return bool
	*/
	public function addIgnoredError(int $iError, int $rowFirst = 0, int $colFirst = 0, int $rowLast = 0, int $colLast = 0): bool
	{
	}

	/**
	* Adds a data validation for the specified range (only for xlsx files)
	*
	* @param int $type One of the ExcelSheet::VALIDATION_TYPE_* constants
	* @param int $op One of the ExcelSheet::VALIDATION_OP_* constants
	* @param int $row_first 0-based
	* @param int $row_last 0-based
	* @param int $col_first 0-based
	* @param int $col_last 0-based
	* @param string $val_1 First value for relational operator
	* @param string $val_2 (optional) Second value for BETWEEN/NOTBETWEEN
	* @param bool $allow_blank (optional, default=true)
	* @param bool $hide_dropdown (optional, default=false)
	* @param bool $show_inputmessage (optional, default=true)
	* @param bool $show_errormessage (optional, default=true)
	* @param string $prompt_title (optional, default='')
	* @param string $prompt (optional, default='')
	* @param string $error_title (optional, default='')
	* @param string $error (optional, default='')
	* @param int $error_style (optional, default=VALIDATION_ERRSTYLE_WARNING)
	* @return bool
	*/
	public function addDataValidation(int $type, int $op, int $row_first, int $row_last, int $col_first, int $col_last, string $val_1, string $val_2 = null, bool $allow_blank = true, bool $hide_dropdown = false, bool $show_inputmessage = true, bool $show_errormessage = true, string $prompt_title = '', string $prompt = '', string $error_title = '', string $error = '', int $error_style = 1): bool
	{
	}

	/**
	* Adds a data validation with double or date values for the specified range (only for xlsx files)
	*
	* @param int $type One of the ExcelSheet::VALIDATION_TYPE_* constants
	* @param int $op One of the ExcelSheet::VALIDATION_OP_* constants
	* @param int $row_first 0-based
	* @param int $row_last 0-based
	* @param int $col_first 0-based
	* @param int $col_last 0-based
	* @param float $val_1 First value for relational operator
	* @param float $val_2 (optional) Second value for BETWEEN/NOTBETWEEN
	* @param bool $allow_blank (optional, default=true)
	* @param bool $hide_dropdown (optional, default=false)
	* @param bool $show_inputmessage (optional, default=true)
	* @param bool $show_errormessage (optional, default=true)
	* @param string $prompt_title (optional, default='')
	* @param string $prompt (optional, default='')
	* @param string $error_title (optional, default='')
	* @param string $error (optional, default='')
	* @param int $error_style (optional, default=VALIDATION_ERRSTYLE_WARNING)
	* @return bool
	*/
	public function addDataValidationDouble(int $type, int $op, int $row_first, int $row_last, int $col_first, int $col_last, float $val_1, float $val_2 = null, bool $allow_blank = true, bool $hide_dropdown = false, bool $show_inputmessage = true, bool $show_errormessage = true, string $prompt_title = '', string $prompt = '', string $error_title = '', string $error = '', int $error_style = 1): bool
	{
	}

	/**
	* Removes all data validations for the sheet (only for xlsx files)
	*
	* @return bool
	*/
	public function removeDataValidations(): bool
	{
	}

	/**
	* Returns the 0-based first filled row in the sheet
	*
	* @return int|false
	*/
	public function firstFilledRow(): int|false
	{
	}

	/**
	* Returns the 0-based last filled row in the sheet
	*
	* @return int|false
	*/
	public function lastFilledRow(): int|false
	{
	}

	/**
	* Returns the 0-based first filled column in the sheet
	*
	* @return int|false
	*/
	public function firstFilledCol(): int|false
	{
	}

	/**
	* Returns the 0-based last filled column in the sheet
	*
	* @return int|false
	*/
	public function lastFilledCol(): int|false
	{
	}

	/**
	* Removes a picture at the specified cell position
	*
	* @param int $row 0-based row number
	* @param int $col 0-based column number
	* @return bool
	*/
	public function removePicture(int $row, int $col): bool
	{
	}

	/**
	* Removes a picture by index
	*
	* @param int $index
	* @return bool
	*/
	public function removePictureByIndex(int $index): bool
	{
	}

	/**
	* Returns whether the cell contains a rich string
	*
	* @param int $row 0-based row number
	* @param int $col 0-based column number
	* @return bool
	*/
	public function isRichStr(int $row, int $col): bool
	{
	}

	/**
	* Reads a rich string from the cell
	*
	* @param int $row 0-based row number
	* @param int $col 0-based column number
	* @return ExcelRichString|false
	*/
	public function readRichStr(int $row, int $col): ExcelRichString|false
	{
	}

	/**
	* Writes a rich string to the cell
	*
	* @param int $row 0-based row number
	* @param int $col 0-based column number
	* @param ExcelRichString $richString
	* @param ExcelFormat|null $format (optional, default=null)
	* @return bool
	*/
	public function writeRichStr(int $row, int $col, ExcelRichString $richString, ?ExcelFormat $format = null): bool
	{
	}

	/**
	* Returns the number of form controls in the sheet
	*
	* @return int|false
	*/
	public function formControlSize(): int|false
	{
	}

	/**
	* Returns a form control by index
	*
	* @param int $index
	* @return ExcelFormControl|false
	*/
	public function formControl(int $index): ExcelFormControl|false
	{
	}

	/**
	* Returns the active cell position
	*
	* @return array|false Array with keys "row"(int), "col"(int)
	*/
	public function getActiveCell(): array|false
	{
	}

	/**
	* Sets the active cell position
	*
	* @param int $row
	* @param int $col
	* @return bool
	*/
	public function setActiveCell(int $row, int $col): bool
	{
	}

	/**
	* Returns the selection range as a string reference
	*
	* @return string|null|false
	*/
	public function selectionRange(): string|null|false
	{
	}

	/**
	* Adds a selection range
	*
	* @param string $sqref Selection range reference string
	* @return bool
	*/
	public function addSelectionRange(string $sqref): bool
	{
	}

	/**
	* Removes all selection ranges
	*
	* @return bool
	*/
	public function removeSelection(): bool
	{
	}

	/**
	* Returns the tab color
	*
	* @return int|false
	*/
	public function tabColor(): int|false
	{
	}

	/**
	* Returns the tab RGB color
	*
	* @return array|false Array with keys "red"(int), "green"(int), "blue"(int)
	*/
	public function getTabRgbColor(): array|false
	{
	}

	/**
	* Sets the tab RGB color
	*
	* @param int $red
	* @param int $green
	* @param int $blue
	* @return bool
	*/
	public function setTabRgbColor(int $red, int $green, int $blue): bool
	{
	}

	/**
	* Returns the hyperlink index at the specified cell
	*
	* @param int $row 0-based row number
	* @param int $col 0-based column number
	* @return int|false
	*/
	public function hyperlinkIndex(int $row, int $col): int|false
	{
	}

	/**
	* Returns the column width in pixels
	*
	* @param int $col 0-based column number
	* @return int|false
	*/
	public function colWidthPx(int $col): int|false
	{
	}

	/**
	* Returns the row height in pixels
	*
	* @param int $row 0-based row number
	* @return int|false
	*/
	public function rowHeightPx(int $row): int|false
	{
	}

	/**
	* Returns the column format
	*
	* @param int $col 0-based column number
	* @return ExcelFormat|false
	*/
	public function colFormat(int $col): ExcelFormat|false
	{
	}

	/**
	* Returns the row format
	*
	* @param int $row 0-based row number
	* @return ExcelFormat|false
	*/
	public function rowFormat(int $row): ExcelFormat|false
	{
	}

	/**
	* Sets column width in pixels
	*
	* @param int $colFirst 0-based first column
	* @param int $colLast 0-based last column
	* @param int $widthPx Width in pixels
	* @param ExcelFormat|null $format (optional, default=null)
	* @param mixed $hidden (optional)
	* @return bool
	*/
	public function setColPx(int $colFirst, int $colLast, int $widthPx, ?ExcelFormat $format = null, $hidden = null): bool
	{
	}

	/**
	* Sets row height in pixels
	*
	* @param int $row 0-based row number
	* @param int $heightPx Height in pixels
	* @param ExcelFormat|null $format (optional, default=null)
	* @param mixed $hidden (optional)
	* @return bool
	*/
	public function setRowPx(int $row, int $heightPx, ?ExcelFormat $format = null, $hidden = null): bool
	{
	}

	/**
	* Sets borders for a range of cells
	*
	* @param int $rowFirst 0-based first row
	* @param int $rowLast 0-based last row
	* @param int $colFirst 0-based first column
	* @param int $colLast 0-based last column
	* @param int $borderStyle One of ExcelFormat::BORDERSTYLE_* constants
	* @param int $borderColor One of ExcelFormat::COLOR_* constants
	* @return bool
	*/
	public function setBorder(int $rowFirst, int $rowLast, int $colFirst, int $colLast, int $borderStyle, int $borderColor): bool
	{
	}

	/**
	* Adds a table to the sheet (xlsx only)
	*
	* @param string $name
	* @param int $rowFirst 0-based first row
	* @param int $rowLast 0-based last row
	* @param int $colFirst 0-based first column
	* @param int $colLast 0-based last column
	* @param bool $hasHeaders (optional)
	* @param int $style (optional) One of ExcelTable::TABLESTYLE_* constants
	* @return ExcelTable|false
	*/
	public function addTable(string $name, int $rowFirst, int $rowLast, int $colFirst, int $colLast, bool $hasHeaders = null, int $style = null): ExcelTable|false
	{
	}

	/**
	* Returns a table by name
	*
	* @param string $name
	* @return ExcelTable|false
	*/
	public function getTableByName(string $name): ExcelTable|false
	{
	}

	/**
	* Returns a table by index
	*
	* @param int $index
	* @return ExcelTable|false
	*/
	public function getTableByIndex(int $index): ExcelTable|false
	{
	}

	/**
	* Applies a specific AutoFilter to the sheet
	*
	* @param ExcelAutoFilter $autoFilter
	* @return bool
	*/
	public function applyFilter2(ExcelAutoFilter $autoFilter): bool
	{
	}

	/**
	* Adds conditional formatting to the sheet
	*
	* @since libxl 5.1.0 (with range parameters)
	* @param int $rowFirst 0-based first row
	* @param int $rowLast 0-based last row
	* @param int $colFirst 0-based first column
	* @param int $colLast 0-based last column
	* @return ExcelConditionalFormatting|false
	*/
	public function addConditionalFormatting(int $rowFirst = null, int $rowLast = null, int $colFirst = null, int $colLast = null): ExcelConditionalFormatting|false
	{
	}

	/**
	* Returns a conditional formatting by index
	*
	* @since libxl 5.1.0
	* @param int $index
	* @return ExcelConditionalFormatting|false
	*/
	public function conditionalFormatting(int $index): ExcelConditionalFormatting|false
	{
	}

	/**
	* Removes a conditional formatting by index
	*
	* @since libxl 5.1.0
	* @param int $index
	* @return bool
	*/
	public function removeConditionalFormatting(int $index): bool
	{
	}

	/**
	* Returns the number of conditional formattings in the sheet
	*
	* @since libxl 5.1.0
	* @return int|false
	*/
	public function conditionalFormattingSize(): int|false
	{
	}

} // end ExcelSheet
