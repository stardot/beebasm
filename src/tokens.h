/*************************************************************************************************/
/**
	tokens.h

	All the data used by the assembler
*/
/*************************************************************************************************/

// Token table

LineParser::Token	LineParser::m_gaTokenTable[] =
{
	{ ".",			&LineParser::HandleDefineLabel },
	{ "\\",			&LineParser::HandleDefineComment },
	{ ";",			&LineParser::HandleDefineComment },
	{ ":",			&LineParser::HandleStatementSeparator },
	{ "PRINT",		&LineParser::HandlePrint },
	{ "ORG",		&LineParser::HandleOrg },
	{ "INCLUDE",	&LineParser::HandleInclude },
	{ "EQUB",		&LineParser::HandleEqub },
	{ "EQUS",		&LineParser::HandleEqub },
	{ "EQUW",		&LineParser::HandleEquw },
	{ "SAVE",		&LineParser::HandleSave },
	{ "FOR",		&LineParser::HandleFor },
	{ "NEXT",		&LineParser::HandleNext },
	{ "IF",			&LineParser::HandleIf },
	{ "ELSE",		&LineParser::HandleElse },
	{ "ENDIF",		&LineParser::HandleEndif },
	{ "ALIGN",		&LineParser::HandleAlign },
	{ "SKIP",		&LineParser::HandleSkip }
};





