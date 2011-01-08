<?php
/*
  +---------------------------------------------------------------------------+
  | ExcelFormat                                                               |
  |                                                                           |
  | Reference file for NuSphere PHPEd (and possibly other IDE's) for use with |
  | php_excel interface to libxl by Ilia Alshanetsky <ilia@ilia.ws>          |
  |                                                                           |
  | php_excel "PECL" style module (http://github.com/iliaal/php_excel)        |
  | libxl library (http://www.libxl.com)                                      |
  |                                                                           |
  | Rob Gagnon <rgagnon24@gmail.com>                                          |
  +---------------------------------------------------------------------------+
*/
class ExcelFormat {
	const COLOR_BLACK = 8;
	const COLOR_WHITE = 9;
	const COLOR_RED = 10;
	const COLOR_BRIGHTGREEN = 11;
	const COLOR_BLUE = 12;
	const COLOR_YELLOW = 13;
	const COLOR_PINK = 14;
	const COLOR_TURQUOISE = 15;
	const COLOR_DARKRED = 16;
	const COLOR_GREEN = 17;
	const COLOR_DARKBLUE = 18;
	const COLOR_DARKYELLOW = 19;
	const COLOR_VIOLET = 20;
	const COLOR_TEAL = 21;
	const COLOR_GRAY25 = 22;
	const COLOR_GRAY50 = 23;
	const COLOR_PERIWINKLE_CF = 24;
	const COLOR_PLUM_CF = 25;
	const COLOR_IVORY_CF = 26;
	const COLOR_LIGHTTURQUOISE_CF = 27;
	const COLOR_DARKPURPLE_CF = 28;
	const COLOR_CORAL_CF = 29;
	const COLOR_OCEANBLUE_CF = 30;
	const COLOR_ICEBLUE_CF = 31;
	const COLOR_DARKBLUE_CL = 32;
	const COLOR_PINK_CL = 33;
	const COLOR_YELLOW_CL = 34;
	const COLOR_TURQUOISE_CL = 35;
	const COLOR_VIOLET_CL = 36;
	const COLOR_DARKRED_CL = 37;
	const COLOR_TEAL_CL = 38;
	const COLOR_BLUE_CL = 39;
	const COLOR_SKYBLUE = 40;
	const COLOR_LIGHTTURQUOISE = 41;
	const COLOR_LIGHTGREEN = 42;
	const COLOR_LIGHTYELLOW = 43;
	const COLOR_PALEBLUE = 44;
	const COLOR_ROSE = 45;
	const COLOR_LAVENDER = 46;
	const COLOR_TAN = 47;
	const COLOR_LIGHTBLUE = 48;
	const COLOR_AQUA = 49;
	const COLOR_LIME = 50;
	const COLOR_GOLD = 51;
	const COLOR_LIGHTORANGE = 52;
	const COLOR_ORANGE = 53;
	const COLOR_BLUEGRAY = 54;
	const COLOR_GRAY40 = 55;
	const COLOR_DARKTEAL = 56;
	const COLOR_SEAGREEN = 57;
	const COLOR_DARKGREEN = 58;
	const COLOR_OLIVEGREEN = 59;
	const COLOR_BROWN = 60;
	const COLOR_PLUM = 61;
	const COLOR_INDIGO = 62;
	const COLOR_GRAY80 = 63;
	const COLOR_DEFAULT_FOREGROUND = 64;
	const COLOR_DEFAULT_BACKGROUND = 65;
	const COLOR_TOOLTIP = 81;
	const COLOR_AUTO = 32767;

	const AS_DATE = 1;
	const AS_FORMULA = 2;
	const AS_NUMERIC_STRING = 3;

	const NUMFORMAT_GENERAL = 0;
	const NUMFORMAT_NUMBER = 1;
	const NUMFORMAT_NUMBER_D2 = 2;
	const NUMFORMAT_NUMBER_SEP = 3;
	const NUMFORMAT_NUMBER_SEP_D2 = 4;
	const NUMFORMAT_CURRENCY_NEGBRA = 5;
	const NUMFORMAT_CURRENCY_NEGBRARED = 6;
	const NUMFORMAT_CURRENCY_D2_NEGBRA = 7;
	const NUMFORMAT_CURRENCY_D2_NEGBRARED = 8;
	const NUMFORMAT_PERCENT = 9;
	const NUMFORMAT_PERCENT_D2 = 10;
	const NUMFORMAT_SCIENTIFIC_D2 = 11;
	const NUMFORMAT_FRACTION_ONEDIG = 12;
	const NUMFORMAT_FRACTION_TWODIG = 13;
	const NUMFORMAT_DATE = 14;
	const NUMFORMAT_CUSTOM_D_MON_YY = 15;
	const NUMFORMAT_CUSTOM_D_MON = 16;
	const NUMFORMAT_CUSTOM_MON_YY = 17;
	const NUMFORMAT_CUSTOM_HMM_AM = 18;
	const NUMFORMAT_CUSTOM_HMMSS_AM = 19;
	const NUMFORMAT_CUSTOM_HMM = 20;
	const NUMFORMAT_CUSTOM_HMMSS = 21;
	const NUMFORMAT_CUSTOM_MDYYYY_HMM = 22;
	const NUMFORMAT_NUMBER_SEP_NEGBRA = 37;
	const NUMFORMAT_NUMBER_SEP_NEGBRARED = 38;
	const NUMFORMAT_NUMBER_D2_SEP_NEGBRA = 39;
	const NUMFORMAT_NUMBER_D2_SEP_NEGBRARED = 40;
	const NUMFORMAT_ACCOUNT = 41;
	const NUMFORMAT_ACCOUNTCUR = 42;
	const NUMFORMAT_ACCOUNT_D2 = 43;
	const NUMFORMAT_ACCOUNT_D2_CUR = 44;
	const NUMFORMAT_CUSTOM_MMSS = 45;
	const NUMFORMAT_CUSTOM_H0MMSS = 46;
	const NUMFORMAT_CUSTOM_MMSS0 = 47;
	const NUMFORMAT_CUSTOM_000P0E_PLUS0 = 48;
	const NUMFORMAT_TEXT = 49;

	const ALIGNH_GENERAL = 0;
	const ALIGNH_LEFT = 1;
	const ALIGNH_CENTER = 2;
	const ALIGNH_RIGHT = 3;
	const ALIGNH_FILL = 4;
	const ALIGNH_JUSTIFY = 5;
	const ALIGNH_MERGE = 6;
	const ALIGNH_DISTRIBUTED = 7;

	const ALIGNV_TOP = 0;
	const ALIGNV_CENTER = 1;
	const ALIGNV_BOTTOM = 2;
	const ALIGNV_JUSTIFY = 3;
	const ALIGNV_DISTRIBUTED = 4;

	const BORDERSTYLE_NONE = 0;
	const BORDERSTYLE_THIN = 1;
	const BORDERSTYLE_MEDIUM = 2;
	const BORDERSTYLE_DASHED = 3;
	const BORDERSTYLE_DOTTED = 4;
	const BORDERSTYLE_THICK = 5;
	const BORDERSTYLE_DOUBLE = 6;
	const BORDERSTYLE_HAIR = 7;
	const BORDERSTYLE_MEDIUMDASHED = 8;
	const BORDERSTYLE_DASHDOT = 9;
	const BORDERSTYLE_MEDIUMDASHDOT = 10;
	const BORDERSTYLE_DASHDOTDOT = 11;
	const BORDERSTYLE_MEDIUMDASHDOTDOT = 12;
	const BORDERSTYLE_SLANTDASHDOT = 13;

	const BORDERDIAGONAL_NONE = 0;
	const BORDERDIAGONAL_DOWN = 1;
	const BORDERDIAGONAL_UP = 2;
	const BORDERDIAGONAL_BOTH = 3;

	const FILLPATTERN_NONE = 0;
	const FILLPATTERN_SOLID = 1;
	const FILLPATTERN_GRAY50 = 2;
	const FILLPATTERN_GRAY75 = 3;
	const FILLPATTERN_GRAY25 = 4;
	const FILLPATTERN_HORSTRIPE = 5;
	const FILLPATTERN_VERSTRIPE = 6;
	const FILLPATTERN_REVDIAGSTRIPE = 7;
	const FILLPATTERN_DIAGSTRIPE = 8;
	const FILLPATTERN_DIAGCROSSHATCH = 9;
	const FILLPATTERN_THICKDIAGCROSSHATCH = 10;
	const FILLPATTERN_THINHORSTRIPE = 11;
	const FILLPATTERN_THINVERSTRIPE = 12;
	const FILLPATTERN_THINREVDIAGSTRIPE = 13;
	const FILLPATTERN_THINDIAGSTRIPE = 14;
	const FILLPATTERN_THINHORCROSSHATCH = 15;
	const FILLPATTERN_THINDIAGCROSSHATCH = 16;
	const FILLPATTERN_GRAY12P5 = 17;
	const FILLPATTERN_GRAY6P25 = 18;
	/**
	* Create a format within an Excel workbook
	*
	* @see ExcelBook::addFormat()
	* @param ExcelBook $book
	* @return ExcelFormat
	*/
	public function __construct($book) {
	} // __construct

	/**
	* Get, or set the color of the bottom border of a cell
	*
	* @param int $color (optional, default=null) One of ExcelFormat::COLOR_* constants
	* @return int
	*/
	public function borderBottomColor($color = null) {
	} // borderBottomColor

	/**
	* Get, or set the border style for the bottom of a cell
	*
	* @param int $style (optional, default=null) One of ExcelFormat::BORDERSTYLE_* constants
	* @return int
	*/
	public function borderBottomStyle($style = null) {
	} // borderBottomStyle

	/**
	* Set the border color on all sides of a cell
	*
	* @param int $color (optional, default=null) One of ExcelFormat::COLOR_* constants
	* @return int The color, or true if no value supplied for $color
	*/
	public function borderColor($color = null) {
	} // borderColor

	/**
	* Get, or set the color of the diagonal of a cell
	*
	* @param int $color (optional, default=null) One of ExcelFormat::COLOR_* constants
	* @return int
	*/
	public function borderDiagonalColor($color = null) {
	} // borderDiagonalColor

	/**
	* Get, or set the border for the diagonal of a cell
	*
	* @param int $style (optional, default=null) One of ExcelFormat::BORDERDIAGONAL_* constants
	* @return int
	*/
	public function borderDiagonalStyle($style = null) {
	} // borderDiagonalStyle

	/**
	* Get, or set the color of the left side border of a cell
	*
	* @param int $color (optional, default=null) One of ExcelFormat::COLOR_* constants
	* @return int
	*/
	public function borderLeftColor($color = null) {
	} // borderLeftColor

	/**
	* Get, or set the border style for the left side of a cell
	*
	* @param int $style (optional, default=null) One of ExcelFormat::BORDERSTYLE_* constants
	* @return int
	*/
	public function borderLeftStyle($style = null) {
	} // borderLeftStyle

	/**
	* Get, or set the color of the right side border of a cell
	*
	* @param int $color (optional, default=null) One of ExcelFormat::COLOR_* constants
	* @return int
	*/
	public function borderRightColor($color = null) {
	} // borderRightColor

	/**
	* Get, or set the border style for the right side of a cell
	*
	* @param int $style (optional, default=null) One of ExcelFormat::BORDERSTYLE_* constants
	* @return int
	*/
	public function borderRightStyle($style = null) {
	} // borderRightStyle

	/**
	* Set the cell border style on all sides of a cell
	*
	* @param int $style (optional, default=null) One of ExcelFormat::BORDERSTYLE_* constants
	* @return int The border style, or true if no value supplied for $style
	*/
	public function borderStyle($style = null) {
	} // borderStyle

	/**
	* Get, or set the color of the top border of a cell
	*
	* @param int $color (optional, default=null) One of ExcelFormat::COLOR_* constants
	* @return int
	*/
	public function borderTopColor($color = null) {
	} // borderTopColor

	/**
	* Get, or set the border style for the top of a cell
	*
	* @param int $style (optional, default=null) One of ExcelFormat::BORDERSTYLE_* constants
	* @return int
	*/
	public function borderTopStyle($style = null) {
	} // borderTopStyle

	/**
	* Get, or set the cell fill pattern
	*
	* @param int $pattern (optional, default=null) One of ExcelFormat::FILLPATTERN_* constants
	* @return int
	*/
	public function fillPattern($pattern = null) {
	} // fillPattern

	/**
	* Get the font for this format
	*
	* @see ExcelFormat::setFont()
	* @return ExcelFont
	*/
	public function getFont() {
	} // getFont

	/**
	* Get, or set whether the cell is hidden
	*
	* @param bool $hidden (optional, default=null)
	* @return bool
	*/
	public function hidden($hidden = null) {
	} // hidden

	/**
	* Get, or set the cell horizontal alignment
	*
	* @see ExcelFormat::verticalAlign()
	* @param int $halign_mode (optional, default=null) One of ExcelFormat::ALIGNH_* constants
	* @return int
	*/
	public function horizontalAlign($halign_mode = null) {
	} // horizontalAlign

	/**
	* Get, or set the cell text indentation level
	*
	* @param int $indent (optional, default=null) A number from 0-15
	* @return int
	*/
	public function indent($indent = null) {
	} // indent

	/**
	* Get, or set whether a cell is locked
	*
	* @param bool $locked (optional, default=null)
	* @return bool
	*/
	public function locked($locked) {
	} // locked

	/**
	* Get, or set the cell number format
	*
	* @param int $number_format Number format identifier.  One of ExcelFormat::NUMFORMAT_* constants
	* @return int
	*/
	public function numberFormat($number_format) {
	} // numberFormat

	/**
	* Get, or set the pattern background color
	*
	* @param int $color (optional, default=null) One of ExcelFormat::COLOR_* constants
	* @return int
	*/
	public function patternBackgroundColor($color = null) {
	} // patternBackgroundColor

	/**
	* Get, or set the pattern foreground color
	*
	* @param int $color (optional, default=null) One of ExcelFormat::COLOR_* constants
	* @return int
	*/
	public function patternForegroundColor($color = null) {
	} // patternForegroundColor

	/**
	* Get, or set the cell data rotation
	*
	* @param int $angle (optional, default=null) 0 to 90 (rotate left 0-90 degrees), 91 to 180 (rotate right 1-90 degrees), or 255 for vertical text
	* @return int The angle of rotation, or false if setting an invalid value
	*/
	public function rotate($angle = null) {
	} // rotate

	/**
	* Set the font for this format
	*
	* @see ExcelFormat::getFont()
	* @param ExcelFont $font
	* @return bool
	*/
	public function setFont($font) {
	} // setFont

	/**
	* Get, or set whether the cell is shrink-to-fit
	*
	* @param bool $shrink (optional, default=null)
	* @return bool
	*/
	public function shrinkToFit($shrink = null) {
	} // shrinkToFit

	/**
	* Get, or set the cell vertical alignment
	*
	* @see ExcelFormat::horizontalAlign()
	* @param int $valign_mode (optional, default=null) One of ExcelFormat::ALIGNV_* constants
	* @return int
	*/
	public function verticalAlign($valign_mode = null) {
	} // verticalAlign

	/**
	* Get, or set the cell text wrapping
	*
	* @param bool $wrap (optional, default=null)
	* @return bool
	*/
	public function wrap($wrap = null) {
	} // wrap

} // end ExcelFormat

