// This file is part of kfilter.
// kfilter is a C++ variable-dimension extended kalman filter library.
//
// Copyright (C) 2004        Vincent Zalzal, Sylvain Marleau
// Copyright (C) 2001, 2004  Richard Gourdeau
// Copyright (C) 2004        GRPR and DGE's Automation sector
//                           �cole Polytechnique de Montr�al
//
// Code adapted from algorithms presented in :
//      Bierman, G. J. "Factorization Methods for Discrete Sequential
//      Estimation", Academic Press, 1977.
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

#ifndef KFILTER_IMPL_HPP
#define KFILTER_IMPL_HPP

//! \file
//! \brief Contains the implementation of the \c KFilter base template class.

namespace Kalman {

  template<typename T, K_UINT_32 BEG, bool OQ, bool OVR, bool DBG>
  KFilter<T, BEG, OQ, OVR, DBG>::~KFilter() {}

  template<typename T, K_UINT_32 BEG, bool OQ, bool OVR, bool DBG>
  void KFilter<T, BEG, OQ, OVR, DBG>::makeBaseB() {}

  template<typename T, K_UINT_32 BEG, bool OQ, bool OVR, bool DBG>
  void KFilter<T, BEG, OQ, OVR, DBG>::makeB() {}

  template<typename T, K_UINT_32 BEG, bool OQ, bool OVR, bool DBG>
  void KFilter<T, BEG, OQ, OVR, DBG>::makeProcess() {

    // x = Ax + Bu + Ww    n.1 = n.n * n.1 + n.nu * nu.1
    makeB();

    K_UINT_32 i, j;
    x__.resize(EKFilter<T, BEG, OQ, OVR, DBG>::n);

    for (i = BEG; i < EKFilter<T, BEG, OQ, OVR, DBG>::n + BEG; ++i) {

      EKFilter<T, BEG, OQ, OVR, DBG>::x__(i) = T(0.0);

      for (j = BEG; j < EKFilter<T, BEG, OQ, OVR, DBG>::n + BEG; ++j)
        EKFilter<T, BEG, OQ, OVR, DBG>::x__(i) += EKFilter<T, BEG, OQ, OVR, DBG>::A(i,j) * EKFilter<T, BEG, OQ, OVR, DBG>::x(j);

      for (j = BEG; j < EKFilter<T, BEG, OQ, OVR, DBG>::nu + BEG; ++j)
        EKFilter<T, BEG, OQ, OVR, DBG>::x__(i) += B(i,j) * EKFilter<T, BEG, OQ, OVR, DBG>::u(j);

    }

    EKFilter<T, BEG, OQ, OVR, DBG>::x.swap(EKFilter<T, BEG, OQ, OVR, DBG>::x__);

  }

  template<typename T, K_UINT_32 BEG, bool OQ, bool OVR, bool DBG>
  void KFilter<T, BEG, OQ, OVR, DBG>::makeMeasure() {
    
    // z = Hx + Vv
    K_UINT_32 i, j;

    EKFilter<T, BEG, OQ, OVR, DBG>::z.resize(EKFilter<T, BEG, OQ, OVR, DBG>::m);
    for (i = BEG; i < EKFilter<T, BEG, OQ, OVR, DBG>::m + BEG; ++i) {

      EKFilter<T, BEG, OQ, OVR, DBG>::z(i) = T(0.0);

      for (j = BEG; j < EKFilter<T, BEG, OQ, OVR, DBG>::n + BEG; ++j)
        EKFilter<T, BEG, OQ, OVR, DBG>::z(i) += EKFilter<T, BEG, OQ, OVR, DBG>::H(i,j) * EKFilter<T, BEG, OQ, OVR, DBG>::x(j);

    }

  }

  template<typename T, K_UINT_32 BEG, bool OQ, bool OVR, bool DBG>
  void KFilter<T, BEG, OQ, OVR, DBG>::sizeUpdate() {

    if (EKFilter<T, BEG, OQ, OVR, DBG>::flags & ( KALMAN_N_MODIFIED | KALMAN_NU_MODIFIED ) ) {
      B.resize(EKFilter<T, BEG, OQ, OVR, DBG>::n, EKFilter<T, BEG, OQ, OVR, DBG>::nu);
      makeBaseB();
    }   
    
    EKFilter<T, BEG, OQ, OVR, DBG>::sizeUpdate();
  }

}

#endif
