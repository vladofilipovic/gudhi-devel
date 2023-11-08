/*    This file is part of the Gudhi Library - https://gudhi.inria.fr/ - which is released under MIT.
 *    See file LICENSE or go to https://gudhi.inria.fr/licensing/ for full license details.
 *    Author(s):       Hannah Schreiber
 *
 *    Copyright (C) 2023 Inria
 *
 *    Modification(s):
 *      - YYYY/MM Author: Description of the modification
 */
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "persistence_matrix"
// #define BOOST_MPL_CFG_NO_PREPROCESSED_HEADERS
// #define BOOST_MPL_LIMIT_LIST_SIZE 10
#include <boost/test/unit_test.hpp>
// #include <boost/mpl/list.hpp>
// #include <boost/mp11.hpp>

#include "pm_column_tests.h"
#include "pm_column_tests_boost_type_lists.h"

using option_name_list = mp_list_q<c_base_options>;
using no_row_access_columns = columns_list<no_ra_option_list<option_name_list> >;
using only_row_access_columns = columns_list<only_ra_option_list<option_name_list> >;
using z2_no_row_access_columns = columns_list<z2_no_ra_option_list<option_name_list> >;
using z2_only_row_access_columns = columns_list<z2_only_ra_option_list<option_name_list> >;
using z5_no_row_access_columns = columns_list<z5_no_ra_option_list<option_name_list> >;
using z5_only_row_access_columns = columns_list<z5_only_ra_option_list<option_name_list> >;
using z5_columns = columns_list<z5_option_list<option_name_list> >;
using z2_columns = columns_list<z2_option_list<option_name_list> >;

// using opt = Base_col_options<true,Column_types::HEAP,false,false,false>;
// using mat = Column_mini_matrix<opt>;
// typedef boost::mpl::list<Heap_column<mat> > test;

BOOST_AUTO_TEST_CASE_TEMPLATE(Base_column_constructors, Column, no_row_access_columns) {
	column_test_common_constructors<Column>();
	column_test_base_boundary_constructors<Column>();
}

BOOST_AUTO_TEST_CASE_TEMPLATE(Base_z5_column_content_access, Column, z5_no_row_access_columns) {
	std::vector<Column> matrix = build_column_matrix<Column>();
	column_test_common_z5_content_access(matrix);
}

BOOST_AUTO_TEST_CASE_TEMPLATE(Base_z2_column_content_access, Column, z2_no_row_access_columns) {
	std::vector<Column> matrix = build_column_matrix<Column>();
	column_test_common_z2_content_access(matrix);
}

BOOST_AUTO_TEST_CASE_TEMPLATE(Base_z5_column_with_row_access_content_access, Column, z5_only_row_access_columns) {
	typename Column::Master::row_container_type rows;	//do not destroy before matrix
	std::vector<Column> matrix = build_column_matrix<Column>(rows);
	column_test_common_z5_content_access(matrix);
}

BOOST_AUTO_TEST_CASE_TEMPLATE(Base_z2_column_with_row_access_content_access, Column, z2_only_row_access_columns) {
	typename Column::Master::row_container_type rows;	//do not destroy before matrix
	std::vector<Column> matrix = build_column_matrix<Column>(rows);
	column_test_common_z2_content_access(matrix);
}

BOOST_AUTO_TEST_CASE_TEMPLATE(Base_z5_column_operators, Column, z5_no_row_access_columns) {
	std::vector<Column> matrix = build_column_matrix<Column>();
	column_test_common_z5_operators(matrix);

	matrix = build_column_matrix<Column>();
	column_test_base_z5_operators(matrix);
}

BOOST_AUTO_TEST_CASE_TEMPLATE(Base_z2_column_operators, Column, z2_no_row_access_columns) {
	std::vector<Column> matrix = build_column_matrix<Column>();
	column_test_common_z2_operators(matrix);

	matrix = build_column_matrix<Column>();
	column_test_base_z2_operators(matrix);
}

BOOST_AUTO_TEST_CASE_TEMPLATE(Base_z5_column_with_row_access_operators, Column, z5_only_row_access_columns) {
	typename Column::Master::row_container_type rows;	//do not destroy before matrix
	std::vector<Column> matrix = build_column_matrix<Column>(rows);
	column_test_common_z5_operators(matrix);

	matrix.clear();
	rows.clear();
	matrix = build_column_matrix<Column>(rows);
	column_test_base_z5_operators(matrix);
}

BOOST_AUTO_TEST_CASE_TEMPLATE(Base_z2_column_with_row_access_operators, Column, z2_only_row_access_columns) {
	typename Column::Master::row_container_type rows;	//do not destroy before matrix
	std::vector<Column> matrix = build_column_matrix<Column>(rows);
	column_test_common_z2_operators(matrix);

	matrix.clear();
	rows.clear();
	matrix = build_column_matrix<Column>(rows);
	column_test_base_z2_operators(matrix);
}

BOOST_AUTO_TEST_CASE_TEMPLATE(Base_column_row_access_constructors, Column, only_row_access_columns) {
	typename Column::Master::row_container_type rows;	//do not destroy before matrix
	std::vector<Column> matrix = build_column_matrix<Column>(rows);

	column_test_row_access_constructors(matrix, rows);
}

BOOST_AUTO_TEST_CASE_TEMPLATE(Base_z5_column_other, Column, z5_columns) {
	column_test_base_boundary_z5_methods<Column>();
}

BOOST_AUTO_TEST_CASE_TEMPLATE(Base_z2_column_other, Column, z2_columns) {
	column_test_base_boundary_z2_methods<Column>();
}