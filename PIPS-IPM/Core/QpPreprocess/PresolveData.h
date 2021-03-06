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
#include <queue>

class PresolveData
{
private:
      sData* presProb;

      StochPostsolver* const postsolver;

      const double limit_max_bound_accepted;

      const int length_array_outdated_indicators;
      bool* array_outdated_indicators;
      bool& outdated_lhsrhs;
      bool& outdated_nnzs;
      bool& outdated_linking_var_bounds;
      bool& outdated_activities;
      bool& outdated_obj_vector;
      bool& postsolve_linking_row_propagation_needed;

      /* counter to indicate how many linking row bounds got changed locally and thus need activity recomputation */
      int linking_rows_need_act_computation;

      /* number of non-zero elements of each row / column */
      SmartPointer<StochVectorBase<int> > nnzs_row_A;
      SmartPointer<StochVectorBase<int> > nnzs_row_C;
      SmartPointer<StochVectorBase<int> > nnzs_col;

      /* size of non-zero changes array = #linking rows A + #linking rows C + # linking variables */
      int length_array_nnz_chgs;
      int* array_nnz_chgs;
      SmartPointer<SimpleVectorBase<int> > nnzs_row_A_chgs;
      SmartPointer<SimpleVectorBase<int> > nnzs_row_C_chgs;
      SmartPointer<SimpleVectorBase<int> > nnzs_col_chgs;

      /* In the constructor all unbounded entries will be counted.
       * Unbounded entries mean variables with non-zero multiplier that are unbounded in either upper or lower direction.
       * Activities will be computed once the amount of unbounded variables in upper or lower direction falls below 2 so
       * that bound strengthening becomes possible.
       */
      /* StochVecs for upper and lower activities and unbounded entries */
      StochVectorHandle actmax_eq_part;
      StochVectorHandle actmin_eq_part;

      SmartPointer<StochVectorBase<int> > actmax_eq_ubndd;
      SmartPointer<StochVectorBase<int> > actmin_eq_ubndd;

      StochVectorHandle actmax_ineq_part;
      StochVectorHandle actmin_ineq_part;

      SmartPointer<StochVectorBase<int> > actmax_ineq_ubndd;
      SmartPointer<StochVectorBase<int> > actmin_ineq_ubndd;

      /// changes in boundedness and activities of linking rows get stored and synchronized
      int lenght_array_act_chgs;
      double* array_act_chgs;
      SimpleVectorHandle actmax_eq_chgs;
      SimpleVectorHandle actmin_eq_chgs;
      SimpleVectorHandle actmax_ineq_chgs;
      SimpleVectorHandle actmin_ineq_chgs;

      int* array_act_unbounded_chgs;
      SmartPointer<SimpleVectorBase<int> > actmax_eq_ubndd_chgs;
      SmartPointer<SimpleVectorBase<int> > actmin_eq_ubndd_chgs;
      SmartPointer<SimpleVectorBase<int> > actmax_ineq_ubndd_chgs;
      SmartPointer<SimpleVectorBase<int> > actmin_ineq_ubndd_chgs;

      /* handling changes in bounds */
      int lenght_array_bound_chgs;
      double* array_bound_chgs;
      SimpleVectorHandle bound_chgs_A;
      SimpleVectorHandle bound_chgs_C;

      /* storing so far found singleton rows and columns */
      std::queue<INDEX> singleton_rows;
      std::queue<INDEX> singleton_cols;

      const int my_rank;
      const bool distributed;

      const double INF_NEG;
      const double INF_POS;

      // number of children
      const int nChildren;

      /* should we track a row/column through the presolving process - set in StochOptions */
      const bool track_row;
      const bool track_col;

      const INDEX tracked_row;
      const INDEX tracked_col;

      // objective offset created by presolving
      double objOffset;
      double obj_offset_chgs;
      SimpleVectorHandle objective_vec_chgs;

      // store free variables which bounds are only implied by bound tightening to remove bounds later again
      SmartPointer<StochVectorBase<int> > lower_bound_implied_by_system;
      SmartPointer<StochVectorBase<int> > lower_bound_implied_by_row;
      SmartPointer<StochVectorBase<int> > lower_bound_implied_by_node;

      // TODO a vector of INDEX would be nicer
      SmartPointer<StochVectorBase<int> > upper_bound_implied_by_system;
      SmartPointer<StochVectorBase<int> > upper_bound_implied_by_row;
      SmartPointer<StochVectorBase<int> > upper_bound_implied_by_node;

      /* storing biggest and smallest absolute nonzero-coefficient in system matrix (including objective vector) */
      StochVectorHandle absmin_col;
      StochVectorHandle absmax_col;

      bool in_bound_tightening;
      std::vector<int> store_linking_row_boundTightening_A;
      std::vector<int> store_linking_row_boundTightening_C;

public :

      PresolveData(const sData* sorigprob, StochPostsolver* postsolver);
      ~PresolveData();

      const sData& getPresProb() const { return *presProb; };

      double getObjOffset() const { return objOffset; };
      int getNChildren() const { return nChildren; };

      void getRowActivities( const INDEX& row, double& max_act, double& min_act, int& max_ubndd, int& min_ubndd) const;
      void getRowBounds( const INDEX& row, double& lhs, double& rhs) const;
      void getColBounds( const INDEX& col, double& xlow, double& xupp) const;


      double getRowCoeff( const INDEX& row, const INDEX& col ) const;

      const StochVectorBase<int>& getNnzsRow(SystemType system_type) const { return (system_type == EQUALITY_SYSTEM) ? *nnzs_row_A : *nnzs_row_C; }
      const StochVectorBase<int>& getNnzsRowA() const { return *nnzs_row_A; }; // todo maybe this is a problem - these counters might not be up to date
      const StochVectorBase<int>& getNnzsRowC() const { return *nnzs_row_C; };
      const StochVectorBase<int>& getNnzsCol() const { return *nnzs_col; };

      int getNnzsRow(const INDEX& row) const;
      int getNnzsCol(const INDEX& col) const;

      std::queue<INDEX>& getSingletonRows() { return singleton_rows; };
      std::queue<INDEX>& getSingletonCols() { return singleton_cols; };

      sData* finalize();

      /* reset originally free variables' bounds to +- inf iff their current bounds are still implied by the problem */
      void resetOriginallyFreeVarsBounds( const sData& orig_prob );

      /* whether or not there is currently changes buffered that need synchronization among all procs */
      bool reductionsEmpty();

      /* checks activities, non-zeros and root node */
      bool presDataInSync() const;

      /// synchronizing the problem over all mpi processes if necessary
      // TODO : add a allreduceEverything method that simply calls all the others
      void allreduceLinkingVarBounds();
      void allreduceAndApplyLinkingRowActivities();
      void allreduceAndApplyNnzChanges();
      void allreduceAndApplyBoundChanges();
      void allreduceAndApplyObjVecChanges();
      void allreduceObjOffset();

      bool wasColumnRemoved( const INDEX& col ) const;
      bool wasRowRemoved( const INDEX& row ) const;

      /// interface methods called from the presolvers when they detect a possible modification
      void startColumnFixation();
      void fixColumn( const INDEX& col, double value);
      void fixEmptyColumn( const INDEX& col, double val);

      void removeSingletonRow(const INDEX& row, const INDEX& col, double xlow_new, double xupp_new, double coeff);
      void removeSingletonRowSynced(const INDEX& row, const INDEX& col, double xlow_new, double xupp_new, double coeff);

      void syncPostsolveOfBoundsPropagatedByLinkingRows();

      void startBoundTightening();
      bool rowPropagatedBounds( const INDEX& row, const INDEX& col, double ubx, double lbx);
      void endBoundTightening();

      void startParallelRowPresolve();
      void substituteVariableNearlyParallelRows(const INDEX& row1, const INDEX& row2, const INDEX& col1, const INDEX& col2, double scalar,
         double translation, double parallelity );
      void tightenBoundsNearlyParallelRows( const INDEX& row1, const INDEX& row2, const INDEX& col1, const INDEX& col2, double xlow_new, double xupp_new, double scalar,
            double translation, double parallel_factor );

      void removeRedundantParallelRow( const INDEX& rm_row, const INDEX& par_row );
      void removeRedundantRow( const INDEX& row );

      void startSingletonColumnPresolve();
      void fixColumnInequalitySingleton( const INDEX& col, const INDEX& row, double value, double coeff );
      void removeImpliedFreeColumnSingletonEqualityRow( const INDEX& row, const INDEX& col);
      void removeImpliedFreeColumnSingletonEqualityRowSynced( const INDEX& row, const INDEX& col );

      void removeFreeColumnSingletonInequalityRow( const INDEX& row, const INDEX& col, double coeff );
      void removeFreeColumnSingletonInequalityRowSynced( const INDEX& row, const INDEX& col, double coeff );

      void tightenRowBoundsParallelRow( const INDEX& row_tightened, const INDEX& row_tightening, double clow_new, double cupp_new, double factor );

      /* call whenever a single entry has been deleted from the matrix */
      void deleteEntryAtIndex(const INDEX& row, const INDEX& col, int col_index);

      /* methods for verifying state of presData or querying the problem */
      bool verifyNnzcounters() const;
      bool verifyActivities() const;

      bool nodeIsDummy(int node) const;
      bool hasLinking(SystemType system_type) const;

      bool varBoundImpliedFreeBy( bool upper, const INDEX& col, const INDEX& row);
private:
      bool iTrackColumn() const;
      bool iTrackRow() const;

      void setRowBounds( const INDEX& row, double clow, double cupp);
      bool updateColBounds( const INDEX& col, double xlow, double xupp);

      void setRowUpperBound( const INDEX& row, double rhs )
      {
         row.inEqSys() ? setRowBounds( row, rhs, rhs ) : setRowBounds( row, INF_NEG, rhs );
      }

      void setRowLowerBound( const INDEX& row, double lhs )
      {
         row.inEqSys() ? setRowBounds( row, lhs, lhs ) : setRowBounds( row, lhs, INF_POS );
      }

      bool updateColLowerBound( const INDEX& col, double xlow )
      {
         return updateColBounds( col, xlow, INF_POS );
      }

      bool updateColUpperBound( const INDEX& col, double xupp )
      {
         return updateColBounds( col, INF_NEG, xupp );
      }

      void adaptObjectiveSubstitutedRow( const INDEX& row, const INDEX& col, double obj_coeff, double col_coeff );
      void addCoeffColToRow( double coeff, const INDEX& col, const INDEX& row );

      INDEX getRowMarkedAsImplyingColumnBound(const INDEX& col, bool upper_bound);
      void markRowAsImplyingColumnBound(const INDEX& col, const INDEX& row, bool upper_bound);

      void markColumnRemoved( const INDEX& col );

      void varboundImpliedFreeFullCheck(bool& upper_implied, bool& lower_implied, const INDEX& col, const INDEX& row) const;

      /// methods for printing debug information
      // initialize row and column nnz counter
      void initNnzCounter(StochVectorBase<int>& nnzs_row_A, StochVectorBase<int>& nnzs_row_C, StochVectorBase<int>& nnzs_col) const;
      void initSingletons();

      void initAbsminAbsmaxInCols(StochVector& absmin, StochVector& absmax) const;

      void setUndefinedVarboundsTo(double value);
      void setUndefinedRowboundsTo(double value);

      // TODO : this should probably go into StochVector and SimpleVector
      void setNotIndicatedEntriesTo(StochVector& svec, StochVector& sivec, double value);


      void addActivityOfBlock( const SparseStorageDynamic& matrix, SimpleVector& min_partact, 
            SimpleVectorBase<int>& unbounded_min, SimpleVector& max_partact,
            SimpleVectorBase<int>& unbounded_max, const SimpleVector& xlow, 
            const SimpleVector& ixlow, const SimpleVector& xupp, 
            const SimpleVector& ixupp) const ;

      long resetOriginallyFreeVarsBounds(const SimpleVector& ixlow_orig, const SimpleVector& ixupp_orig, int node);

      void adjustMatrixRhsLhsBy( const INDEX& row, double value, bool at_root );
      /// methods for modifying the problem
      void adjustRowActivityFromDeletion( const INDEX& row, const INDEX& col, double coeff );
      /// set bounds if new bound is better than old bound
      void updateRowActivities( const INDEX& col, double xlow_new, double xupp_new, double xlow_old, double xupp_old);

      void updateRowActivitiesBlock( const INDEX& row, const INDEX& col, double xlow_new, double xupp_new, double xlow_old, double xupp_old);

      void updateRowActivitiesBlock( const INDEX& row, const INDEX& col, double bound, double old_bound, bool upper);

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

      void recomputeActivities(bool linkinig_only, StochVector& actmax_eq_part, StochVector& actmin_eq_part, StochVectorBase<int>& actmax_eq_ubndd,
         StochVectorBase<int>& actmin_eq_ubndd, StochVector& actmax_ineq_part, StochVector& actmin_ineq_part, StochVectorBase<int>& actmax_ineq_ubndd,
         StochVectorBase<int>& actmin_ineq_ubndd) const;

      double computeLocalLinkingRowMinOrMaxActivity(const INDEX& row, bool upper) const;
      void computeRowMinOrMaxActivity(const INDEX& row, bool upper);

      void removeColumn(const INDEX& col, double fixation);
      void removeColumnFromMatrix(const INDEX& row, const INDEX& col, double fixation);
      void removeRow( const INDEX& row );
      void removeRowFromMatrix(const INDEX& row, const INDEX& col);

      void reduceNnzCounterRowBy(const INDEX& row, int amount, bool at_root);
      void increaseNnzCounterRowBy(const INDEX& row, int amount, bool at_root);

      void changeNnzCounterRow(const INDEX& row, int amount, bool at_root);

      void reduceNnzCounterColumnBy(const INDEX& col, int amount, bool at_root);
      void increaseNnzCounterColumnBy(const INDEX& col, int amount, bool at_root);

      void changeNnzCounterColumn(const INDEX& col, int amount, bool at_root);

      /// methods for querying the problem in order to get certain structures etc.
      StochGenMatrix& getSystemMatrix(SystemType system_type) const;
      SparseGenMatrix* getSparseGenMatrix(const INDEX& row, const INDEX& col) const;

      void checkBoundsInfeasible(const INDEX& col, double xlow_new, double xupp_new) const;
public:
      void writeRowLocalToStreamDense(std::ostream& out, const INDEX& row) const;
      void printRowColStats() const;
private:
      void writeMatrixRowToStreamDense(std::ostream& out, const SparseGenMatrix& mat, int node, int row, const SimpleVector& ixupp, const SimpleVector& xupp,
            const SimpleVector& ixlow, const SimpleVector& xlow) const;
      void printVarBoundStatistics(std::ostream& out) const;
};

#endif /* PIPS_IPM_CORE_QPPREPROCESS_PRESOLVEDATA_H_ */
