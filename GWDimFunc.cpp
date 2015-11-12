#include "GWDimFunc.hpp"
#include <NXSigningResource.cpp>

extern int GW_report_error(char *file, int line, char *call, int irc)
{
	if (irc)
	{
		char err[133], msg[133];

		sprintf(msg, "*** ERROR code %d at line %d in %s:\n+++ ", irc, line, file);
		UF_get_fail_message(irc, err);
		
		UF_print_syslog(msg, FALSE);
		UF_print_syslog(err, FALSE);
		UF_print_syslog("\n", FALSE);
		UF_print_syslog(call, FALSE);
		UF_print_syslog(";\n", FALSE);

		if (!UF_UI_open_listing_window())
		{
			UF_UI_write_listing_window(msg);
			UF_UI_write_listing_window(err);
			UF_UI_write_listing_window("\n");
			UF_UI_write_listing_window(call);
			UF_UI_write_listing_window(";\n");
		}
	}

	return(irc);
}

extern void GW_trim(string &str)
{
	size_t s = str.find_first_not_of(" \t");
	size_t e = str.find_last_not_of(" \t");
	str = str.substr(s, e-s+1);
}

extern NXString GW_ask_dimension_string(Annotations::Dimension *pmiDimension)
{
	NXString	dimensionString, dimensionText, strTolerance;
	int			type, subtype;
	std::vector<NXString>	mainTextLines, dualTextLines, beforeTexts, afterTexts;

	pmiDimension->GetDimensionText(mainTextLines, dualTextLines);
	beforeTexts = pmiDimension->GetAppendedText()->GetBeforeText();
	afterTexts = pmiDimension->GetAppendedText()->GetAfterText();
	for (std::vector<NXString>::iterator beforeTexts_it = beforeTexts.begin(); beforeTexts_it != beforeTexts.end(); beforeTexts_it++)
	{
		dimensionString += *beforeTexts_it;
	}
	for (std::vector<NXString>::iterator dualTextLines_it = dualTextLines.begin(); dualTextLines_it != dualTextLines.end(); dualTextLines_it++)
	{
		dimensionText += *dualTextLines_it;
	}
	for (std::vector<NXString>::iterator mainTextLines_it = mainTextLines.begin(); mainTextLines_it != mainTextLines.end(); mainTextLines_it++)
	{
		dimensionText += *mainTextLines_it;
	}
	UF_CALL(UF_OBJ_ask_type_and_subtype(pmiDimension->GetTag(), &type, &subtype));
	 
	char str_subtype[50];
	sprintf_s(str_subtype,"subtype is %d", subtype);
	UF_print_syslog(str_subtype, FALSE);
	if (UF_dim_cylindrical_subtype == subtype ||
		UF_dim_diameter_subtype == subtype ||
		UF_dim_hole_subtype == subtype)
	{
		dimensionString += GW_ask_diameter_string(pmiDimension, dimensionText);
	}
	else if (UF_dim_radius_subtype == subtype ||
		UF_dim_folded_radius_subtype == subtype)
	{
		dimensionString += GW_ask_radius_string(pmiDimension, dimensionText);
	}
	else if (UF_dim_chamfer_subtype == subtype)
	{
		dimensionString += GW_ask_chamfer_string(pmiDimension, dimensionText);
	}
	else if (UF_dim_angular_minor_subtype == subtype) {
		dimensionString += dimensionText;
		dimensionString += "°";
		UF_print_syslog("has write dushu", true);
	}
	else
	{
		dimensionString += dimensionText;
	}
	dimensionString += GW_ask_tolerance_string(pmiDimension);
	for (std::vector<NXString>::iterator afterTexts_it = afterTexts.begin(); afterTexts_it != afterTexts.end(); afterTexts_it++)
	{
		dimensionString += *afterTexts_it;
	}
	return dimensionString;
}

extern NXString GW_ask_diameter_string(Annotations::Dimension *pmiDimension, NXString dimensionText)
{
	NXString radiusString;

	Annotations::DimensionPreferences *dimensionPreferences = pmiDimension->GetDimensionPreferences();
	Annotations::DiameterRadiusPreferences *diameterRadiusPreferences = dimensionPreferences->GetDiameterRadiusPreferences();
	Annotations::DiameterSymbol diameterSymbol = diameterRadiusPreferences->DiameterSymbol();
	NXString diameterSymbolText = diameterRadiusPreferences->DiameterSymbolText();
	switch (diameterSymbol)
	{
	case Annotations::DiameterSymbolDia:			/** Specifies DIA symbol */
		diameterSymbolText = "Φ";
		break;
	case Annotations::DiameterSymbolStandard:		/** Specifies standard symbol */
		diameterSymbolText = "Φ";
		break;
	case Annotations::DiameterSymbolUserDefined:	/** Specifies user defined symbol */
		break;
	case Annotations::DiameterSymbolSpherical:		/** Specifies spherical symbol */
		break;
	case Annotations::DiameterSymbolLast:			/** Last enum value. Should not be used. */
		break;
	default:
		break;
	};

	Annotations::DiameterRadiusSymbolPlacement symbolPlacement = diameterRadiusPreferences->SymbolPlacement();
	switch (symbolPlacement)
	{
	case Annotations::DiameterRadiusSymbolPlacementBelow:	/** Specifies diameter/radius symbol is placed below dimension text */
		radiusString += diameterSymbolText;
		radiusString += dimensionText;
		break;
	case Annotations::DiameterRadiusSymbolPlacementAbove:	/** Specifies diameter/radius symbol is placed above dimension text */
		radiusString += diameterSymbolText;
		radiusString += dimensionText;
		break;
	case Annotations::DiameterRadiusSymbolPlacementAfter:	/** Specifies diameter/radius symbol is placed after dimension text */
		radiusString += dimensionText;
		radiusString += diameterSymbolText;
		break;
	case Annotations::DiameterRadiusSymbolPlacementBefore:	/** Specifies diameter/radius symbol is placed before dimension text */
		radiusString += diameterSymbolText;
		radiusString += dimensionText;
		break;
	case Annotations::DiameterRadiusSymbolPlacementOmit:	/** Specifies diameter/radius symbol is omitted */
		radiusString = dimensionText;
		break;
	case Annotations::DiameterRadiusSymbolPlacementLast:	/** Last enum value. Should not be used. */
		break;
	default:
		radiusString = dimensionText;
		break;
	}

	delete diameterRadiusPreferences;
	delete dimensionPreferences;
	return radiusString;
}

extern NXString GW_ask_radius_string(Annotations::Dimension *pmiDimension, NXString dimensionText)
{
	NXString radiusString;

	Annotations::DimensionPreferences *dimensionPreferences = pmiDimension->GetDimensionPreferences();
	Annotations::DiameterRadiusPreferences *diameterRadiusPreferences = dimensionPreferences->GetDiameterRadiusPreferences();
	Annotations::RadiusSymbol radiusSymbol = diameterRadiusPreferences->RadiusSymbol();
	NXString radiusSymbolText = diameterRadiusPreferences->RadiusSymbolText();
	switch (radiusSymbol)
	{
	case Annotations::RadiusSymbolR:			/** Specifies R symbol */
		radiusSymbolText = "R";
		break;
	case Annotations::RadiusSymbolRAD:			/** Specfies RAD symbol */
		radiusSymbolText = "RAD";
		break;
	case Annotations::RadiusSymbolUserDefined:	/** Specfies user defined symbol */
		break;
	case Annotations::RadiusSymbolSR:			/** Specifes SR symbol */
		radiusSymbolText = "SR";
		break;
	case Annotations::RadiusSymbolCR:			/** Specifies CR symbol */
		radiusSymbolText = "CR";
		break;
	case Annotations::RadiusSymbolLast:			/** Last enum value. Should not be used. */
		break;
	default:
		break;
	};
	Annotations::DiameterRadiusSymbolPlacement symbolPlacement = diameterRadiusPreferences->SymbolPlacement();
	switch (symbolPlacement)
	{
	case Annotations::DiameterRadiusSymbolPlacementBelow:	/** Specifies diameter/radius symbol is placed below dimension text */
		radiusString += radiusSymbolText;
		radiusString += dimensionText;
		break;
	case Annotations::DiameterRadiusSymbolPlacementAbove:	/** Specifies diameter/radius symbol is placed above dimension text */
		radiusString += radiusSymbolText;
		radiusString += dimensionText;
		break;
	case Annotations::DiameterRadiusSymbolPlacementAfter:	/** Specifies diameter/radius symbol is placed after dimension text */
		radiusString += dimensionText;
		radiusString += radiusSymbolText;
		break;
	case Annotations::DiameterRadiusSymbolPlacementBefore:	/** Specifies diameter/radius symbol is placed before dimension text */
		radiusString += radiusSymbolText;
		radiusString += dimensionText;
		break;
	case Annotations::DiameterRadiusSymbolPlacementOmit:	/** Specifies diameter/radius symbol is omitted */
		radiusString = dimensionText;
		break;
	case Annotations::DiameterRadiusSymbolPlacementLast:	/** Last enum value. Should not be used. */
		break;
	default:
		radiusString = dimensionText;
		break;
	}

	delete diameterRadiusPreferences;
	delete dimensionPreferences;
	return radiusString;
}

extern NXString GW_ask_chamfer_string(Annotations::Dimension *pmiDimension, NXString dimensionText)
{
	NXString chamferString;

	Annotations::DimensionPreferences *dimensionPreferences = pmiDimension->GetDimensionPreferences();
	Annotations::ChamferDimensionPreferences *chamferDimensionPreferences = dimensionPreferences->GetChamferDimensionPreferences();

	Annotations::ChamferForm chamferForm = chamferDimensionPreferences->Form();
	switch(chamferForm)
	{
	case Annotations::ChamferFormSymbol:	/** C5 */
		chamferString += "C";
		chamferString += dimensionText;
		break;
	case Annotations::ChamferFormSize:		/** 5 x 5 */
		chamferString += dimensionText;
		chamferString += "×";
		chamferString += dimensionText;
		break;
	case Annotations::ChamferFormSizeAngle:	/** 5 x 45 */
		chamferString += dimensionText;
		chamferString += "×45°";
		break;
	case Annotations::ChamferFormAngleSize:	/** 45 x 5 */
		chamferString += "45°×";
		chamferString += dimensionText;
		break;
	case Annotations::ChamferFormLast:		/** Last enum value. Should not be used. */
		break;
	default:
		chamferString = dimensionText;
		break;
	};

	delete chamferDimensionPreferences;
	delete dimensionPreferences;
	return chamferString;
}

extern NXString GW_ask_tolerance_string(Annotations::Dimension *pmiDimension)
{
	NXString	strTolerance;
	char	szUpperValue[256], szLowerValue[256], szAbsUpperValue[256];
	double	dUpperValue = 0.0, dLowerValue = 0.0;

	strTolerance = "";
	Annotations::LinearTolerance *linearTolerance4;
	Annotations::DimensionPreferences *dimensionPreferences = pmiDimension->GetDimensionPreferences();
	Annotations::UnitsFormatPreferences *unitsFormatPreferences = dimensionPreferences->GetUnitsFormatPreferences();
	NXOpen::Annotations::DimensionUnit primaryDimensionUnit = unitsFormatPreferences->PrimaryDimensionUnit();
	if (NXOpen::Annotations::DimensionUnitMillimeters == primaryDimensionUnit ||
		NXOpen::Annotations::DimensionUnitMeters == primaryDimensionUnit)
	{
		dUpperValue = pmiDimension->UpperMetricToleranceValue();
		dLowerValue = pmiDimension->LowerMetricToleranceValue();
	}
	else
	{
		dUpperValue = pmiDimension->UpperToleranceValue();
		dLowerValue = pmiDimension->LowerToleranceValue();
	}
	delete unitsFormatPreferences;
	delete dimensionPreferences;
	char dUpperValuebuffer[100];
	char dLowerValuebuffer[100];
	char returndLowerValuebuffer[100];
	sprintf(dUpperValuebuffer, "dUpperValue is  %lf ", dUpperValue);
	sprintf(dLowerValuebuffer, "dLowerValuebuffer is  %lf ", dLowerValue);
	UF_print_syslog(dUpperValuebuffer, true);
	UF_print_syslog(dLowerValuebuffer, true);
	sprintf_s(returndLowerValuebuffer, "%.6lf", fabs(dLowerValue));
//	sprintf(returndLowerValuebuffer, "+%.6lf", dLowerValue);
	GW_trim_string_right(returndLowerValuebuffer, '0');
	GW_trim_string_right(returndLowerValuebuffer, '.');

	if (dUpperValue > TOLERANCE) 
		sprintf(szUpperValue, "+%.6lf", dUpperValue);
	 
	else
		sprintf(szUpperValue, "%.6lf", dUpperValue);

	 

	GW_trim_string_right(szUpperValue, '0');
	GW_trim_string_right(szUpperValue, '.');
	if (dLowerValue > TOLERANCE)
		sprintf(szLowerValue, "+%.6lf", dLowerValue);
	else
		sprintf(szLowerValue, "%.6lf", dLowerValue);
	GW_trim_string_right(szLowerValue, '0');
	GW_trim_string_right(szLowerValue, '.');
	sprintf(szAbsUpperValue, "%.6lf", fabs(dUpperValue));
	GW_trim_string_right(szAbsUpperValue, '0');
	GW_trim_string_right(szAbsUpperValue, '.');
	UF_print_syslog(returndLowerValuebuffer, true);
	Annotations::ToleranceType toleranceType = pmiDimension->ToleranceType();
	switch (toleranceType)
	{
	case Annotations::ToleranceTypeNone:				// No tolerance
		break;
	case Annotations::ToleranceTypeLimitOneLine:		// -LIM +LIM
		strTolerance += "(";
		strTolerance += szUpperValue;
		strTolerance += "/";
		strTolerance += szLowerValue;
		strTolerance += ")";
		UF_print_syslog("is ToleranceTypeLimitOneLine", true);
		break;
	case Annotations::ToleranceTypeLimitTwoLines:		// +LIM <br> -LIM
		strTolerance += "(";
		strTolerance += szUpperValue;
		strTolerance += "/";
		strTolerance += szLowerValue;
		strTolerance += ")";
		UF_print_syslog("is ToleranceTypeLimitTwoLines", true);
		break;
	case Annotations::ToleranceTypeBilateralOneLine:	// +-TOL
		strTolerance += "±";
		strTolerance += szAbsUpperValue;
		UF_print_syslog("is ToleranceTypeBilateralOneLine", true);
		break;
	case Annotations::ToleranceTypeBilateralTwoLines:	// +TOL <br> -TOL
		strTolerance += "(";
		strTolerance += szUpperValue;
		strTolerance += "/";
		strTolerance += szLowerValue;
		strTolerance += ")";
		UF_print_syslog("is ToleranceTypeBilateralTwoLines", true);
		break;
	case Annotations::ToleranceTypeUnilateralAbove:		// +TOL <br> -.000
		strTolerance += "(+";
		strTolerance += szAbsUpperValue;
		strTolerance += "/0)";
		UF_print_syslog("is ToleranceTypeUnilateralAbove", true);
		break;
	case Annotations::ToleranceTypeUnilateralBelow:		// +.000 <br> -TOL
		strTolerance += "(0";
		strTolerance += "/-";
	//	strTolerance += szAbsUpperValue;
		strTolerance += returndLowerValuebuffer;
		strTolerance += ")";
		UF_print_syslog("is ToleranceTypeUnilateralBelow", true);
		break;
	case Annotations::ToleranceTypeBasic:				// |xxx|
		UF_print_syslog("is ToleranceTypeBasic", true);
		break;
	case Annotations::ToleranceTypeReference:			// (xxx)
		UF_print_syslog("is ToleranceTypeReference", true);
		break;
	case Annotations::ToleranceTypeLimitLargerFirst:	// +LIM -LIM
		strTolerance += "(";
		strTolerance += szUpperValue;
		strTolerance += "/";
		strTolerance += szLowerValue;
		strTolerance += ")";
		UF_print_syslog("is ToleranceTypeLimitLargerFirst", true);
		break;
	case Annotations::ToleranceTypeLimitLargerBelow:	// -LIM <br> +LIM 
		strTolerance += "(";
		strTolerance += szLowerValue;
		strTolerance += "/";
		strTolerance += szUpperValue;
		strTolerance += ")";

		UF_print_syslog("is ToleranceTypeLimitLargerBelow", true);
		break;
	case Annotations::ToleranceTypeLimitsAndFits:		// A5
		strTolerance += "(";
		strTolerance += szUpperValue;
		strTolerance += "/";
		strTolerance += szLowerValue;
		strTolerance += ")";
		//{
		//	Annotations::FitDisplayStyle fitDisplayStyle = dimensionPreferences->LimitFitDisplayStyle();
		//	switch (fitDisplayStyle)
		//	{
		//	case Annotations::FitDisplayStyleFitSymbols:				/** Display only the grade and deviation */
		//		break;
		//	case Annotations::FitDisplayStyleFitSymbolsAndLimits:		/** Display grade, deviation, and limit values */
		//		break;
		//	case Annotations::FitDisplayStyleFitSymbolsAndTolerances:	/** Display grade, deviation, and tolerance values */
		//		break;
		//	case Annotations::FitDisplayStyleTolerancesOnly:			/** Display the tolerance values only */
		//		break;
		//	};
		//}
		UF_print_syslog("is ToleranceTypeLimitsAndFits", true);
		break;
	case Annotations::ToleranceTypeNotToScale:			// xxx <br> ---
		UF_print_syslog("is ToleranceTypeNotToScale", true);
		break;
	case Annotations::ToleranceTypeDiameterReference:	// (DIA xxx)
		UF_print_syslog("is ToleranceTypeDiameterReference", true);
		break;
	}
	const char* tem_str = strTolerance.GetLocaleText();
	char temp_buffer_str[100];
	strcpy(temp_buffer_str, tem_str);
	UF_print_syslog(temp_buffer_str, true);
	return strTolerance;
}

extern bool GW_ask_tolerance_value(Annotations::Dimension *pmiDimension, double *dUpperValue, double *dLowerValue)
{
	*dUpperValue = 0.0;
	*dLowerValue = 0.0;
	Annotations::DimensionPreferences *dimensionPreferences = pmiDimension->GetDimensionPreferences();
	Annotations::UnitsFormatPreferences *unitsFormatPreferences = dimensionPreferences->GetUnitsFormatPreferences();
	NXOpen::Annotations::DimensionUnit primaryDimensionUnit = unitsFormatPreferences->PrimaryDimensionUnit();
	if (NXOpen::Annotations::DimensionUnitMillimeters == primaryDimensionUnit ||
		NXOpen::Annotations::DimensionUnitMeters == primaryDimensionUnit)
	{
		*dUpperValue = pmiDimension->UpperMetricToleranceValue();
		*dLowerValue = pmiDimension->LowerMetricToleranceValue();
	}
	else
	{
		*dUpperValue = pmiDimension->UpperToleranceValue();
		*dLowerValue = pmiDimension->LowerToleranceValue();
	}
	delete unitsFormatPreferences;
	delete dimensionPreferences;

	std::vector<NXString>	mainTextLines, dualTextLines;
	pmiDimension->GetDimensionText(mainTextLines, dualTextLines);
	if (0 == mainTextLines.size())
		return false;
	double dimensionValue = atof(mainTextLines.front().GetLocaleText());
	if (dimensionValue < *dUpperValue-*dLowerValue)
		return false;

	switch (pmiDimension->ToleranceType())
	{
	case Annotations::ToleranceTypeNone:				// No tolerance
		*dUpperValue = 0.0;
		*dLowerValue = 0.0;
		return false;
	case Annotations::ToleranceTypeLimitOneLine:		// -LIM +LIM
		return true;
	case Annotations::ToleranceTypeLimitTwoLines:		// +LIM <br> -LIM
		return true;
	case Annotations::ToleranceTypeBilateralOneLine:	// +-TOL
		*dLowerValue = -(*dUpperValue);
		return true;
	case Annotations::ToleranceTypeBilateralTwoLines:	// +TOL <br> -TOL
		return true;
	case Annotations::ToleranceTypeUnilateralAbove:		// +TOL <br> -.000
		*dLowerValue = 0.0;
		return true;
	case Annotations::ToleranceTypeUnilateralBelow:		// +.000 <br> -TOL
		*dUpperValue = 0.0;
		return true;
	case Annotations::ToleranceTypeBasic:				// |xxx|
		*dUpperValue = 0.0;
		*dLowerValue = 0.0;
		return false;
	case Annotations::ToleranceTypeReference:			// (xxx)
		*dUpperValue = 0.0;
		*dLowerValue = 0.0;
		return false;
	case Annotations::ToleranceTypeLimitLargerFirst:	// +LIM -LIM
		return true;
	case Annotations::ToleranceTypeLimitLargerBelow:	// -LIM <br> +LIM 
		*dUpperValue = -(*dUpperValue);
		*dLowerValue = -(*dLowerValue);
		return true;
	case Annotations::ToleranceTypeLimitsAndFits:		// A5
		*dUpperValue = 0.0;
		*dLowerValue = 0.0;
		return false;
	case Annotations::ToleranceTypeNotToScale:			// xxx <br> ---
		*dUpperValue = 0.0;
		*dLowerValue = 0.0;
		return false;
	case Annotations::ToleranceTypeDiameterReference:	// (DIA xxx)
		*dUpperValue = 0.0;
		*dLowerValue = 0.0;
		return false;
	default:
		*dUpperValue = 0.0;
		*dLowerValue = 0.0;
		return false;
	}
	return false;
}

extern bool GW_set_tolerance_value(Annotations::Dimension *pmiDimension, double dUpperValue, double dLowerValue)
{
	int		mpi[100];
	double	mpr[70];
	char	radius_val[28], diameter_val[28];

	UF_CALL(UF_DRF_ask_object_preferences(pmiDimension->GetTag(), mpi, mpr, radius_val, diameter_val));
	Annotations::DimensionPreferences *dimensionPreferences = pmiDimension->GetDimensionPreferences();
	Annotations::UnitsFormatPreferences *unitsFormatPreferences = dimensionPreferences->GetUnitsFormatPreferences();
	NXOpen::Annotations::DimensionUnit primaryDimensionUnit = unitsFormatPreferences->PrimaryDimensionUnit();
	if (NXOpen::Annotations::DimensionUnitMillimeters == primaryDimensionUnit ||
		NXOpen::Annotations::DimensionUnitMeters == primaryDimensionUnit)
	{
		mpr[24] = dUpperValue;
		mpr[25] = dLowerValue;
	}
	else
	{
		mpr[1] = dUpperValue;
		mpr[2] = dUpperValue;
	}
	if (1 == mpi[6])
	{
		mpi[6] = 5;
	}
	else if (fabs(dUpperValue+dLowerValue) < 0.0001)
	{
		if (fabs(dUpperValue) > 0.0001)
			mpi[6] = 4;
		else
			mpi[6] = 5;
	}
	else if (fabs(dUpperValue+dLowerValue) > 0.0001 && 4 == mpi[6])
	{
		if (4 == mpi[6] || 6 == mpi[6] || 7 == mpi[6])
			mpi[6] = 5;
	}

	if (fabs(dUpperValue) < 0.0001 && fabs(dLowerValue) > 0.0001)
	{
		mpi[6] = 7;
	}
	else if (fabs(dLowerValue) < 0.0001 && fabs(dUpperValue) > 0.0001)
	{
		mpi[6] = 6;
	}
	UF_CALL(UF_DRF_set_object_preferences(pmiDimension->GetTag(), mpi, mpr, radius_val, diameter_val));
	return true;
}

extern bool GW_is_find_attribute(NXOpen::NXObject *object, NXString title, NXOpen::NXObject::AttributeType type)
{
	UF_print_syslog("qqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqq11111", false);
	std::vector<NXOpen::NXObject::AttributeInformation> attrInforVector;
	UF_print_syslog("qqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqq22222222", false);
	attrInforVector = object->GetUserAttributes();
	//attrInforVector = object->GetAttributeTitlesByType(type);
	UF_print_syslog("qqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqq33333333", false);
	for (std::vector<NXOpen::NXObject::AttributeInformation>::iterator attrInforVector_it=attrInforVector.begin(); attrInforVector_it!=attrInforVector.end(); attrInforVector_it++)
	{
		UF_print_syslog("qqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqq44444", false);
		if (0 == strcmp(title.GetLocaleText(), attrInforVector_it->Title.GetLocaleText()))
		{
			UF_print_syslog("qqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqq555555", false);
			if (NXOpen::NXObject::AttributeTypeAny == type ||
				attrInforVector_it->Type == type)
			{
				UF_print_syslog("qqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqq66666666", false);
				return true;
			}
		}
	}
	UF_print_syslog("qqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqq77777777777", false);
	return false;
}

extern Annotations::Pmi *GW_ask_annotation_pmi(Annotations::Annotation *annotation)
{
	Session *theSession = Session::GetSession();
	UI *theUI = UI::GetUI();

	try
	{
		Part *sourcePart = dynamic_cast<Part*>(annotation->OwningPart());
		return sourcePart->PmiManager()->GetDisplayInstanceParent(annotation);
	}
	catch (std::exception& ex)
	{
		theUI->NXMessageBox()->Show("Block Styler", NXOpen::NXMessageBox::DialogTypeError, ex.what());
	}
	return NULL;
}

extern bool GW_is_same_double(double data1, double data2)
{
	double sub_value = data1 - data2;
	if (-TOLERANCE < sub_value && TOLERANCE > sub_value)
		return true;
	else
		return false;
}

extern void GW_trim_tail_zero(char *number_str)
{
	char *pdest = strrchr(number_str, '.');
	if (NULL == pdest)
		return;
	size_t result = pdest - number_str + 1;
	size_t str_lenth = strlen(number_str);
	for (size_t i=str_lenth-1; i>result-2; i--)
	{
		if ('0' != number_str[i] && '.' != number_str[i])
			return;
		else
			number_str[i] = '\0';
	}
}

extern int GW_split_strings(const char *szInput, const char *szSplit, std::vector<NXString> &stringArray)
{
	stringArray.clear();
	if (0 == strlen(szInput) || 0 == strlen(szSplit))
		return 0;
	std::string strInput = szInput;
	strInput += szSplit;
	std::string::size_type size = strInput.size();
	for (std::string::size_type i=0; i<size; i++)
	{
		std::string::size_type pos = strInput.find(szSplit, i);
		if (pos < size)
		{
			std::string strTemp = strInput.substr(i, pos-i);
			stringArray.push_back(strTemp.c_str());
			i = pos + strlen(szSplit)-1;
		}
	}
	return (int)(stringArray.size());
}

extern void GW_trim_string_right(char *szString, char cChar)
{
	size_t str_length = strlen(szString);
	for (size_t i=str_length-1; i>=0; i--)
	{
		if (szString[i] != cChar)
			return;
		szString[i] = '\0';
	}
}

extern NXString GW_ask_annotation_name(Annotations::Annotation *pmiAnnotation)
{
	std::vector<NXString>	mainTextLines, dualTextLines;
	NXString annotationName;

	Annotations::Dimension *pmiDimension = dynamic_cast<Annotations::Dimension *>(pmiAnnotation);
	if (pmiDimension)
	{
		pmiDimension->GetDimensionText(mainTextLines, dualTextLines);
		for (std::vector<NXString>::iterator mainTextLines_it = mainTextLines.begin(); mainTextLines_it != mainTextLines.end(); mainTextLines_it++)
		{
			annotationName += *mainTextLines_it;
		}
		return annotationName;
	}
	Annotations::SurfaceFinish *surfaceFinish = dynamic_cast<Annotations::SurfaceFinish *>(pmiAnnotation);
	if (surfaceFinish)
	{
		annotationName = "Surface Finish";
		return annotationName;
	}
	Annotations::Fcf *fcf = dynamic_cast<Annotations::Fcf *>(pmiAnnotation);
	if (fcf)
	{
		annotationName = "Feature Control Frame";
		return annotationName;
	}else{
		char text[50] ;
		DRAFT_ask_note_value(pmiAnnotation->GetTag(), text);
		UF_print_syslog(text,true);
		annotationName +=text;
	}
	return annotationName;
}

extern tag_t GW_ugmgr_create_part(char *part_number, char *part_revision, char *part_file_type, char* part_file_name)
{
	tag_t	part_tag = NULL_TAG;
	char	encoded_name[MAX_FSPEC_SIZE+1];

	if (UF_CALL(UF_UGMGR_encode_part_filename(part_number, part_revision, part_file_type, part_file_name, encoded_name)))
		return NULL_TAG;
	if (UF_CALL(UF_PART_new(encoded_name, 1, &part_tag)))
		return NULL_TAG;
	return part_tag;
}

extern tag_t GW_ugmgr_create_component_part(tag_t parent_part, char *part_number, char *part_revision, char *part_file_type, char* part_file_name, const char* part_type)
{
	tag_t	instance;
	char	encoded_name[MAX_FSPEC_SIZE+1];
	double	origin[3] = {0.0, 0.0, 0.0}, csys_matrix[6] = {1.0, 0.0, 0.0, 0.0, 1.0, 0.0};

#ifdef DEBUG
	UF_CALL(UF_ASSEM_create_component_part(parent_part, part_number, "", "", 1, -1, origin, csys_matrix, 0, NULL, &instance));
#else
	if (UF_CALL(UF_UGMGR_encode_part_filename(part_number, part_revision, part_file_type, part_file_name, encoded_name)))
		return NULL_TAG;
	if (NULL == part_type || 0 == strlen(part_type))
	{
		if (UF_CALL(UF_ASSEM_create_component_part(parent_part, encoded_name, "", "", 1, -1, origin, csys_matrix, 0, NULL, &instance)))
			return NULL_TAG;
	}
	else
	{
		if (UF_CALL(UF_UGMGR_create_component_part(parent_part, encoded_name, "", "", 1, -1, origin, csys_matrix, 0, NULL, part_type, &instance)))
			return NULL_TAG;
	}
#endif // DEBUG

	return UF_ASSEM_ask_child_of_instance(instance);
}

static UF_UGMGR_tag_t find_folder_in_folder(UF_UGMGR_tag_t parent_folder, const char *find_name)
{
	UF_UGMGR_tag_t			find_folder = NULL_TAG, *folder_contents = NULL;
	char					folder_name[UF_UGMGR_NAME_SIZE];
	int						i, count;
	UF_UGMGR_object_type_t	object_type;

	UF_CALL(UF_UGMGR_list_folder_contents(parent_folder, &count, &folder_contents));
	for (i=0; i<count; i++)
	{
		UF_CALL(UF_UGMGR_ask_object_type(folder_contents[i], &object_type));
		if (UF_UGMGR_type_folder == object_type)
		{
			UF_CALL(UF_UGMGR_ask_folder_name(folder_contents[i], folder_name));
			if (strcmp(folder_name, find_name) == 0 )
			{
				find_folder = folder_contents[i];
				break;
			}
		}
	}
	UF_free(folder_contents);
	return find_folder;
}

extern UF_UGMGR_tag_t GW_ugmgr_find_folder(const std::vector<std::string> &folderPath)
{
	UF_UGMGR_tag_t parent_folder, find_folder = NULL_TAG;

	UF_CALL(UF_UGMGR_ask_root_folder(&find_folder));
	parent_folder = find_folder;
	for (std::vector<std::string>::const_iterator folderPath_cit=folderPath.begin(); folderPath_cit!=folderPath.end(); folderPath_cit++)
	{
		find_folder = find_folder_in_folder(parent_folder, folderPath_cit->c_str());
		if (NULL_TAG == find_folder)
			return parent_folder;
		parent_folder = find_folder;
	}
	return find_folder;
}

extern void GW_cycle_components(tag_t occ_parent, std::vector<tag_t> &occArray)
{
	tag_t	*atOccChildren;

	if (NULL_TAG == occ_parent)
		return;
	int nPartNum = UF_ASSEM_ask_part_occ_children(occ_parent, &atOccChildren);
	for (int i=0; i<nPartNum; i++)
	{
		occArray.push_back(atOccChildren[i]);
		GW_cycle_components(atOccChildren[i], occArray);
	}
	UF_free(atOccChildren);
}

extern std::string GW_ask_component_name(tag_t occ_part)
{
	char	refset_name[MAX_ENTITY_NAME_SIZE+1], instance_name[UF_CFI_MAX_FILE_NAME_SIZE];
	double	origin[3], csys_matrix[9], transform[4][4];
	char	sPartPath[_MAX_PATH], sPartName[_MAX_FNAME];
	std::string strCompName;

	UF_CALL(UF_ASSEM_ask_component_data(occ_part, sPartPath, refset_name, instance_name, origin, csys_matrix, transform));
	uc4574(sPartPath, 2, sPartName);
	strCompName = sPartName;
	return strCompName;
}

extern bool GW_is_in_object_array(const std::vector<tag_t> objectArray, tag_t object_tag)
{
	for (std::vector<tag_t>::const_iterator objectArray_cit=objectArray.begin(); objectArray_cit!=objectArray.end(); objectArray_cit++)
	{
		if (object_tag == *objectArray_cit)
		{
			return true;
		}
	}
	return false;
}

extern bool GW_is_in_string_array(const std::vector<NXString> stringArray, NXString strInput)
{
	for (std::vector<NXString>::const_iterator stringArray_cit=stringArray.begin(); stringArray_cit!=stringArray.end(); stringArray_cit++)
	{
		if (0 == strcmp(strInput.GetLocaleText(), stringArray_cit->GetLocaleText()))
		{
			return true;
		}
	}
	return false;
}

extern int GW_ask_parts_in_assembly(std::vector<tag_t> &partArray)
{
	std::vector<tag_t> occArray;
	partArray.clear();
	tag_t occ_root = UF_ASSEM_ask_root_part_occ(UF_PART_ask_display_part());
	GW_cycle_components(occ_root, occArray);
	for (std::vector<tag_t>::const_iterator occArray_cit=occArray.begin(); occArray_cit!=occArray.end(); occArray_cit++)
	{
		tag_t part_tag = UF_ASSEM_ask_prototype_of_occ(*occArray_cit);
		if (!GW_is_in_object_array(partArray, part_tag))
		{
			partArray.push_back(part_tag);
		}
	}
	return (int)(partArray.size());
}

extern int GW_ask_parts_in_session(std::vector<tag_t> &partArray)
{
	partArray.clear();
	int nPartNum = UF_PART_ask_num_parts();
	for (int i=0; i<nPartNum; i++)
	{
		tag_t tProPart = UF_PART_ask_nth_part(i);
		partArray.push_back(tProPart);
	}
	return (int)(partArray.size());
}

extern int GW_ask_part_names_in_session(std::vector<NXString> &nameArray)
{
	std::vector<tag_t> partArray;
	char	part_name[MAX_FSPEC_SIZE+1];

	nameArray.clear();
	GW_ask_parts_in_session(partArray);
	for (std::vector<tag_t>::iterator partArray_it = partArray.begin(); partArray_it != partArray.end(); partArray_it++)
	{
		GW_ask_part_name(*partArray_it, part_name);
		nameArray.push_back(part_name);
	}
	return (int)(nameArray.size());
}

extern void GW_set_object_attribute(tag_t object_tag, const char *szTitle, const char *szValue)
{
	UF_ATTR_value_t	value;

	if (0 == strlen(szTitle) || strlen(szTitle) > UF_ATTR_MAX_TITLE_LEN || strlen(szValue) > UF_ATTR_MAX_STRING_LEN)
		return;
	value.type = UF_ATTR_string;
	value.value.string = (char*)szValue;
	UF_CALL(UF_ATTR_assign(object_tag, (char*)szTitle, value));
}

extern bool GW_ask_object_attribute(tag_t object_tag, const char *szTitle, char *szValue)
{
	UF_ATTR_value_t	value;
	int		title_type;

	UF_ATTR_find_attribute (object_tag, UF_ATTR_any, (char*)szTitle, &title_type);
	if (UF_ATTR_string != title_type)
		return false;
	UF_CALL(UF_ATTR_read_value (object_tag, (char*)szTitle, UF_ATTR_any, &value));
	if (UF_ATTR_string != value.type || NULL == value.value.string)
		return false;
	strcpy(szValue, value.value.string);
	UF_free(value.value.string);
	return true;
}

extern bool GW_delete_object_attribute(tag_t object_tag, const char *szTitle)
{
	UF_ATTR_value_t	value;
	int		title_type;

	UF_ATTR_find_attribute (object_tag, UF_ATTR_any, (char*)szTitle, &title_type);
	if (UF_ATTR_string != title_type)
		return false;
	if (UF_ATTR_delete(object_tag, UF_ATTR_string, (char*)szTitle))
		return false;
	return true;
}

extern tag_t GW_load_part(tag_t part_tag)
{
	UF_PART_load_status_t	load_status;
	char	part_name[MAX_FSPEC_SIZE+1];

	if (part_tag)
	{
		UF_PART_ask_part_name(part_tag, part_name);
		if (1 != UF_PART_is_loaded(part_name))
		{
			UF_CALL(UF_PART_open_quiet(part_name, &part_tag, &load_status));
			UF_PART_free_load_status(&load_status);
		}
	}
	return part_tag;
}

extern bool GW_ask_new_part_number(std::vector<NXString> &nameArray, char *part_number)
{
	UF_UGMGR_tag_t	database_part_tag = NULL_TAG;
	char	part_name[MAX_FSPEC_SIZE+1];
	NXString	strPartNumber;

	tag_t work_part = UF_ASSEM_ask_work_part();
	GW_ask_part_name(work_part, part_name);
	std::string strInteger = "1";
	strPartNumber = part_name;
	strPartNumber += "-";
	strPartNumber += strInteger.c_str();
	while (GW_is_in_string_array(nameArray, strPartNumber))
	{
		nameArray.push_back(strPartNumber);
		GW_carry_over_integer(strInteger);
		strPartNumber = part_name;
		strPartNumber += "-";
		strPartNumber += strInteger.c_str();
	}

#ifndef DEBUG
	while ((0 == UF_UGMGR_ask_part_tag((char*)(strPartNumber.GetLocaleText()), &database_part_tag)) && database_part_tag)
	{
		nameArray.push_back(strPartNumber);
		GW_carry_over_integer(strInteger);
		strPartNumber = part_name;
		strPartNumber += "-";
		strPartNumber += strInteger.c_str();
	}
#endif // _DEBUG

	strcpy(part_number, strPartNumber.GetLocaleText());
	return true;
}

extern bool GW_ask_part_name(tag_t part_tag, char *part_name)
{
	char	sPartPath[_MAX_PATH];

	if (NULL_TAG == part_tag)
		return false;
	if (UF_PART_ask_part_name(part_tag, sPartPath))
		return FALSE;
	uc4574(sPartPath, 2, part_name);
	return true;
}

extern int GW_ask_tail_number_string(const char *szInput, char *sTailNum)
{
	std::string	strTailNum;
	int nLength = (int)(strlen(szInput));
	for (int i=nLength-1; i>=0; i--)
	{
		if (szInput[i] >= 0x30 && szInput[i] <= 0x39)
			strTailNum += szInput[i];
		else
			break;
	}
	strcpy(sTailNum, strrev((char*)(strTailNum.c_str())));
	return (int)(strlen(sTailNum));
}

extern bool GW_carry_over_integer(std::string &strInteger)
{
	char	cChar;

	int nLength = int(strInteger.length());
	if (0 == nLength)
	{
		strInteger = "1";
		return true;
	}
	for (int i=nLength-1; i>=0; i--)
	{
		cChar = strInteger[i];
		if (!isdigit(cChar))
			return false;
		if ('9' == cChar)
		{
			strInteger[i] = '0';
		}
		else
		{
			strInteger[i] = cChar+1;
			return true;
		}
	}
	strInteger[0] = '1';
	strInteger += '0';
	return TRUE;
}
int DRAFT_ask_dimension_text1(tag_t dim_tag, char *text)
{
	int i;
	int subtype; 
	int mpi[100];
	double mpr[70]; 
	char rad[27];
	char dia[27];    
	double origin[ 3 ];  
	UF_DRF_dim_info_p_t dim_info;
		memset(text, 0, sizeof(text));
	strcpy(text, "");
	if (dim_tag == NULL_TAG)
		return (-1);

	UF_DRF_ask_object_preferences (dim_tag, mpi, mpr, rad, dia); 
	if (mpi[16] == 2)
	{
		mpi[16] = 1;
		UF_DRF_set_object_preferences (dim_tag, mpi, mpr, rad, dia); 
	}	

	UF_DRF_ask_dim_info(dim_tag, &subtype, origin, &dim_info);
	for (i=0; i<dim_info->num_text; i++) 
	{
		if (dim_info->text_info[i].text_type == UF_DRF_DIM_TEXT)
		{
			strcpy(text, dim_info->text_info[i].text->string);
			break;
		}
	}	
	UF_DRF_free_dimension( &dim_info );

	return (0);
}
extern int DRAFT_ask_dimension_text(tag_t dim_tag, char *text)
{
	int i;
	int subtype; 
	int mpi[100];
	double mpr[70]; 
	char rad[27];
	char dia[27];    
	double origin[ 3 ];  
	UF_DRF_dim_info_p_t dim_info;

	strcpy(text, "");
	if (dim_tag == NULL_TAG)
		return (-1);

	UF_DRF_ask_object_preferences (dim_tag, mpi, mpr, rad, dia); 
	if (mpi[16] == 2)
	{
		mpi[16] = 1;
		UF_DRF_set_object_preferences (dim_tag, mpi, mpr, rad, dia); 
	}	

	UF_DRF_ask_dim_info(dim_tag, &subtype, origin, &dim_info);
	for (i=0; i<dim_info->num_text; i++) 
	{
		if (dim_info->text_info[i].text_type == UF_DRF_DIM_TEXT)
		{
			strcpy(text,dim_info->text_info[i].text->string);
			break;
		}
	}	
	UF_DRF_free_dimension( &dim_info );

	return (0);
}
extern NXString GW_ask_pmi_annotation_text(Annotations::Annotation *pmiAnnotation)
{

	UI *theUI = UI::GetUI();
	Session *theSession = Session::GetSession();
	Part *workPart(theSession->Parts()->Work());
	NXString	annotationText = "";

	Annotations::Dimension *pmiDimension = dynamic_cast<Annotations::Dimension *>(pmiAnnotation);
	if (pmiDimension)
	{
		UF_print_syslog("IS pmiDimension", FALSE);
		return GW_remove_sharp_bracket_text(GW_ask_dimension_string(pmiDimension));
		/*
		
		
			char text[50] ;
		DRAFT_ask_dimension_text1(pmiAnnotation->GetTag(), text);
		UF_print_syslog("尺寸是",true);
		 UF_print_syslog(text,true);
		annotationText +=text;
		 */
	 
	
		 
	//	return annotationText;
	}
	Annotations::SurfaceFinish *surfaceFinish = dynamic_cast<Annotations::SurfaceFinish *>(pmiAnnotation);
	if (surfaceFinish)
	{
		UF_print_syslog("IS surfaceFinish", FALSE);
		try
		{
			Annotations::SurfaceFinishBuilder *surfaceFinishBuilder = workPart->PmiManager()->PmiAttributes()->CreateSurfaceFinishBuilder(surfaceFinish);
			annotationText += surfaceFinishBuilder->A1();
			if (strcmp(surfaceFinishBuilder->A1().GetLocaleText(), surfaceFinishBuilder->A2().GetLocaleText()))
				annotationText += surfaceFinishBuilder->A2();
		 
			annotationText += surfaceFinishBuilder->B();
			annotationText += surfaceFinishBuilder->C();
			annotationText += surfaceFinishBuilder->D();;
			annotationText += surfaceFinishBuilder->E();
			annotationText += surfaceFinishBuilder->F1();
			annotationText += surfaceFinishBuilder->F2();
			surfaceFinishBuilder->Destroy();
		}
		catch (std::exception& ex)
		{
			theUI->NXMessageBox()->Show("Block Styler", NXOpen::NXMessageBox::DialogTypeError, ex.what());
		}
		return annotationText;
	}
	//Annotations::Fcf *fcf = dynamic_cast<Annotations::Fcf *>(pmiAnnotation);
	/*
	if (fcf)
	{
		UF_print_syslog("IS fcf", FALSE);


		try
		{
			Annotations::PmiFeatureControlFrameBuilder *pmiFeatureControlFrameBuilder = workPart->Annotations()->CreatePmiFeatureControlFrameBuilder(fcf);
			Annotations::FeatureControlFrameBuilder::FcfCharacteristic fcfCharacteristic = pmiFeatureControlFrameBuilder->Characteristic();
			for (int index = 0; index < pmiFeatureControlFrameBuilder->FeatureControlFrameDataList()->Length()-1; index++)
			{
				TaggedObject *taggedObject = pmiFeatureControlFrameBuilder->FeatureControlFrameDataList()->FindItem(index);
				Annotations::FeatureControlFrameDataBuilder *featureControlFrameDataBuilder = dynamic_cast<Annotations::FeatureControlFrameDataBuilder *>(taggedObject);
	 
				Annotations::FeatureControlFrameDataBuilder::ToleranceZoneShape toleranceZoneShape = featureControlFrameDataBuilder->ZoneShape();
		 
				switch (toleranceZoneShape)
				{
				case Annotations::FeatureControlFrameDataBuilder::ToleranceZoneShapeNone:				 
					break;
				case Annotations::FeatureControlFrameDataBuilder::ToleranceZoneShapeDiameter:		 
					annotationText += "Φ";
					break;
				case Annotations::FeatureControlFrameDataBuilder::ToleranceZoneShapeSphericalDiameter:	 
					annotationText += "SΦ";
					break;
				case Annotations::FeatureControlFrameDataBuilder::ToleranceZoneShapeSquare:				 
					annotationText += "□";
					break;
				};
				char	sToleranceValue[256];
				strcpy(sToleranceValue, featureControlFrameDataBuilder->ToleranceValue().GetLocaleText());
				UF_print_syslog("first sToleranceValue",true);
				UF_print_syslog(sToleranceValue,true);
				if (NULL != strchr(sToleranceValue, '.'))
				{
					GW_trim_string_right(sToleranceValue, '0');
					GW_trim_string_right(sToleranceValue, '.');
				}
				UF_print_syslog("second sToleranceValue",true);
				UF_print_syslog(sToleranceValue,true);
				annotationText += sToleranceValue;
				Annotations::FeatureControlFrameDataBuilder::ToleranceMaterialModifier materialModifier = featureControlFrameDataBuilder->MaterialModifier();
				switch (materialModifier)
				{
				case Annotations::FeatureControlFrameDataBuilder::ToleranceMaterialModifierNone:					 
					break;
				case Annotations::FeatureControlFrameDataBuilder::ToleranceMaterialModifierLeastMaterialCondition: 
					annotationText += "(L)";
					break;
				case Annotations::FeatureControlFrameDataBuilder::ToleranceMaterialModifierMaximumMaterialCondition: 
					annotationText += "(M)";
					break;
				case Annotations::FeatureControlFrameDataBuilder::ToleranceMaterialModifierRegardlessOfFeatureSize:	 
					annotationText += "(S)";
					break;
				};
				annotationText += " ";
				NXString primaryDatumReference = GW_ask_compound_datum_reference_text(featureControlFrameDataBuilder->PrimaryCompoundDatumReference());
				if (0 == strlen(primaryDatumReference.GetLocaleText()))
					primaryDatumReference = GW_ask_datum_reference_text(featureControlFrameDataBuilder->PrimaryDatumReference());
				if (strlen(primaryDatumReference.GetLocaleText()))
				{
					annotationText += primaryDatumReference;
					annotationText += " ";
				}
				NXString secondaryDatumReference = GW_ask_compound_datum_reference_text(featureControlFrameDataBuilder->SecondaryCompoundDatumReference());
				if (0 == strlen(secondaryDatumReference.GetLocaleText()))
					secondaryDatumReference = GW_ask_datum_reference_text(featureControlFrameDataBuilder->SecondaryDatumReference());
				if (strlen(secondaryDatumReference.GetLocaleText()))
				{
					annotationText += secondaryDatumReference;
					annotationText += " ";
				}
				NXString tertiaryDatumReference = GW_ask_compound_datum_reference_text(featureControlFrameDataBuilder->TertiaryCompoundDatumReference());
				if (0 == strlen(tertiaryDatumReference.GetLocaleText()))
					tertiaryDatumReference = GW_ask_datum_reference_text(featureControlFrameDataBuilder->TertiaryDatumReference());
				if (strlen(tertiaryDatumReference.GetLocaleText()))
				{
					annotationText += tertiaryDatumReference;
					annotationText += " ";
				}
			}
			pmiFeatureControlFrameBuilder->Destroy();
		}
		catch (std::exception& ex)
		{
			theUI->NXMessageBox()->Show("Block Styler", NXOpen::NXMessageBox::DialogTypeError, ex.what());
		}
		return annotationText;
	}
	*/
	else{
		
		UF_print_syslog("IS ELSE", FALSE);
		char info[200] ;
		GetGdtChar(pmiAnnotation->GetTag(), info);
		UF_print_syslog(info, true);
		ReplaceStr(info, "<&70>", "");
		ReplaceStr(info, "<&60>", "");
		ReplaceStr(info, "<&90>", "");
		ReplaceStr(info, "<$s>", "°");
		ReplaceStr(info, "<P>", "");
		ReplaceStr(info, "<+>", "");
		ReplaceStr(info, "<&1>", "一");
		//DRAFT_ask_note_value(pmiAnnotation->GetTag(), text);
	
		annotationText += info;
	}
	return annotationText;
	 
}

extern int ReplaceStr(char* sSrc, char* sMatchStr, char* sReplaceStr)
{
	int StringLen;
	char caNewString[64];
	char* FindPos;
	FindPos = (char *)strstr(sSrc, sMatchStr);
	if ((!FindPos) || (!sMatchStr))
		return -1;

	while (FindPos)
	{
		memset(caNewString, 0, sizeof(caNewString));
		StringLen = FindPos - sSrc;
		strncpy(caNewString, sSrc, StringLen);
		strcat(caNewString, sReplaceStr);
		strcat(caNewString, FindPos + strlen(sMatchStr));
		strcpy(sSrc, caNewString);

		FindPos = (char *)strstr(sSrc, sMatchStr);
	}
	free(FindPos);
	return 0;
}

extern NXString GW_ask_datum_reference_text(Annotations::DatumReferenceBuilder *datumReferenceBuilder)
{
	UI *theUI = UI::GetUI();
	Session *theSession = Session::GetSession();
	Part *workPart(theSession->Parts()->Work());
	NXString	datumReferenceText = "";

	try
	{
		 
		datumReferenceText = datumReferenceBuilder->Letter();
		Annotations::DatumReferenceBuilder::DatumReferenceMaterialCondition materialCondition = datumReferenceBuilder->MaterialCondition();
		switch (materialCondition)
		{
		case Annotations::DatumReferenceBuilder::DatumReferenceMaterialConditionNone:						/** none */
			break;
		case Annotations::DatumReferenceBuilder::DatumReferenceMaterialConditionLeastMaterialCondition:		/** least material condition */
			datumReferenceText += "(L)";
			break;
		case Annotations::DatumReferenceBuilder::DatumReferenceMaterialConditionMaximumMaterialCondition:	/** maximum material condition */
			datumReferenceText += "(M)";
			break;
		case Annotations::DatumReferenceBuilder::DatumReferenceMaterialConditionRegardlessOfFeatureSize:	/** regardless of size */
			datumReferenceText += "(S)";
			break;
		}
		if (datumReferenceBuilder->FreeState())
			datumReferenceText += "(F)";
	}
	catch (std::exception& ex)
	{
		theUI->NXMessageBox()->Show("Block Styler", NXOpen::NXMessageBox::DialogTypeError, ex.what());
	}
	return datumReferenceText;
}

extern NXString GW_ask_compound_datum_reference_text(Annotations::CompoundDatumReferenceBuilder *compoundDatumReferenceBuilder)
{
	UI *theUI = UI::GetUI();
	Session *theSession = Session::GetSession();
	Part *workPart(theSession->Parts()->Work());
	NXString	compoundDatumReferenceText = "";

	try
	{
	 	std::vector<NXOpen::Annotations::DatumReferenceBuilder *>	datumReferenceBuilders;
	
	
		  NXOpen::Annotations::DatumReferenceBuilderList list = *compoundDatumReferenceBuilder->DatumReferenceList();
		datumReferenceBuilders =   list.GetContents();
		for (std::vector<NXOpen::Annotations::DatumReferenceBuilder *>::iterator datumReferenceBuilders_it = datumReferenceBuilders.begin(); datumReferenceBuilders_it != datumReferenceBuilders.end(); datumReferenceBuilders_it++)
		{
			if (strlen(compoundDatumReferenceText.GetLocaleText()))
				compoundDatumReferenceText += "-";
			compoundDatumReferenceText += GW_ask_datum_reference_text(*datumReferenceBuilders_it);
		}
	}
	catch (std::exception& ex)
	{
		// theUI->NXMessageBox()->Show("Block Styler", NXOpen::NXMessageBox::DialogTypeError, ex.what());
	}
	return compoundDatumReferenceText;
}

extern NXString GW_remove_sharp_bracket_text(NXString inputString)
{
	std::string	strBuffer(inputString.GetLocaleText());
	NXString returnString;

	while (true)
	{
		std::string::size_type begin = strBuffer.find("<");
		if (-1 == begin)
			break;
		std::string::size_type end = strBuffer.find(">");
		if (-1 == end)
			break;
		strBuffer = strBuffer.erase(begin, end-begin+1);
	}
	returnString = strBuffer.c_str();
	return returnString;
}

extern int GW_yes_or_no_box(const char *message, const char *title_string)
{
	UF_UI_message_buttons_t	buttons;
	int	response;

	buttons.button1 = true;
	buttons.button2 = true;
	buttons.button3 = false;

	buttons.label1 = "Yes";
	buttons.label2 = "No";
	buttons.label3 = "Cancel";

	buttons.response1 = 1;
	buttons.response2 = 2;
	buttons.response3 = 3;
	UF_CALL(UF_UI_message_dialog((char*)title_string, UF_UI_MESSAGE_QUESTION, (char**)(&message), 1, true, &buttons, &response));
	return response;
}

extern Part* GW_find_associate_part(const Annotations::Annotation *pmiAnnotation)
{
	std::vector<tag_t>	partArray;
	char pmi_handle[UF_ATTR_MAX_STRING_LEN];

	tag_t annotation_tag = pmiAnnotation->GetTag();
	GW_ask_parts_in_assembly(partArray);
	for (std::vector<tag_t>::const_iterator partArray_cit=partArray.begin(); partArray_cit!=partArray.end(); partArray_cit++)
	{
		if (GW_ask_object_attribute(*partArray_cit, GW_PMI_HANDLE_NAME, pmi_handle))
		{
			tag_t object_tag = UF_TAG_ask_tag_of_handle(pmi_handle);
			if (object_tag == annotation_tag)
			{
				return dynamic_cast<Part *>(NXOpen::NXObjectManager::Get(*partArray_cit));
			}
		}
	}

	return NULL;
}

extern void DRAFT_ask_note_value(tag_t note_tag, char *text)
{ 
	char buf[133];
	int ir4, ir5;
	int cycle_flag = 0;
	int search_mask[4] = {0, 0, 1, 0};
	int ann_data[10], ann_data_type;
	int ann_data_form, num_segments;
	double ann_origin[2], radius_angle;

	strcpy(text, "");

	UF_DRF_ask_ann_data(&note_tag, search_mask, &cycle_flag, 
		ann_data, &ann_data_type, &ann_data_form, 
		&num_segments, ann_origin, &radius_angle);
	if (ann_data_type == 3 || num_segments == 1)
	{
		uc5574(1, ann_data, buf, &ir4, &ir5);
		strcpy(text, buf);
		string return_str = text;

		UF_print_syslog("---------------------", FALSE);
		UF_print_syslog(text, FALSE);
	}
}

extern DllExport void  GetGdtChar(tag_t objTag, char  info[200])
{
	int type;
	int subtype;
	char *gdtInfo;
	char *endInfo;
	char returnChar[100] = "";
	strcpy(info, "");
	UF_OBJ_ask_type_and_subtype(objTag, &type, &subtype);
	if (type != UF_drafting_entity_type && type != UF_dimension_type)
	{
		return;
	}
	if (type == UF_drafting_entity_type &&
		(subtype == UF_draft_fpt_subtype ||
			subtype == UF_draft_note_subtype ||
			subtype == UF_draft_label_subtype))
	{
		int ir4;
		int ir5;
		int cycle_flag = 0;
		int search_mask[4];
		int ann_data[10];
		int ann_data_type;
		int ann_data_form;
		int num_segments;
		double ann_origin[2];
		double radius_angle;

		search_mask[0] = 0;
		search_mask[1] = 0;
		search_mask[2] = 1;
		search_mask[3] = 0;
		UF_DRF_ask_ann_data(&objTag, search_mask, &cycle_flag,
			ann_data, &ann_data_type, &ann_data_form,
			&num_segments, ann_origin, &radius_angle);
		for (int i = 0; i<num_segments; i++)
		{
			uc5574(i + 1, ann_data, info, &ir4, &ir5);
			if (strstr(info, "<&90>") != NULL)
			{
				break;
			}
		}
	}
	else if (type == UF_dimension_type)
	{
		int numAdd = 0;
		int numText;
		UF_DRF_appended_text_p_t appendText;
		UF_DRF_ask_appended_text(objTag, &numText, &appendText);
		for (int i = 0; i<numText; i++)
		{
			int j;
			for (j = 0; j<appendText->num_lines; j++)
			{
				strcpy(info, appendText->text[j]);
				if (strstr(info, "<&90>") != NULL)
				{
					break;
				}
			}
			if (j == appendText->num_lines)
			{
				numAdd++;
				appendText++;
			}
			else
			{
				break;
			}
		}
		appendText = appendText - numAdd;
		UF_DRF_free_appended_text(numText, appendText);
	}

	//判断其是否为形位公差
	UF_print_syslog("---------info------------", TRUE);
	UF_print_syslog(info, TRUE);
	gdtInfo = strstr(info, "<&70>");
	if (gdtInfo == NULL)
	{
		gdtInfo = strstr(info, "<&60>");
	}
	if (gdtInfo == NULL)
	{
		//	strcpy(info, "");
		return ;
	}
	endInfo = strstr(info, "<&90>");
	if (endInfo != NULL)
	{
		int length = strlen(gdtInfo) - strlen(endInfo) + strlen("<&90>");


		strncat(returnChar, gdtInfo, length);
		//	gdtInfo[3] = '\0';
		strcpy(info, returnChar);
	}
	else
	{
		//	strcpy(info, "");
		return ;
	}

	//根据加号数量判断时形位公差还是基准
	int addCnt = 0;
	for (int i = 0; i<strlen(info); i++)
	{
		if (info[i] == '+')
		{
			++addCnt;
		}
		if (addCnt > 2)
		{

			return  ;
		}
	}
}