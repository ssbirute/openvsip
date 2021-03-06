//
// Copyright (c) 2005, 2006, 2008 by CodeSourcery
// Copyright (c) 2013 Stefan Seefeld
// All rights reserved.
//
// This file is part of OpenVSIP. It is made available under the
// See the accompanying file LICENSE.GPL for details.

#include <vsip/initfin.hpp>
#include <vsip/support.hpp>
#include <vsip/dense.hpp>
#include <vsip/vector.hpp>
#include <ovxx/dda.hpp>
#include <test.hpp>

using namespace ovxx;

// Sum values in a dense block via Direct_data low-level interface.

float
raw_sum(Dense<1, float>& block)
{
  ovxx::dda::Accessor<
    Dense<1, float>,
    Dense<1, float>::layout_type,
    ovxx::dda::direct_access>
    raw(block);
  float*      ptr   = raw.ptr();
  stride_type stride = raw.stride(0);

  float sum = 0.0f;
  
  for (index_type i=0; i<block.size(); ++i)
  {
    sum += *ptr;
    ptr += stride;
  }
  return sum;
}



// Sum values in a dense block via dda::Data interface.

float
ext_sum(Dense<1, float>& block)
{
  vsip::dda::Data<Dense<1, float>, vsip::dda::in> data(block);
  float const *ptr   = data.ptr();
  stride_type stride = data.stride(0);

  float sum = 0.0f;
  
  for (index_type i=0; i<block.size(); ++i)
  {
    sum += *ptr;
    ptr += stride;
  }

  return sum;
}



// Sum values in a dense block via block interface.
float
blk_sum(Dense<1, float>& block)
{
  float sum = 0.0f;
  
  for (index_type i=0; i<block.size(); ++i)
    sum += block.get(i);

  return sum;
}



// Sum values in a block via dda::Data interface.

/// For Dense, this should use data_interface::Direct_data interface
/// For Strided, this should use data_interface::Copy_data interface

template <typename Block>
typename Block::value_type
gen_ext_sum(Block& block)
{
  typedef typename Block::value_type value_type;
  
  vsip::dda::Data<Block, vsip::dda::in> data(block);

  value_type const *ptr = data.ptr();
  stride_type stride = data.stride(0);

  value_type sum = value_type();
  
  for (index_type i=0; i<block.size(); ++i)
  {
    sum += *ptr;
    ptr += stride;
  }

  return sum;
}



// Sum values in a block via block interface.

template <typename Block>
typename Block::value_type
gen_blk_sum(Block const& block)
{
  typedef typename Block::value_type value_type;

  value_type sum = value_type();
  
  for (index_type i=0; i<block.size(); ++i)
    sum += block.get(i);

  return sum;
}



// Sum values in a view using view-interface.

template <typename T,
	  typename Block>
T
sum_view(const_Vector<T, Block> view)
{
  T sum = T();
  for (index_type i=0; i<view.size(); ++i)
    sum += view.get(i);
  return sum;
}



// Sum values in a view using dda::Data-interface.

template <typename T,
	  typename Block>
T
sum_ext(const_Vector<T, Block> view)
{
  vsip::dda::Data<Block, vsip::dda::in> data(view.block());
  float const *ptr   = data.ptr();
  stride_type stride = data.stride(0);

  T sum = T();
  
  for (index_type i=0; i<view.size(); ++i)
  {
    sum  += *ptr;
    ptr += stride;
  }

  return sum;
}



/// Use external-data interface for element-wise vector addition.

template <typename T,
	  typename Block1,
	  typename Block2,
	  typename Block3>
void
vector_add(
  const_Vector<T, Block1> res,
  const_Vector<T, Block2> op1,
  const_Vector<T, Block3> op2)
{
  vsip::dda::Data<Block1, vsip::dda::out> data_res(res.block());
  float*   p_res   = data_res.ptr();
  stride_type str_res = data_res.stride(0);

  vsip::dda::Data<Block2, vsip::dda::in> data1(op1.block());
  float const *p1   = data1.ptr();
  stride_type str1 = data1.stride(0);

  vsip::dda::Data<Block3, vsip::dda::in> data2(op2.block());
  float const *p2   = data2.ptr();
  stride_type str2 = data2.stride(0);

  for (index_type i=0; i<res.size(); ++i)
  {
    *p_res = *p1 + *p2;

    p1    += str1;
    p2    += str2;
    p_res += str_res;
  }
}



// Dot-product of two views using view interface.

template <typename T1,
	  typename T2,
	  typename Block1,
	  typename Block2>
typename Promotion<T1, T2>::type
dotp_view(
  const_Vector<T1, Block1> op1,
  const_Vector<T2, Block2> op2)
{
  typedef typename Promotion<T1, T2>::type value_type;

  test_assert(op1.size() == op2.size());

  value_type sum = value_type();
  
  for (index_type i=0; i<op1.size(); ++i)
  {
    sum  += op1.get(i) * op2.get(i);
  }

  return sum;
}



// Dot-product of two views using dda::Data interface.

template <typename T1,
	  typename T2,
	  typename Block1,
	  typename Block2>
typename Promotion<T1, T2>::type
dotp_ext(
  const_Vector<T1, Block1> op1,
  const_Vector<T2, Block2> op2)
{
  typedef typename Promotion<T1, T2>::type value_type;

  test_assert(op1.size() == op2.size());

  vsip::dda::Data<Block1, vsip::dda::in> data1(op1.block());
  T1 const *p1   = data1.ptr();
  stride_type str1 = data1.stride(0);

  vsip::dda::Data<Block2, vsip::dda::in> data2(op2.block());
  T2 const *p2   = data2.ptr();
  stride_type str2 = data2.stride(0);

  value_type sum = value_type();
  
  for (index_type i=0; i<op1.size(); ++i)
  {
    sum  += *p1 * *p2;
    p1 += str1;
    p2 += str2;
  }

  return sum;
}



// Test various block summations.

void
test_block_sum()
{
  length_type const size = 10;

  Dense<1, float> block(size,   0.f);
  ovxx::Strided<1, float> pb(size, 0.f);

  block.put(0, 1.f);
  block.put(1, 3.14f);
  block.put(2, 2.78f);

  pb.put(0, 1.f);
  pb.put(1, 3.14f);
  pb.put(2, 2.78f);

  float sum = 1.f + 3.14f + 2.78f;

  test_assert(equal(sum, raw_sum(block)));
  test_assert(equal(sum, ext_sum(block)));
  test_assert(equal(sum, blk_sum(block)));

  test_assert(equal(sum, gen_ext_sum(block)));
  test_assert(equal(sum, gen_blk_sum(block)));
  test_assert(equal(sum, gen_ext_sum(pb)));
  test_assert(equal(sum, gen_blk_sum(pb)));
}



/// Test low-level data interface to 1-dimensional blocks.

template <typename Block, ovxx::dda::access_kind K>
void
test_1_low()
{
  length_type const size = 10;

  typedef typename Block::value_type value_type;

  Block block(size, 0.0);

  value_type val0 =  1.0f;
  value_type val1 =  2.78f;
  value_type val2 =  3.14f;
  value_type val3 = -1.5f;

  // Place values in block.
  block.put(0, val0);
  block.put(1, val1);

  {
    ovxx::dda::Accessor<Block, typename Block::layout_type, K>
      raw(block);

    // Check properties of LLDI.
    test_assert(raw.stride(0) == 1);
    test_assert(raw.size(0) == size);

    float* ptr = raw.ptr();

    // Check that block values are reflected.
    test_assert(equal(ptr[0], val0));
    test_assert(equal(ptr[1], val1));

    // Place values in raw data.
    ptr[1] = val2;
    ptr[2] = val3;
  }

  // Check that raw data values are reflected.
  test_assert(equal(block.get(1), val2));
  test_assert(equal(block.get(2), val3));
}



/// Test high-level data interface to 1-dimensional blocks.

template <typename Block>
void
test_1_ext()
{
  length_type const size = 10;

  typedef typename Block::value_type value_type;

  Block block(size, 0.0);

  value_type val0 =  1.0f;
  value_type val1 =  2.78f;
  value_type val2 =  3.14f;
  value_type val3 = -1.5f;

  // Place values in block.
  block.put(0, val0);
  block.put(1, val1);

  {
    vsip::dda::Data<Block, vsip::dda::inout> data(block);

    // Check properties of DDI.
    test_assert(data.stride(0) == 1);
    test_assert(data.size(0) == size);

    float* ptr = data.ptr();

    // Check that block values are reflected.
    test_assert(equal(ptr[0], val0));
    test_assert(equal(ptr[1], val1));

    // Place values in raw data.
    ptr[1] = val2;
    ptr[2] = val3;
  }

  // Check that raw data values are reflected.
  test_assert(equal(block.get(1), val2));
  test_assert(equal(block.get(2), val3));
}



// Test high-level data interface to 2-dim Dense.

void
test_dense_2()
{
  length_type const num_rows = 10;
  length_type const num_cols = 15;

  typedef Dense<2, float, row2_type> Row_major_block;
  typedef Dense<2, float, col2_type> Col_major_block;

  Row_major_block row_blk(Domain<2>(num_rows, num_cols), 0.0);
  Col_major_block col_blk(Domain<2>(num_rows, num_cols), 0.0);

  vsip::dda::Data<Row_major_block, vsip::dda::inout> row_data(row_blk);
  vsip::dda::Data<Col_major_block, vsip::dda::inout> col_data(col_blk);

  test_assert(row_data.stride(0) == static_cast<stride_type>(num_cols));
  test_assert(row_data.stride(1) == 1U);
  test_assert(row_data.size(0) == num_rows);
  test_assert(row_data.size(1) == num_cols);

  test_assert(col_data.stride(0) == 1U);
  test_assert(col_data.stride(1) == static_cast<stride_type>(num_rows));
  test_assert(col_data.size(0) == num_rows);
  test_assert(col_data.size(1) == num_cols);
}



// Test that 1-dimensional and 2-dimensional data interfaces to Dense
// are consistent.

template <typename T,
	  typename Order>
void
test_dense_12()
{
  using vsip::Layout;
  using vsip::dda::in;

  length_type const num_rows = 10;
  length_type const num_cols = 15;

  typedef Dense<2, T, Order> Block;

  Block block(Domain<2>(num_rows, num_cols));

  // place values into block for comparison.
  for (index_type r=0; r<num_rows; ++r)
    for (index_type c=0; c<num_cols; ++c)
      block.put(r, c, T(r*num_cols+c));

  // Check 2-dimensional data access.
  {
    vsip::dda::Data<Block, vsip::dda::in, Layout<2, Order,
      Block::packing,
      Block::storage_format> > data(block);

    test_assert(data.stride(Order::Dim0) == 
	   static_cast<stride_type>(Order::Dim0 == 0 ? num_cols : num_rows));
    test_assert(data.stride(Order::Dim1) == 1);

    test_assert(data.size(0) == num_rows);
    test_assert(data.size(1) == num_cols);

    // Cost should be zero:
    //  - Block is Dense, supports Direct_data,
    //  - Requested layout is same as blocks.
    test_assert(data.ct_cost == 0);

    for (index_type r=0; r<data.size(0); ++r)
      for (index_type c=0; c<data.size(1); ++c)
	test_assert(equal(data.ptr()[r*data.stride(0) + c*data.stride(1)],
		     T(r*num_cols + c)));
  }



  // Check 1-dimensional data access.
  {
    vsip::dda::Data<Block, vsip::dda::in, Layout<1, row1_type,
      Block::packing,
      Block::storage_format> > data(block);

    test_assert(data.stride(0) == 1);

    test_assert(data.size(0) == num_rows*num_cols);

    // Cost should be zero:
    //  - Block is Dense, supports Direct_data,
    //  - Requested 1-dim layout is supported.
    test_assert(data.ct_cost == 0);

    for (index_type r=0; r<num_rows; ++r)
    {
      for (index_type c=0; c<num_cols; ++c)
      {
	index_type idx = (Order::Dim0 == 0)
	            ? (r * num_cols + c)	// row-major
	            : (r + c * num_rows);	// col-major

	test_assert(equal(data.ptr()[idx], T(r*num_cols + c)));
      }
    }
  }
}



/// Test view sum and dot-product functions.

void
test_view_functions()
{
  length_type size = 10;
  Vector<float> view1(size);
  Vector<float> view2(size);
  
  for (index_type i=0; i<size; ++i)
  {
    view1.put(i, float(i+1));
    view2.put(i, float(2*i+1));
  }

  test_assert(equal(sum_view(view1), sum_ext(view1)));
  test_assert(equal(sum_view(view2), sum_ext(view2)));

  float prod_v = dotp_view(view1, view2);
  float prod_e  = dotp_ext(view1, view2);

  test_assert(equal(prod_v, prod_e));
}



/// Test vector_add function.

void
test_vector_add()
{
  length_type size = 10;
  Vector<float> view1(size);
  Vector<float> view2(size);
  Vector<float> res(size);
  
  for (index_type i=0; i<size; ++i)
  {
    view1.put(i, float(i+1));
    view2.put(i, float(2*i+1));
  }

  vector_add(res, view1, view2);

  for (index_type i=0; i<size; ++i)
  {
    test_assert(equal(res.get(i), view1.get(i) + view2.get(i)));
  }
}



int
main(int argc, char** argv)
{
  vsip::vsipl init(argc, argv);

  using ovxx::dda::direct_access;
  using ovxx::dda::copy_access;

  test_block_sum();

  // Test low-level data interface.
  test_1_low<Dense<1, float>, direct_access>();
  test_1_low<Dense<1, float>, copy_access>();

  // test_1_low<Strided<1, float>, vsip::impl::Ref_count_policy, Direct_data>();
  test_1_low<ovxx::Strided<1, float>, copy_access>();

  // Test high-level data interface.
  test_1_ext<Dense<1,      float> >();
  test_1_ext<ovxx::Strided<1, float> >();

  test_dense_2();

  // Test 1-dim direct data views of N-dim blocks.
  // test_dense_12<float, row2_type>();
  // test_dense_12<float, col2_type>();
  // test_dense_12<int,   row2_type>();
  // test_dense_12<int,   col2_type>();
  // test_dense_12<short, row2_type>();
  // test_dense_12<short, col2_type>();

  test_view_functions();

  test_vector_add();
}
