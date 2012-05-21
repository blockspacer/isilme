//  Copyright (C) 2010-2012 Alexander Alimov
//
//	This file is part of Isilme SDK.
//
//		Isilme SDK is free software: you can redistribute it and/or modify
//		it under the terms of the GNU Lesser General Public License as published by
//		the Free Software Foundation, either version 3 of the License, or
//		(at your option) any later version.
//
//		Isilme SDK is distributed in the hope that it will be useful,
//		but WITHOUT ANY WARRANTY; without even the implied warranty of
//		MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//		GNU Lesser General Public License for more details.
//
//		You should have received a copy of the GNU Lesser General Public License
//		along with Isilme SDK.  If not, see <http://www.gnu.org/licenses/>.
//

#ifndef ISILME_LOGIC_QUANTIFIERS_H
#define ISILME_LOGIC_QUANTIFIERS_H

#include "Atom.h"

namespace logic
{
	class ExtQuntifier : public Atom
	{
	public:
		ExtQuntifier(std::string id, ValueSetPtr values, AtomPtr term);

		virtual bool Calc(ContextPtr context) override;

		static ValueSetPtr Subset(std::string id, ValueSetPtr values, AtomPtr term, ContextPtr context);
	private:
		AtomPtr mTerm;
		ValueSetPtr mValues;
		std::string mIdentifier;
	};

	class AnyQuantifier : public Atom
	{
	public:
		AnyQuantifier(std::string id, ValueSetPtr values, AtomPtr term);

		virtual bool Calc(ContextPtr context) override;
	private:
		AtomPtr mTerm;
		ValueSetPtr mValues;
		std::string mIdentifier;
	};
}
#endif