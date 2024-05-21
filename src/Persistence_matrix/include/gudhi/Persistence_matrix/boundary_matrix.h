/*    This file is part of the Gudhi Library - https://gudhi.inria.fr/ - which is released under MIT.
 *    See file LICENSE or go to https://gudhi.inria.fr/licensing/ for full license details.
 *    Author(s):       Hannah Schreiber
 *
 *    Copyright (C) 2022-24 Inria
 *
 *    Modification(s):
 *      - YYYY/MM Author: Description of the modification
 */

/**
 * @file boundary_matrix.h
 * @author Hannah Schreiber
 * @brief Contains the @ref Boundary_matrix class.
 */

#ifndef PM_BOUNDARY_MATRIX_H
#define PM_BOUNDARY_MATRIX_H

#include <cassert>
#include <iostream> //print() only
#include <vector>
#include <utility>  //std::swap, std::move & std::exchange

namespace Gudhi {
namespace persistence_matrix {

// TODO: factorize/inheritate/compose with @ref basematrix "base matrix"?
/**
 * @class Boundary_matrix boundary_matrix.h gudhi/Persistence_matrix/boundary_matrix.h
 * @ingroup persistence_matrix
 *
 * @brief %Matrix structure to store the ordered @ref boundarymatrix "boundary matrix" \f$ R \f$ of a filtered complex
 * in order to compute its persistent homology. Provides an access to its columns and rows as well as the possibility
 * to remove the last faces of the filtration while maintaining a valid barcode.
 * 
 * @tparam Master_matrix An instanciation of @ref Matrix from which all types and options are deduced.
 */
template <class Master_matrix>
class Boundary_matrix : public Master_matrix::Matrix_dimension_option,
                        public Master_matrix::template Base_swap_option<Boundary_matrix<Master_matrix> >,
                        public Master_matrix::Base_pairing_option,
                        public Master_matrix::Matrix_row_access_option 
{
 public:
  using index = typename Master_matrix::index;                        /**< Container index type. */
  using id_index = typename Master_matrix::id_index;                  /**< @ref IDIdx index type. */
  using dimension_type = typename Master_matrix::dimension_type;      /**< Dimension value type. */
  /**
   * @brief Field operators class. Necessary only if @ref PersistenceMatrixOptions::is_z2 is false.
   */
  using Field_operators = typename Master_matrix::Field_operators;
  using Field_element_type = typename Master_matrix::element_type;    /**< Type of an field element. */
  using Column_type = typename Master_matrix::Column_type;            /**< Column type. */
  using boundary_type = typename Master_matrix::boundary_type;        /**< Type of an input column. */
  using Row_type = typename Master_matrix::Row_type;                  /**< Row type,
                                                                           only necessary with row access option. */
  using Cell_constructor = typename Master_matrix::Cell_constructor;  /**< Factory of @ref Cell classes. */
  using Column_settings = typename Master_matrix::Column_settings;    /**< Structure giving access to the columns to
                                                                           necessary external classes. */

  /**
   * @brief Constructs an empty matrix.
   * 
   * @param colSettings Pointer to an existing setting structure for the columns. The structure should contain all
   * the necessary external classes specifically necessary for the choosen column type, such as custom allocators.
   */
  Boundary_matrix(Column_settings* colSettings);
  /**
   * @brief Constructs a new matrix from the given ranges of @ref Matrix::cell_rep_type. Each range corresponds to
   * a column  (the order of the ranges are preserved). The content of the ranges is assumed to be sorted by increasing
   * IDs. The IDs of the simplices are also assumed to be consecutifs, ordered by filtration value, starting with 0. 
   * 
   * @tparam Boundary_type Range type for @ref Matrix::cell_rep_type ranges.
   * Assumed to have a begin(), end() and size() method.
   * @param orderedBoundaries Range of boundaries: @p orderedBoundaries is interpreted as a boundary matrix of a 
   * filtered **simplicial** complex, whose boundaries are ordered by filtration order. 
   * Therefore, `orderedBoundaries[i]` should store the boundary of the \f$ i^{th} \f$ simplex in the filtration,
   * as an ordered list of indices of its facets (again those indices correspond to their respective position
   * in the matrix). That is why the indices of the simplices are assumed to be consecutifs and starting with 0 
   * (an empty boundary is interpreted as a vertex boundary and not as a non existing simplex). 
   * All dimensions up to the maximal dimension of interest have to be present. If only a higher dimension is of 
   * interest and not everything should be stored, then use the @ref insert_boundary method instead
   * (after creating the matrix with the
   * @ref Boundary_matrix(unsigned int numberOfColumns, Column_settings* colSettings)
   * constructor preferably).
   * @param colSettings Pointer to an existing setting structure for the columns. The structure should contain all
   * the necessary external classes specifically necessary for the choosen column type, such as custom allocators.
   */
  template <class Boundary_type = boundary_type>
  Boundary_matrix(const std::vector<Boundary_type>& orderedBoundaries, 
                  Column_settings* colSettings);
  /**
   * @brief Constructs a new empty matrix and reserves space for the given number of columns.
   * 
   * @param numberOfColumns Number of columns to reserve space for.
   * @param colSettings Pointer to an existing setting structure for the columns. The structure should contain all
   * the necessary external classes specifically necessary for the choosen column type, such as custom allocators.
   */
  Boundary_matrix(unsigned int numberOfColumns, Column_settings* colSettings);
  /**
   * @brief Copy constructor. If @p colSettings is not a null pointer, its value is kept
   * instead of the one in the copied matrix.
   * 
   * @param matrixToCopy Matrix to copy.
   * @param colSettings Either a pointer to an existing setting structure for the columns or a null pointer.
   * The structure should contain all the necessary external classes specifically necessary for the choosen column type,
   * such as custom allocators. If null pointer, the pointer stored in @p matrixToCopy is used instead.
   */
  Boundary_matrix(const Boundary_matrix& matrixToCopy, 
                  Column_settings* colSettings = nullptr);
  /**
   * @brief Move constructor.
   * 
   * @param other Matrix to move.
   */
  Boundary_matrix(Boundary_matrix&& other) noexcept;

  /**
   * @brief Inserts at the end of the matrix a new ordered column corresponding to the given boundary. 
   * This means that it is assumed that this method is called on boundaries in the order of the filtration. 
   * It also assumes that the faces in the given boundary are identified by their relative position in the filtration, 
   * starting at 0. If it is not the case, use the other
   * @ref insert_boundary(id_index faceIndex, const Boundary_type& boundary, dimension_type dim) "insert_boundary"
   * instead by indicating the face ID used in the boundaries when the face is inserted.
   *
   * Different to the constructor, the boundaries do not have to come from a simplicial complex, but also from
   * a more general cell complex. This includes cubical complexes or Morse complexes for example.
   *
   * At the insertion, the boundary will be copied as is. The column will only be reduced later when the barcode
   * is requested in order to apply some optimisations with the additional knowledge. Hence, the barcode will also
   * not be updated, so call @ref Base_pairing::get_current_barcode "get_current_barcode" only when the matrix is
   * complete.
   * 
   * @tparam Boundary_type Range of @ref Matrix::cell_rep_type. Assumed to have a begin(), end() and size() method.
   * @param boundary Boundary generating the new column. The content should be ordered by ID.
   * @param dim Dimension of the face whose boundary is given. If the complex is simplicial, 
   * this parameter can be omitted as it can be deduced from the size of the boundary.
   * @return The @ref MatIdx index of the inserted boundary.
   */
  template <class Boundary_type = boundary_type>
  index insert_boundary(const Boundary_type& boundary, dimension_type dim = -1);
  /**
   * @brief It does the same as the other version, but allows the boundary faces to be identified without restrictions
   * except that all IDs have to be strictly increasing in the order of filtration. Note that you should avoid then
   * to use the other insertion method to avoid overwriting IDs.
   *
   * As a face has to be inserted before one of its cofaces in a valid filtration (recall that it is assumed that
   * the faces are inserted by order of filtration), it is sufficient to indicate the ID of the face being inserted.
   * 
   * @tparam Boundary_type Range of @ref Matrix::cell_rep_type. Assumed to have a begin(), end() and size() method.
   * @param faceIndex @ref IDIdx index to use to indentify the new face.
   * @param boundary Boundary generating the new column. The indices of the boundary have to correspond to the 
   * @p faceIndex values of precedent calls of the method for the corresponding faces and should be ordered in 
   * increasing order.
   * @param dim Dimension of the face whose boundary is given. If the complex is simplicial, 
   * this parameter can be omitted as it can be deduced from the size of the boundary.
   * @return The @ref MatIdx index of the inserted boundary.
   */
  template <class Boundary_type = boundary_type>
  index insert_boundary(id_index faceIndex, const Boundary_type& boundary, dimension_type dim = -1);
  /**
   * @brief Returns the column at the given @ref MatIdx index.
   * The type of the column depends on the choosen options, see @ref PersistenceMatrixOptions::column_type.
   *
   * Note that before returning the column, all column cells can eventually be reordered, if lazy swaps occurred.
   * It is therefore recommended to avoid calling @ref get_column between column or row swaps, otherwise the benefits
   * of the the lazyness is lost.
   * 
   * @param columnIndex @ref MatIdx index of the column to return.
   * @return Reference to the column.
   */
  Column_type& get_column(index columnIndex);
  /**
   * @brief Only available if @ref PersistenceMatrixOptions::has_row_access is true.
   * Returns the row at the given @ref rowindex "row index" of the matrix.
   * The type of the row depends on the choosen options, see @ref PersistenceMatrixOptions::has_intrusive_rows.
   *
   * Note that before returning the row, all column cells can eventually be reordered, if lazy swaps occurred.
   * It is therefore recommended to avoid calling @ref get_row between column or row swaps, otherwise the benefits
   * of the the lazyness is lost.
   * 
   * @param rowIndex @ref rowindex "Row index" of the row to return.
   * @return Reference to the row.
   */
  Row_type& get_row(index rowIndex);
  /**
   * @brief Only available if @ref PersistenceMatrixOptions::has_removable_columns is true.
   * Removes the last face in the filtration from the matrix and updates the barcode if this one was already computed.
   * 
   * @return The pivot of the removed face.
   */
  index remove_last();
  /**
   * @brief If @ref PersistenceMatrixOptions::has_row_access and @ref PersistenceMatrixOptions::has_removable_rows
   * are true: assumes that the row is empty and removes it. If @ref PersistenceMatrixOptions::has_map_column_container
   * and @ref PersistenceMatrixOptions::has_column_and_row_swaps are true: cleans up maps used for the lazy row swaps.
   * Otherwise, does nothing.
   *
   * @warning The removed rows are always assumed to be empty. If it is not the case, the deleted row cells are not
   * removed from their columns. And in the case of intrusive rows, this will generate a segmentation fault when 
   * the column cells are destroyed later. The row access is just meant as a "read only" access to the rows and the
   * @ref erase_row method just as a way to specify that a row is empty and can therefore be removed from dictionnaries.
   * This allows to avoid testing the emptiness of a row at each column cell removal, what can be quite frequent. 
   * 
   * @param rowIndex @ref rowindex "Row index" of the empty row.
   */
  void erase_row(index rowIndex);

  /**
   * @brief Returns the current number of columns in the matrix.
   * 
   * @return The number of columns.
   */
  index get_number_of_columns() const;

  /**
   * @brief Returns the dimension of the given column.
   * 
   * @param columnIndex @ref MatIdx index of the column representing the face.
   * @return Dimension of the face.
   */
  dimension_type get_column_dimension(index columnIndex) const;

  /**
   * @brief Adds column at @p sourceColumnIndex onto the column at @p targetColumnIndex in the matrix.
   *
   * @warning They will be no verification to ensure that the addition makes sense for the validity of a
   * boundary matrix of a filtered complex. For example, a right-to-left addition could corrupt the computation
   * of the barcode if done blindly. So should be used with care.
   * 
   * @param sourceColumnIndex @ref MatIdx index of the source column.
   * @param targetColumnIndex @ref MatIdx index of the target column.
   */
  void add_to(index sourceColumnIndex, index targetColumnIndex);
  /**
   * @brief Multiplies the target column with the coefficiant and then adds the source column to it.
   * That is: `targetColumn = (targetColumn * coefficient) + sourceColumn`.
   *
   * @warning They will be no verification to ensure that the addition makes sense for the validity of a
   * boundary matrix of a filtered complex. For example, a right-to-left addition could corrupt the computation
   * of the barcode if done blindly. So should be used with care.
   * 
   * @param sourceColumnIndex @ref MatIdx index of the source column.
   * @param coefficient Value to multiply.
   * @param targetColumnIndex @ref MatIdx index of the target column.
   */
  void multiply_target_and_add_to(index sourceColumnIndex, 
                                  const Field_element_type& coefficient,
                                  index targetColumnIndex);
  /**
   * @brief Multiplies the source column with the coefficiant before adding it to the target column.
   * That is: `targetColumn += (coefficient * sourceColumn)`. The source column will **not** be modified.
   *
   * @warning They will be no verification to ensure that the addition makes sense for the validity of a
   * boundary matrix of a filtered complex. For example, a right-to-left addition could corrupt the computation
   * of the barcode if done blindly. So should be used with care.
   * 
   * @param coefficient Value to multiply.
   * @param sourceColumnIndex @ref MatIdx index of the source column.
   * @param targetColumnIndex @ref MatIdx index of the target column.
   */
  void multiply_source_and_add_to(const Field_element_type& coefficient, 
                                  index sourceColumnIndex,
                                  index targetColumnIndex);

  /**
   * @brief Zeroes the cell at the given coordinates.
   *
   * @warning They will be no verification to ensure that the zeroing makes sense for the validity of a
   * boundary matrix of a filtered complex. So should be used while knowing what one is doing.
   * 
   * @param columnIndex @ref MatIdx index of the column of the cell.
   * @param rowIndex @ref rowindex "Row index" of the row of the cell.
   */
  void zero_cell(index columnIndex, index rowIndex);
  /**
   * @brief Zeroes the column at the given index.
   *
   * @warning They will be no verification to ensure that the zeroing makes sense for the validity of a
   * boundary matrix of a filtered complex. So should be used while knowing what one is doing.
   * 
   * @param columnIndex @ref MatIdx index of the column to zero.
   */
  void zero_column(index columnIndex);
  /**
   * @brief Indicates if the cell at given coordinates has value zero.
   * 
   * @param columnIndex @ref MatIdx index of the column of the cell.
   * @param rowIndex @ref rowindex "Row index" of the row of the cell.
   * @return true If the cell has value zero.
   * @return false Otherwise.
   */
  bool is_zero_cell(index columnIndex, index rowIndex) const;
  /**
   * @brief Indicates if the column at given index has value zero.
   * 
   * @param columnIndex @ref MatIdx index of the column.
   * @return true If the column has value zero.
   * @return false Otherwise.
   */
  bool is_zero_column(index columnIndex);

  /**
   * @brief Returns the pivot of the given column.
   * 
   * @param columnIndex @ref MatIdx index of the column.
   * @return Pivot of the coluimn at @p columnIndex.
   */
  index get_pivot(index columnIndex);

  /**
   * @brief Resets the matrix to an empty matrix.
   * 
   * @param colSettings Pointer to an existing setting structure for the columns. The structure should contain all
   * the necessary external classes specifically necessary for the choosen column type, such as custom allocators.
   */
  void reset(Column_settings* colSettings) {
    matrix_.clear();
    nextInsertIndex_ = 0;
    colSettings_ = colSettings;
  }

  /**
   * @brief Assign operator.
   */
  Boundary_matrix& operator=(const Boundary_matrix& other);
  /**
   * @brief Swap operator.
   */
  friend void swap(Boundary_matrix& matrix1, Boundary_matrix& matrix2) {
    swap(static_cast<typename Master_matrix::Matrix_dimension_option&>(matrix1),
         static_cast<typename Master_matrix::Matrix_dimension_option&>(matrix2));
    swap(static_cast<typename Master_matrix::template Base_swap_option<Boundary_matrix<Master_matrix> >&>(matrix1),
         static_cast<typename Master_matrix::template Base_swap_option<Boundary_matrix<Master_matrix> >&>(matrix2));
    swap(static_cast<typename Master_matrix::Base_pairing_option&>(matrix1),
         static_cast<typename Master_matrix::Base_pairing_option&>(matrix2));
    matrix1.matrix_.swap(matrix2.matrix_);
    std::swap(matrix1.nextInsertIndex_, matrix2.nextInsertIndex_);
    std::swap(matrix1.colSettings_, matrix2.colSettings_);

    if constexpr (Master_matrix::Option_list::has_row_access) {
      swap(static_cast<typename Master_matrix::Matrix_row_access_option&>(matrix1),
           static_cast<typename Master_matrix::Matrix_row_access_option&>(matrix2));
    }
  }

  void print();  // for debug

 private:
  using dim_opt = typename Master_matrix::Matrix_dimension_option;
  using swap_opt = typename Master_matrix::template Base_swap_option<Boundary_matrix<Master_matrix> >;
  using pair_opt = typename Master_matrix::Base_pairing_option;
  using ra_opt = typename Master_matrix::Matrix_row_access_option;
  using matrix_type = typename Master_matrix::column_container_type;

  friend swap_opt;
  friend pair_opt;

  matrix_type matrix_;          /**< Column container. */
  index nextInsertIndex_;       /**< Next unused column index. */
  Column_settings* colSettings_;  /**< Cell factory. */

  static const bool activeDimOption =
      Master_matrix::Option_list::has_matrix_maximal_dimension_access || Master_matrix::maxDimensionIsNeeded;
  static const bool activeSwapOption =
      Master_matrix::Option_list::has_column_and_row_swaps || Master_matrix::Option_list::has_vine_update;
  static const bool activePairingOption = Master_matrix::Option_list::has_column_pairings &&
                                          !Master_matrix::Option_list::has_vine_update &&
                                          !Master_matrix::Option_list::can_retrieve_representative_cycles;
};

template <class Master_matrix>
inline Boundary_matrix<Master_matrix>::Boundary_matrix(Column_settings* colSettings)
    : dim_opt(-1),
      swap_opt(),
      pair_opt(),
      ra_opt(),
      nextInsertIndex_(0),
      colSettings_(colSettings)
{}

template <class Master_matrix>
template <class Boundary_type>
inline Boundary_matrix<Master_matrix>::Boundary_matrix(const std::vector<Boundary_type>& orderedBoundaries,
                                                       Column_settings* colSettings)
    : dim_opt(-1),
      swap_opt(orderedBoundaries.size()),
      pair_opt(),
      ra_opt(orderedBoundaries.size()),
      nextInsertIndex_(orderedBoundaries.size()),
      colSettings_(colSettings)
{
  matrix_.reserve(orderedBoundaries.size());

  for (index i = 0; i < orderedBoundaries.size(); i++) {
    if constexpr (Master_matrix::Option_list::has_map_column_container) {
      if constexpr (Master_matrix::Option_list::has_row_access) {
        matrix_.try_emplace(i, Column_type(i, orderedBoundaries[i], ra_opt::rows_, colSettings_));
      } else {
        matrix_.try_emplace(i, Column_type(orderedBoundaries[i], colSettings_));
      }
      if constexpr (activeDimOption) {
        dim_opt::update_up(matrix_.at(i).get_dimension());
      }
    } else {
      if constexpr (Master_matrix::Option_list::has_row_access) {
        matrix_.emplace_back(i, orderedBoundaries[i], ra_opt::rows_, colSettings_);
      } else {
        matrix_.emplace_back(orderedBoundaries[i], colSettings_);
      }
      if constexpr (activeDimOption) {
        dim_opt::update_up(matrix_[i].get_dimension());
      }
    }
  }
}

template <class Master_matrix>
inline Boundary_matrix<Master_matrix>::Boundary_matrix(unsigned int numberOfColumns, 
                                                       Column_settings* colSettings)
    : dim_opt(-1),
      swap_opt(numberOfColumns),
      pair_opt(),
      ra_opt(numberOfColumns),
      matrix_(!Master_matrix::Option_list::has_map_column_container && Master_matrix::Option_list::has_row_access
                  ? 0
                  : numberOfColumns),
      nextInsertIndex_(0),
      colSettings_(colSettings)
{
  if constexpr (!Master_matrix::Option_list::has_map_column_container && Master_matrix::Option_list::has_row_access)
    matrix_.reserve(numberOfColumns);
}

template <class Master_matrix>
inline Boundary_matrix<Master_matrix>::Boundary_matrix(const Boundary_matrix& matrixToCopy, 
                                                       Column_settings* colSettings)
    : dim_opt(static_cast<const dim_opt&>(matrixToCopy)),
      swap_opt(static_cast<const swap_opt&>(matrixToCopy)),
      pair_opt(static_cast<const pair_opt&>(matrixToCopy)),
      ra_opt(static_cast<const ra_opt&>(matrixToCopy)),
      nextInsertIndex_(matrixToCopy.nextInsertIndex_),
      colSettings_(colSettings == nullptr ? matrixToCopy.colSettings_ : colSettings) 
{
  matrix_.reserve(matrixToCopy.matrix_.size());
  if constexpr (Master_matrix::Option_list::has_map_column_container) {
    for (const auto& p : matrixToCopy.matrix_) {
      const Column_type& col = p.second;
      if constexpr (Master_matrix::Option_list::has_row_access) {
        matrix_.try_emplace(p.first, Column_type(col, col.get_column_index(), ra_opt::rows_, colSettings_));
      } else {
        matrix_.try_emplace(p.first, Column_type(col, colSettings_));
      }
    }
  } else {
    for (const auto& col : matrixToCopy.matrix_) {
      if constexpr (Master_matrix::Option_list::has_row_access) {
        matrix_.emplace_back(col, col.get_column_index(), ra_opt::rows_, colSettings_);
      } else {
        matrix_.emplace_back(col, colSettings_);
      }
    }
  }
}

template <class Master_matrix>
inline Boundary_matrix<Master_matrix>::Boundary_matrix(Boundary_matrix&& other) noexcept
    : dim_opt(std::move(static_cast<dim_opt&>(other))),
      swap_opt(std::move(static_cast<swap_opt&>(other))),
      pair_opt(std::move(static_cast<pair_opt&>(other))),
      ra_opt(std::move(static_cast<ra_opt&>(other))),
      matrix_(std::move(other.matrix_)),
      nextInsertIndex_(std::exchange(other.nextInsertIndex_, 0)),
      colSettings_(std::exchange(other.colSettings_, nullptr)) 
{}

template <class Master_matrix>
template <class Boundary_type>
inline typename Boundary_matrix<Master_matrix>::index Boundary_matrix<Master_matrix>::insert_boundary(
    const Boundary_type& boundary, dimension_type dim) 
{
  return insert_boundary(nextInsertIndex_, boundary, dim);
}

template <class Master_matrix>
template <class Boundary_type>
inline typename Boundary_matrix<Master_matrix>::index Boundary_matrix<Master_matrix>::insert_boundary(
    id_index faceIndex, const Boundary_type& boundary, dimension_type dim) 
{
  if (dim == -1) dim = boundary.size() == 0 ? 0 : boundary.size() - 1;

  if constexpr (activeSwapOption) {
    if (swap_opt::rowSwapped_) swap_opt::_orderRows();
  }

  if constexpr (Master_matrix::Option_list::has_row_access && !Master_matrix::Option_list::has_removable_rows) {
    id_index pivot;
    if constexpr (Master_matrix::Option_list::is_z2) {
      pivot = *std::prev(boundary.end());
    } else {
      pivot = std::prev(boundary.end())->first;
    }
    if (ra_opt::rows_->size() <= pivot) ra_opt::rows_->resize(pivot + 1);
  }

  if constexpr (Master_matrix::Option_list::has_map_column_container) {
    if constexpr (activeSwapOption) {
      swap_opt::indexToRow_.emplace(faceIndex, faceIndex);
      swap_opt::rowToIndex_.emplace(faceIndex, faceIndex);
    }

    if constexpr (Master_matrix::Option_list::has_row_access) {
      matrix_.try_emplace(nextInsertIndex_,
                          Column_type(nextInsertIndex_, boundary, dim, ra_opt::rows_, colSettings_));
    } else {
      matrix_.try_emplace(nextInsertIndex_, boundary, dim, colSettings_);
    }
  } else {
    if constexpr (activeSwapOption) {
      for (index i = swap_opt::indexToRow_.size(); i <= faceIndex; ++i) {
        swap_opt::indexToRow_.push_back(i);
        swap_opt::rowToIndex_.push_back(i);
      }
    }

    if constexpr (Master_matrix::Option_list::has_row_access) {
      matrix_.emplace_back(nextInsertIndex_, boundary, dim, ra_opt::rows_, colSettings_);
    } else {
      if (matrix_.size() <= nextInsertIndex_) {
        matrix_.emplace_back(boundary, dim, colSettings_);
      } else {
        matrix_[nextInsertIndex_] = Column_type(boundary, dim, colSettings_);
      }
    }
  }

  if constexpr (activeDimOption) {
    dim_opt::update_up(boundary.size() == 0 ? 0 : boundary.size() - 1);
  }

  return nextInsertIndex_++;
}

template <class Master_matrix>
inline typename Boundary_matrix<Master_matrix>::Column_type& Boundary_matrix<Master_matrix>::get_column(
    index columnIndex) 
{
  if constexpr (activeSwapOption) {
    if (swap_opt::rowSwapped_) swap_opt::_orderRows();
  }

  if constexpr (Master_matrix::Option_list::has_map_column_container) {
    return matrix_.at(columnIndex);
  } else {
    return matrix_[columnIndex];
  }
}

template <class Master_matrix>
inline typename Boundary_matrix<Master_matrix>::Row_type& Boundary_matrix<Master_matrix>::get_row(index rowIndex) 
{
  static_assert(Master_matrix::Option_list::has_row_access, "'get_row' is not implemented for the chosen options.");

  if constexpr (activeSwapOption) {
    if (swap_opt::rowSwapped_) swap_opt::_orderRows();
  }

  return ra_opt::get_row(rowIndex);
}

template <class Master_matrix>
inline typename Boundary_matrix<Master_matrix>::index Boundary_matrix<Master_matrix>::remove_last() 
{
  static_assert(Master_matrix::Option_list::has_removable_columns,
                "'remove_last' is not implemented for the chosen options.");

  if (nextInsertIndex_ == 0) return -1;  // empty matrix
  --nextInsertIndex_;

  if constexpr (activeDimOption) {
    dim_opt::update_down(matrix_.at(nextInsertIndex_).get_dimension());
  }

  id_index pivot;
  if constexpr (Master_matrix::Option_list::has_map_column_container) {
    auto it = matrix_.find(nextInsertIndex_);
    pivot = it->second.get_pivot();
    if constexpr (activeSwapOption) {
      // if the removed column is positive, the pivot won't change value
      if (swap_opt::rowSwapped_ && pivot != static_cast<id_index>(-1)) {
        swap_opt::_orderRows();
        pivot = it->second.get_pivot();
      }
    }
    matrix_.erase(it);
  } else {
    pivot = matrix_[nextInsertIndex_].get_pivot();
    if constexpr (activeSwapOption) {
      // if the removed column is positive, the pivot won't change value
      if (swap_opt::rowSwapped_ && pivot != static_cast<id_index>(-1)) {
        swap_opt::_orderRows();
        pivot = matrix_[nextInsertIndex_].get_pivot();
      }
    }
    if constexpr (Master_matrix::Option_list::has_row_access) {
      assert(nextInsertIndex_ == matrix_.size() - 1 && "Indexation problem.");
      matrix_.pop_back();
    } else {
      matrix_[nextInsertIndex_].clear();
    }
  }

  erase_row(nextInsertIndex_);  // maximal, so empty

  if constexpr (activePairingOption) {
    pair_opt::_remove_last(nextInsertIndex_);
  }

  return pivot;
}

template <class Master_matrix>
inline void Boundary_matrix<Master_matrix>::erase_row(index rowIndex) 
{
  id_index rowID = rowIndex;
  if constexpr (activeSwapOption) {
    if constexpr (Master_matrix::Option_list::has_map_column_container) {
      auto it = swap_opt::indexToRow_.find(rowIndex);
      rowID = it->second;
      swap_opt::rowToIndex_.erase(rowID);
      swap_opt::indexToRow_.erase(it);
    } else {
      rowID = swap_opt::indexToRow_[rowIndex];
    }
  }

  if constexpr (Master_matrix::Option_list::has_row_access && Master_matrix::Option_list::has_removable_rows) {
    ra_opt::erase_row(rowID);
  }
}

template <class Master_matrix>
inline typename Boundary_matrix<Master_matrix>::index Boundary_matrix<Master_matrix>::get_number_of_columns() const 
{
  if constexpr (Master_matrix::Option_list::has_map_column_container) {
    return matrix_.size();
  } else {
    return nextInsertIndex_;  // matrix could have been resized much bigger while insert
  }
}

template <class Master_matrix>
inline typename Boundary_matrix<Master_matrix>::dimension_type Boundary_matrix<Master_matrix>::get_column_dimension(
    index columnIndex) const 
{
  if constexpr (Master_matrix::Option_list::has_map_column_container) {
    return matrix_.at(columnIndex).get_dimension();
  } else {
    return matrix_[columnIndex].get_dimension();
  }
}

template <class Master_matrix>
inline void Boundary_matrix<Master_matrix>::add_to(index sourceColumnIndex, index targetColumnIndex) 
{
  if constexpr (Master_matrix::Option_list::has_map_column_container) {
    matrix_.at(targetColumnIndex) += matrix_.at(sourceColumnIndex);
  } else {
    matrix_[targetColumnIndex] += matrix_[sourceColumnIndex];
  }
}

template <class Master_matrix>
inline void Boundary_matrix<Master_matrix>::multiply_target_and_add_to(index sourceColumnIndex,
                                                                       const Field_element_type& coefficient,
                                                                       index targetColumnIndex) 
{
  if constexpr (Master_matrix::Option_list::has_map_column_container) {
    matrix_.at(targetColumnIndex).multiply_and_add(coefficient, matrix_.at(sourceColumnIndex));
  } else {
    matrix_[targetColumnIndex].multiply_and_add(coefficient, matrix_[sourceColumnIndex]);
  }
}

template <class Master_matrix>
inline void Boundary_matrix<Master_matrix>::multiply_source_and_add_to(const Field_element_type& coefficient,
                                                                       index sourceColumnIndex,
                                                                       index targetColumnIndex) 
{
  if constexpr (Master_matrix::Option_list::has_map_column_container) {
    matrix_.at(targetColumnIndex).multiply_and_add(matrix_.at(sourceColumnIndex), coefficient);
  } else {
    matrix_[targetColumnIndex].multiply_and_add(matrix_[sourceColumnIndex], coefficient);
  }
}

template <class Master_matrix>
inline void Boundary_matrix<Master_matrix>::zero_cell(index columnIndex, index rowIndex) 
{
  if constexpr (Master_matrix::Option_list::has_map_column_container) {
    if constexpr (activeSwapOption) {
      matrix_.at(columnIndex).clear(swap_opt::indexToRow_[rowIndex]);
    } else {
      matrix_.at(columnIndex).clear(rowIndex);
    }
  } else {
    if constexpr (activeSwapOption) {
      matrix_[columnIndex].clear(swap_opt::indexToRow_[rowIndex]);
    } else {
      matrix_[columnIndex].clear(rowIndex);
    }
  }
}

template <class Master_matrix>
inline void Boundary_matrix<Master_matrix>::zero_column(index columnIndex) 
{
  if constexpr (Master_matrix::Option_list::has_map_column_container) {
    matrix_.at(columnIndex).clear();
  } else {
    matrix_[columnIndex].clear();
  }
}

template <class Master_matrix>
inline bool Boundary_matrix<Master_matrix>::is_zero_cell(index columnIndex, index rowIndex) const 
{
  if constexpr (Master_matrix::Option_list::has_map_column_container) {
    if constexpr (activeSwapOption) {
      return !(matrix_.at(columnIndex).is_non_zero(swap_opt::indexToRow_.at(rowIndex)));
    } else {
      return !(matrix_.at(columnIndex).is_non_zero(rowIndex));
    }
  } else {  // operator[] non const for maps
    if constexpr (activeSwapOption) {
      return !(matrix_[columnIndex].is_non_zero(swap_opt::indexToRow_[rowIndex]));
    } else {
      return !(matrix_[columnIndex].is_non_zero(rowIndex));
    }
  }
}

template <class Master_matrix>
inline bool Boundary_matrix<Master_matrix>::is_zero_column(index columnIndex) 
{
  if constexpr (Master_matrix::Option_list::has_map_column_container) {
    return matrix_.at(columnIndex).is_empty();
  } else {
    return matrix_[columnIndex].is_empty();
  }
}

template <class Master_matrix>
inline typename Boundary_matrix<Master_matrix>::index Boundary_matrix<Master_matrix>::get_pivot(index columnIndex) 
{
  if constexpr (activeSwapOption) {
    if (swap_opt::rowSwapped_) swap_opt::_orderRows();
  }

  if constexpr (Master_matrix::Option_list::has_map_column_container) {
    return matrix_.at(columnIndex).get_pivot();
  } else {
    return matrix_[columnIndex].get_pivot();
  }
}

template <class Master_matrix>
inline Boundary_matrix<Master_matrix>& Boundary_matrix<Master_matrix>::operator=(const Boundary_matrix& other) 
{
  dim_opt::operator=(other);
  swap_opt::operator=(other);
  pair_opt::operator=(other);
  ra_opt::operator=(other);

  matrix_.clear();
  nextInsertIndex_ = other.nextInsertIndex_;
  colSettings_ = other.colSettings_;

  matrix_.reserve(other.matrix_.size());
  if constexpr (Master_matrix::Option_list::has_map_column_container) {
    for (const auto& p : other.matrix_) {
      const Column_type& col = p.second;
      if constexpr (Master_matrix::Option_list::has_row_access) {
        matrix_.try_emplace(p.first, Column_type(col, col.get_column_index(), ra_opt::rows_, colSettings_));
      } else {
        matrix_.try_emplace(p.first, Column_type(col, colSettings_));
      }
    }
  } else {
    for (const auto& col : other.matrix_) {
      if constexpr (Master_matrix::Option_list::has_row_access) {
        matrix_.emplace_back(col, col.get_column_index(), ra_opt::rows_, colSettings_);
      } else {
        matrix_.emplace_back(col, colSettings_);
      }
    }
  }

  return *this;
}

template <class Master_matrix>
inline void Boundary_matrix<Master_matrix>::print() 
{
  if constexpr (activeSwapOption) {
    if (swap_opt::rowSwapped_) swap_opt::_orderRows();
  }
  std::cout << "Boundary_matrix:\n";
  for (index i = 0; i < nextInsertIndex_; ++i) {
    Column_type& col = matrix_[i];
    for (auto e : col.get_content(nextInsertIndex_)) {
      if (e == 0u)
        std::cout << "- ";
      else
        std::cout << e << " ";
    }
    std::cout << "\n";
  }
  std::cout << "\n";
  if constexpr (Master_matrix::Option_list::has_row_access) {
    std::cout << "Row Matrix:\n";
    for (id_index i = 0; i < nextInsertIndex_; ++i) {
      const auto& row = ra_opt::rows_[i];
      for (const auto& cell : row) {
        std::cout << cell.get_column_index() << " ";
      }
      std::cout << "(" << i << ")\n";
    }
    std::cout << "\n";
  }
}

}  // namespace persistence_matrix
}  // namespace Gudhi

#endif  // PM_BOUNDARY_MATRIX_H
