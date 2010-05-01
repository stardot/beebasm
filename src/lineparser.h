/*************************************************************************************************/
/**
	lineparser.h
*/
/*************************************************************************************************/

#ifndef LINEPARSER_H_
#define LINEPARSER_H_

#include <string>

class SourceFile;

class LineParser
{
public:

	// Constructor/destructor

	LineParser( SourceFile* sourceFile, std::string line );
	~LineParser();

	// Process the line

	void Process();

	// Accessors


private:

	typedef void ( LineParser::*TokenHandler )();

	struct Token
	{
		const char*		m_pName;
		TokenHandler	m_handler;
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
		INDX,
		INDY,
		IND16,
		REL,

		NUM_ADDRESSING_MODES
	};

	struct OpcodeData
	{
		short			m_aOpcodes[NUM_ADDRESSING_MODES];
		const char*		m_pName;
	};


	typedef void ( LineParser::*OperatorHandler )();

	struct Operator
	{
		const char*			token;
		int					precedence;
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
	bool			MoveToNextAtom( const char* pTerminators = NULL );
	bool			AdvanceAndCheckEndOfLine();
	bool			AdvanceAndCheckEndOfStatement();
	bool			AdvanceAndCheckEndOfSubStatement();
	void			SkipStatement();
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
	void			HandlePrint();
	void			HandleOrg();
	void			HandleInclude();
	void			HandleEqub();
	void			HandleEquw();
	void			HandleSave();
	void			HandleFor();
	void			HandleNext();
	void			HandleIf();
	void			HandleElse();
	void			HandleEndif();
	void			HandleAlign();
	void			HandleSkip();
	void			HandleGuard();
	void			HandleClear();

	// expression evaluating methods

	double			EvaluateExpression( bool bAllowOneMismatchedCloseBracket = false );
	int				EvaluateExpressionAsInt( bool bAllowOneMismatchedCloseBracket = false );
	double			GetValue();

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


	SourceFile*				m_sourceFile;
	std::string				m_line;
	size_t					m_column;

	static Token			m_gaTokenTable[];
	static OpcodeData		m_gaOpcodeTable[];
	static Operator			m_gaUnaryOperatorTable[];
	static Operator			m_gaBinaryOperatorTable[];

	#define MAX_VALUES		128
	#define MAX_OPERATORS	32

	double					m_valueStack[ MAX_VALUES ];
	Operator				m_operatorStack[ MAX_OPERATORS ];
	int						m_valueStackPtr;
	int						m_operatorStackPtr;
};


#endif // LINEPARSER_H_
