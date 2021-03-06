/* PIPS
   Authors: Cosmin Petra
   See license and copyright information in the documentation */

/* 2015. Modified by Nai-Yuan Chiang for NLP*/


#ifndef STOCHGENMATRIX_H
#define STOCHGENMATRIX_H

#include "DoubleMatrix.h"
#include "SparseGenMatrix.h"
#include <vector>
#include "mpi.h"


class OoqpVector;
class StochVector;
class Variables;

class StochGenMatrix : public GenMatrix {
protected:


// A is the sub matrix corresponding to the linking part, i.e. 2nd stage row, 1st stage var
// B is the sub matrix corresponding to the local part, i.e. 2nd stage row, 2nd stage var


public:
  StochGenMatrix(){}
  /** Constructs a matrix having local A and B blocks having the sizes and number of nz specified by  
      A_m, A_n, A_nnz and B_m, B_n, B_nnz.
      Also sets the global sizes to 'global_m' and 'global_n'. The 'id' parameter is used 
      for output/debug purposes only.
      The matrix that will be created  has no children, just local data.*/
  StochGenMatrix(int id, 
		 long long global_m, long long global_n,
		 int A_m, int A_n, int A_nnz,
		 int B_m, int B_n, int B_nnz,
		 MPI_Comm mpiComm_, int C_m=0, int C_n=0, int C_nnz=0);

  // constructor for combining scenarios
  //StochGenMatrix(const vector<StochGenMatrix*> &blocks); -- not needed; cpetra
  virtual ~StochGenMatrix();

  virtual void AddChild(StochGenMatrix* child);

  std::vector<StochGenMatrix*> children;
  SparseGenMatrix* Amat;
  SparseGenMatrix* Bmat;

  int id;
  long long m,n;
  MPI_Comm mpiComm;
  int iAmDistrib;
 private:
  OoqpVector* workPrimalVec;
  OoqpVector* getWorkPrimalVec(const StochVector& origin);

  virtual void transMult2 ( double beta,   StochVector& y,
		    double alpha,  StochVector& x,
		    OoqpVector& yvecParent);

  virtual void transMult2 ( double beta,   StochVector& y,
		    double alpha,  StochVector& x,
		    OoqpVector& yvecParent, OoqpVector& xCvecParent);

 public:
  virtual void getSize( long long& m, long long& n );
  virtual void getSize( int& m, int& n );

  /** The actual number of structural non-zero elements in this sparse
   *  matrix. This includes so-called "accidental" zeros, elements that
   *  are treated as non-zero even though their value happens to be zero.
   */  
  virtual int numberOfNonZeros();

  virtual int isKindOf( int matType );

  virtual void atPutDense( int row, int col, double * A, int lda,
			   int rowExtent, int colExtent );
  virtual void fromGetDense( int row, int col, double * A, int lda,
			     int rowExtent, int colExtent );
  virtual void ColumnScale( OoqpVector& vec );
  virtual void RowScale( OoqpVector& vec );
  virtual void SymmetricScale( OoqpVector &vec);
  virtual void scalarMult( double num);
  virtual void fromGetSpRow( int row, int col,
			     double A[], int lenA, int jcolA[], int& nnz,
			     int colExtent, int& info );
  virtual void atPutSubmatrix( int destRow, int destCol, DoubleMatrix& M,
			       int srcRow, int srcCol,
			       int rowExtent, int colExtent );
  virtual void atPutSpRow( int col, double A[], int lenA, int jcolA[],
			   int& info );

  virtual void putSparseTriple( int irow[], int len, int jcol[], double A[], 
				int& info );

  virtual void getDiagonal( OoqpVector& vec );
  virtual void setToDiagonal( OoqpVector& vec );

  /** y = beta * y + alpha * this * x */
  virtual void mult ( double beta,  OoqpVector& y,
                      double alpha, OoqpVector& x );

  virtual void mult ( double beta,  OoqpVector& y,
                      double alpha, OoqpVector& x, OoqpVector& yvecParent);

  virtual void transMult ( double beta,   OoqpVector& y,
                           double alpha,  OoqpVector& x );

  virtual double abmaxnorm();

  virtual void writeToStream(std::ostream& out) const;

  /** Make the elements in this matrix symmetric. The elements of interest
   *  must be in the lower triangle, and the upper triangle must be empty.
   *  @param info zero if the operation succeeded. Otherwise, insufficient
   *  space was allocated to symmetrize the matrix.
   */
  virtual void symmetrize( int& info );

  virtual void randomize( double alpha, double beta, double * seed );

  virtual void atPutDiagonal( int idiag, OoqpVector& v );
  virtual void fromGetDiagonal( int idiag, OoqpVector& v );
  void matTransDMultMat(OoqpVector& d, SymMatrix** res);
  void matTransDinvMultMat(OoqpVector& d, SymMatrix** res);

  //copy MtxElts from double values
  virtual void copyMtxFromDouble(int copyLength,double *values){assert( "Has not been yet implemented" && 0 );};

  virtual void setAdditiveDiagonal(OoqpVector& v );


};


/**
 * Dummy Class 
 */

class StochGenDummyMatrix : public StochGenMatrix {
protected:

public:

  StochGenDummyMatrix(int id)
    : StochGenMatrix(id, 0, 0, 0, 0, 0, 0, 0, 0, MPI_COMM_NULL) {};

  virtual ~StochGenDummyMatrix(){};

  virtual void AddChild(StochGenMatrix* child){};

 public:
  virtual void getSize( int& m, int& n ){m=0; n=0;}
  virtual void getSize( long long& m, long long& n ){m=0; n=0;}

  /** The actual number of structural non-zero elements in this sparse
   *  matrix. This includes so-called "accidental" zeros, elements that
   *  are treated as non-zero even though their value happens to be zero.
   */  
  virtual int numberOfNonZeros(){return 0;}

  virtual int isKindOf( int matType );

  virtual void atPutDense( int row, int col, double * A, int lda,
			   int rowExtent, int colExtent ){};
  virtual void fromGetDense( int row, int col, double * A, int lda,
			     int rowExtent, int colExtent ){};
  virtual void ColumnScale( OoqpVector& vec ){};
  virtual void RowScale( OoqpVector& vec ){};
  virtual void SymmetricScale( OoqpVector &vec){};
  virtual void scalarMult( double num){};
  virtual void fromGetSpRow( int row, int col,
			     double A[], int lenA, int jcolA[], int& nnz,
			     int colExtent, int& info ){};

  virtual void atPutSubmatrix( int destRow, int destCol, DoubleMatrix& M,
			       int srcRow, int srcCol,
			       int rowExtent, int colExtent ){};

  virtual void atPutSpRow( int col, double A[], int lenA, int jcolA[],
			   int& info ){};

  virtual void putSparseTriple( int irow[], int len, int jcol[], double A[], 
				int& info ){};

  virtual void getDiagonal( OoqpVector& vec ){};
  virtual void setToDiagonal( OoqpVector& vec ){};

  /** y = beta * y + alpha * this * x */
  virtual void mult ( double beta,  OoqpVector& y,
                      double alpha, OoqpVector& x ){};

  virtual void mult ( double beta,  OoqpVector& y,
                      double alpha, OoqpVector& x, OoqpVector& yvecParent){};

  virtual void transMult ( double beta,   OoqpVector& y,
                           double alpha,  OoqpVector& x ){};
  virtual void transMult2 ( double beta,   StochVector& y,
		    double alpha,  StochVector& x,
		    OoqpVector& yvecParent){};

  virtual void transMult2 ( double beta,   StochVector& y,
		    double alpha,  StochVector& x,
		    OoqpVector& yvecParent, OoqpVector& xCvecParent){};

  virtual double abmaxnorm(){return 0.0;};

  virtual void writeToStream(std::ostream& out) const{};

  /** Make the elements in this matrix symmetric. The elements of interest
   *  must be in the lower triangle, and the upper triangle must be empty.
   *  @param info zero if the operation succeeded. Otherwise, insufficient
   *  space was allocated to symmetrize the matrix.
   */
  virtual void symmetrize( int& info ){};

  virtual void randomize( double alpha, double beta, double * seed ){};

  virtual void atPutDiagonal( int idiag, OoqpVector& v ){};
  virtual void fromGetDiagonal( int idiag, OoqpVector& v ){};

  //copy MtxElts from double values
  virtual void copyMtxFromDouble(int copyLength,double *values){assert( "Has not been yet implemented" && 0 );};

  virtual void setAdditiveDiagonal(OoqpVector& v ){};

};


typedef SmartPointer<StochGenMatrix> StochGenMatrixHandle;

#endif
