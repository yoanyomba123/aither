/*  This file is part of aither.
    Copyright (C) 2015-17  Michael Nucci (michael.nucci@gmail.com)

    Aither is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Aither is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>. */

#ifndef INVFLUXHEADERDEF  // only if the macro INVFLUXHEADERDEF is not defined
                          // execute these lines of code
#define INVFLUXHEADERDEF  // define the macro

#include <vector>        // vector
#include <string>        // string
#include <iostream>      // cout
#include <memory>        // unique_ptr
#include "vector3d.hpp"  // vector3d
#include "varArray.hpp"

using std::vector;
using std::string;
using std::cout;
using std::endl;
using std::cerr;
using std::ostream;
using std::unique_ptr;

// forward class declaration
class eos;
class thermodynamic;
class primVars;
class genArray;
class squareMatrix;
class turbModel;

class inviscidFlux : public residual {
  // private member functions
  void ConstructFromPrim(const primVars &, const unique_ptr<eos> &,
                         const unique_ptr<thermodynamic> &,
                         const vector3d<double> &);

 public:
  // constructors
  inviscidFlux(const int &numEqns, const int &numSpecies)
      : residual(numEqns, numSpecies) {}
  inviscidFlux() : inviscidFlux(0, 0) {}
  inviscidFlux(const primVars &state, const unique_ptr<eos> &eqnState,
               const unique_ptr<thermodynamic> &thermo,
               const vector3d<double> &area)
      : inviscidFlux(state.NumEquations(), state.NumSpecies()) {
    this->ConstructFromPrim(state, eqnState, thermo, area);
  }
  inviscidFlux(const genArray &cons, const unique_ptr<eos> &eqnState,
               const unique_ptr<thermodynamic> &thermo,
               const unique_ptr<turbModel> &turb, const vector3d<double> &area)
      : inviscidFlux(cons.NumEquations(), cons.NumSpecies()) {
    // convert conserved variables to primative variables
    const primVars state(cons, false, eqnState, thermo, turb);
    this->ConstructFromPrim(state, eqnState, thermo, area);
  }

  // move constructor and assignment operator
  inviscidFlux(inviscidFlux&&) noexcept = default;
  inviscidFlux& operator=(inviscidFlux&&) noexcept = default;

  // copy constructor and assignment operator
  inviscidFlux(const inviscidFlux&) = default;
  inviscidFlux& operator=(const inviscidFlux&) = default;

  // member functions
  void RoeFlux(const inviscidFlux&, const genArray&);
  void AUSMFlux(const primVars &, const primVars &, const unique_ptr<eos> &,
                const unique_ptr<thermodynamic> &, const vector3d<double> &,
                const double &, const double &, const double &, const double &,
                const double &);

  // destructor
  ~inviscidFlux() noexcept {}
};

// function definitions
// function to calculate Roe flux with entropy fix
inviscidFlux RoeFlux(const primVars &, const primVars &,
                     const unique_ptr<eos> &, const unique_ptr<thermodynamic> &,
                     const vector3d<double> &);
inviscidFlux AUSMFlux(const primVars &, const primVars &,
                      const unique_ptr<eos> &,
                      const unique_ptr<thermodynamic> &,
                      const vector3d<double> &);
inviscidFlux InviscidFlux(const primVars &, const primVars &,
                          const unique_ptr<eos> &,
                          const unique_ptr<thermodynamic> &,
                          const vector3d<double> &, const string &);
inviscidFlux RusanovFlux(const primVars &, const primVars &,
                         const unique_ptr<eos> &,
                         const unique_ptr<thermodynamic> &,
                         const vector3d<double> &, const bool &);

// function to calculate Roe flux with entropy fix for implicit methods
void ApproxRoeFluxJacobian(const primVars &, const primVars &,
                           const unique_ptr<eos> &, const vector3d<double> &,
                           double &, squareMatrix &, squareMatrix &);

genArray ConvectiveFluxUpdate(const primVars &, const primVars &,
                              const unique_ptr<eos> &,
                              const unique_ptr<thermodynamic> &,
                              const vector3d<double> &);

ostream &operator<<(ostream &os, const inviscidFlux &);

#endif
