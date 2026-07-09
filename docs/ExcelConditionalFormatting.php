<?php
/*
  +---------------------------------------------------------------------------+
  | ExcelConditionalFormatting                                                |
  |                                                                           |
  | Reference file for NuSphere PHPEd (and possibly other IDE's) for use with |
  | php_excel interface to libxl by Ilia Alshanetsky <ilia@ilia.ws>           |
  |                                                                           |
  | php_excel "PECL" style module (http://github.com/iliaal/php_excel)        |
  | libxl library (http://www.libxl.com)                                      |
  |                                                                           |
  +---------------------------------------------------------------------------+
*/
class ExcelConditionalFormatting
{
	const CFORMAT_BEGINWITH = 0;
	const CFORMAT_CONTAINSBLANKS = 1;
	const CFORMAT_CONTAINSERRORS = 2;
	const CFORMAT_CONTAINSTEXT = 3;
	const CFORMAT_DUPLICATEVALUES = 4;
	const CFORMAT_ENDSWITH = 5;
	const CFORMAT_EXPRESSION = 6;
	const CFORMAT_NOTCONTAINSBLANKS = 7;
	const CFORMAT_NOTCONTAINSERRORS = 8;
	const CFORMAT_NOTCONTAINSTEXT = 9;
	const CFORMAT_UNIQUEVALUES = 10;

	const CFOPERATOR_LESSTHAN = 0;
	const CFOPERATOR_LESSTHANOREQUAL = 1;
	const CFOPERATOR_EQUAL = 2;
	const CFOPERATOR_NOTEQUAL = 3;
	const CFOPERATOR_GREATERTHANOREQUAL = 4;
	const CFOPERATOR_GREATERTHAN = 5;
	const CFOPERATOR_BETWEEN = 6;
	const CFOPERATOR_NOTBETWEEN = 7;
	const CFOPERATOR_CONTAINSTEXT = 8;
	const CFOPERATOR_NOTCONTAINS = 9;
	const CFOPERATOR_BEGINSWITH = 10;
	const CFOPERATOR_ENDSWITH = 11;

	const CFTP_LAST7DAYS = 0;
	const CFTP_LASTMONTH = 1;
	const CFTP_LASTWEEK = 2;
	const CFTP_NEXTMONTH = 3;
	const CFTP_NEXTWEEK = 4;
	const CFTP_THISMONTH = 5;
	const CFTP_THISWEEK = 6;
	const CFTP_TODAY = 7;
	const CFTP_TOMORROW = 8;
	const CFTP_YESTERDAY = 9;

	const CFVO_MIN = 0;
	const CFVO_MAX = 1;
	const CFVO_FORMULA = 2;
	const CFVO_NUMBER = 3;
	const CFVO_PERCENT = 4;
	const CFVO_PERCENTILE = 5;

	/**
	* Create a conditional formatting object for a sheet
	*
	* @see ExcelSheet::addConditionalFormatting()
	* @param ExcelSheet $sheet
	* @param int $rowFirst (required for libxl 5.1.0+)
	* @param int $rowLast (required for libxl 5.1.0+)
	* @param int $colFirst (required for libxl 5.1.0+)
	* @param int $colLast (required for libxl 5.1.0+)
	*/
	public function __construct(ExcelSheet $sheet, int $rowFirst, int $rowLast, int $colFirst, int $colLast)
	{
	}

	/**
	* Adds a range to the conditional formatting
	*
	* @param int $rowFirst 0-based first row
	* @param int $rowLast 0-based last row
	* @param int $colFirst 0-based first column
	* @param int $colLast 0-based last column
	* @return bool
	*/
	public function addRange(int $rowFirst, int $rowLast, int $colFirst, int $colLast): bool
	{
	}

	/**
	* Adds a rule to the conditional formatting
	*
	* @param int $type One of ExcelConditionalFormatting::CFORMAT_* constants
	* @param ExcelConditionalFormat $cf
	* @param string $value Rule value
	* @param bool $stopIfTrue (optional)
	* @return bool
	*/
	public function addRule(int $type, ExcelConditionalFormat $cf, string $value, bool $stopIfTrue = false): bool
	{
	}

	/**
	* Adds a top/bottom rule
	*
	* @param ExcelConditionalFormat $cf
	* @param int $value Rank value
	* @param bool $bottom false for top, true for bottom
	* @param bool $percent Whether to use percentage
	* @param bool $stopIfTrue (optional)
	* @return bool
	*/
	public function addTopRule(ExcelConditionalFormat $cf, int $value, bool $bottom, bool $percent, bool $stopIfTrue = false): bool
	{
	}

	/**
	* Adds a numeric operator rule
	*
	* @param int $op One of ExcelConditionalFormatting::CFOPERATOR_* constants
	* @param ExcelConditionalFormat $cf
	* @param float $value1 First comparison value
	* @param float $value2 Second comparison value for BETWEEN/NOTBETWEEN
	* @param bool $stopIfTrue (optional)
	* @return bool
	*/
	public function addOpNumRule(int $op, ExcelConditionalFormat $cf, float $value1, float $value2, bool $stopIfTrue = false): bool
	{
	}

	/**
	* Adds a string operator rule
	*
	* @param int $op One of ExcelConditionalFormatting::CFOPERATOR_* constants
	* @param ExcelConditionalFormat $cf
	* @param string $value1 First comparison value
	* @param string $value2 Second comparison value
	* @param bool $stopIfTrue (optional)
	* @return bool
	*/
	public function addOpStrRule(int $op, ExcelConditionalFormat $cf, string $value1, string $value2, bool $stopIfTrue = false): bool
	{
	}

	/**
	* Adds an above/below average rule
	*
	* @param ExcelConditionalFormat $cf
	* @param bool $above Whether to check above average
	* @param bool $equal Whether to include equal values
	* @param int $stdDev Number of standard deviations (0 for none)
	* @param bool $stopIfTrue (optional)
	* @return bool
	*/
	public function addAboveAverageRule(ExcelConditionalFormat $cf, bool $above, bool $equal, int $stdDev, bool $stopIfTrue = false): bool
	{
	}

	/**
	* Adds a time period rule
	*
	* @param ExcelConditionalFormat $cf
	* @param int $timePeriod One of ExcelConditionalFormatting::CFTP_* constants
	* @param bool $stopIfTrue (optional)
	* @return bool
	*/
	public function addTimePeriodRule(ExcelConditionalFormat $cf, int $timePeriod, bool $stopIfTrue = false): bool
	{
	}

	/**
	* Adds a two-color scale rule with numeric values
	*
	* @param int $minColor Color for minimum value
	* @param int $maxColor Color for maximum value
	* @param int $minType One of ExcelConditionalFormatting::CFVO_* constants
	* @param float $minValue Minimum threshold value
	* @param int $maxType One of ExcelConditionalFormatting::CFVO_* constants
	* @param float $maxValue Maximum threshold value
	* @param bool $stopIfTrue (optional)
	* @return bool
	*/
	public function add2ColorScaleRule(int $minColor, int $maxColor, int $minType, float $minValue, int $maxType, float $maxValue, bool $stopIfTrue = false): bool
	{
	}

	/**
	* Adds a two-color scale rule with formula values
	*
	* @param int $minColor Color for minimum value
	* @param int $maxColor Color for maximum value
	* @param int $minType One of ExcelConditionalFormatting::CFVO_* constants
	* @param string $minValue Minimum formula string
	* @param int $maxType One of ExcelConditionalFormatting::CFVO_* constants
	* @param string $maxValue Maximum formula string
	* @param bool $stopIfTrue (optional)
	* @return bool
	*/
	public function add2ColorScaleFormulaRule(int $minColor, int $maxColor, int $minType, string $minValue, int $maxType, string $maxValue, bool $stopIfTrue = false): bool
	{
	}

	/**
	* Adds a three-color scale rule with numeric values
	*
	* @param int $minColor Color for minimum value
	* @param int $midColor Color for midpoint value
	* @param int $maxColor Color for maximum value
	* @param int $minType One of ExcelConditionalFormatting::CFVO_* constants
	* @param float $minValue Minimum threshold value
	* @param int $midType One of ExcelConditionalFormatting::CFVO_* constants
	* @param float $midValue Midpoint threshold value
	* @param int $maxType One of ExcelConditionalFormatting::CFVO_* constants
	* @param float $maxValue Maximum threshold value
	* @param bool $stopIfTrue (optional)
	* @return bool
	*/
	public function add3ColorScaleRule(int $minColor, int $midColor, int $maxColor, int $minType, float $minValue, int $midType, float $midValue, int $maxType, float $maxValue, bool $stopIfTrue = false): bool
	{
	}

	/**
	* Adds a three-color scale rule with formula values
	*
	* @param int $minColor Color for minimum value
	* @param int $midColor Color for midpoint value
	* @param int $maxColor Color for maximum value
	* @param int $minType One of ExcelConditionalFormatting::CFVO_* constants
	* @param string $minValue Minimum formula string
	* @param int $midType One of ExcelConditionalFormatting::CFVO_* constants
	* @param string $midValue Midpoint formula string
	* @param int $maxType One of ExcelConditionalFormatting::CFVO_* constants
	* @param string $maxValue Maximum formula string
	* @param bool $stopIfTrue (optional)
	* @return bool
	*/
	public function add3ColorScaleFormulaRule(int $minColor, int $midColor, int $maxColor, int $minType, string $minValue, int $midType, string $midValue, int $maxType, string $maxValue, bool $stopIfTrue = false): bool
	{
	}

} // end ExcelConditionalFormatting
