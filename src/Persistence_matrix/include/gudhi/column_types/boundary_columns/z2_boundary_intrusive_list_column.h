/*    This file is part of the Gudhi Library - https://gudhi.inria.fr/ - which is released under MIT.
 *    See file LICENSE or go to https://gudhi.inria.fr/licensing/ for full license details.
 *    Author(s):       Hannah Schreiber
 *
 *    Copyright (C) 2022 Inria
 *
 *    Modification(s):
 *      - YYYY/MM Author: Description of the modification
 */

#ifndef B_Z2_INTRUSIVE_LIST_COLUMN_H
#define B_Z2_INTRUSIVE_LIST_COLUMN_H

#include <iostream>
#include <vector>

#include <boost/intrusive/list.hpp>

#include "../../utilities/utilities.h"
#include "../z2_intrusive_list_column.h"

namespace Gudhi {
namespace persistence_matrix {

template<class Cell_type, class Row_access_option>
class Z2_intrusive_list_boundary_column : public Z2_intrusive_list_column<Cell_type,Row_access_option>
{
private:
	using Base = Z2_intrusive_list_column<Cell_type,Row_access_option>;
	using Base::operator+=;		//kinda ugly, so TODO: organize better

public:
	using Cell = typename Base::Cell;
	using Column_type = typename Base::Column_type;
	using iterator = typename Base::iterator;
	using const_iterator = typename Base::const_iterator;

	Z2_intrusive_list_boundary_column();
	template<class Boundary_type>
	Z2_intrusive_list_boundary_column(const Boundary_type& boundary);
	template<class Boundary_type>
	Z2_intrusive_list_boundary_column(const Boundary_type& boundary, dimension_type dimension);
	template<class Row_container_type>
	Z2_intrusive_list_boundary_column(index columnIndex, Row_container_type &rowContainer);
	template<class Boundary_type, class Row_container_type>
	Z2_intrusive_list_boundary_column(index columnIndex, const Boundary_type& boundary, Row_container_type &rowContainer);
	template<class Boundary_type, class Row_container_type>
	Z2_intrusive_list_boundary_column(index columnIndex, const Boundary_type& boundary, dimension_type dimension, Row_container_type &rowContainer);
	Z2_intrusive_list_boundary_column(const Z2_intrusive_list_boundary_column& column);
	Z2_intrusive_list_boundary_column(const Z2_intrusive_list_boundary_column& column, index columnIndex);
	template<class Row_container_type>
	Z2_intrusive_list_boundary_column(const Z2_intrusive_list_boundary_column& column, index columnIndex, Row_container_type &rowContainer);
	Z2_intrusive_list_boundary_column(Z2_intrusive_list_boundary_column&& column) noexcept;

	int get_pivot() const;
	using Base::clear;
	void clear(index rowIndex);

	Z2_intrusive_list_boundary_column& operator+=(Z2_intrusive_list_boundary_column const &column);
	friend Z2_intrusive_list_boundary_column operator+(Z2_intrusive_list_boundary_column column1, Z2_intrusive_list_boundary_column const& column2){
		column1 += column2;
		return column1;
	}
	friend Z2_intrusive_list_boundary_column operator*(Z2_intrusive_list_boundary_column column, unsigned int const& v){
		column *= v;
		return column;
	}
	friend Z2_intrusive_list_boundary_column operator*(unsigned int const& v, Z2_intrusive_list_boundary_column column){
		column *= v;
		return column;
	}

	Z2_intrusive_list_boundary_column& operator=(const Z2_intrusive_list_boundary_column& other);

	friend void swap(Z2_intrusive_list_boundary_column& col1, Z2_intrusive_list_boundary_column& col2){
		swap(static_cast<Z2_intrusive_list_column<Cell_type,Row_access_option>&>(col1),
			 static_cast<Z2_intrusive_list_column<Cell_type,Row_access_option>&>(col2));
	}
};

template<class Cell_type, class Row_access_option>
inline Z2_intrusive_list_boundary_column<Cell_type,Row_access_option>::Z2_intrusive_list_boundary_column() : Base()
{}

template<class Cell_type, class Row_access_option>
template<class Boundary_type>
inline Z2_intrusive_list_boundary_column<Cell_type,Row_access_option>::Z2_intrusive_list_boundary_column(
		const Boundary_type &boundary)
	: Base(boundary)
{}

template<class Cell_type, class Row_access_option>
template<class Boundary_type>
inline Z2_intrusive_list_boundary_column<Cell_type,Row_access_option>::Z2_intrusive_list_boundary_column(
		const Boundary_type &boundary, dimension_type dimension)
	: Base(boundary, dimension)
{}

template<class Cell_type, class Row_access_option>
template<class Row_container_type>
inline Z2_intrusive_list_boundary_column<Cell_type,Row_access_option>::Z2_intrusive_list_boundary_column(
		index columnIndex, Row_container_type &rowContainer)
	: Base(columnIndex, rowContainer)
{}

template<class Cell_type, class Row_access_option>
template<class Boundary_type, class Row_container_type>
inline Z2_intrusive_list_boundary_column<Cell_type,Row_access_option>::Z2_intrusive_list_boundary_column(
		index columnIndex, const Boundary_type &boundary, Row_container_type &rowContainer)
	: Base(columnIndex, boundary, rowContainer)
{}

template<class Cell_type, class Row_access_option>
template<class Boundary_type, class Row_container_type>
inline Z2_intrusive_list_boundary_column<Cell_type,Row_access_option>::Z2_intrusive_list_boundary_column(
		index columnIndex, const Boundary_type &boundary, dimension_type dimension, Row_container_type &rowContainer)
	: Base(columnIndex, boundary, dimension, rowContainer)
{}

template<class Cell_type, class Row_access_option>
inline Z2_intrusive_list_boundary_column<Cell_type,Row_access_option>::Z2_intrusive_list_boundary_column(
		const Z2_intrusive_list_boundary_column &column)
	: Base(static_cast<const Base&>(column))
{}

template<class Cell_type, class Row_access_option>
inline Z2_intrusive_list_boundary_column<Cell_type,Row_access_option>::Z2_intrusive_list_boundary_column(
		const Z2_intrusive_list_boundary_column &column, index columnIndex)
	: Base(static_cast<const Base&>(column), columnIndex)
{}

template<class Cell_type, class Row_access_option>
template<class Row_container_type>
inline Z2_intrusive_list_boundary_column<Cell_type,Row_access_option>::Z2_intrusive_list_boundary_column(
		const Z2_intrusive_list_boundary_column& column, index columnIndex, Row_container_type &rowContainer)
	: Base(static_cast<const Base&>(column), columnIndex, rowContainer)
{}

template<class Cell_type, class Row_access_option>
inline Z2_intrusive_list_boundary_column<Cell_type,Row_access_option>::Z2_intrusive_list_boundary_column(
		Z2_intrusive_list_boundary_column &&column) noexcept
	: Base(std::move(static_cast<Base&&>(column)))
{}

template<class Cell_type, class Row_access_option>
inline int Z2_intrusive_list_boundary_column<Cell_type,Row_access_option>::get_pivot() const
{
	if (Base::column_.empty()) return -1;

	return Base::column_.back().get_row_index();
}

template<class Cell_type, class Row_access_option>
inline void Z2_intrusive_list_boundary_column<Cell_type,Row_access_option>::clear(index rowIndex)
{
	auto it = Base::column_.begin();
	while (it != Base::column_.end() && it->get_row_index() != rowIndex) it++;
	if (it != Base::column_.end()) Base::_delete_cell(it);
}

template<class Cell_type, class Row_access_option>
inline Z2_intrusive_list_boundary_column<Cell_type,Row_access_option> &
Z2_intrusive_list_boundary_column<Cell_type,Row_access_option>::operator+=(Z2_intrusive_list_boundary_column const &column)
{
	Base::operator+=(column);

	return *this;
}

template<class Cell_type, class Row_access_option>
inline Z2_intrusive_list_boundary_column<Cell_type,Row_access_option> &
Z2_intrusive_list_boundary_column<Cell_type,Row_access_option>::operator=(const Z2_intrusive_list_boundary_column &other)
{
	Base::operator=(static_cast<const Base&>(other));
	return *this;
}

} //namespace persistence_matrix
} //namespace Gudhi

#endif // Z2_INTRUSIVE_LIST_COLUMN_H
