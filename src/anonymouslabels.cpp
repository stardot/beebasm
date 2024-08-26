/*************************************************************************************************/
/**
	anonymouslabels.cpp


	Copyright (C) Charles Reilly 2024

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


#include "symboltable.h"
#include "anonymouslabels.h"


AnonymousLabels AnonymousLabels::m_gInstance;


// Update the forward references when a '+' label is seen
void AnonymousLabelsData::UpdateForwardReferences(int pc)
{
	for (std::vector<ScopedSymbolName>::iterator it = m_forwardReferences.begin(); it != m_forwardReferences.end(); ++it)
	{
		SymbolTable::Instance().AddSymbol(*it, pc);
	}
	m_forwardReferences.clear();
}
