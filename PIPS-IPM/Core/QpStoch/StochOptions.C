#include "StochOptions.h"
#include <limits>

namespace pips_options
{
   StochOptions::StochOptions()
   {
      setDefaults();
   }

   // todo maybe split this up into several submethods?
   void StochOptions::setDefaults()
   {
      // TODO
      /* default bool values */
      bool_options["dummy"] = false;
      bool_options["POSTSOLVE"] = true;

      /* default int values */
      int_options["dummy"] = 1;

      /* default double values */
      double_options["dummy"] = 1.0;

      /** all presolve/postsolve constants and settings */
      // TODO : many of these need adjustments/ have to be thought about
      double_options["PRESOLVE_INFINITY"] = std::numeric_limits<double>::infinity();

      /// STOCH PRESOLVER
      /** limit for max rounds to apply all presolvers */
      int_options["PRESOLVE_MAX_ROUNDS"] = 1;
      /** should the problem be written to std::cout before and after presolve */
      bool_options["PRESOLVE_PRINT_PROBLEM"] = false;
      /** should free variables' bounds be reset after presolve (given the row implying these bounds was not removed */
      bool_options["PRESOLVE_RESET_FREE_VARIABLES"] = false;

      /** turn respective presolvers on/off */
      bool_options["PRESOLVE_BOUND_STRENGTHENING"] = true;
      bool_options["PRESOLVE_PARALLEL_ROWS"] = true;
      bool_options["PRESOLVE_COLUMN_FIXATION"] = true;
      bool_options["PRESOLVE_SINGLETON_ROWS"] = true;
      bool_options["PRESOLVE_SINGLETON_COLUMNS"] = true;

      /// BOUND STRENGTHENING
      /** limit for rounds of bound strengthening per call of presolver */
      int_options["PRESOLVE_BOUND_STR_MAX_ITER"] = 1;
      /** min entry to devide by in order to derive a bound */
      double_options["PRESOLVE_BOUND_STR_NUMERIC_LIMIT_ENTRY"] = 1e-7;
      /** max activity to be devided */
      double_options["PRESOLVE_BOUND_STR_MAX_PARTIAL_ACTIVITY"] = std::numeric_limits<double>::max();
      /** max bounds proposed from bounds strengthening presolver */
      double_options["PRESOLVE_BOUND_STR_NUMERIC_LIMIT_BOUNDS"] = 1e12;

      /// COLUMN FIXATION
      /** limit on the possible impact a column can have on the problem */
      double_options["PRESOLVE_COLUMN_FIXATION_MAX_FIXING_IMPACT"] = 1.0e-12; // for variable fixing

      /// MODEL CLEANUP
      /** limit for the size of a matrix entry below which it will be removed from the problem */
      double_options["PRESOLVE_MODEL_CLEANUP_MIN_MATRIX_ENTRY"] = 1.0e-10;//1.0e-10; // for model cleanup // was 1.0e-10
      /** max for the matrix entry when the impact of entry times (bux-blx) is considered */
      double_options["PRESOLVE_MODEL_CLEANUP_MAX_MATRIX_ENTRY_IMPACT"] = 1.0e-3; // was 1.0e-3
      /** difference in orders between feastol and the impact of entry times (bux-blx) for an entry to get removed */
      double_options["PRESOLVE_MODEL_CLEANUP_MATRIX_ENTRY_IMPACT_FEASDIST"] = 1.0e-2;  // for model cleanup // was 1.0e-2

      /// PARALLEL ROWS
      /** tolerance for comparing two double values in two different rows and for them being considered equal */
      double_options["PRESOLVE_PARALLEL_ROWS_TOL_COMPARE_ENTRIES"] = 1.0e-8;

      /// PRESOLVE DATA
      double_options["PRESOLVE_MAX_BOUND_ACCEPTED"] = 1e10;

      /// LINEAR SOLVERS
      bool_options["PARDISO_FOR_GLOBAL_SC"] = true;
      bool_options["PARDISO_SPARSE_RHS_LEAF"] = false;
      /** -1 is choose default */
      int_options["PARDISO_SYMB_INTERVAL"] = -1;
      int_options["PARDISO_PIVOT_PERTURBATION"] = -1;
      int_options["PARDISO_NITERATIVE_REFINS"] = -1;
      int_options["PARDISO_PIVOT_PERTURBATION_ROOT"] = -1;
      int_options["PARDISO_NITERATIVE_REFINS_ROOT"] = -1;

      /// PRECONDITIONERS
      bool_options["PRECONDITION_DISTRIBUTED"] = true;
      bool_options["PRECONDITION_SPARSE"] = true;
      /** -1.0 is choose default */
      double_options["PRECONDITION_DIAGDOM_BOUND"] = -1.0;

      /// INTERIOR-POINT ALGORITHM
      bool_options["IP_ACCURACY_REDUCED"] = false;
      bool_options["IP_PRINT_TIMESTAMP"] = false;
      bool_options["IP_STEPLENGTH_CONSERVATIVE"] = false;

      /// SCHUR COMPLEMENT COMPUTATION
      bool_options["SC_COMPUTE_BLOCKWISE"] = false;


   }
}