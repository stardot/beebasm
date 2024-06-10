/*************************************************************************************************/
/**
	sourcecode.h


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

#ifndef SOURCECODE_H_
#define SOURCECODE_H_

#include <string>

#include "scopedsymbolname.h"
#include "value.h"

class Macro;

class SourceCode
{
public:

	// Constructor/destructor

	SourceCode( const std::string& filename, int lineNumber, const std::string& text, const SourceCode* parent );
	~SourceCode();

	// Process the file

	virtual void Process();

	// Accessors

	inline const std::string&	GetFilename() const				{ return m_filename; }
	inline int				GetLineNumber() const			{ return m_lineNumber; }
	inline const SourceCode*GetParent() const				{ return m_parent; }
	inline int				GetLineStartPointer() const		{ return m_lineStartPointer; }

	virtual bool			GetLine( std::string& lineFromFile );
	virtual int				GetFilePointer() { return m_textPointer; }
	virtual void			SetFilePointer( int i );
	virtual bool			IsAtEnd() { return m_textPointer == static_cast<int>(m_text.length()); }


	// For loop / if related stuff
	// Should use a std::vector here, but I can't really be bothered to change it now

	#define MAX_FOR_LEVELS	256
	#define MAX_IF_LEVELS	256

protected:

	struct For
	{
		ScopedSymbolName	m_varName;
		double				m_current;
		double				m_end;
		double				m_step;
		int					m_filePtr;
		int					m_id;
		int					m_count;
		std::string			m_line;
		int					m_column;
		int					m_lineNumber;
	};

	For						m_forStack[ MAX_FOR_LEVELS ];
	int						m_forStackPtr;
	int						m_initialForStackPtr;

	struct If
	{
		bool				m_condition;
		bool                m_hadElse;
		bool				m_passed;
		bool				m_isMacroDefinition;
		std::string			m_line;
		int					m_column;
		int					m_lineNumber;
	};

	int						m_ifStackPtr;
	int						m_initialIfStackPtr;
	If						m_ifStack[ MAX_IF_LEVELS ];

	Macro*					m_currentMacro;


public:

	void					OpenBrace( const std::string& line, int column );
	void					CloseBrace( const std::string& line, int column );

	void					AddFor( const ScopedSymbolName& varName,
									double start,
									double end,
									double step,
									int filePtr,
									const std::string& line,
									int column );

	void					UpdateFor( const std::string& line, int column );

	void					CopyForStack( const SourceCode* copyFrom );

	inline int 				GetForLevel() const { return m_forStackPtr; }
	inline int 				GetInitialForStackPtr() const { return m_initialForStackPtr; }
	inline Macro*			GetCurrentMacro() { return m_currentMacro; }

	bool					GetSymbolValue(std::string name, Value& value);
	ScopedSymbolName		GetScopedSymbolName( const std::string& symbolName, int level = -1 ) const;

	bool					ShouldOutputAsm();

	bool					IsIfConditionTrue() const;
	void					AddIfLevel( const std::string& line, int column );
	void					SetCurrentIfAsMacroDefinition();
	void					SetCurrentIfCondition( bool b );
	void					StartElse( const std::string& line, int column );
	void					StartElif( const std::string& line, int column );
	void					ToggleCurrentIfCondition( const std::string& line, int column );
	void					RemoveIfLevel( const std::string& line, int column );
	void					StartMacro( const std::string& line, int column );
	void					EndMacro( const std::string& line, int column );
	bool					IsRealForLevel( int level ) const;


protected:

	std::string				m_filename;
	int						m_lineNumber;
	const SourceCode*		m_parent;
	int						m_lineStartPointer;
	std::string				m_text;
	int						m_textPointer;
};


#endif // SOURCECODE_H_
