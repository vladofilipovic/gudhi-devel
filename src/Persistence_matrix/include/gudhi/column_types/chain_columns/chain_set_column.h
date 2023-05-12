/*    This file is part of the Gudhi Library - https://gudhi.inria.fr/ - which is released under MIT.
 *    See file LICENSE or go to https://gudhi.inria.fr/licensing/ for full license details.
 *    Author(s):       Hannah Schreiber
 *
 *    Copyright (C) 2022 Inria
 *
 *    Modification(s):
 *      - YYYY/MM Author: Description of the modification
 */

#ifndef C_SET_COLUMN_H
#define C_SET_COLUMN_H

#include <iostream>
#include <list>
#include <set>

#include "../../utilities/utilities.h"
#include "../set_column.h"

namespace Gudhi {
namespace persistence_matrix {

template<class Dictionnary_type, class Field_element_type, class Cell_type, class Row_access_option>
class Set_chain_column : public Set_column<Field_element_type,Cell_type,Row_access_option>
{
private:
	using Base = Set_column<Field_element_type,Cell_type,Row_access_option>;
	using Base::operator+=;				//kinda ugly, so TODO: organize better
	using Base::multiply_and_add;		//kinda ugly, so TODO: organize better

public:
	using Cell = typename Base::Cell;
	using Column_type = typename Base::Column_type;
	using iterator = typename Base::iterator;
	using const_iterator = typename Base::const_iterator;

	Set_chain_column(Dictionnary_type& pivotToColumnIndex);
	template<class Chain_type>
	Set_chain_column(const Chain_type& chain, dimension_type dimension, Dictionnary_type& pivotToColumnIndex);
	template<class Row_container_type>
	Set_chain_column(index columnIndex, Row_container_type &rowContainer, Dictionnary_type& pivotToColumnIndex);
	template<class Chain_type, class Row_container_type>
	Set_chain_column(index columnIndex, const Chain_type& chain, dimension_type dimension, Row_container_type &rowContainer, Dictionnary_type& pivotToColumnIndex);
	Set_chain_column(const Set_chain_column& column);
	Set_chain_column(const Set_chain_column& column, index columnIndex);
	Set_chain_column(Set_chain_column&& column) noexcept;

	int get_pivot() const;
	Field_element_type get_pivot_value();
	index get_paired_chain_index() const;
	bool is_paired() const;
	void assign_paired_chain(index other_col);
	void unassign_paired_chain();

	Set_chain_column& operator+=(Set_chain_column &column);
	friend Set_chain_column operator+(Set_chain_column column1, Set_chain_column &column2){
		column1 += column2;
		return column1;
	}
	friend Set_chain_column operator*(Set_chain_column column, unsigned int const& v){
		column *= v;
		return column;
	}
	friend Set_chain_column operator*(unsigned int const& v, Set_chain_column column){
		column *= v;
		return column;
	}

	Set_chain_column& multiply_and_add(const Field_element_type& v, Set_chain_column& column);
	Set_chain_column& multiply_and_add(Set_chain_column& column, const Field_element_type& v);

	Set_chain_column& operator=(Set_chain_column other);

	friend void swap(Set_chain_column& col1, Set_chain_column& col2){
		swap(static_cast<Set_column<Field_element_type,Cell_type,Row_access_option>&>(col1),
			 static_cast<Set_column<Field_element_type,Cell_type,Row_access_option>&>(col2));
		std::swap(col1.pivotToColumnIndex_, col2.pivotToColumnIndex_);
		std::swap(col1.pivot_, col2.pivot_);
		std::swap(col1.pairedColumn_, col2.pairedColumn_);
	}

private:
	Dictionnary_type* pivotToColumnIndex_;
	int pivot_;		//simplex index associated to the chain
	int pairedColumn_;
};

template<class Dictionnary_type, class Field_element_type, class Cell_type, class Row_access_option>
inline Set_chain_column<Dictionnary_type,Field_element_type,Cell_type,Row_access_option>::Set_chain_column(Dictionnary_type& pivotToColumnIndex)
	: Base(),
	  pivotToColumnIndex_(&pivotToColumnIndex),
	  pivot_(-1),
	  pairedColumn_(-1)
{}

template<class Dictionnary_type, class Field_element_type, class Cell_type, class Row_access_option>
template<class Chain_type>
inline Set_chain_column<Dictionnary_type,Field_element_type,Cell_type,Row_access_option>::Set_chain_column(
		const Chain_type& chain, dimension_type dimension, Dictionnary_type& pivotToColumnIndex)
	: Base(chain, dimension),
	  pivotToColumnIndex_(&pivotToColumnIndex),
	  pivot_(chain.empty() ? -1 : chain.rbegin()->first),
	  pairedColumn_(-1)
{}

template<class Dictionnary_type, class Field_element_type, class Cell_type, class Row_access_option>
template<class Row_container_type>
inline Set_chain_column<Dictionnary_type,Field_element_type,Cell_type,Row_access_option>::Set_chain_column(
		index columnIndex, Row_container_type &rowContainer, Dictionnary_type &pivotToColumnIndex)
	: Base(columnIndex, rowContainer),
	  pivotToColumnIndex_(&pivotToColumnIndex),
	  pivot_(-1),
	  pairedColumn_(-1)
{}

template<class Dictionnary_type, class Field_element_type, class Cell_type, class Row_access_option>
template<class Chain_type, class Row_container_type>
inline Set_chain_column<Dictionnary_type,Field_element_type,Cell_type,Row_access_option>::Set_chain_column(
		index columnIndex, const Chain_type& chain, dimension_type dimension, Row_container_type &rowContainer, Dictionnary_type &pivotToColumnIndex)
	: Base(columnIndex, chain, dimension, rowContainer),
	  pivotToColumnIndex_(&pivotToColumnIndex),
	  pivot_(chain.empty() ? -1 : chain.rbegin()->first),
	  pairedColumn_(-1)
{}

template<class Dictionnary_type, class Field_element_type, class Cell_type, class Row_access_option>
inline Set_chain_column<Dictionnary_type,Field_element_type,Cell_type,Row_access_option>::Set_chain_column(
		const Set_chain_column& column)
	: Base(static_cast<const Base&>(column)),
	  pivotToColumnIndex_(column.pivotToColumnIndex_),
	  pivot_(column.pivot_),
	  pairedColumn_(column.pairedColumn_)
{}

template<class Dictionnary_type, class Field_element_type, class Cell_type, class Row_access_option>
inline Set_chain_column<Dictionnary_type,Field_element_type,Cell_type,Row_access_option>::Set_chain_column(
		const Set_chain_column& column, index columnIndex)
	: Base(static_cast<const Base&>(column), columnIndex),
	  pivotToColumnIndex_(column.pivotToColumnIndex_),
	  pivot_(column.pivot_),
	  pairedColumn_(column.pairedColumn_)
{}

template<class Dictionnary_type, class Field_element_type, class Cell_type, class Row_access_option>
inline Set_chain_column<Dictionnary_type,Field_element_type,Cell_type,Row_access_option>::Set_chain_column(
		Set_chain_column&& column) noexcept
	: Base(std::move(static_cast<Base&&>(column))),
	  pivotToColumnIndex_(std::move(column.pivotToColumnIndex_)),
	  pivot_(std::exchange(column.pivot_, -1)),
	  pairedColumn_(std::exchange(column.pairedColumn_, 0))
{}

template<class Dictionnary_type, class Field_element_type, class Cell_type, class Row_access_option>
inline int Set_chain_column<Dictionnary_type,Field_element_type,Cell_type,Row_access_option>::get_pivot() const
{
	return pivot_;
}


template<class Dictionnary_type, class Field_element_type, class Cell_type, class Row_access_option>
inline Field_element_type Set_chain_column<Dictionnary_type,Field_element_type,Cell_type,Row_access_option>::get_pivot_value()
{
	if (pivot_ == -1) return Field_element_type();

	iterator it;
	if constexpr (Row_access_option::isActive_){
		it = Base::column_.find(Cell(0, Row_access_option::columnIndex_, pivot_));
	} else {
		it = Base::column_.find(Cell(0, pivot_));
	}
	if (it == Base::column_.end())
		return Field_element_type();	//should never happen if chain column is used properly

	return it->get_element();
}

template<class Dictionnary_type, class Field_element_type, class Cell_type, class Row_access_option>
inline index Set_chain_column<Dictionnary_type,Field_element_type,Cell_type,Row_access_option>::get_paired_chain_index() const
{
	return pairedColumn_;
}

template<class Dictionnary_type, class Field_element_type, class Cell_type, class Row_access_option>
inline bool Set_chain_column<Dictionnary_type,Field_element_type,Cell_type,Row_access_option>::is_paired() const
{
	return pairedColumn_ != -1;
}

template<class Dictionnary_type, class Field_element_type, class Cell_type, class Row_access_option>
inline void Set_chain_column<Dictionnary_type,Field_element_type,Cell_type,Row_access_option>::assign_paired_chain(index other_col)
{
	pairedColumn_ = other_col;
}

template<class Dictionnary_type, class Field_element_type, class Cell_type, class Row_access_option>
inline void Set_chain_column<Dictionnary_type,Field_element_type,Cell_type,Row_access_option>::unassign_paired_chain()
{
	pairedColumn_ = -1;
}

template<class Dictionnary_type, class Field_element_type, class Cell_type, class Row_access_option>
inline Set_chain_column<Dictionnary_type,Field_element_type,Cell_type,Row_access_option> &
Set_chain_column<Dictionnary_type,Field_element_type,Cell_type,Row_access_option>::operator+=(Set_chain_column &column)
{
	Base::operator+=(column);

	//assumes that the addition never zeros out this column. If the use of those columns changes at some point, we should think about it.
	if (!Base::is_non_zero(pivot_)){
		std::swap(pivotToColumnIndex_->at(pivot_),
				  pivotToColumnIndex_->at(column.get_pivot()));
		std::swap(pivot_, column.pivot_);
	}

	return *this;
}

template<class Dictionnary_type, class Field_element_type, class Cell_type, class Row_access_option>
inline Set_chain_column<Dictionnary_type,Field_element_type,Cell_type,Row_access_option> &
Set_chain_column<Dictionnary_type,Field_element_type,Cell_type,Row_access_option>::multiply_and_add(const Field_element_type& v, Set_chain_column& column)
{
	Base::multiply_and_add(v, column);

	//assumes that the addition never zeros out this column. If the use of those columns changes at some point, we should think about it.
	if (!Base::is_non_zero(pivot_)){
		std::swap(pivotToColumnIndex_->at(pivot_),
				  pivotToColumnIndex_->at(column.get_pivot()));
		std::swap(pivot_, column.pivot_);
	}

	return *this;
}

template<class Dictionnary_type, class Field_element_type, class Cell_type, class Row_access_option>
inline Set_chain_column<Dictionnary_type,Field_element_type,Cell_type,Row_access_option> &
Set_chain_column<Dictionnary_type,Field_element_type,Cell_type,Row_access_option>::multiply_and_add(Set_chain_column& column, const Field_element_type& val)
{
	Base::multiply_and_add(column, val);

	//assumes that the addition never zeros out this column. If the use of those columns changes at some point, we should think about it.
	if (!Base::is_non_zero(pivot_)){
		std::swap(pivotToColumnIndex_->at(pivot_),
				  pivotToColumnIndex_->at(column.get_pivot()));
		std::swap(pivot_, column.pivot_);
	}

	return *this;
}

template<class Dictionnary_type, class Field_element_type, class Cell_type, class Row_access_option>
inline Set_chain_column<Dictionnary_type,Field_element_type,Cell_type,Row_access_option> &
Set_chain_column<Dictionnary_type,Field_element_type,Cell_type,Row_access_option>::operator=(Set_chain_column other)
{
	Base::operator=(static_cast<Base&>(other));
	std::swap(pivotToColumnIndex_, other.pivotToColumnIndex_);
	std::swap(pivot_, other.pivot_);
	std::swap(pairedColumn_, other.pairedColumn_);
	return *this;
}

} //namespace persistence_matrix
} //namespace Gudhi

#endif // C_SET_COLUMN_H
