<?php
/*
  +---------------------------------------------------------------------------+
  | ExcelBook                                                                 |
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
class ExcelBook
{
	const PICTURETYPE_PNG = 0;
	const PICTURETYPE_JPEG = 1;
	const PICTURETYPE_WMF = 2;
	const PICTURETYPE_DIB = 3;
	const PICTURETYPE_EMF = 4;
	const PICTURETYPE_PICT = 5;
	const PICTURETYPE_TIFF = 6;
	const PICTURETYPE_GIF = 7;
	const PICTURETYPE_SVG = 8;

	const SCOPE_UNDEFINED = -2;
	const SCOPE_WORKBOOK = -1;

	const SHEETTYPE_SHEET = 0;
	const SHEETTYPE_CHART = 1;
	const SHEETTYPE_UNKNOWN = 2;

	const POSITION_MOVE_AND_SIZE = 0;
	const POSITION_ONLY_MOVE = 1;
	const POSITION_ABSOLUTE = 2;

	const CALCMODE_MANUAL = 0;
	const CALCMODE_AUTO = 1;
	const CALCMODE_AUTONOTABLE = 2;

	const CELLSTYLE_NORMAL = 0;
	const CELLSTYLE_BAD = 1;
	const CELLSTYLE_GOOD = 2;
	const CELLSTYLE_NEUTRAL = 3;
	const CELLSTYLE_CALC = 4;
	const CELLSTYLE_CHECKCELL = 5;
	const CELLSTYLE_EXPLANATORY = 6;
	const CELLSTYLE_INPUT = 7;
	const CELLSTYLE_OUTPUT = 8;
	const CELLSTYLE_HYPERLINK = 9;
	const CELLSTYLE_LINKEDCELL = 10;
	const CELLSTYLE_NOTE = 11;
	const CELLSTYLE_WARNING = 12;
	const CELLSTYLE_TITLE = 13;
	const CELLSTYLE_HEADING1 = 14;
	const CELLSTYLE_HEADING2 = 15;
	const CELLSTYLE_HEADING3 = 16;
	const CELLSTYLE_HEADING4 = 17;
	const CELLSTYLE_TOTAL = 18;
	const CELLSTYLE_COMMA = 19;
	const CELLSTYLE_COMMA0 = 20;
	const CELLSTYLE_CURRENCY = 21;
	const CELLSTYLE_CURRENCY0 = 22;
	const CELLSTYLE_PERCENT = 23;

	/**
	* Create a new Excel workbook
	*
	* @param string|null $license_name (optional, default=null)
	* @param string|null $license_key (optional, default=null)
	* @param bool $excel_2007 (optional, default=false)
	* @return ExcelBook
	*/
	public function __construct(?string $license_name = null, ?string $license_key = null, bool $excel_2007 = false)
	{
	}

	/**
	* Returns whether the library requires a license key
	*
	* @return bool
	*/
	public static function requiresKey(): bool
	{
	}

	/**
	* Add or copy an ExcelFont object
	*
	* @param ExcelFont $font (optional, default=null) Font to copy
	* @return ExcelFont|false
	*/
	public function addFont(ExcelFont $font = null): ExcelFont|false
	{
	}

	/**
	* Add or copy an ExcelFormat object
	*
	* @param ExcelFormat|null $format (optional, default=null) Format to copy
	* @return ExcelFormat|false
	*/
	public function addFormat(?ExcelFormat $format = null): ExcelFormat|false
	{
	}

	/**
	* Get an array of all ExcelFormat objects used inside a workbook
	*
	* @return array|false Array of ExcelFormat objects
	*/
	public function getAllFormats(): array|false
	{
	}

	/**
	* Get Excel error string
	*
	* @return string|false Description of last error, or false if no error
	*/
	public function getError(): string|false
	{
	}

	/**
	* Load Excel from file
	*
	* @param string $filename
	* @return bool
	*/
	public function loadFile(string $filename): bool
	{
	}

	/**
	* Load Excel data string
	*
	* @param string $data
	* @return bool
	*/
	public function load(string $data): bool
	{
	}

	/**
	* Save Excel file
	*
	* @param string $filename (optional, default="") If empty or omitted, returns binary string
	* @return string|bool If $filename is empty or omitted, returns string; otherwise returns bool
	*/
	public function save(string $filename = ''): string|true|false
	{
	}

	/**
	* Get an Excel worksheet
	*
	* @param int $sheet_number (optional, default=0) 0-based worksheet number
	* @return ExcelSheet|false
	*/
	public function getSheet(int $sheet = 0): ExcelSheet|false
	{
	}

	/**
	* Get an excel sheet by name
	*
	* @param string $name
	* @param bool $case_insensitive (optional, default=false)
	* @return ExcelSheet|false
	*/
	public function getSheetByName(string $name, bool $case_insensitive = false): ExcelSheet|false
	{
	}

	/**
	* Add a worksheet to a workbook
	*
	* @param string $name The name for the new worksheet
	* @return ExcelSheet|false
	*/
	public function addSheet(string $name): ExcelSheet|false
	{
	}

	/**
	* Create a copy of a worksheet in a workbook
	*
	* @param string $name The name for the new worksheet
	* @param int $sheet_number The 0-based number of the source worksheet to copy
	* @return ExcelSheet|false
	*/
	public function copySheet(string $name, int $sheet_number): ExcelSheet|false
	{
	}

	/**
	* Delete an Excel worksheet
	*
	* @param int $sheet_number 0-based worksheet number to delete
	* @return bool
	*/
	public function deleteSheet(int $sheet): bool
	{
	}

	/**
	* Get the number of worksheets inside a workbook
	*
	* @return int|false
	*/
	public function sheetCount(): int|false
	{
	}

	/**
	* Get or set the active Excel worksheet number
	*
	* @see ExcelBook::getActiveSheet()
	* @see ExcelBook::setActiveSheet()
	* @param int $sheet_number (optional) If supplied, the 0-based worksheet number to set as active
	* @return int|false 0-based active worksheet number
	*/
	public function activeSheet(int $sheet = -1): int|false
	{
	}

	/**
	* Get a custom cell format
	*
	* @see ExcelBook::addCustomFormat()
	* @param int $id
	* @return string|false
	*/
	public function getCustomFormat(int $id): string|false
	{
	}

	/**
	* Create a custom cell format
	*
	* @see ExcelBook::getCustomFormat()
	* @param string $format_string
	* @return int|false The ID assigned to the new format
	*/
	public function addCustomFormat(string $format): int|false
	{
	}

	/**
	* Pack a unix timestamp into an Excel double
	*
	* @see ExcelBook::unpackDate()
	* @param int $timestamp
	* @return float|false
	*/
	public function packDate(int $timestamp): float|false
	{
	}

	/**
	* Pack a date from single values into an Excel double
	*
	* @param int $year
	* @param int $month
	* @param int $day
	* @param int $hour
	* @param int $min
	* @param int $sec
	* @return float|false
	*/
	public function packDateValues(int $year, int $month, int $day, int $hour, int $min, int $sec): float|false
	{
	}

	/**
	* Unpack an Excel double into a unix timestamp
	*
	* @see ExcelBook::packDate()
	* @param float $date
	* @return int|false
	*/
	public function unpackDate(float $date): int|false
	{
	}

	/**
	* Get the active worksheet inside a workbook
	*
	* @see ExcelBook::activeSheet()
	* @see ExcelBook::setActiveSheet()
	* @return int|false 0-based active worksheet number
	*/
	public function getActiveSheet(): int|false
	{
	}

	/**
	* Set the active worksheet
	*
	* @see ExcelBook::getActiveSheet()
	* @see ExcelBook::activeSheet()
	* @param int $sheet_number 0-based worksheet to make active
	* @return bool
	*/
	public function setActiveSheet(int $sheet): bool
	{
	}

	/**
	* Get the default font
	*
	* @see ExcelBook::setDefaultFont()
	* @return array|false Array with keys "font"(string) and "font_size"(int)
	*/
	public function getDefaultFont(): array|false
	{
	}

	/**
	* Set the default font and size
	*
	* @see ExcelBook::getDefaultFont()
	* @param string $font
	* @param int $font_size
	* @return void
	*/
	public function setDefaultFont(string $font, int $font_size)
	{
	}

	/**
	* Set the locale
	*
	* @param string $locale
	* @return void
	*/
	public function setLocale(string $locale)
	{
	}

	/**
	* Add a picture from file
	*
	* @see ExcelBook::addPictureFromString()
	* @see ExcelSheet::addPictureScaled()
	* @see ExcelSheet::addPictureDim()
	* @param string $filename
	* @return int|false A picture ID
	*/
	public function addPictureFromFile($filename): int|false
	{
	}

	/**
	* Add a picture from string
	*
	* @see ExcelBook::addPictureFromFile()
	* @see ExcelSheet::addPictureScaled()
	* @see ExcelSheet::addPictureDim()
	* @param string $data
	* @return int|false A picture ID
	*/
	public function addPictureFromString($data): int|false
	{
	}

	/**
	* Returns whether RGB mode is active
	*
	* @see ExcelBook::setRGBMode()
	* @return bool
	*/
	public function rgbMode(): bool
	{
	}

	/**
	* Sets RGB mode on or off
	*
	* @see ExcelBook::rgbMode()
	* @param bool $mode
	* @return void
	*/
	public function setRGBMode(bool $mode)
	{
	}

	/**
	* Packs red, green, and blue components in color value. Used for xlsx format only.
	*
	* @see ExcelBook::colorUnpack()
	* @param int $r
	* @param int $g
	* @param int $b
	* @return int|false
	*/
	public function colorPack(int $r, int $g, int $b): int|false
	{
	}

	/**
	* Unpacks color value into red, green, and blue components. Used for xlsx format only.
	*
	* @see ExcelBook::colorPack()
	* @param int $color One of ExcelFormat::COLOR_* constants
	* @return array|false Array with keys "red"(int), "green"(int), and "blue"(int)
	*/
	public function colorUnpack(int $color): array|false
	{
	}

	/**
	* Returns whether the 1904 date system is active
	*
	* @return bool
	*/
	public function isDate1904(): bool
	{
	}

	/**
	* Sets the date system mode
	*
	* @param bool $date_type true for 1904, false for 1900
	* @return bool
	*/
	public function setDate1904(bool $date_type): bool
	{
	}

	/**
	* Returns BIFF version of binary file. Used for xls format only.
	*
	* @return int|false
	*/
	public function biffVersion(): int|false
	{
	}

	/**
	* Sets the R1C1 reference mode
	*
	* @param bool $active
	* @return void
	*/
	public function setRefR1C1(bool $active)
	{
	}

	/**
	* Returns whether the R1C1 reference mode is active
	*
	* @return bool
	*/
	public function getRefR1C1(): bool
	{
	}

	/**
	* Returns a picture at position index
	*
	* @param int $index
	* @return array|false Array with keys "data"(string) and "type"(int)
	*/
	public function getPicture(int $index): array|false
	{
	}

	/**
	* Returns a number of pictures in this workbook
	*
	* @return int|false
	*/
	public function getNumPictures(): int|false
	{
	}

	/**
	* Inserts a new sheet at position index
	*
	* @param int $index
	* @param string $name
	* @param ExcelSheet|null $sheet (optional) Existing sheet from this workbook to copy
	* @return ExcelSheet|false
	*/
	public function insertSheet(int $index, string $name, ?ExcelSheet $sheet = null): ExcelSheet|false
	{
	}

	/**
	* Returns whether the workbook is a template
	*
	* @return bool
	*/
	public function isTemplate(): bool
	{
	}

	/**
	* Sets the template flag
	*
	* @param bool $mode
	* @return void
	*/
	public function setTemplate(bool $mode)
	{
	}

	/**
	* Returns type of sheet with specified index
	*
	* @param int $sheet
	* @return int|false One of ExcelBook::SHEETTYPE_* constants
	*/
	public function sheetType(int $sheet): int|false
	{
	}

	/**
	* Get LibXL version
	*
	* @return string
	*/
	public function getLibXlVersion(): string
	{
	}

	/**
	* Get PHP excel extension version
	*
	* @return string
	*/
	public function getPhpExcelVersion(): string
	{
	}

	/**
	* Adds a picture to the workbook as link (only for xlsx files)
	*
	* @param string $filename
	* @param bool $insert (optional, default=false) false stores only a link, true stores picture and link
	* @return int|false Picture identifier
	*/
	public function addPictureAsLink(string $filename, bool $insert = false): int|false
	{
	}

	/**
	* Moves a sheet with specified index to a new position
	*
	* @param int $src_index
	* @param int $dest_index
	* @return bool
	*/
	public function moveSheet(int $src_index, int $dest_index): bool
	{
	}

	/**
	* Load Excel sheet info without loading the full file
	*
	* @param string $filename
	* @return bool
	*/
	public function loadInfo(string $filename): bool
	{
	}

	/**
	* Returns the sheet name by index
	*
	* @param int $index
	* @return string|false
	*/
	public function getSheetName(int $index): string|false
	{
	}

	/**
	* Creates a new rich string
	*
	* @return ExcelRichString|false
	*/
	public function addRichString(): ExcelRichString|false
	{
	}

	/**
	* Returns the calculation mode
	*
	* @return int|false One of ExcelBook::CALCMODE_* constants
	*/
	public function calcMode(): int|false
	{
	}

	/**
	* Sets the calculation mode
	*
	* @param int $mode One of ExcelBook::CALCMODE_* constants
	* @return bool
	*/
	public function setCalcMode(int $mode): bool
	{
	}

	/**
	* Creates a new conditional format
	*
	* @return ExcelConditionalFormat|false
	*/
	public function addConditionalFormat(): ExcelConditionalFormat|false
	{
	}

	/**
	* Creates a new format from a predefined cell style
	*
	* @param int $style One of ExcelBook::CELLSTYLE_* constants
	* @return ExcelFormat|false
	*/
	public function addFormatFromStyle(int $style): ExcelFormat|false
	{
	}

	/**
	* Removes VBA module from the workbook
	*
	* @return bool
	*/
	public function removeVBA(): bool
	{
	}

	/**
	* Removes printer settings from all sheets
	*
	* @return bool
	*/
	public function removePrinterSettings(): bool
	{
	}

	/**
	* Sets password for the workbook (xlsx only)
	*
	* @since libxl 5.0.0
	* @param string $password
	* @return bool
	*/
	public function setPassword(string $password): bool
	{
	}

	/**
	* Returns the DPI awareness mode
	*
	* @since libxl 5.0.0
	* @return int|false
	*/
	public function dpiAwareness(): int|false
	{
	}

	/**
	* Sets the DPI awareness mode
	*
	* @since libxl 5.0.0
	* @param int $value
	* @return bool
	*/
	public function setDpiAwareness(int $value): bool
	{
	}

	/**
	* Load Excel sheet info from raw data without loading the full file
	*
	* @since libxl 5.0.1
	* @param string $data
	* @return bool
	*/
	public function loadInfoRaw(string $data): bool
	{
	}

	/**
	* Returns the error code of the last operation
	*
	* @since libxl 5.1.0
	* @return int|false
	*/
	public function errorCode(): int|false
	{
	}

	/**
	* Returns a conditional format by index
	*
	* @since libxl 5.1.0
	* @param int $index
	* @return ExcelConditionalFormat|false
	*/
	public function conditionalFormat(int $index): ExcelConditionalFormat|false
	{
	}

	/**
	* Returns the number of conditional formats
	*
	* @since libxl 5.1.0
	* @return int|false
	*/
	public function conditionalFormatSize(): int|false
	{
	}

	/**
	* Clears the workbook
	*
	* @since libxl 5.1.0
	* @return bool
	*/
	public function clear(): bool
	{
	}

	/**
	* Returns the core properties object for the workbook
	*
	* @return ExcelCoreProperties|false
	*/
	public function coreProperties(): ExcelCoreProperties|false
	{
	}

	/**
	* Removes all phonetic runs from the workbook
	*
	* @return bool
	*/
	public function removeAllPhonetics(): bool
	{
	}

} // end ExcelBook
