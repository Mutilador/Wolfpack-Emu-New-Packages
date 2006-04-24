//
//  Copyright (c) 2000-2002
//  Joerg Walter, Mathias Koch
//
//  Permission to use, copy, modify, distribute and sell this software
//  and its documentation for any purpose is hereby granted without fee,
//  provided that the above copyright notice appear in all copies and
//  that both that copyright notice and this permission notice appear
//  in supporting documentation.  The authors make no representations
//  about the suitability of this software for any purpose.
//  It is provided "as is" without express or implied warranty.
//
//  The authors gratefully acknowledge the support of
//  GeNeSys mbH & Co. KG in producing this work.
//

#ifndef BOOST_UBLAS_FWD_H
#define BOOST_UBLAS_FWD_H

#include <memory>

namespace boost { namespace numeric { namespace ublas {

    // Storage types
    template<class T, class ALLOC = std::allocator<T> >
    class unbounded_array;

    template<class T, std::size_t N, class ALLOC = std::allocator<T> >
    class bounded_array;

    template <class Z = std::size_t, class D = std::ptrdiff_t>
    class basic_range;
    template <class Z = std::size_t, class D = std::ptrdiff_t>
    class basic_slice;
    typedef basic_range<> range;
    typedef basic_slice<> slice;
    template<class A = unbounded_array<std::size_t> >
    class indirect_array;

    template<class I, class T, class ALLOC = std::allocator<std::pair<const I, T> > >
    class map_std;
    template<class I, class T, class ALLOC = std::allocator<std::pair<I, T> > >
    class map_array;

    // Expression types
    struct scalar_tag {};
    
    struct vector_tag {};
    template<class E>
    class vector_expression;
    template<class C>
    class vector_container;

    template<class E>
    class vector_reference;

    struct matrix_tag {};

    template<class E>
    class matrix_expression;
    template<class C>
    class matrix_container;

    template<class E>
    class matrix_reference;

    template<class E>
    class vector_range;
    template<class E>
    class vector_slice;
    template<class E, class IA = indirect_array<> >
    class vector_indirect;

    template<class E>
    class matrix_row;
    template<class E>
    class matrix_column;
    template<class E>
    class matrix_range;
    template<class E>
    class matrix_slice;
    template<class E, class IA = indirect_array<> >
    class matrix_indirect;

    template<class T, class A = unbounded_array<T> >
    class vector;
    template<class T, std::size_t N>
    class bounded_vector;

    template<class T = int>
    class unit_vector;
    template<class T = int>
    class zero_vector;
    template<class T = int>
    class scalar_vector;

    template<class T, std::size_t N>
    class c_vector;

    // Sparse vectors
    template<class T, class A = map_std<std::size_t, T> >
    class mapped_vector;
    template<class T, std::size_t IB = 0, class IA = unbounded_array<std::size_t>, class TA = unbounded_array<T> >
    class compressed_vector;
    template<class T, std::size_t IB = 0, class IA = unbounded_array<std::size_t>, class TA = unbounded_array<T> >
    class coordinate_vector;

    // Matrix orientation type
    struct unknown_orientation_tag {};
    struct row_major_tag {};
    struct column_major_tag {};

    // Matrix storage layout parameterisation
    template <class Z = std::size_t, class D = std::ptrdiff_t>
    struct basic_row_major;
    typedef basic_row_major<> row_major;

    template <class Z = std::size_t, class D = std::ptrdiff_t>
    struct basic_column_major;
    typedef basic_column_major<> column_major;

    template<class T, class L = row_major, class A = unbounded_array<T> >
    class matrix;
    template<class T, std::size_t M, std::size_t N, class L = row_major>
    class bounded_matrix;

    template<class T = int>
    class identity_matrix;
    template<class T = int>
    class zero_matrix;
    template<class T = int>
    class scalar_matrix;

    template<class T, std::size_t M, std::size_t N>
    class c_matrix;

    template<class T, class L = row_major, class A = unbounded_array<unbounded_array<T> > >
    class vector_of_vector;

    // Triangular matrix type
    struct lower_tag {};
    struct upper_tag {};
    struct unit_lower_tag : public lower_tag {};
    struct unit_upper_tag : public upper_tag {};
    struct strict_lower_tag : public lower_tag {};
    struct strict_upper_tag : public upper_tag {};

    // Triangular matrix parameterisation
    template <class Z = std::size_t>
    struct basic_full;
    typedef basic_full<> full;

    template <class Z = std::size_t>
    struct basic_lower;
    typedef basic_lower<> lower;

    template <class Z = std::size_t>
    struct basic_upper;
    typedef basic_upper<> upper;

    template <class Z = std::size_t>
    struct basic_unit_lower;
    typedef basic_unit_lower<> unit_lower;

    template <class Z = std::size_t>
    struct basic_unit_upper;
    typedef basic_unit_upper<> unit_upper;

    template <class Z = std::size_t>
    struct basic_strict_lower;
    typedef basic_strict_lower<> strict_lower;

    template <class Z = std::size_t>
    struct basic_strict_upper;
    typedef basic_strict_upper<> strict_upper;

    // Special matrices
    template<class T, class L = row_major, class A = unbounded_array<T> >
    class banded_matrix;
    template<class T, class L = row_major, class A = unbounded_array<T> >
    class diagonal_matrix;

    template<class T, class TRI = lower, class L = row_major, class A = unbounded_array<T> >
    class triangular_matrix;
    template<class M, class TRI = lower>
    class triangular_adaptor;

    template<class T, class TRI = lower, class L = row_major, class A = unbounded_array<T> >
    class symmetric_matrix;
    template<class M, class TRI = lower>
    class symmetric_adaptor;

    template<class T, class TRI = lower, class L = row_major, class A = unbounded_array<T> >
    class hermitian_matrix;
    template<class M, class TRI = lower>
    class hermitian_adaptor;

    // Sparse matrices
    template<class T, class L = row_major, class A = map_std<std::size_t, T> >
    class mapped_matrix;
    template<class T, class L = row_major, class A = map_std<std::size_t, map_std<std::size_t, T> > >
    class mapped_vector_of_mapped_vector;
    template<class T, class L = row_major, std::size_t IB = 0, class IA = unbounded_array<std::size_t>, class TA = unbounded_array<T> >
    class compressed_matrix;
    template<class T, class L = row_major, std::size_t IB = 0, class IA = unbounded_array<std::size_t>, class TA = unbounded_array<T> >
    class coordinate_matrix;

}}}

#endif
