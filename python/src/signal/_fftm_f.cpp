//
// Copyright (c) 2014 Stefan Seefeld
// All rights reserved.
//
// This file is part of OpenVSIP. It is made available under the
// license contained in the accompanying LICENSE.BSD file.

#include "fftm.hpp"

BOOST_PYTHON_MODULE(_fftm_f)
{
  using namespace pyvsip;
  define_real_fftm<float>();
}
