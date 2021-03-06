/* PIPS
   Authors: Cosmin Petra
   See license and copyright information in the documentation */

#include "sFactory.h"

#include "sData.h"
#include "stochasticInput.hpp"
#include "sTreeImpl.h"
#include "sTreeCallbacks.h"
#include "StochInputTree.h"
#include "StochSymMatrix.h"
#include "StochGenMatrix.h"
#include "StochVector.h"


#include "sVars.h"
#include "sResiduals.h"

#include "sLinsysRoot.h"
#include "sLinsysLeaf.h"

#include "Ma57Solver.h"
#include "Ma27Solver.h"
#include "PardisoSolver.h"
#include "DeSymIndefSolver.h"

#include "pipsport.h"
#include "mpi.h"

#include <stdio.h>
#include <stdlib.h>


sFactory::sFactory( stochasticInput& in, MPI_Comm comm)
  : QpGen(0,0,0), data(nullptr), m_tmTotal(0.0)
{
  tree = new sTreeImpl(in, comm);
  //tree->computeGlobalSizes();
  //tree->GetGlobalSizes(nx, my, mz);
  //decide how the CPUs are assigned
  tree->assignProcesses(comm);
  tree->loadLocalSizes();
}


sFactory::sFactory( StochInputTree* inputTree, MPI_Comm comm)
  : QpGen(0,0,0), data(nullptr), m_tmTotal(0.0)
{

  tree = new sTreeCallbacks(inputTree);

  //decide how the CPUs are assigned
  tree->assignProcesses(comm);

  tree->computeGlobalSizes();
  //now the sizes of the problem are available, set them for the parent class
  tree->GetGlobalSizes(nx, my, mz);

}

sFactory::sFactory( int nx_, int my_, int mz_, int nnzQ_, int nnzA_, int nnzC_ )
  : QpGen( nx_, my_, mz_ ),
    nnzQ(nnzQ_), nnzA(nnzA_), nnzC(nnzC_),
    tree(nullptr), data(nullptr), resid(nullptr), linsys(nullptr),
    m_tmTotal(0.0)
{ };

sFactory::sFactory()
  : QpGen( 0,0,0 ), m_tmTotal(0.0)
{ };

sFactory::~sFactory()
{
  if(tree) delete tree;
}

sLinsysLeaf*
sFactory::newLinsysLeaf(sData* prob,
			OoqpVector* dd, OoqpVector* dq,
			OoqpVector* nomegaInv, OoqpVector* rhs)
{
#ifdef WITH_PARDISO
   PardisoSolver* s=nullptr;
#else
   Ma27Solver* s=nullptr;
#endif

  return new sLinsysLeaf(this, prob, dd, dq, nomegaInv, rhs, s);
}


#ifdef TIMING
#define TIM t = MPI_Wtime();
#define REP(s) t = MPI_Wtime()-t;if (p) printf("makeData: %s took %f sec\n",s,t);
#else
#define TIM
#define REP(s)
#endif


void dumpaug(int nx, SparseGenMatrix &A, SparseGenMatrix &C) {

    long long my, mz, nx_1, nx_2;
    A.getSize(my,nx_1);
    C.getSize(mz,nx_2);
    assert(nx_1 == nx_2);

    int nnzA = A.numberOfNonZeros();
    int nnzC = C.numberOfNonZeros();
    cout << "augdump  nx=" << nx << endl;
    cout << "A: " << my << "x" << nx_1 << "   nnz=" << nnzA << endl
	 << "C: " << mz << "x" << nx_1 << "   nnz=" << nnzC << endl;

	vector<double> eltsA(nnzA), eltsC(nnzC), elts(nnzA+nnzC);
	vector<int> colptrA(nx_1+1),colptrC(nx_1+1), colptr(nx_1+1), rowidxA(nnzA), rowidxC(nnzC), rowidx(nnzA+nnzC);
	A.getStorageRef().transpose(&colptrA[0],&rowidxA[0],&eltsA[0]);
	C.getStorageRef().transpose(&colptrC[0],&rowidxC[0],&eltsC[0]);

	int nnz = 0;
	for (int col = 0; col < nx_1; col++) {
		colptr[col] = nnz;
		for (int r = colptrA[col]; r < colptrA[col+1]; r++) {
			int row = rowidxA[r]+nx+1; // +1 for fortran
			rowidx[nnz] = row;
			elts[nnz++] = eltsA[r];
		}
		for (int r = colptrC[col]; r < colptrC[col+1]; r++) {
			int row = rowidxC[r]+nx+my+1;
			rowidx[nnz] = row;
			elts[nnz++] = eltsC[r];
		}
	}
	colptr[nx_1] = nnz;
	assert(nnz==nnzA+nnzC);

	ofstream fd("augdump.dat");
	fd << scientific;
	fd.precision(16);
	fd << (nx + my + mz) << endl;
	fd << nx_1 << endl;
	fd << nnzA+nnzC << endl;
	int i;
	for (i = 0; i <= nx_1; i++)
	fd << colptr[i] << " ";
	fd << endl;
	for (i = 0; i < nnz; i++)
	fd << rowidx[i] << " ";
	fd << endl;
	for (i = 0; i < nnz; i++)
	fd << elts[i] << " ";
	fd << endl;
	printf("finished dumping aug\n");


}

Data * sFactory::makeData()
{
#ifdef TIMING
  double t2=MPI_Wtime();
  int mype; MPI_Comm_rank(tree->commWrkrs,&mype);
#endif

  // use the tree to get the data from user and to create OOQP objects

  //TIM;
  StochGenMatrixHandle     A( tree->createA() );
  //REP("A");
  //TIM;
  StochVectorHandle        b( tree->createb() );
  //REP("b");
  //TIM;
  StochGenMatrixHandle     C( tree->createC() );
  //REP("C");
  //TIM;
  StochVectorHandle     clow( tree->createclow()  );
  //REP("clow");
  //TIM;
  StochVectorHandle    iclow( tree->createiclow() );
  //REP("iclow");
  //TIM;
  StochVectorHandle     cupp( tree->createcupp()  );
  //REP("cupp");
  //TIM;
  StochVectorHandle    icupp( tree->createicupp() );
  //REP("icupp");

  //dumpaug(((sTreeImpl*)tree)->nx(), *(A->children[1]->Amat), *(C->children[1]->Amat));

  //TIM;
  StochSymMatrixHandle     Q( tree->createQ() );
  //REP("Q");
  //TIM;
  StochVectorHandle        c( tree->createc() );
  //REP("c");
  //TIM;
  StochVectorHandle     xlow( tree->createxlow()  );
  //REP("xlow");
  //TIM;
  StochVectorHandle    ixlow( tree->createixlow() );
  //REP("ixlow");
  //TIM;
  StochVectorHandle     xupp( tree->createxupp()  );
  //REP("xupp");
  //TIM;
  StochVectorHandle    ixupp( tree->createixupp() );
  //REP("ixupp");

#ifdef TIMING
  MPI_Barrier(tree->commWrkrs);
  t2 = MPI_Wtime() - t2;
  if (mype == 0) {
    cout << "IO second part took " << t2 << " sec\n";
  }
#endif


  data = new sData( tree,
		    c, Q,
		    xlow, ixlow, ixlow->numberOfNonzeros(),
		    xupp, ixupp, ixupp->numberOfNonzeros(),
		    A, b,
		    C, clow, iclow, iclow->numberOfNonzeros(),
		    cupp, icupp, icupp->numberOfNonzeros());

  return data;
}

Variables* sFactory::makeVariables( Data * prob_in )
{
  sData* prob = dynamic_cast<sData*>(prob_in);

  OoqpVectorHandle x      = OoqpVectorHandle( tree->newPrimalVector() );
  OoqpVectorHandle s      = OoqpVectorHandle( tree->newDualZVector() );
  OoqpVectorHandle y      = OoqpVectorHandle( tree->newDualYVector() );
  OoqpVectorHandle z      = OoqpVectorHandle( tree->newDualZVector() );
  OoqpVectorHandle v      = OoqpVectorHandle( tree->newPrimalVector() );
  OoqpVectorHandle gamma  = OoqpVectorHandle( tree->newPrimalVector() );
  OoqpVectorHandle w      = OoqpVectorHandle( tree->newPrimalVector() );
  OoqpVectorHandle phi    = OoqpVectorHandle( tree->newPrimalVector() );
  OoqpVectorHandle t      = OoqpVectorHandle( tree->newDualZVector() );
  OoqpVectorHandle lambda = OoqpVectorHandle( tree->newDualZVector() );
  OoqpVectorHandle u      = OoqpVectorHandle( tree->newDualZVector() );
  OoqpVectorHandle pi     = OoqpVectorHandle( tree->newDualZVector() );

  // OoqpVector * s      = tree->newDualZVector();
  // OoqpVector * y      = tree->newDualYVector();
  // OoqpVector * z      = tree->newDualZVector();
  // OoqpVector * v      = tree->newPrimalVector();
  // OoqpVector * gamma  = tree->newPrimalVector();
  // OoqpVector * w      = tree->newPrimalVector();
  // OoqpVector * phi    = tree->newPrimalVector();
  // OoqpVector * t      = tree->newDualZVector();
  // OoqpVector * lambda = tree->newDualZVector();
  // OoqpVector * u      = tree->newDualZVector();
  // OoqpVector * pi     = tree->newDualZVector();

  sVars* vars = new sVars( tree, x, s, y, z,
			   v, gamma, w, phi,
			   t, lambda, u, pi,
			   prob->ixlow, prob->ixlow->numberOfNonzeros(),
			   prob->ixupp, prob->ixupp->numberOfNonzeros(),
			   prob->iclow, prob->iclow->numberOfNonzeros(),
			   prob->icupp, prob->icupp->numberOfNonzeros());
  registeredVars.push_back(vars);
  return vars;
}


Residuals* sFactory::makeResiduals( Data * prob_in )
{
  sData* prob = dynamic_cast<sData*>(prob_in);
  resid =  new sResiduals(tree,
			  prob->ixlow, prob->ixupp,
			  prob->iclow, prob->icupp);
  return resid;
}



LinearSystem* sFactory::makeLinsys( Data * prob_in )
{
  linsys = newLinsysRoot();

  return linsys;
}


void sFactory::joinRHS( OoqpVector& rhs_in,  OoqpVector& rhs1_in,
			  OoqpVector& rhs2_in, OoqpVector& rhs3_in )
{
  assert(0 && "not implemented here");
}

void
sFactory::separateVars( OoqpVector& x_in, OoqpVector& y_in,
			OoqpVector& z_in, OoqpVector& vars_in )
{
  assert(0 && "not implemented here");
}

void sFactory::iterateStarted()
{
  iterTmMonitor.recIterateTm_start();
  tree->startMonitors();
}


void sFactory::iterateEnded()
{
  tree->stopMonitors();

  if(tree->balanceLoad()) {
    // balance needed
    data->sync();

    for(size_t i=0; i<registeredVars.size(); i++)
      registeredVars[i]->sync();

    resid->sync();

    linsys->sync();

    printf("Should not get here! OMG OMG OMG\n");
  }
  //logging and monitoring
  iterTmMonitor.recIterateTm_stop();
  m_tmTotal += iterTmMonitor.tmIterate;

  if(tree->rankMe==tree->rankZeroW) {
#ifdef TIMING
    extern double g_iterNumber;
    printf("TIME %g SOFAR %g ITER %d\n", iterTmMonitor.tmIterate, m_tmTotal, (int)g_iterNumber);
    //#elseif STOCH_TESTING
    //printf("ITERATION WALLTIME: iter=%g  Total=%g\n", iterTmMonitor.tmIterate, m_tmTotal);
#endif
  }
}

void sFactory::writeProblemToStream(ostream& out, bool printRhs) const{
	int rank;
	MPI_Comm_rank(tree->commWrkrs, &rank);
	int world_size;
	MPI_Comm_size(tree->commWrkrs, &world_size);
	bool iAmDistrib = (world_size>1);

	if (!iAmDistrib) {
		out << "--------- Matrix A --------- " << endl;
		data->A->writeToStreamDense(out);
		if (printRhs) {
			out << "--------- Rhs b --------- " << endl;
			data->bA->writeToStream(out);
		}
		out << "--------- Matrix C --------- " << endl;
		data->C->writeToStreamDense(out);
		if (printRhs) {
			out << "--------- Rhs clow --------- " << endl;
			data->bl->writeToStream(out);
			out << "--------- Rhs cupp --------- " << endl;
			data->bu->writeToStream(out);
		}
	}
	else {	// distributed case
		int token;

		if (rank == 0) {
			out << "--------- Matrix A --------- " << endl;
			token = 2;
		}
		data->A->writeToStreamDense(out);
		if (rank == world_size - 1) {
			MPI_Send(&token, 1, MPI_INT, 0, 1, tree->commWrkrs);
		}

		if (printRhs) {
			if (rank == 0) {
				MPI_Recv(&token, 1, MPI_INT, (world_size - 1), 1, tree->commWrkrs, MPI_STATUS_IGNORE);
				out << "---------- Rhs b ----------- " << endl;
				data->bA->writeToStream(out);
			}
			if (rank == world_size - 1) {
				MPI_Send(&token, 1, MPI_INT, 0, 1, tree->commWrkrs);
			}
		}
		if (rank == 0) {
			MPI_Recv(&token, 1, MPI_INT, (world_size - 1), 1, tree->commWrkrs, MPI_STATUS_IGNORE);
			out << "--------- Matrix C --------- " << endl;
		}
		data->C->writeToStreamDense(out);
		if (rank == world_size - 1) {
			MPI_Send(&token, 1, MPI_INT, 0, 2, tree->commWrkrs);
		}
		if (printRhs) {
			if (rank == 0) {
				MPI_Recv(&token, 1, MPI_INT, (world_size - 1), 1, tree->commWrkrs, MPI_STATUS_IGNORE);
				out << "---------- Rhs clow ----------- " << endl;
				// todo: verify if bl is clow
				data->bl->writeToStream(out);
			}
			if (rank == world_size - 1) {
				MPI_Send(&token, 1, MPI_INT, 0, 1, tree->commWrkrs);
			}
			if (rank == 0) {
				MPI_Recv(&token, 1, MPI_INT, (world_size - 1), 1, tree->commWrkrs, MPI_STATUS_IGNORE);
				out << "---------- Rhs cupp ----------- " << endl;
				// todo: verify if bu is cupp
				data->bu->writeToStream(out);
			}
			if (rank == world_size - 1) {
				MPI_Send(&token, 1, MPI_INT, 0, 1, tree->commWrkrs);
			}
		}

	}


}
