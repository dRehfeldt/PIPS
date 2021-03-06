/* PIPS
   Authors: Cosmin Petra
   See license and copyright information in the documentation */

#include "sFactoryAug.h"

#include "sData.h"

#include "StochTree.h"
#include "StochInputTree.h"

#include "sLinsysRootAug.h"

sFactoryAug::sFactoryAug( StochInputTree* inputTree, MPI_Comm comm)
  : sFactory(inputTree, comm)
{ };

sFactoryAug::sFactoryAug( stochasticInput& in, MPI_Comm comm)
  : sFactory(in,comm)
{ };

sFactoryAug::sFactoryAug( int nx_, int my_, int mz_, int nnzQ_, int nnzA_, int nnzC_ )
  : sFactory(nx_, my_, mz_, nnzQ_, nnzA_, nnzC_)
{ };

sFactoryAug::sFactoryAug()
{ };

sFactoryAug::~sFactoryAug()
{ };


sLinsysRoot* sFactoryAug::newLinsysRoot()
{
  return new sLinsysRootAug(this, data);
}

sLinsysRoot* 
sFactoryAug::newLinsysRoot(sData* prob,
			   OoqpVector* dd,OoqpVector* dq,
			   OoqpVector* nomegaInv, OoqpVector* rhs)
{
  return new sLinsysRootAug(this, prob, dd, dq, nomegaInv, rhs);
}
