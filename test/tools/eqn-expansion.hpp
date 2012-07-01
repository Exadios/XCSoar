/* Copyright_License {

  XCSoar Glide Computer - http://www.xcsoar.org/
  Copyright (C) 2000-2012 The XCSoar Project
  A detailed list of copyright holders can be found in the file "AUTHORS".

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  as published by the Free Software Foundation; either version 2
  of the License, or (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
}
 */
#ifndef EQN_EXPANSION_HPP
#define EQN_EXPANSION_HPP

#include <string>
#include "Math/MatrixT.hpp"
#include "Math/VectorT.hpp"

class Term;
const Term operator+(const Term& lhs, const Term& rhs);
const Term operator-(const Term& lhs, const Term& rhs);
const Term operator*(const Term& lhs, const Term& rhs);

class Term
{
  public:
    Term();
    Term(int);
    Term(const std::string& name);
    Term(const Term& rhs);
    virtual ~Term();

    Term& operator=(const Term& rhs);
    void operator=(int a);
    void operator()(int a);
//    void operator+(const Term& rhs);
//    void operator-(const Term& rhs);
//    void operator*(const Term& rhs);
    friend const Term operator+(const Term& lhs, const Term& rhs);
    friend const Term operator-(const Term& lhs, const Term& rhs);
    friend const Term operator*(const Term& lhs, const Term& rhs);

    std::string& Get();
    const std::string& Get() const;

  private:
    std::string name;

};

std::ostream& operator<<(std::ostream& out, const Term&);

#endif // EQN_EXPANSION_HPP
