/*************************************************************************************************/
/**
	anonymouslabels.h


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

#ifndef ANONYMOUSLABELS_H_
#define ANONYMOUSLABELS_H_


#include "scopedsymbolname.h"
#include <vector>


class AnonymousLabels
{
public:
	static inline AnonymousLabels& Instance() { return m_gInstance; }

	// Get PC of last '-' label
	int GetBackReference() { return m_backReference; }
	// Set PC of '-' label
	void SetBackReference(int pc) { m_backReference = pc; }
	// Remember name of symbol referring to the next '+ label
	void AddForwardReference(const ScopedSymbolName& symbolName) { m_forwardReferences.push_back( symbolName ); }
	// Update the forward references when a '+' label is seen
	void UpdateForwardReferences(int pc);
	// Forget everything
	void Clear() { m_backReference = -1; m_forwardReferences.clear(); }

private:
	int m_backReference;
	std::vector<ScopedSymbolName> m_forwardReferences;

	static AnonymousLabels m_gInstance;
};


#endif // ANONYMOUSLABELS_H_
