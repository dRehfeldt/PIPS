/* OOQP                                                               *
 * Authors: E. Michael Gertz, Stephen J. Wright                       *
 * (C) 2001 University of Chicago. See Copyright Notification in OOQP */

#include "GondzioSolver.h"
#include "Variables.h"
#include "Residuals.h"
#include "LinearSystem.h"
#include "Status.h"
#include "Data.h"
#include "ProblemFormulation.h"
#include "QpGenOptions.h"

#include <cstring>
#include <iostream>
#include <fstream>
using namespace std;

#include <cstdio>
#include <cassert>
#include <cmath>

// gmu is needed by MA57!
double gmu;
// double grnorm;
extern int gOoqpPrintLevel;

GondzioSolver::GondzioSolver( ProblemFormulation * of, Data * prob, const Scaler* scaler ) :
      Solver(scaler)
{
  factory              = of;
  step                 = factory->makeVariables( prob );
  corrector_step       = factory->makeVariables( prob );
  corrector_resid      = factory->makeResiduals( prob );

  maxit = 300;
  printlevel = 0; // has no meaning right now 
  tsig = 3.0;     // the usual value for the centering exponent (tau)

  NumberGondzioCorrections = 0;

  maximum_correctors = qpgen_options::getIntParameter("GONDZIO_MAX_CORRECTORS");

  // the two StepFactor constants set targets for increase in step
  // length for each corrector
  StepFactor0 = 0.08; 
  StepFactor1 = 1.08; 

  // accept the enhanced step if it produces a small improvement in
  // the step length
  AcceptTol = 0.01;

  //define the Gondzio correction box 
  beta_min = 0.1;  
  beta_max = 10.0;

  // allocate space to track the sequence of complementarity gaps,
  // residual norms, and merit functions.
  mu_history = new double[maxit];
  rnorm_history = new double[maxit];
  phi_history = new double[maxit];
  phi_min_history = new double[maxit];

 // Use the defaultStatus method
  status   = 0; 
}

int GondzioSolver::solve(Data *prob, Variables *iterate, Residuals * resid )
{
  int done;
  double mu, muaff;
  int StopCorrections;
  double alpha_target, alpha_enhanced, rmin, rmax;
  int status_code;
  double alpha = 1, sigma = 1;

  gmu = 1000;
  //  grnorm = 1000;
  setDnorm(*prob);
  // initialization of (x,y,z) and factorization routine.
  sys = factory->makeLinsys( prob );
  this->start( factory, iterate, prob, resid, step );

  iter = 0; 
  NumberGondzioCorrections = 0; 
  done = 0;
  mu = iterate->mu();
  gmu = mu;

  do
    {
      iter ++;
      // evaluate residuals and update algorithm status:
      resid->calcresids(prob, iterate);

      //  termination test:
      status_code = this->doStatus( prob, iterate, resid, iter, mu, 0 );
      if( status_code != NOT_FINISHED ) break;
      if( gOoqpPrintLevel >= 10  ) {
		this->doMonitor( prob, iterate, resid, alpha, sigma,
						 iter, mu, status_code, 0 );
      }
      // *** Predictor step ***

      resid->set_r3_xz_alpha( iterate, 0.0 );

      sys->factor(prob, iterate);
      sys->solve(prob, iterate, resid, step);
      step->negate();

      alpha = iterate->stepbound(step);

      // calculate centering parameter 
      muaff = iterate->mustep(step, alpha);
      sigma = pow(muaff/mu, tsig);
      
      if( gOoqpPrintLevel >= 10 ) {
		this->doMonitor( prob, iterate, resid,
						 alpha, sigma, iter, mu, status_code, 2 );
      }

      // *** Corrector step ***
      // form right hand side of linear system:
      resid->add_r3_xz_alpha( step, -sigma*mu );

      sys->solve(prob, iterate, resid, step);
      step->negate();

      // calculate distance to boundary along the Mehrotra
      // predictor-corrector step:
      alpha = iterate->stepbound(step);

      // prepare for Gondzio corrector loop: zero out the
      // corrector_resid structure:
      corrector_resid->clear_r1r2();

      // calculate the target box:
      rmin = sigma * mu * beta_min;
      rmax = sigma * mu * beta_max;

      StopCorrections = 0; NumberGondzioCorrections = 0;

      // enter the Gondzio correction loop:
      while (NumberGondzioCorrections < maximum_correctors 
	     && alpha < 1.0
	     && !StopCorrections) {

	// copy current variables into corrector_step
	corrector_step->copy(iterate);

	// calculate target steplength
	alpha_target = StepFactor1 * alpha + StepFactor0;
	if(alpha_target > 1.0) alpha_target = 1.0;

	// add a step of this length to corrector_step
	corrector_step->saxpy(step, alpha_target);

	// place XZ into the r3 component of corrector_resids
	corrector_resid->set_r3_xz_alpha(corrector_step, 0.0);

	// do the projection operation
	corrector_resid->project_r3(rmin, rmax);

	// solve for corrector direction
	sys->solve(prob,iterate,corrector_resid,corrector_step);

	// add the current step to corrector_step, and calculate the
	// step to boundary along the resulting direction
	corrector_step->saxpy(step, 1.0);
	alpha_enhanced = iterate->stepbound(corrector_step);

	// if the enhanced step length is actually 1, make it official
	// and stop correcting
	if(alpha_enhanced == 1.0) {
	  step->copy(corrector_step);
	  alpha = alpha_enhanced;
	  NumberGondzioCorrections++;
	  StopCorrections = 1;
	} else if(alpha_enhanced >= (1.0+AcceptTol)*alpha) {
	  // if enhanced step length is significantly better than the
	  // current alpha, make the enhanced step official, but maybe
	  // keep correcting
	  step->copy(corrector_step);
	  alpha = alpha_enhanced;
	  NumberGondzioCorrections++;
	  StopCorrections = 0;
	} else {
	  // otherwise quit the correction loop
	  StopCorrections = 1;
	}
      }

      // We've finally decided on a step direction, now calculate the
      // length using Mehrotra's heuristic.x
      alpha = finalStepLength(iterate, step);

      // alternatively, just use a crude step scaling factor.
      // alpha = 0.995 * iterate->stepbound( step );

      // actually take the step (at last!) and calculate the new mu

      iterate->saxpy(step, alpha);
      mu = iterate->mu();
      gmu = mu;


    } while(!done);
  
  resid->calcresids(prob, iterate);
  if( gOoqpPrintLevel >= 10 ) {
    this->doMonitor(prob, iterate, resid, alpha, sigma,
					iter, mu, status_code, 1 );
  }

  // print the results, if you really want to..
  // iterate->print();

  return status_code;
}


void GondzioSolver::defaultMonitor( const Data * /* data */, const Variables * /* vars */,
									const Residuals * resids,
									double alpha, double sigma,
									int i, double mu, 
									int status_code,
									int level ) const
{
   switch( level ) {
      case 0 : case 1:
      {

         const Residuals* resids_unscaled = resids;
         if( scaler )
            resids_unscaled = scaler->getResidualsUnscaled(*resids);

         const double gap = resids_unscaled->dualityGap();
         const double rnorm = resids_unscaled->residualNorm();

         if( scaler )
            delete resids_unscaled;

         std::cout << std::endl << "Duality Gap: " << gap << std::endl;

         if( i > 1 )
         {
            std::cout << " Number of Corrections = " <<  NumberGondzioCorrections
                  << " alpha = " << alpha << std::endl;
         }
         std::cout << " *** Iteration " << i << " *** " << std::endl;
         std::cout << " mu = " << mu << " relative residual norm = "
               << rnorm / dnorm_orig << std::endl;

         if( level == 1)
         {
            // Termination has been detected by the status check; print
            // appropriate message
            if(status_code == SUCCESSFUL_TERMINATION)
               std::cout << std::endl << " *** SUCCESSFUL TERMINATION ***" << std::endl;
            else if (status_code == MAX_ITS_EXCEEDED)
               std::cout << std::endl << " *** MAXIMUM ITERATIONS REACHED *** " << std::endl;
            else if (status_code == INFEASIBLE)
               std::cout << std::endl << " *** TERMINATION: PROBABLY INFEASIBLE *** " << std::endl;
            else if (status_code == UNKNOWN)
               std::cout << std::endl << " *** TERMINATION: STATUS UNKNOWN *** " << std::endl;
         }
  } break;
  case 2:
     std::cout << " *** sigma = " << sigma << std::endl;
     break;
  }
}


GondzioSolver::~GondzioSolver()
{
  delete corrector_resid;
  delete corrector_step;
  delete step;
  delete sys;

  delete [] mu_history;
  delete [] rnorm_history;
  delete [] phi_history;
  delete [] phi_min_history;
}

