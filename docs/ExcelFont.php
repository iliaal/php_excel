<?php
/*
  +---------------------------------------------------------------------------+
  | ExcelFont                                                                 |
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
class ExcelFont
{
	const NORMAL = 0;
	const SUPERSCRIPT = 1;
	const SUBSCRIPT = 2;

	const UNDERLINE_NONE = 0;
	const UNDERLINE_SINGLE = 1;
	const UNDERLINE_DOUBLE = 2;
	const UNDERLINE_SINGLEACC = 33;
	const UNDERLINE_DOUBLEACC = 34;

	/**
	* Create a font within an Excel workbook
	*
	* @see ExcelBook::addFont()
	* @param ExcelBook $book
	* @return ExcelFont
	*/
	public function __construct($book)
	{
	} // __construct

	/**
	* Get, or set if bold is on or off
	*
	* @param bool $bold (optional, default=null)
	* @return bool
	*/
	public function bold($bold = null)
	{
	} // bold

	/**
	* Get, or set the font color
	*
	* @param int $color (optional, default=null) One of ExcelFormat::COLOR_* constants
	* @return int
	*/
	public function color($color = null)
	{
	} // color

	/**
	* Get, or set if italics are on or off
	*
	* @param bool $italics (optional, default=null)
	* @return bool
	*/
	public function italics($italics = null)
	{
	} // italics

	/**
	* Get, or set the font script mode
	*
	* @param int $mode (optional, default=null) One of ExcelFont::NORMAL, ::SUBSCRIPT, or ::SUPERSCRIPT
	* @return int
	*/
	public function mode($mode = null)
	{
	} // mode

	/**
	* Get, or set the font name
	*
	* @param string $font_name (optional, default=null)
	* @return string
	*/
	public function name($font_name = null)
	{
	} // name

	/**
	* Get, or set the font size
	*
	* @param int $size (optional, default=null)
	* @return int The current font size
	*/
	public function size($size = null)
	{
	} // size

	/**
	* Get, or set if strike-through is on or off
	*
	* @param bool $strike (optional, default=null)
	* @return bool
	*/
	public function strike($strike = null)
	{
	} // strike

	/**
	* Get, or set the underline style
	*
	* @param int $underline (optional, default=null) One of ExcelFont::UNDERLINE_* constants
	* @return int
	*/
	public function underline($underline = null)
	{
	} // underline

} // end ExcelFont
