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

#ifndef FLUXJACOBIANHEADERDEF  // only if the macro is not defined
                               // execute these lines of code
#define FLUXJACOBIANHEADERDEF  // define the macro

#include <vector>          // vector
#include <memory>          // unique_ptr
#include <type_traits>
#include "vector3d.hpp"
#include "tensor.hpp"
#include "uncoupledScalar.hpp"
#include "matrix.hpp"
#include "varArray.hpp"
#include "arrayView.hpp"

using std::vector;
using std::ostream;
using std::unique_ptr;

// forward class declarations
class primitive;
class conserved;
class eos;
class transport;
class thermodynamic;
class turbModel;
class input;

// This class holds the flux jacobians for the flow and turbulence equations.
// In the LU-SGS method the jacobians are scalars.

class fluxJacobian {
  squareMatrix flowJacobian_;
  squareMatrix turbJacobian_;

 public:
  // constructors
  fluxJacobian(const double &flow, const double &turb);
  fluxJacobian(const int &flowSize, const int &turbSize);
  fluxJacobian(const squareMatrix &flow, const squareMatrix &turb)
      : flowJacobian_(flow), turbJacobian_(turb) {}
  fluxJacobian() : fluxJacobian(0.0, 0.0) {}
  explicit fluxJacobian(const uncoupledScalar &specRad) :
      fluxJacobian(specRad.FlowVariable(), specRad.TurbVariable()) {}

  // move constructor and assignment operator
  fluxJacobian(fluxJacobian&&) noexcept = default;
  fluxJacobian& operator=(fluxJacobian&&) noexcept = default;

  // copy constructor and assignment operator
  fluxJacobian(const fluxJacobian&) = default;
  fluxJacobian& operator=(const fluxJacobian&) = default;

  // member functions
  squareMatrix FlowJacobian() const {return flowJacobian_;}
  squareMatrix TurbulenceJacobian() const {return turbJacobian_;}

  void AddToFlowJacobian(const squareMatrix &jac) {flowJacobian_ += jac;}
  void AddToTurbJacobian(const squareMatrix &jac) {turbJacobian_ += jac;}
  void SubtractFromFlowJacobian(const squareMatrix &jac) {flowJacobian_ -= jac;}
  void SubtractFromTurbJacobian(const squareMatrix &jac) {turbJacobian_ -= jac;}

  void MultiplyOnDiagonal(const double &, const bool &);
  void AddOnDiagonal(const double &, const bool &);

  template <typename T>
  void RusanovFluxJacobian(const T &, const unique_ptr<eos> &,
                           const unique_ptr<thermodynamic> &,
                           const unitVec3dMag<double> &, const bool &,
                           const input &, const unique_ptr<turbModel> &);
  template <typename T>
  void InvFluxJacobian(const T &, const unique_ptr<eos> &,
                       const unique_ptr<thermodynamic> &,
                       const unitVec3dMag<double> &, const input &,
                       const unique_ptr<turbModel> &);
  template <typename T1, typename T2>
  void ApproxRoeFluxJacobian(const T1 &, const T2 &, const unique_ptr<eos> &,
                             const unique_ptr<thermodynamic> &,
                             const unitVec3dMag<double> &, const bool &,
                             const input &, const unique_ptr<turbModel> &);
  template <typename T>
  void DelprimitiveDelConservative(const T &, const unique_ptr<thermodynamic> &,
                                   const unique_ptr<eos> &, const input &);

  template <typename T>
  void ApproxTSLJacobian(const T &, const double &, const double &,
                         const double &, const unique_ptr<eos> &,
                         const unique_ptr<transport> &,
                         const unique_ptr<thermodynamic> &,
                         const unitVec3dMag<double> &, const double &,
                         const unique_ptr<turbModel> &, const input &,
                         const bool &, const tensor<double> &);

  void Zero() {
    flowJacobian_.Zero();
    turbJacobian_.Zero();
  }

  template <typename T, 
            typename = std::enable_if_t<std::is_base_of<varArray, T>::value>>
  T ArrayMult(T) const;
  template <typename T,
            typename = std::enable_if_t<std::is_same<varArrayView, T>::value ||
                                        std::is_same<primitiveView, T>::value ||
                                        std::is_same<conservedView, T>::value ||
                                        std::is_same<residualView, T>::value>>
  auto ArrayMult(const T &arrView) const{
    auto arr = arrView.CopyData();
    return this->ArrayMult(arr);
  }
  bool IsScalar() const;
  void Inverse(const bool &);

  inline fluxJacobian & operator+=(const fluxJacobian &);
  inline fluxJacobian & operator-=(const fluxJacobian &);
  inline fluxJacobian & operator*=(const fluxJacobian &);
  inline fluxJacobian & operator/=(const fluxJacobian &);

  inline fluxJacobian & operator+=(const double &);
  inline fluxJacobian & operator-=(const double &);
  inline fluxJacobian & operator*=(const double &);
  inline fluxJacobian & operator/=(const double &);

  inline fluxJacobian operator+(const double &s) const {
    auto lhs = *this;
    return lhs += s;
  }
  inline fluxJacobian operator-(const double &s) const {
    auto lhs = *this;
    return lhs -= s;
  }
  inline fluxJacobian operator*(const double &s) const {
    auto lhs = *this;
    return lhs *= s;
  }
  inline fluxJacobian operator/(const double &s) const {
    auto lhs = *this;
    return lhs /= s;
  }

  friend inline const fluxJacobian operator-(const double &lhs,
                                             fluxJacobian rhs);
  friend inline const fluxJacobian operator/(const double &lhs,
                                             fluxJacobian rhs);

  // destructor
  ~fluxJacobian() noexcept {}
};

// function definitions

// operator overload for addition
fluxJacobian & fluxJacobian::operator+=(const fluxJacobian &other) {
  flowJacobian_ += other.flowJacobian_;
  turbJacobian_ += other.turbJacobian_;
  return *this;
}

// operator overload for subtraction with a scalar
fluxJacobian & fluxJacobian::operator-=(const fluxJacobian &other) {
  flowJacobian_ -= other.flowJacobian_;
  turbJacobian_ -= other.turbJacobian_;
  return *this;
}

// operator overload for elementwise multiplication
fluxJacobian & fluxJacobian::operator*=(const fluxJacobian &other) {
  flowJacobian_ *= other.flowJacobian_;
  turbJacobian_ *= other.turbJacobian_;
  return *this;
}

// operator overload for elementwise division
fluxJacobian & fluxJacobian::operator/=(const fluxJacobian &other) {
  flowJacobian_ /= other.flowJacobian_;
  turbJacobian_ /= other.turbJacobian_;
  return *this;
}

inline const fluxJacobian operator+(fluxJacobian lhs, const fluxJacobian &rhs) {
  return lhs += rhs;
}

inline const fluxJacobian operator-(fluxJacobian lhs, const fluxJacobian &rhs) {
  return lhs -= rhs;
}

inline const fluxJacobian operator*(fluxJacobian lhs, const fluxJacobian &rhs) {
  return lhs *= rhs;
}

inline const fluxJacobian operator/(fluxJacobian lhs, const fluxJacobian &rhs) {
  return lhs /= rhs;
}

// operator overloads for double -------------------------------------
// operator overload for addition
fluxJacobian & fluxJacobian::operator+=(const double &scalar) {
  flowJacobian_ += scalar;
  turbJacobian_ += scalar;
  return *this;
}

// operator overload for subtraction with a scalar
fluxJacobian & fluxJacobian::operator-=(const double &scalar) {
  flowJacobian_ -= scalar;
  turbJacobian_ -= scalar;
  return *this;
}

// operator overload for elementwise multiplication
fluxJacobian & fluxJacobian::operator*=(const double &scalar) {
  flowJacobian_ *= scalar;
  turbJacobian_ *= scalar;
  return *this;
}

// operator overload for elementwise division
fluxJacobian & fluxJacobian::operator/=(const double &scalar) {
  flowJacobian_ /= scalar;
  turbJacobian_ /= scalar;
  return *this;
}

inline const fluxJacobian operator+(const double &lhs, fluxJacobian rhs) {
  return rhs += lhs;
}

inline const fluxJacobian operator-(const double &lhs, fluxJacobian rhs) {
  rhs.flowJacobian_ = lhs - rhs.flowJacobian_;
  rhs.turbJacobian_ = lhs - rhs.turbJacobian_;
  return rhs;
}

inline const fluxJacobian operator*(const double &lhs, fluxJacobian rhs) {
  return rhs *= lhs;
}

inline const fluxJacobian operator/(const double &lhs, fluxJacobian rhs) {
  rhs.flowJacobian_ = lhs / rhs.flowJacobian_;
  rhs.turbJacobian_ = lhs / rhs.turbJacobian_;
  return rhs;
}

ostream &operator<<(ostream &os, const fluxJacobian &jacobian);

varArray RusanovScalarOffDiagonal(const primitiveView &, const varArrayView &,
                                  const unitVec3dMag<double> &, const double &,
                                  const double &, const double &,
                                  const double &, const unique_ptr<eos> &,
                                  const unique_ptr<thermodynamic> &,
                                  const unique_ptr<transport> &,
                                  const unique_ptr<turbModel> &, const bool &,
                                  const bool &);
varArray RusanovBlockOffDiagonal(const primitiveView &, const varArrayView &,
                                 const unitVec3dMag<double> &, const double &,
                                 const double &, const double &, const double &,
                                 const unique_ptr<eos> &,
                                 const unique_ptr<thermodynamic> &,
                                 const unique_ptr<transport> &,
                                 const unique_ptr<turbModel> &, const input &,
                                 const bool &, const tensor<double> &);

varArray RoeOffDiagonal(const primitiveView &, const primitiveView &,
                        const varArrayView &, const unitVec3dMag<double> &,
                        const double &, const double &, const double &,
                        const double &, const unique_ptr<eos> &,
                        const unique_ptr<thermodynamic> &,
                        const unique_ptr<transport> &,
                        const unique_ptr<turbModel> &, const bool &,
                        const bool &, const bool &);

varArray OffDiagonal(const primitiveView &, const primitiveView &,
                     const varArrayView &, const unitVec3dMag<double> &,
                     const double &, const double &, const double &,
                     const double &, const tensor<double> &,
                     const unique_ptr<eos> &, const unique_ptr<thermodynamic> &,
                     const unique_ptr<transport> &,
                     const unique_ptr<turbModel> &, const input &,
                     const bool &);

#endif
