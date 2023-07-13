/*    This file is part of the Gudhi Library - https://gudhi.inria.fr/ - which is released under MIT.
 *    See file LICENSE or go to https://gudhi.inria.fr/licensing/ for full license details.
 *    Author(s):       Hannah Schreiber
 *
 *    Copyright (C) 2022 Inria
 *
 *    Modification(s):
 *      - YYYY/MM Author: Description of the modification
 */

#ifndef Z2_VECTOR_COLUMN_H
#define Z2_VECTOR_COLUMN_H

#include <iostream>
#include <iterator>
#include <list>
#include <unordered_set>

#include <boost/iterator/indirect_iterator.hpp>
#include <gudhi/Simple_object_pool.h>

#include "../utilities/utilities.h"
// #include "cell.h"

namespace Gudhi {
namespace persistence_matrix {

template<class Cell_type, class Row_access_option>
class Z2_vector_column : public Row_access_option
{
public:
//	using Cell = Z2_base_cell;
	using Cell = Cell_type;
	using Column_type = std::vector<Cell*>;
	using iterator = boost::indirect_iterator<typename Column_type::iterator>;
	using const_iterator = boost::indirect_iterator<typename Column_type::const_iterator>;
	using reverse_iterator = typename Column_type::reverse_iterator;
	using const_reverse_iterator = typename Column_type::const_reverse_iterator;

	Z2_vector_column();
	template<class Container_type>
	Z2_vector_column(const Container_type& nonZeroRowIndices);
	template<class Container_type>
	Z2_vector_column(const Container_type& nonZeroRowIndices, dimension_type dimension);
	template<class Row_container_type>
	Z2_vector_column(index columnIndex, Row_container_type &rowContainer);
	template<class Container_type, class Row_container_type>
	Z2_vector_column(index columnIndex, const Container_type& nonZeroRowIndices, Row_container_type &rowContainer);
	template<class Container_type, class Row_container_type>
	Z2_vector_column(index columnIndex, const Container_type& nonZeroRowIndices, dimension_type dimension, Row_container_type &rowContainer);
	Z2_vector_column(const Z2_vector_column& column);
	Z2_vector_column(const Z2_vector_column& column, index columnIndex);
	template<class Row_container_type>
	Z2_vector_column(const Z2_vector_column& column, index columnIndex, Row_container_type &rowContainer);
	Z2_vector_column(Z2_vector_column&& column) noexcept;
	~Z2_vector_column();

	std::vector<bool> get_content(int columnLength = -1) const;
	bool is_non_zero(index rowIndex) const;
	bool is_empty() const;
	dimension_type get_dimension() const;
	template<class Map_type>
	void reorder(Map_type& valueMap);
	void clear();

	iterator begin() noexcept;
	const_iterator begin() const noexcept;
	iterator end() noexcept;
	const_iterator end() const noexcept;
	reverse_iterator rbegin() noexcept;
	const_reverse_iterator rbegin() const noexcept;
	reverse_iterator rend() noexcept;
	const_reverse_iterator rend() const noexcept;

	template<class Cell_range>
	Z2_vector_column& operator+=(Cell_range const &column);
	friend Z2_vector_column operator+(Z2_vector_column column1, Z2_vector_column const &column2){
		column1 += column2;
		return column1;
	}

	Z2_vector_column& operator*=(unsigned int v);
	friend Z2_vector_column operator*(Z2_vector_column column, unsigned int const& v){
		column *= v;
		return column;
	}
	friend Z2_vector_column operator*(unsigned int const& v, Z2_vector_column column){
		column *= v;
		return column;
	}

	friend bool operator==(const Z2_vector_column& c1, const Z2_vector_column& c2){
		if (&c1 == &c2) return true;
		if (c1.column_.size() != c2.column_.size()) return false;

		for (unsigned int i = 0; i < c1.column_.size(); ++i){
			if (*c1.column_[i] != *c2.column_[i]) return false;
		}

		return true;
	}
	friend bool operator<(const Z2_vector_column& c1, const Z2_vector_column& c2){
		if (&c1 == &c2) return false;

		auto it1 = c1.column_.begin();
		auto it2 = c2.column_.begin();
		while (it1 != c1.column_.end() && it2 != c2.column_.end()) {
			if ((*it1)->get_row_index() != (*it2)->get_row_index())
				return (*it1)->get_row_index() < (*it2)->get_row_index();
			++it1; ++it2;
		}
		return it2 != c2.column_.end();
	}

	Z2_vector_column& operator=(Z2_vector_column other);

	friend void swap(Z2_vector_column& col1, Z2_vector_column& col2){
		swap(static_cast<Row_access_option&>(col1),
			 static_cast<Row_access_option&>(col2));
		std::swap(col1.dim_, col2.dim_);
		col1.column_.swap(col2.column_);
	}

protected:
	using real_iterator = typename Column_type::iterator;
	using real_const_iterator = typename Column_type::const_iterator;

	int dim_;
	Column_type column_;
	inline static Simple_object_pool<Cell> cellPool_;

	void _delete_cell(Cell* cell);
	void _insert_cell(index rowIndex, Column_type& column);
	void _update_cell(index rowIndex, index position);
};

template<class Cell_type, class Row_access_option>
inline Z2_vector_column<Cell_type,Row_access_option>::Z2_vector_column() : dim_(0)
{
//	static_assert(!Row_access_option::isActive_, "When row access option enabled, a row container has to be provided.");
}

template<class Cell_type, class Row_access_option>
template<class Container_type>
inline Z2_vector_column<Cell_type,Row_access_option>::Z2_vector_column(const Container_type &nonZeroRowIndices)
	: dim_(nonZeroRowIndices.size() == 0 ? 0 : nonZeroRowIndices.size() - 1),
	  column_(nonZeroRowIndices.size())
{
//	static_assert(!Row_access_option::isActive_, "When row access option enabled, a row container has to be provided.");

	unsigned int i = 0;
	for (index id : nonZeroRowIndices){
		_update_cell(id, i++);
	}
}

template<class Cell_type, class Row_access_option>
template<class Container_type>
inline Z2_vector_column<Cell_type,Row_access_option>::Z2_vector_column(const Container_type &nonZeroRowIndices, dimension_type dimension)
	: dim_(dimension),
	  column_(nonZeroRowIndices.size())
{
//	static_assert(!Row_access_option::isActive_, "When row access option enabled, a row container has to be provided.");

	unsigned int i = 0;
	for (index id : nonZeroRowIndices){
		_update_cell(id, i++);
	}
}

template<class Cell_type, class Row_access_option>
template<class Row_container_type>
inline Z2_vector_column<Cell_type,Row_access_option>::Z2_vector_column(
		index columnIndex, Row_container_type &rowContainer)
	: Row_access_option(columnIndex, rowContainer), dim_(0)
{}

template<class Cell_type, class Row_access_option>
template<class Container_type, class Row_container_type>
inline Z2_vector_column<Cell_type,Row_access_option>::Z2_vector_column(
		index columnIndex, const Container_type &nonZeroRowIndices, Row_container_type &rowContainer)
	: Row_access_option(columnIndex, rowContainer), dim_(nonZeroRowIndices.size() == 0 ? 0 : nonZeroRowIndices.size() - 1), column_(nonZeroRowIndices.size())
{
	unsigned int i = 0;
	for (index id : nonZeroRowIndices){
		_update_cell(id, i++);
	}
}

template<class Cell_type, class Row_access_option>
template<class Container_type, class Row_container_type>
inline Z2_vector_column<Cell_type,Row_access_option>::Z2_vector_column(
		index columnIndex, const Container_type &nonZeroRowIndices, dimension_type dimension, Row_container_type &rowContainer)
	: Row_access_option(columnIndex, rowContainer), dim_(dimension), column_(nonZeroRowIndices.size())
{
	unsigned int i = 0;
	for (index id : nonZeroRowIndices){
		_update_cell(id, i++);
	}
}

template<class Cell_type, class Row_access_option>
inline Z2_vector_column<Cell_type,Row_access_option>::Z2_vector_column(const Z2_vector_column &column)
	: dim_(column.dim_),
	  column_(column.column_.size())
{
	static_assert(!Row_access_option::isActive_,
			"Copy constructor not available when row access option enabled.");

	unsigned int i = 0;
	for (const Cell* cell : column.column_){
		_update_cell(cell->get_row_index(), i++);
	}
}

template<class Cell_type, class Row_access_option>
inline Z2_vector_column<Cell_type,Row_access_option>::Z2_vector_column(
		const Z2_vector_column &column, index columnIndex)
	: Row_access_option(columnIndex, *column.rows_),
	  dim_(column.dim_),
	  column_(column.column_.size())
{
	unsigned int i = 0;
	for (const Cell* cell : column.column_){
		_update_cell(cell->get_row_index(), i++);
	}
}

template<class Cell_type, class Row_access_option>
template<class Row_container_type>
inline Z2_vector_column<Cell_type,Row_access_option>::Z2_vector_column(
		const Z2_vector_column &column, index columnIndex, Row_container_type &rowContainer)
	: Row_access_option(columnIndex, rowContainer),
	  dim_(column.dim_),
	  column_(column.column_.size())
{
	unsigned int i = 0;
	for (const Cell* cell : column.column_){
		_update_cell(cell->get_row_index(), i++);
	}
}

template<class Cell_type, class Row_access_option>
inline Z2_vector_column<Cell_type,Row_access_option>::Z2_vector_column(Z2_vector_column &&column) noexcept
	: Row_access_option(std::move(column)),
	  dim_(std::exchange(column.dim_, 0)),
	  column_(std::move(column.column_))
{}

template<class Cell_type, class Row_access_option>
inline Z2_vector_column<Cell_type,Row_access_option>::~Z2_vector_column()
{
	for (Cell* cell : column_){
		_delete_cell(cell);
	}
}

template<class Cell_type, class Row_access_option>
inline std::vector<bool> Z2_vector_column<Cell_type,Row_access_option>::get_content(int columnLength) const
{
	if (columnLength < 0) columnLength = column_.back()->get_row_index() + 1;

	std::vector<bool> container(columnLength, 0);
	for (auto it = column_.begin(); it != column_.end() && (*it)->get_row_index() < static_cast<index>(columnLength); ++it){
		container[(*it)->get_row_index()] = 1;
	}
	return container;
}

template<class Cell_type, class Row_access_option>
inline bool Z2_vector_column<Cell_type,Row_access_option>::is_non_zero(index rowIndex) const
{
	for (const Cell* v : column_){
		if (v->get_row_index() == rowIndex) return true;
	}
	return false;
}

template<class Cell_type, class Row_access_option>
inline bool Z2_vector_column<Cell_type,Row_access_option>::is_empty() const
{
	return column_.empty();
}

template<class Cell_type, class Row_access_option>
inline dimension_type Z2_vector_column<Cell_type,Row_access_option>::get_dimension() const
{
	return dim_;
}

template<class Cell_type, class Row_access_option>
template<class Map_type>
inline void Z2_vector_column<Cell_type,Row_access_option>::reorder(Map_type &valueMap)
{
	for (Cell* v : column_) {
		v->set_row_index(valueMap[v->get_row_index()]);
		if constexpr (Row_access_option::isActive_){
			Row_access_option::unlink(v);
		}
	}
	//all cells have to be deleted first, to avoid problem with insertion when row is a set
	if constexpr (Row_access_option::isActive_){
		for (Cell* cell : column_) {
			Row_access_option::insert_cell(cell->get_row_index(), cell);
		}
	}
	std::sort(column_.begin(), column_.end(), [](const Cell* c1, const Cell* c2){return *c1 < *c2;});
}

template<class Cell_type, class Row_access_option>
inline void Z2_vector_column<Cell_type,Row_access_option>::clear()
{
	for (Cell* cell : column_){
		_delete_cell(cell);
	}
	column_.clear();
}

template<class Cell_type, class Row_access_option>
inline typename Z2_vector_column<Cell_type,Row_access_option>::iterator
Z2_vector_column<Cell_type,Row_access_option>::begin() noexcept
{
	return column_.begin();
}

template<class Cell_type, class Row_access_option>
inline typename Z2_vector_column<Cell_type,Row_access_option>::const_iterator
Z2_vector_column<Cell_type,Row_access_option>::begin() const noexcept
{
	return column_.begin();
}

template<class Cell_type, class Row_access_option>
inline typename Z2_vector_column<Cell_type,Row_access_option>::iterator
Z2_vector_column<Cell_type,Row_access_option>::end() noexcept
{
	return column_.end();
}

template<class Cell_type, class Row_access_option>
inline typename Z2_vector_column<Cell_type,Row_access_option>::const_iterator
Z2_vector_column<Cell_type,Row_access_option>::end() const noexcept
{
	return column_.end();
}

template<class Cell_type, class Row_access_option>
inline typename Z2_vector_column<Cell_type,Row_access_option>::reverse_iterator
Z2_vector_column<Cell_type,Row_access_option>::rbegin() noexcept
{
	return column_.rbegin();
}

template<class Cell_type, class Row_access_option>
inline typename Z2_vector_column<Cell_type,Row_access_option>::const_reverse_iterator
Z2_vector_column<Cell_type,Row_access_option>::rbegin() const noexcept
{
	return column_.rbegin();
}

template<class Cell_type, class Row_access_option>
inline typename Z2_vector_column<Cell_type,Row_access_option>::reverse_iterator
Z2_vector_column<Cell_type,Row_access_option>::rend() noexcept
{
	return column_.rend();
}

template<class Cell_type, class Row_access_option>
inline typename Z2_vector_column<Cell_type,Row_access_option>::const_reverse_iterator
Z2_vector_column<Cell_type,Row_access_option>::rend() const noexcept
{
	return column_.rend();
}

template<class Cell_type, class Row_access_option>
template<class Cell_range>
inline Z2_vector_column<Cell_type,Row_access_option> &Z2_vector_column<Cell_type,Row_access_option>::operator+=(const Cell_range &column)
{
	if (column.begin() == column.end()) return *this;
	if (column_.empty()){
//		column_.resize(column.column_.size());
//		unsigned int i = 0;
		for (const Cell& cell : column)
			_insert_cell(cell.get_row_index(), column_);
//			_update_cell(cell.get_row_index(), i++);
		return *this;
	}

	Column_type newColumn;

	auto itToAdd = column.begin();
	real_iterator itTarget = column_.begin();

	while (itToAdd != column.end() && itTarget != column_.end())
	{
		const Cell& cellToAdd = *itToAdd;
		Cell* cellTarget = *itTarget;
		unsigned int curRowToAdd = cellToAdd.get_row_index();
		unsigned int curRowTarget = cellTarget->get_row_index();

		if (curRowToAdd == curRowTarget){
			_delete_cell(cellTarget);
			itTarget++;
			itToAdd++;
		} else if (curRowToAdd < curRowTarget){
			_insert_cell(curRowToAdd, newColumn);
			itToAdd++;
		} else {
			newColumn.push_back(cellTarget);
			itTarget++;
		}
	}

	while (itToAdd != column.end()){
		_insert_cell(itToAdd->get_row_index(), newColumn);
		itToAdd++;
	}

	while (itTarget != column_.end()){
		newColumn.push_back(*itTarget);
		itTarget++;
	}

	column_.swap(newColumn);

	return *this;
}

template<class Cell_type, class Row_access_option>
inline Z2_vector_column<Cell_type,Row_access_option> &Z2_vector_column<Cell_type,Row_access_option>::operator*=(unsigned int v)
{
	if (v % 2 == 0){
		clear();
	}

	return *this;
}

template<class Cell_type, class Row_access_option>
inline Z2_vector_column<Cell_type,Row_access_option> &Z2_vector_column<Cell_type,Row_access_option>::operator=(Z2_vector_column other)
{
	static_assert (!Row_access_option::isActive_, "= assignement not enabled with row access option.");

	std::swap(dim_, other.dim_);
	column_.swap(other.column_);
	return *this;
}

template<class Cell_type, class Row_access_option>
inline void Z2_vector_column<Cell_type,Row_access_option>::_delete_cell(Cell* cell)
{
	if constexpr (Row_access_option::isActive_){
		Row_access_option::unlink(cell);
	}
	// delete cell;
	cellPool_.destroy(cell);
}

template<class Cell_type, class Row_access_option>
inline void Z2_vector_column<Cell_type,Row_access_option>::_insert_cell(index rowIndex, Column_type &column)
{
	if constexpr (Row_access_option::isActive_){
		// Cell *new_cell = new Cell(Row_access_option::columnIndex_, rowIndex);
		Cell *new_cell = cellPool_.construct(Row_access_option::columnIndex_, rowIndex);
		column.push_back(new_cell);
		Row_access_option::insert_cell(rowIndex, new_cell);
	} else {
		// Cell *new_cell = new Cell(rowIndex);
		Cell *new_cell = cellPool_.construct(rowIndex);
		column.push_back(new_cell);
	}
}

template<class Cell_type, class Row_access_option>
inline void Z2_vector_column<Cell_type,Row_access_option>::_update_cell(index rowIndex, index position)
{
	if constexpr (Row_access_option::isActive_){
		// Cell *new_cell = new Cell(Row_access_option::columnIndex_, rowIndex);
		Cell *new_cell = cellPool_.construct(Row_access_option::columnIndex_, rowIndex);
		column_[position] = new_cell;
		Row_access_option::insert_cell(rowIndex, new_cell);
	} else {
		// Cell *new_cell = new Cell(rowIndex);
		Cell *new_cell = cellPool_.construct(rowIndex);
		column_[position] = new_cell;
	}
}

} //namespace persistence_matrix
} //namespace Gudhi

template<class Cell_type, class Row_access_option>
struct std::hash<Gudhi::persistence_matrix::Z2_vector_column<Cell_type,Row_access_option> >
{
	size_t operator()(const Gudhi::persistence_matrix::Z2_vector_column<Cell_type,Row_access_option>& column) const
	{
		std::size_t seed = 0;
		for (auto& cell : column){
			seed ^= std::hash<unsigned int>()(cell.get_row_index()) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
		}
		return seed;
	}
};

#endif // Z2_VECTOR_COLUMN_H
