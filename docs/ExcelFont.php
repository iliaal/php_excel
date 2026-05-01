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
	const SUBSCRIPT = 1;
	const SUPERSCRIPT = 2;

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
	*/
	final public function __construct(ExcelBook $book)
	{
	}

	/**
	* Get, or set the font size
	*
	* @param ?int $size (optional) Null = getter mode.
	* @return int|false The current font size
	*/
	public function size(?int $size = null): int|false
	{
	}

	/**
	* Get, or set the font name
	*
	* @param ?string $name (optional) Null = getter mode.
	* @return string|false
	*/
	public function name(?string $name = null): string|false
	{
	}

	/**
	* Get, or set the underline style
	*
	* @param ?int $underline_style (optional) One of ExcelFont::UNDERLINE_* constants. Null = getter mode.
	* @return int|false
	*/
	public function underline(?int $underline_style = null): int|false
	{
	}

	/**
	* Get, or set the font script mode
	*
	* @param ?int $mode (optional) One of ExcelFont::NORMAL, ::SUBSCRIPT, or ::SUPERSCRIPT. Null = getter mode.
	* @return int|false
	*/
	public function mode(?int $mode = null): int|false
	{
	}

	/**
	* Get, or set the font color
	*
	* @param ?int $color (optional) One of ExcelFormat::COLOR_* constants. Null = getter mode.
	* @return int|false
	*/
	public function color(?int $color = null): int|false
	{
	}

	/**
	* Get, or set if bold is on or off
	*
	* @param ?bool $bold (optional) Null = getter mode.
	* @return bool
	*/
	public function bold(?bool $bold = null): bool
	{
	}

	/**
	* Get, or set if strike-through is on or off
	*
	* @param ?bool $strike (optional) Null = getter mode.
	* @return bool
	*/
	public function strike(?bool $strike = null): bool
	{
	}

	/**
	* Get, or set if italics are on or off
	*
	* @param ?bool $italics (optional) Null = getter mode.
	* @return bool
	*/
	public function italics(?bool $italics = null): bool
	{
	}

} // end ExcelFont
