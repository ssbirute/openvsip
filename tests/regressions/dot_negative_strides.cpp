//
// Copyright (c) 2008 by CodeSourcery
// Copyright (c) 2013 Stefan Seefeld
// All rights reserved.
//
// This file is part of OpenVSIP. It is made available under the
// license contained in the accompanying LICENSE.GPL file.

//    Regression test: The FORTRAN dot routine (et al.) expects a pointer to
//      the start of an array, even if the strides are negative.

#include <vsip/math.hpp>
#include <vsip/vector.hpp>
#include <vsip/initfin.hpp>
#include <test.hpp>
#include <iostream>

int main(int, char **)
{
  typedef std::complex<float> value_type;

  vsip::vsipl library;

  float data_a_i[] = { 1.1, 1.2, 2.1, 2.2, -3.1, -3.3};
  float data_a_r[] = { 2.1, 3.2, -2.1, -2.2, +5.1, +5.3};
  float data_b_i[] = {10.1, 11.2, 22.1, 12.2, -13.1, -0.3};
  float data_b_r[] = {8.1, 10.2, -12.1, 10.2, -11.1, -2.3};
  std::complex<float> ans = value_type(-155.58,-24.42);

  vsip::Vector<value_type> super(6);
  vsip::Vector<value_type> a(6);
  vsip::Vector<value_type>::subview_type a_inv = super(vsip::Domain<1>(5, -1, 6));
  vsip::Vector<value_type> b(6);
  for (size_t i = 0; i != a.size(); ++i)
  {
    a.put(i, value_type(data_a_r[i], data_a_i[i]));
    a_inv.put(i, value_type(data_a_r[i], data_a_i[i]));
    b.put(i, value_type(data_b_r[i], data_b_i[i]));
  }
  // Compute a simple dot-product...
  // ...and compare it to one computed on a view with negative strides.
  value_type result1 = vsip::dot(a, b);
  value_type result2 = vsip::dot(a_inv, b);
  test_assert(ovxx::equal(result1, result2));
  return 0;
}
