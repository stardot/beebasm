/*************************************************************************************************/
/**
	mostream.h

	ostream that has multiple outputs


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

#ifndef MOSTREAM_H_
#define MOSTREAM_H_

#include <vector>
#include <string>
#include <ostream>
#include <sstream>

class mstreambuf : public std::stringbuf
{
public:
        void add( std::ostream *pOutputStream );
        void remove( std::ostream *pOutputStream );
protected:
        int sync();
        int_type overflow( int_type meta );
private:
        std::vector< std::ostream * > m_outputs;
        std::string m_buffer;
};

class mostream : public std::ostream
{
public:
        mostream();

        void add( std::ostream *pOutputStream );
        void remove( std::ostream *pOutputStream );
protected:
private:
        mstreambuf m_buf;
};

#endif
