/*
 * PresolveData.h
 *
 *  Created on: 06.04.2018
 *      Author: bzfrehfe
 */

#ifndef PIPS_IPM_CORE_QPPREPROCESS_PRESOLVEDATA_H_
#define PIPS_IPM_CORE_QPPREPROCESS_PRESOLVEDATA_H_

#include "sData.h"
#include "StochPostsolver.h"
#include "StochVectorHandle.h"
#include "SimpleVectorHandle.h"
#include "SparseStorageDynamic.h"
#include "SystemType.h"
#include <algorithm>
#include <list>
#include <limits>

struct sCOLINDEX
{
      sCOLINDEX(int node, int index) : node(node), index(index) {};
      int node;
      int index;
};

/* here we use node == -2 for the linking block in the root node */
struct sROWINDEX
{
      sROWINDEX(int node, int index, SystemType system_type) :
         node(node), index(index), system_type(system_type){};
      int node;
      int index;
      SystemType system_type;
};

class PresolveData
{
private:
      sData* presProb;

      StochPostsolver* const postsolver;

      // todo make these ints
      bool outdated_lhsrhs;
      bool outdated_nnzs;
      bool outdated_linking_var_bounds;

      bool outdated_activities;

      /* counter to indicate how many linking row bounds got changed locally and thus need activity recomputation */
      int linking_rows_need_act_computation;

      /* number of non-zero elements of each row / column */
      StochVectorHandle nnzs_row_A;
      StochVectorHandle nnzs_row_C;
      StochVectorHandle nnzs_col;

      /* size of non-zero changes array = #linking rows A + #linking rows C + # linking variables */
      int length_array_nnz_chgs;
      double* array_nnz_chgs;
      SimpleVectorHandle nnzs_row_A_chgs;
      SimpleVectorHandle nnzs_row_C_chgs;
      SimpleVectorHandle nnzs_col_chgs;

      /* In the constructor all unbounded entries will be counted.
       * Unbounded entries mean variables with non-zero multiplier that are unbounded in either upper or lower direction.
       * Activities will be computed once the amount of unbounded variables in upper or lower direction falls below 2 so
       * that bound strengthening becomes possible.
       */
      /* StochVecs for upper and lower activities and unbounded entries */
      StochVectorHandle actmax_eq_part;
      StochVectorHandle actmin_eq_part;

      StochVectorHandle actmax_eq_ubndd;
      StochVectorHandle actmin_eq_ubndd;

      StochVectorHandle actmax_ineq_part;
      StochVectorHandle actmin_ineq_part;

      StochVectorHandle actmax_ineq_ubndd;
      StochVectorHandle actmin_ineq_ubndd;

      /// changes in boundedness and activities of linking rows get stored and synchronized
      int lenght_array_act_chgs;
      double* array_act_chgs;
      SimpleVectorHandle actmax_eq_chgs;
      SimpleVectorHandle actmin_eq_chgs;
      SimpleVectorHandle actmax_ineq_chgs;
      SimpleVectorHandle actmin_ineq_chgs;

      double* array_act_unbounded_chgs;
      SimpleVectorHandle actmax_eq_ubndd_chgs;
      SimpleVectorHandle actmin_eq_ubndd_chgs;
      SimpleVectorHandle actmax_ineq_ubndd_chgs;
      SimpleVectorHandle actmin_ineq_ubndd_chgs;

      /* handling changes in bounds */
      int lenght_array_bound_chgs;
      double* array_bound_chgs;
      SimpleVectorHandle bound_chgs_A;
      SimpleVectorHandle bound_chgs_C;

      /* storing so far found singleton rows and columns */
      std::vector<sROWINDEX> singleton_rows;
      std::vector<sCOLINDEX> singleton_cols;

      int my_rank;
      bool distributed;

      // number of children
      int nChildren;

      // objective offset created by presolving
      double objOffset;
      double obj_offset_chgs;

      // necessary ?
      int elements_deleted;
      int elements_deleted_transposed;

public :
      const sData& getPresProb() const { return *presProb; };

      void getRowActivities(SystemType system_type, int node, BlockType block_type, int row,
            double& max_act, double& min_act, int& max_ubndd, int& min_ubndd) const;

      const StochVector& getNnzsRowA() const { return *nnzs_row_A; }; // todo maybe this is a problem - these counters might not be up to date
      const StochVector& getNnzsRowC() const { return *nnzs_row_C; };
      const StochVector& getNnzsCol() const { return *nnzs_col; };

      const std::vector<sROWINDEX>& getSingletonRows() const { return singleton_rows; };
      const std::vector<sCOLINDEX>& getSingletonCols() const { return singleton_cols; };

      /* methods for initializing the object */
   private:
      // initialize row and column nnz counter
      void initNnzCounter(StochVector& nnzs_row_A, StochVector& nnzs_row_C, StochVector& nnzs_col) const;
      void initSingletons();
      void setUndefinedVarboundsTo(double value);

   public:
      PresolveData(const sData* sorigprob, StochPostsolver* postsolver);
      ~PresolveData();

      sData* finalize();
      bool reductionsEmpty();

      /* compute and update activities */
      void recomputeActivities() { recomputeActivities(false); }

      /* computes all row activities and number of unbounded variables per row
       * If there is more than one unbounded variable in the min/max activity of a row
       * +/-infinity() is stored. Else the actual partial activity is computed and stored.
       * For rows with one unbounded variable we store the partial activity without that
       * one variable, for rows with zero unbounded vars the stored activity is the actual
       * activity of that row.
       */
      void recomputeActivities(bool linking_only);
   private:
      void addActivityOfBlock( const SparseStorageDynamic& matrix, SimpleVector& min_partact, SimpleVector& unbounded_min, SimpleVector& max_partact,
            SimpleVector& unbounded_max, const SimpleVector& xlow, const SimpleVector& ixlow, const SimpleVector& xupp, const SimpleVector& ixupp) const;


public:
      // todo getter, setter for element access of nnz counter???
      double getObjOffset() const { return objOffset; };
      double addObjOffset(double addOffset);
      void setObjOffset(double offset);
      int getNChildren() const { return nChildren; };

      /// synchronizing the problem over all mpi processes if necessary
      void allreduceLinkingVarBounds();
      void allreduceAndApplyLinkingRowActivities();
      void allreduceAndApplyNnzChanges();
      void allreduceAndApplyBoundChanges();

      /// interface methods called from the presolvers when they detect a possible modification
      // todo make bool and give feedback or even better - return some enum maybe?
      void fixColumn(int node, int col, double value);
      bool rowPropagatedBounds( SystemType system_type, int node, BlockType block_type, int row, int col, double ubx, double lbx);
      void removeRedundantRow(SystemType system_type, int node, int row, bool linking);
      void removeParallelRow(SystemType system_type, int node, int row, bool linking);

      /* call whenever a single entry has been deleted from the matrix */
      void deleteEntry(SystemType system_type, int node, BlockType block_type, int row_index, int& index_k, int& row_end);
      void updateTransposedSubmatrix( SystemType system_type, int node, BlockType block_type, std::vector<std::pair<int, int> >& elements);

      /* methods for verifying state of presData or querying the problem */
      bool verifyNnzcounters() const;
      bool verifyActivities();
      bool elementsDeletedInTransposed() { return elements_deleted == elements_deleted_transposed; };

      bool nodeIsDummy(int node, SystemType system_type) const;
      bool hasLinking(SystemType system_type) const;

private:
      void adjustMatrixRhsLhsBy(SystemType system_type, int node, BlockType block_type, int row_index, double value);
/// methods for modifying the problem
      void adjustRowActivityFromDeletion(SystemType system_type, int node, BlockType block_type, int row, int col, double coeff);
      /// set bounds if new bound is better than old bound
      bool updateUpperBoundVariable(int node, int col, double ubx)
      { return updateBoundsVariable(node, col, ubx, -std::numeric_limits<double>::infinity()); };
      bool updateLowerBoundVariable(int node, int col, double lbx)
      { return updateBoundsVariable(node, col, std::numeric_limits<double>::infinity(), lbx); };

      bool updateBoundsVariable(int node, int col, double ubx, double lbx);
      void updateRowActivities(int node, int col, double ubx, double lbx, double old_ubx, double old_lbx);
      void updateRowActivitiesLinkingConsBlock(SystemType system_type, int node, BlockType block_type, int col, double bound,
            double old_bound, bool upper);
      void updateRowActivitiesNonLinkingConsBlock(SystemType system_type, int node, BlockType block_type, int col, double bound,
            double old_bound, bool upper);

      double computeLocalLinkingRowMinOrMaxActivity(SystemType system_type, int row, bool upper) const;
      void computeRowMinOrMaxActivity(SystemType system_type, int node, BlockType block_type, int row, bool upper);

      void removeColumn(int node, int col, double fixation);
      void removeColumnFromMatrix(SystemType system_type, int node, BlockType block_type, int col, double fixation);
      void removeRow(SystemType system_type, int node, int row, bool linking);
      void removeRowFromMatrix(SystemType system_type, int node, BlockType block_type, int row);
      void removeEntryInDynamicStorage(SparseStorageDynamic& storage, int row, int col) const;

      void removeIndexRow(SystemType system_type, int node, BlockType block_type, int row_index, int amount);
      void removeIndexColumn(int node, BlockType block_type, int col_index, int amount);
/// methods for querying the problem in order to get certain structures etc.
      SparseGenMatrix* getSparseGenMatrix(SystemType system_type, int node, BlockType block_type) const;

      SimpleVector& getSimpleVecRowFromStochVec(const OoqpVector& ooqpvec, int node, BlockType block_type) const
         { return getSimpleVecRowFromStochVec(dynamic_cast<const StochVector&>(ooqpvec), node, block_type); };
      SimpleVector& getSimpleVecColFromStochVec(const OoqpVector& ooqpvec, int node) const
         { return getSimpleVecColFromStochVec(dynamic_cast<const StochVector&>(ooqpvec), node); };

      SimpleVector& getSimpleVecRowFromStochVec(const StochVector& stochvec, int node, BlockType block_type) const;
      SimpleVector& getSimpleVecColFromStochVec(const StochVector& stochvec, int node) const;

public:
      void writeRowLocalToStreamDense(std::ostream& out, SystemType system_type, int node, BlockType block_type, int row) const;
private:
      void writeMatrixRowToStreamDense(std::ostream& out, const SparseGenMatrix& mat, int node, int row, const SimpleVector& ixupp, const SimpleVector& xupp,
            const SimpleVector& ixlow, const SimpleVector& xlow) const;

};

#endif /* PIPS_IPM_CORE_QPPREPROCESS_PRESOLVEDATA_H_ */

