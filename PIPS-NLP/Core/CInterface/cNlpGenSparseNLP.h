/* OOQP                                                               *
 * Authors: E. Michael Gertz, Stephen J. Wright                       *
 * (C) 2001 University of Chicago. See Copyright Notification in OOQP */
/* 2015. Modified by Nai-Yuan Chiang for NLP */

#ifndef CNLPGENSPARSE_NLP
#define CNLPGENSPARSE_NLP

#include "cNlpGen.h"

extern "C" {
void newTriple( int ** irow, int nnz, int ** jcol, double ** M, int * ierr );
void freeTriple( int ** irow, int ** jcol, double ** M );

void makehb( int irow[], int nnz, int krow[], int m, int * ierr );

void newNlpGenSparse( double ** c,      int nx,
		     int    ** irowQ,  int nnzQ,  int  ** jcolQ,  double ** dQ,
		     double ** xlow,              char ** ixlow,
		     double ** xupp,              char ** ixupp,
		     int    ** irowA,  int nnzA,  int  ** jcolA,  double ** dA,
		     double ** b,      int my,
		     int    ** irowC,  int nnzC,  int  ** jcolC,  double ** dC,
		     double ** clow,   int mz,    char ** iclow,
		     double ** cupp,              char ** icupp,
		     int    *  ierr );

void freeNlpGenSparse( double ** c,      
		      int    ** irowQ,  int  ** jcolQ,  double ** dQ,
		      double ** xlow,   char ** ixlow,
		      double ** xupp,   char ** ixupp,
		      int    ** irowA,  int  ** jcolA,  double ** dA,
		      double ** b,
		      int    ** irowC,  int  ** jcolC,  double ** dC,
		      double ** clow,   char ** iclow,
		      double ** cupp,   char ** icupp );

void NlpGenHbNLPSetup( double    c[],  int  nx,
			  int   krowQ[],  int  jcolQ[],  double dQ[],
			  double xlow[],  char ixlow[], 
			  double xupp[],  char ixupp[],
			  int   krowA[],  int  my,       int  jcolA[],
			  double   dA[],
			  double   bA[],
			  int   krowC[],  int  mz,       int  jcolC[],
			  double dC[],
			  double clow[],  char iclow[],
			  double cupp[],  char icupp[],
			  NlpGenContext * ctx,
			  int * ierr );

void Nlpsolvehb( double    c[],  int  nx,
		int   krowQ[],  int  jcolQ[],  double dQ[],
	        double xlow[],  char ixlow[], 
		double xupp[],  char ixupp[],
		int   krowA[],  int  my,       int  jcolA[],  double dA[],
		double   bA[],
		int   krowC[],  int  mz,       int  jcolC[],  double dC[],
		double clow[],  char iclow[],
		double cupp[],  char icupp[],
		double    x[],  double gamma[],     double phi[],
		double    y[],
		double    z[],  double lambda[],  double pi[],
		double    *objectiveValue,
		int print_level,
		int * status_code );

void Nlpsolvesp( double    c[],  int  nx,
		int   irowQ[],  int  nnzQ,     int  jcolQ[],  double dQ[],
		double xlow[],  char ixlow[], 
		double xupp[],  char ixupp[],
		int   irowA[],  int  nnzA,     int  jcolA[],  double dA[],
		double   bA[],  int  my,
		int   irowC[],  int  nnzC,     int  jcolC[],  double dC[],
		double clow[],  int  mz,       char iclow[],
		double cupp[],  char icupp[],
		double    x[],  double gamma[],     double phi[],
		double    y[],
		double    z[],  double lambda[],  double pi[],
		double   *objectiveValue,
		int print_level,
		int * status_code );

};


#endif



