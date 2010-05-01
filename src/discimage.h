/*************************************************************************************************/
/**
	discimage.h
*/
/*************************************************************************************************/

#ifndef DISCIMAGE_H_
#define DISCIMAGE_H_

#include <fstream>


class DiscImage
{
public:

	explicit DiscImage( const char* pOutput, const char* pInput = NULL );
	~DiscImage();

	void AddFile( const char* pName, const unsigned char* pAddr, int load, int exec, int len );


private:

	std::ofstream				m_outputFile;
	std::ifstream				m_inputFile;
	const char*					m_outputFilename;
	const char*					m_inputFilename;
	unsigned char				m_aCatalog[ 0x200 ];

};



#endif // DISCIMAGE_H_
