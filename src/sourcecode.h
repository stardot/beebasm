/*************************************************************************************************/
/**
	sourcecode.h


	Copyright (C) Rich Talbot-Watkins 2007 - 2011

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

#include <fstream>
#include <string>
#include <vector>

class Macro;

class SourceCode
{
public:

	// Constructor/destructor

	SourceCode( const std::string& filename, int lineNumber );
	~SourceCode();

	// Process the file

	virtual void Process();

	// Accessors

	inline const std::string&	GetFilename() const				{ return m_filename; }
	inline int				GetLineNumber() const			{ return m_lineNumber; }
	inline int				GetLineStartPointer() const		{ return m_lineStartPointer; }

	virtual std::istream&	GetLine( std::string& lineFromFile ) = 0;
	virtual int				GetFilePointer() = 0;
	virtual void			SetFilePointer( int i ) = 0;
	virtual bool			IsAtEnd() = 0;


	// For loop / if related stuff

	#define MAX_FOR_LEVELS	32
	#define MAX_IF_LEVELS	32

protected:

	struct For
	{
		std::string			m_varName;
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

	struct If
	{
		bool				m_condition;
		bool                m_hadElse;
		bool				m_passed;
		std::string			m_line;
		int					m_column;
		int					m_lineNumber;
	};

	int						m_ifStackPtr;
	If						m_ifStack[ MAX_IF_LEVELS ];

	Macro*					m_currentMacro;


public:

	void					OpenBrace( const std::string& line, int column );
	void					CloseBrace( const std::string& line, int column );

	void					AddFor( const std::string& varName,
									double start,
									double end,
									double step,
									int filePtr,
									const std::string& line,
									int column );

	void					UpdateFor( const std::string& line, int column );

	void					CopyForStack( const SourceCode* copyFrom );

	inline int 				GetForLevel() const { return m_forStackPtr; }
	inline Macro*			GetCurrentMacro() { return m_currentMacro; }

	std::string				GetSymbolNameSuffix( int level = -1 ) const;

	bool					IsIfConditionTrue() const;
	void					AddIfLevel( const std::string& line, int column );
	void					SetCurrentIfCondition( bool b );
	void					StartElse( const std::string& line, int column );
	void					StartElif( const std::string& line, int column );
	void					ToggleCurrentIfCondition( const std::string& line, int column );
	void					RemoveIfLevel( const std::string& line, int column );
	void					StartMacro( const std::string& line, int column );
	void					EndMacro( const std::string& line, int column );


protected:

	std::string				m_filename;
	int						m_lineNumber;
	int						m_lineStartPointer;
};


#endif // SOURCECODE_H_
