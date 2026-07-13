<?php
/*
  +---------------------------------------------------------------------------+
  | ExcelConditionalFormat                                                    |
  |                                                                           |
  | Reference file for NuSphere PHPEd (and possibly other IDE's) for use with |
  | php_excel interface to libxl by Ilia Alshanetsky <ilia@ilia.ws>           |
  |                                                                           |
  | php_excel "PECL" style module (http://github.com/iliaal/php_excel)        |
  | libxl library (http://www.libxl.com)                                      |
  |                                                                           |
  +---------------------------------------------------------------------------+
*/
class ExcelConditionalFormat
{
	/**
	* Create a conditional format within an Excel workbook
	*
	* @see ExcelBook::addConditionalFormat()
	* @param ExcelBook $book
	*/
	public function __construct(ExcelBook $book)
	{
	}

	/**
	* Returns the font for this conditional format
	*
	* @return ExcelFont
	*/
	public function font(): mixed
	{
	}

	/**
	* Returns the number format identifier
	*
	* @return int
	*/
	public function numFormat(): mixed
	{
	}

	/**
	* Sets the number format identifier
	*
	* @param int $value
	* @return bool
	*/
	public function setNumFormat(int $value): bool
	{
	}

	/**
	* Returns the custom number format string
	*
	* @return string
	*/
	public function customNumFormat(): mixed
	{
	}

	/**
	* Sets the custom number format string
	*
	* @param string $value
	* @return bool
	*/
	public function setCustomNumFormat(string $value): bool
	{
	}

	/**
	* Sets the border style on all sides
	*
	* @param int $value One of ExcelFormat::BORDERSTYLE_* constants
	* @return bool
	*/
	public function setBorder(int $value): bool
	{
	}

	/**
	* Sets the border color on all sides
	*
	* @param int $value One of ExcelFormat::COLOR_* constants
	* @return bool
	*/
	public function setBorderColor(int $value): bool
	{
	}

	/**
	* Returns the left border style
	*
	* @return int
	*/
	public function borderLeft(): mixed
	{
	}

	/**
	* Sets the left border style
	*
	* @param int $value One of ExcelFormat::BORDERSTYLE_* constants
	* @return bool
	*/
	public function setBorderLeft(int $value): bool
	{
	}

	/**
	* Returns the right border style
	*
	* @return int
	*/
	public function borderRight(): mixed
	{
	}

	/**
	* Sets the right border style
	*
	* @param int $value One of ExcelFormat::BORDERSTYLE_* constants
	* @return bool
	*/
	public function setBorderRight(int $value): bool
	{
	}

	/**
	* Returns the top border style
	*
	* @return int
	*/
	public function borderTop(): mixed
	{
	}

	/**
	* Sets the top border style
	*
	* @param int $value One of ExcelFormat::BORDERSTYLE_* constants
	* @return bool
	*/
	public function setBorderTop(int $value): bool
	{
	}

	/**
	* Returns the bottom border style
	*
	* @return int
	*/
	public function borderBottom(): mixed
	{
	}

	/**
	* Sets the bottom border style
	*
	* @param int $value One of ExcelFormat::BORDERSTYLE_* constants
	* @return bool
	*/
	public function setBorderBottom(int $value): bool
	{
	}

	/**
	* Returns the left border color
	*
	* @return int
	*/
	public function borderLeftColor(): mixed
	{
	}

	/**
	* Sets the left border color
	*
	* @param int $value One of ExcelFormat::COLOR_* constants
	* @return bool
	*/
	public function setBorderLeftColor(int $value): bool
	{
	}

	/**
	* Returns the right border color
	*
	* @return int
	*/
	public function borderRightColor(): mixed
	{
	}

	/**
	* Sets the right border color
	*
	* @param int $value One of ExcelFormat::COLOR_* constants
	* @return bool
	*/
	public function setBorderRightColor(int $value): bool
	{
	}

	/**
	* Returns the top border color
	*
	* @return int
	*/
	public function borderTopColor(): mixed
	{
	}

	/**
	* Sets the top border color
	*
	* @param int $value One of ExcelFormat::COLOR_* constants
	* @return bool
	*/
	public function setBorderTopColor(int $value): bool
	{
	}

	/**
	* Returns the bottom border color
	*
	* @return int
	*/
	public function borderBottomColor(): mixed
	{
	}

	/**
	* Sets the bottom border color
	*
	* @param int $value One of ExcelFormat::COLOR_* constants
	* @return bool
	*/
	public function setBorderBottomColor(int $value): bool
	{
	}

	/**
	* Returns the fill pattern
	*
	* @return int
	*/
	public function fillPattern(): mixed
	{
	}

	/**
	* Sets the fill pattern
	*
	* @param int $value One of ExcelFormat::FILLPATTERN_* constants
	* @return bool
	*/
	public function setFillPattern(int $value): bool
	{
	}

	/**
	* Returns the pattern foreground color
	*
	* @return int
	*/
	public function patternForegroundColor(): mixed
	{
	}

	/**
	* Sets the pattern foreground color
	*
	* @param int $value One of ExcelFormat::COLOR_* constants
	* @return bool
	*/
	public function setPatternForegroundColor(int $value): bool
	{
	}

	/**
	* Returns the pattern background color
	*
	* @return int
	*/
	public function patternBackgroundColor(): mixed
	{
	}

	/**
	* Sets the pattern background color
	*
	* @param int $value One of ExcelFormat::COLOR_* constants
	* @return bool
	*/
	public function setPatternBackgroundColor(int $value): bool
	{
	}

} // end ExcelConditionalFormat
