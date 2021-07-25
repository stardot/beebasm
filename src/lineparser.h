/*************************************************************************************************/
/**
	lineparser.h


	Copyright (C) Rich Talbot-Watkins 2007 - 2012

	This file is part of BeebAsm.

	BeebAsm is free software: you can redistribute it and/or modify it under the terms of the GNU
	General Public License as published by the Free Software Foundation, either version 3 of the
	License, or (at your option) any later version.

	BeebAsm is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without
	even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License along with BeebAsm, as
	COPYING.txt.  If not, see <http://www.gnu.org/licenses/>.
*/
/*************************************************************************************************/

#ifndef LINEPARSER_H_
#define LINEPARSER_H_

#include <string>
#include "value.h"

class SourceCode;

class LineParser
{
public:

	// Constructor/destructor

	LineParser( SourceCode* sourceCode, std::string line );
	~LineParser();

	// Process the line

	void Process();

	// Accessors


private:

	typedef void ( LineParser::*TokenHandler )();
	typedef void ( SourceCode::*DirectiveHandler )( const std::string& line, int column );

	struct Token
	{
		const char*			m_pName;
		TokenHandler		m_handler;
		DirectiveHandler	m_directiveHandler;
	};

	enum ADDRESSING_MODE
	{
		IMP,
		ACC,
		IMM,
		ZP,
		ZPX,
		ZPY,
		ABS,
		ABSX,
		ABSY,
		IND,
		INDX,
		INDY,
		IND16,
		IND16X,
		REL,

		NUM_ADDRESSING_MODES
	};

	struct OpcodeData
	{
		short			m_aOpcodes[NUM_ADDRESSING_MODES];
		const char*		m_pName;
		int				m_cpu;
	};


	typedef void ( LineParser::*OperatorHandler )();

	struct Operator
	{
		const char*			token;
		int					precedence;
		int					parameterCount;
		OperatorHandler		handler;
	};

	enum TYPE
	{
		VALUE_OR_UNARY,
		BINARY
	};


	// line parsing methods

	int				GetTokenAndAdvanceColumn();
	void			HandleToken( int i, int oldColumn );
	int				GetInstructionAndAdvanceColumn();
	int				GetInstructionAndAdvanceColumn(bool requireDistinctOpcodes);
	int				CheckMacroMatches();
	bool			MoveToNextAtom( const char* pTerminators = NULL );
	bool			AdvanceAndCheckEndOfLine();
	bool			AdvanceAndCheckEndOfStatement();
	bool			AdvanceAndCheckEndOfSubStatement(bool includeComma);
	void			SkipStatement();
	void			SkipExpression( int bracketCount, bool bAllowOneMismatchedCloseBracket );
	std::string		GetSymbolName();

	// assembler generating methods

	void			HandleAssembler( int tokenNumber );
	bool			HasAddressingMode( int opcodeIndex, ADDRESSING_MODE mode );
	unsigned int	GetOpcode( int opcodeIndex, ADDRESSING_MODE mode );
	void			Assemble1( int instructionIndex, ADDRESSING_MODE mode );
	void			Assemble2( int instructionIndex, ADDRESSING_MODE mode, unsigned int value );
	void			Assemble3( int instructionIndex, ADDRESSING_MODE mode, unsigned int value );

	// language handling methods

	void			HandleDefineLabel();
	void			HandleDefineComment();
	void			HandleStatementSeparator();
	void			HandleDirective();
	void			HandlePrint();
	void			HandleCpu();
	void			HandleOrg();
	void			HandleInclude();
	void			HandleIncBin();
	void			HandleEqub();
	void			HandleEqus(const String& equs);
	void			HandleEquw();
	void			HandleEqud();
	void			HandleAssert();
	void			HandleSave();
	void			HandleFor();
	void			HandleNext();
	void			HandleOpenBrace();
	void			HandleCloseBrace();
	void			HandleIf();
	void			HandleAlign();
	void			HandleSkip();
	void			HandleSkipTo();
	void			HandleGuard();
	void			HandleClear();
	void			HandleMapChar();
	void			HandlePutText();
	void			HandlePutFile();
	void			HandlePutFileCommon( bool bText );
	void			HandlePutBasic();
	void			HandleMacro();
	void			HandleEndMacro();
	void			HandleError();
	void			HandleCopyBlock();
	void			HandleRandomize();
	void			HandleAsm();

	// expression evaluating methods

	Value			EvaluateExpression( bool bAllowOneMismatchedCloseBracket = false );
	double			EvaluateExpressionAsDouble( bool bAllowOneMismatchedCloseBracket = false );
	int				EvaluateExpressionAsInt( bool bAllowOneMismatchedCloseBracket = false );
	unsigned int	EvaluateExpressionAsUnsignedInt( bool bAllowOneMismatchedCloseBracket = false );
	std::string		EvaluateExpressionAsString( bool bAllowOneMismatchedCloseBracket = false );
	Value			GetValue();

	// convenience functions for getting operator parameters from the stack
	std::pair<Value, Value> StackTopTwoValues();
	String StackTopString();
	double StackTopNumber();
	std::pair<double, double> StackTopTwoNumbers();
	std::pair<int, int> StackTopTwoInts();

	void			EvalAdd();
	void			EvalSubtract();
	void			EvalMultiply();
	void			EvalDivide();
	void			EvalPower();
	void			EvalDiv();
	void			EvalMod();
	void			EvalShiftLeft();
	void			EvalShiftRight();
	void			EvalAnd();
	void			EvalOr();
	void			EvalEor();
	void			EvalEqual();
	void			EvalNotEqual();
	void			EvalLessThanOrEqual();
	void			EvalMoreThanOrEqual();
	void			EvalLessThan();
	void			EvalMoreThan();

	void			EvalNegate();
	void			EvalPosate();
	void			EvalLo();
	void			EvalHi();
	void			EvalSin();
	void			EvalCos();
	void			EvalTan();
	void			EvalArcSin();
	void			EvalArcCos();
	void			EvalArcTan();
	void			EvalSqrt();
	void			EvalDegToRad();
	void			EvalRadToDeg();
	void			EvalInt();
	void			EvalAbs();
	void			EvalSgn();
	void			EvalRnd();
	void			EvalNot();
	void			EvalLog();
	void			EvalLn();
	void			EvalExp();
	void			EvalTime();
	void			EvalStr();
	void			EvalStrHex();
	void			EvalVal();
	void			EvalEval();
	void			EvalLen();
	void			EvalChr();
	void			EvalAsc();
	void			EvalMid();
	void			EvalString();
	void			EvalUpper();
	void			EvalLower();

	Value			FormatAssemblyTime(const char* formatString);

	SourceCode*				m_sourceCode;
	std::string				m_line;
	size_t					m_column;

	static const Token		m_gaTokenTable[];
	static const OpcodeData	m_gaOpcodeTable[];
	static const Operator	m_gaUnaryOperatorTable[];
	static const Operator	m_gaBinaryOperatorTable[];

	#define MAX_VALUES		128
	#define MAX_OPERATORS	32

	Value					m_valueStack[ MAX_VALUES ];
	Operator				m_operatorStack[ MAX_OPERATORS ];
	int						m_valueStackPtr;
	int						m_operatorStackPtr;

	friend class ArgListParser;
};


#endif // LINEPARSER_H_
