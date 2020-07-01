// Copyright (C) 2020 Netherlands eScience Center <f.zapata@esciencecenter.nl>
//
// This Source Code Form is subject to the terms of the Mozilla
// Public License v. 2.0. If a copy of the MPL was not distributed
// with this file, You can obtain one at https://mozilla.org/MPL/2.0/.

#ifndef SPECTRA_JD_SYM_EIGS_DPR_H
#define SPECTRA_JD_SYM_EIGS_DPR_H

#include <Eigen/Dense>
#include "JDSymEigsBase.h"
#include "Util/SelectionRule.h"

namespace Spectra {

///
/// \ingroup EigenSolver
///
/// This class implement the DPR correction for the Davidson algorithms.
/// The algorithms in the Davidson family only differ in how the correction
/// vectors are computed and optionally in the initial orthogonal basis set.
///
/// the DPR correction compute the new correction vector using the following expression:
/// \f[ correction = -(\boldsymbol{D} - \rho \boldsymbol{I})^{-1} \boldsymbol{r} \f]
/// where
/// \f$D\f$ is the diagonal of the target matrix, \f$\rho\f$ the Ritz eigenvalue,
/// \f$I\f$ the identity matrix and \f$r\f$ the residue vector.
///

template <typename Scalar, typename OpType>
class JDSymEigsDPR : public JDSymEigsBase<Scalar, OpType>
{
private:
    using Index = Eigen::Index;
    using Matrix = Eigen::Matrix<Scalar, Eigen::Dynamic, Eigen::Dynamic>;
    using Vector = Eigen::Matrix<Scalar, Eigen::Dynamic, 1>;

    Vector diagonal_ = Vector::Zero(this->operator_dimension_);
    std::vector<Eigen::Index> indices_sorted_;

    void extract_diagonal()
    {
        for (Index i = 0; i < this->operator_dimension_; i++)
        {
            diagonal_(i) = this->matrix_operator_(i, i);
        }
    }

    void calculate_indices_diagonal_sorted(SortRule selection)
    {
        indices_sorted_ = argsort(selection, diagonal_);
    }

public:
    JDSymEigsDPR(OpType& op, Index nev) :
        JDSymEigsBase<Scalar, OpType>{op, nev} {}

    /// Create initial search space based on the diagonal
    /// and the spectrum'target (highest or lowest)
    /// \param selection spectrum section to target (e.g. lowest, etc.)
    /// \return Matrix with the initial orthonormal basis
    Matrix SetupInitialSearchSpace(SortRule selection) final
    {
        extract_diagonal();
        calculate_indices_diagonal_sorted(selection);

        Matrix initial_basis = Matrix::Zero(this->operator_dimension_, this->initial_search_space_size_);

        for (Index k = 0; k < this->initial_search_space_size_; k++)
        {
            Index row = indices_sorted_[k];
            initial_basis(row, k) = 1.0;
        }
        return initial_basis;
    }

    /// compute the corrections using the DPR method.
    /// \return new correction vectors.
    Matrix CalculateCorrectionVector() const final
    {
        const Matrix& residues = this->ritz_pairs_.Residues();
        const Vector& eigvals = this->ritz_pairs_.RitzValues();
        Matrix correction = Matrix::Zero(this->operator_dimension_, this->size_update_);
        for (Index k = 0; k < this->size_update_; k++)
        {
            Vector tmp = eigvals(k) - diagonal_.array();
            correction.col(k) = residues.col(k).array() / tmp.array();
        }
        return correction;
    }
};

}  // namespace Spectra

#endif  // SPECTRA_JD_SYM_EIGS_DPR_H