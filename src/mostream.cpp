/*************************************************************************************************/
/**
	mostream.cpp

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

#include "mostream.h"
#include <algorithm>

void mstreambuf::add( std::ostream *pOutputStream )
{
        if ( std::find(m_outputs.begin(), m_outputs.end(), pOutputStream) == m_outputs.end() )
		{
                m_outputs.push_back( pOutputStream );
		}
}

void mstreambuf::remove( std::ostream *pOutputStream )
{
        m_outputs.erase( std::remove( m_outputs.begin(), m_outputs.end(), pOutputStream ), m_outputs.end() );
}

int mstreambuf::sync()
{
        if ( !m_buffer.empty() )
        {
                for ( size_t i = 0; i < m_outputs.size(); ++i )
				{
                        ( *m_outputs[i] ) << m_buffer;
				}
				
				for ( size_t i = 0; i < m_outputs.size(); ++i )
				{
                        m_outputs[i]->flush();//since we're never
											  //explicitly printing
											  //std::endl properly.
                }

                m_buffer.clear();
        }

        return 0;
}

mstreambuf::int_type mstreambuf::overflow( int_type meta )
{
        if( meta == traits_type::eof() )
		{
                return traits_type::eof();
		}

        char c = traits_type::to_char_type( meta );
        m_buffer.push_back( c );
        if ( c == '\n' )
		{
                this->sync();
		}

        return traits_type::not_eof( meta );
}

mostream::mostream():
	std::ostream( 0 )
{
	this->init( &m_buf );
}

void mostream::add( std::ostream *pOutputStream )
{
	m_buf.add( pOutputStream );
}

void mostream::remove( std::ostream *pOutputStream )
{
	m_buf.remove( pOutputStream );
}
