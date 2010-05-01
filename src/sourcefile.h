/*************************************************************************************************/
/**
	sourcefile.h
*/
/*************************************************************************************************/

#ifndef SOURCEFILE_H_
#define SOURCEFILE_H_

#include <fstream>
#include <string>
#include <vector>


class SourceFile
{
public:

	// Constructor/destructor (RAII class)

	explicit SourceFile( const char* pFilename );
	~SourceFile();

	// Process the file

	void Process();

	// Accessors

	inline const char*		GetFilename() const				{ return m_pFilename; }
	inline int				GetLineNumber() const			{ return m_lineNumber; }
	inline int				GetFilePointer() const			{ return m_filePointer; }
	inline void				SetFilePointer( int i )			{ m_filePointer = i; m_file.seekg( i ); }

	// For loop / if related stuff

	#define MAX_FOR_LEVELS	16
	#define MAX_IF_LEVELS	16

private:

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
		std::string			m_line;
		int					m_column;
		int					m_lineNumber;
	};

	int						m_ifStackPtr;
	If						m_ifStack[ MAX_IF_LEVELS ];

public:

	void					AddFor( std::string varName,
									double start,
									double end,
									double step,
									int filePtr,
									std::string line,
									int column );

	void					UpdateFor( std::string line, int column );

	inline int 				GetForLevel() const { return m_forStackPtr; }

	std::string				GetSymbolNameSuffix( int level = -1 ) const;

	bool					IsIfConditionTrue() const;
	void					AddIfLevel( std::string line, int column );
	void					SetCurrentIfCondition( bool b );
	void					ToggleCurrentIfCondition( std::string line, int column );
	void					RemoveIfLevel( std::string line, int column );


private:

	std::ifstream			m_file;
	const char*				m_pFilename;
	int						m_lineNumber;
	int						m_filePointer;
};


#endif // SOURCEFILE_H_
