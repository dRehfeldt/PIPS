/* PIPS
   Authors: Cosmin Petra
   See license and copyright information in the documentation */

#ifndef STOCHLEAFLINSYS
#define STOCHLEAFLINSYS

#include "sLinsys.h"
#include "Ma57Solver.h"
#include "sTree.h"
#include "sFactory.h"
#include "sData.h"
#include "SparseSymMatrix.h"
#include "SparseGenMatrix.h"

#include "pipsport.h"

/** This class solves the linear system corresponding to a leaf node.
 *  It just redirects the call to QpGenSparseLinsys.
 */
class sLinsysLeaf : public sLinsys
{
 public:
  //sLinsysLeaf(QpGenStoch * factory_, sData * prob_);
  template<class LINSOLVER>
    sLinsysLeaf(sFactory* factory,
		sData* prob_,				    
		OoqpVector* dd_, OoqpVector* dq_, OoqpVector* nomegaInv_,
		OoqpVector* rhs_, LINSOLVER *linsolver=nullptr);

  virtual ~sLinsysLeaf();

  virtual void factor2( sData *prob, Variables *vars);
  virtual void Lsolve ( sData *prob, OoqpVector& x );
  virtual void Dsolve ( sData *prob, OoqpVector& x );
  virtual void Ltsolve( sData *prob, OoqpVector& x );

  //virtual void Lsolve2 ( OoqpVector& x );
  //virtual void Dsolve2 ( OoqpVector& x );
  virtual void Ltsolve2( sData *prob, StochVector& x, SimpleVector& xp);

  virtual void putZDiagonal( OoqpVector& zdiag );
  //virtual void solveCompressed( OoqpVector& rhs );
  virtual void putXDiagonal( OoqpVector& xdiag_ );

  //void Ltsolve_internal(  sData *prob, StochVector& x, SimpleVector& xp);
  void sync();
  virtual void deleteChildren();
 protected:
  sLinsysLeaf() {};

  static void mySymAtPutSubmatrix(SymMatrix& kkt, 
				  GenMatrix& B, GenMatrix& D, 
				  int locnx, int locmy, int locmz);
}; 

template<class LINSOLVER>
sLinsysLeaf::sLinsysLeaf(sFactory *factory_, sData* prob,
			 OoqpVector* dd_, 
			 OoqpVector* dq_,
			 OoqpVector* nomegaInv_,
			 OoqpVector* rhs_,
			 LINSOLVER* thesolver)
  : sLinsys(factory_, prob, dd_, dq_, nomegaInv_, rhs_)
{
#ifdef TIMING
  int myRank; MPI_Comm_rank(MPI_COMM_WORLD, &myRank);
  const double t0 = MPI_Wtime();
#endif

  // size = ?  nnz = ?
  int nnzQ, nnzB, nnzD;

  prob->getLocalSizes(locnx, locmy, locmz, locmyl, locmzl);
  const int n = locnx+locmy+locmz;

  prob->getLocalNnz(nnzQ, nnzB, nnzD);

#ifdef TIMING
  if( myRank == 0 ) std::cout << "Rank 0: building local Schur matrix ..." << std::endl;
#endif

  //alocate the matrix and copy the data into
  SparseSymMatrix* kktsp = new SparseSymMatrix(n, n+nnzQ+nnzB+nnzD);
  kkt = kktsp;

  SimpleVectorHandle v( new SimpleVector(n) );
  v->setToZero();
  kkt->setToDiagonal(*v);

  kkt->symAtPutSubmatrix( 0, 0, prob->getLocalQ(), 0, 0, locnx, locnx);
  if(locmz>0) {
    kkt->symAtPutSubmatrix( locnx, 0, prob->getLocalB(), 0, 0, locmy, locnx);
    kkt->symAtPutSubmatrix( locnx+locmy, 0, prob->getLocalD(), 0, 0, locmz, locnx);
    
  } else
    mySymAtPutSubmatrix(*kkt, prob->getLocalB(), prob->getLocalD(), locnx, locmy, locmz);

#ifdef TIMING
  if( myRank == 0 ) std::cout << "Rank 0: finished " << std::endl;
#endif

  // create the solver for the linear system
  solver = new LINSOLVER(kktsp);

#ifdef TIMING
  const double t1 = MPI_Wtime() - t0;
  if (myRank == 0) printf("Rank 0: new sLinsysLeaf took %f sec\n",t1);
#endif

  mpiComm = (dynamic_cast<StochVector*>(dd_))->mpiComm;
}



#endif
