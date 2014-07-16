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
class ExcelBook {
	/**
	* Create a new Excel workbook
	*
	* @param string $license_name
	* @param string $license_key
	* @param bool $excel_2007 (optional, default=false)
	* @return ExcelBook
	*/
	public function __construct($license_name, $license_key, $excel_2007 = false) {
	} // __construct

	/**
	* Get or set the active Excel worksheet number
	*
	* @see ExcelBook::getActiveSheet()
	* @see ExcelBook::setActiveSheet()
	* @param int $sheet_number (optional, default=null) If supplied, the 0-based worksheet number to set as active
	* @return int 0-based active worksheet number
	*/
	public function activeSheet($sheet_number = null) {
	} // activeSheet

	/**
	* Create a custom cell format
	*
	* @see ExcelBook::getCustomFormat()
	* @param string $format_string
	* @return int The ID assigned to the new format
	*/
	public function addCustomFormat($format_string) {
	} // addCustomFormat

	/**
	* Add or copy an ExcelFont object
	*
	* @param ExcelFont $font (optional, default=null) Font to copy
	* @return ExcelFont
	*/
	public function addFont($font = null) {
	} // addFont

	/**
	* Add or copy an ExcelFormat object
	*
	* @param ExcelFormat $format (optional, default=null) Format to copy
	* @return ExcelFormat
	*/
	public function addFormat($format) {
	} // addFormat

	/**
	* Add a picture from file
	*
	* @see ExcelBook::addPictureFromString()
	* @see ExcelSheet::addPictureScaled()
	* @see ExcelSheet::addPictureDim()
	* @param string $filename
	* @return int A picture ID
	*/
	public function addPictureFromFile($filename) {
	} // addPictureFromFile

	/**
	* Add a picture from string
	*
	* @see ExcelBook::addPictureFromFile()
	* @see ExcelSheet::addPictureScaled()
	* @see ExcelSheet::addPictureDim()
	* @param string $data
	* @return int A picture ID
	*/
	public function addPictureFromString($data) {
	} // addPictureFromString

	/**
	* Add a worksheet to a workbook
	*
	* @param string $name The name for the new worksheet
	* @return ExcelSheet The worksheet created
	*/
	public function addSheet($name) {
	} // addSheet

	/**
	* Returns BIFF version of binary file. Used for xls format only.
	*
	* @return int BIFF version
	*/
	public function biffVersion() {
	} // biffVersion

	/**
	* Packs red, green, and blue components in color value.  Used for xlsx format only.
	*
	* @see ExcelBook::colorUnpack()
	* @param int $red
	* @param int $green
	* @param int $blue
	* @return int
	*/
	public function colorPack($red, $green, $blue) {
	} // colorPack

	/**
	* Unpacks color value into red, green, and blue components.  Used for xlsx format only.
	*
	* @see ExcelBook::colorPack()
	* @param int $color One of ExcelFormat::COLOR_* constants
	* @return array with keys "red"(int), "green"(int), and "blue"(int)
	*/
	public function colorUnpack($color) {
	} // colorUnpack

	/**
	* Create a copy of a worksheet in a workbook
	*
	* @param string $name The name for the new worksheet
	* @param int $sheet_number The 0-based number of the source worksheet to copy
	* @return ExcelSheet The worksheet created
	*/
	public function copySheet($name, $sheet_number) {
	} // copySheet

	/**
	* Delete an Excel worksheet
	*
	* @param int $sheet_number 0-based worksheet number to delete
	* @return bool True if sheet deleted, false if $sheet_number invalid
	*/
	public function deleteSheet($sheet_number) {
	} // deleteSheet

	/**
	* Get the active worksheet inside a workbook
	*
	* @see ExcelBook::activeSheet()
	* @see ExcelBook::setActiveSheet()
	* @return int 0-based active worksheet number
	*/
	public function getActiveSheet() {
	} // getActiveSheet

	/**
	* Get an array of all ExcelFormat objects used inside a workbook
	*
	* @return array of ExcelFormat objects
	*/
	public function getAllFormats() {
	} // getAllFormats

	/**
	* Get a custom cell format
	*
	* @see ExcelBook::addCustomFormat()
	* @param int $id
	* @return string
	*/
	public function getCustomFormat($id) {
	} // getCustomFormat

	/**
	* Get the default font
	*
	* @see ExcelBook::setDefaultFont()
	* @return array with keys "font"(string) and "font_size"(int)
	*/
	public function getDefaultFont() {
	} // getDefaultFont

	/**
	* Get Excel error string
	*
	* @return string Description of last error that occurred, or false if no error
	*/
	public function getError() {
	} // getError

	/**
	* Returns a number of pictures in this workbook.
	*
	* @return int Number of pictures in Workbook
	*/
	public function getNumPictures() {
	} // getNumPictures

	/**
	* Returns a picture at position index.
	*
	* @param int $index
	* @return array with keys "data"(string) and "type"(int)
	*/
	public function getPicture($index) {
	} // getPicture

	/**
	* Returns whether the R1C1 reference mode is active.
	*
	* @return bool
	*/
	public function getRefR1C1() {
	} // getRefR1C1

	/**
	* Get an Excel worksheet
	*
	* @param int $sheet_number (optional, default=0) 0-based worksheet number
	* @return ExcelSheet or false if $sheet_number invalid
	*/
	public function getSheet($sheet_number = 0) {
	} // getSheet

	/**
	* Get an excel sheet by name.
	*
	* @param string $name
	* @param bool $case_insensitive (optional, default=false)
	* @return ExcelSheet
	*/
	public function getSheetByName($name, $case_insensitive = false) {
	} // getSheetByName

	/**
	* Inserts a new sheet to this book at position index, returns the sheet handle. Set initSheet
	* to 0 if you wish to add a new empty sheet or use existing sheet's handle for copying.
	*
	* @param int $index
	* @param string $name
	* @param ExcelSheet $sheet (optional)
	* @return ExcelSheet
	*/
	public function insertSheet($index, $name, $sheet = null) {
	} // insertSheet

	/**
	* Returns whether the 1904 date system is active:
	* true - 1904 date system,
	* false - 1900 date system
	*
	* @return bool
	*/
	public function isDate1904() {
	} // isDate1904

	/**
	* Returns whether the workbook is a template.
	*
	* @return bool
	*/
	public function isTemplate() {
	} // isTemplate

	/**
	* Load Excel data string
	*
	* @param string $data
	* @return bool
	*/
	public function load($data) {
	} // load

	/**
	* Load Excel from file
	*
	* @param string $filename
	* @return bool
	*/
	public function loadFile($filename) {
	} // loadFile

	/**
	* Pack a unix timestamp into an Excel double
	*
	* @see ExcelBook::unpackDate()
	* @param int $timestamp
	* @return float
	*/
	public function packDate($timestamp) {
	} // packDate
	
	/**
	* Pack a date from single values into an Excel double
	*
	* @param int $year
	* @param int $month
	* @param int $day
	* @param int $hour
	* @param int $minute
	* @param int $second
	* @return float
	*/
	public function packDateValues($year, $month, $day, $hour, $minute, $second) {
	} // packDateValues

	/**
	* Returns whether RGB mode is active
	*
	* @see ExcelBook::setRGBMode()
	* @return bool
	*/
	public function rgbMode() {
	} // rgbMode

	/**
	* Save Excel file
	*
	* @param string $filename (optional, default=null)
	* @return mixed If $filename is null, returns string, otherwise returns bool true if OK, false if not
	*/
	public function save($filename = null) {
	} // save

	/**
	* Set the active worksheet
	*
	* @see ExcelBook::getActiveSheet()
	* @see ExcelBook::activeSheet()
	* @param int $sheet_number 0-based worksheet to make active
	* @return bool
	*/
	public function setActiveSheet($sheet_number) {
	} // setActiveSheet

	/**
	* Sets the date system mode:
	* true - 1904 date system,
	* false - 1900 date system (default)
	*
	* @param bool $date_type
	* @return bool
	*/
	public function setDate1904($date_type) {
	} // setDate1904

	/**
	* Set the default font and size
	*
	* @see ExcelBook::getDefaultFont()
	* @param string $font_name
	* @param string $font_size
	* @return void
	*/
	public function setDefaultFont($font_name, $font_size) {
	} // setDefaultFont

	/**
	* Set the locale<br>
	* possible values: '.1252' (Windows-1252 or Cp1252), '.OCP' (OEM CodePage), default: '.ACP' (ANSI CodePage) if empty
	*
	* @param string $locale
	* @return void
	*/
	public function setLocale($locale) {
	} // setLocale

	/**
	* Sets the R1C1 reference mode.
	*
	* @param bool $active
	* @return void
	*/
	public function setRefR1C1($active) {
	} // setRefR1C1

	/**
	* Sets RGB mode on or off
	*
	* @see ExcelBook::rgbMode()
	* @param bool $mode
	* @return void
	*/
	public function setRGBMode($mode) {
	} // setRGBMode

	/**
	* Sets the template flag, if the workbook is template.
	*
	* @param bool $mode
	* @return void
	*/
	public function setTemplate($mode) {
	} // setTemplate

	/**
	* Get the number of worksheets inside a workbook
	*
	* @return int
	*/
	public function sheetCount() {
	} // sheetCount

	/**
	* Unpack an Excel double into a unix timestamp
	*
	* @see ExcelBook::packDate()
	* @param float $date
	* @return int
	*/
	public function unpackDate($date) {
	} // unpackDate

} // end ExcelBook
