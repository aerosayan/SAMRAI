/*************************************************************************
 *
 * This file is part of the SAMRAI distribution.  For full copyright
 * information, see COPYRIGHT and COPYING.LESSER.
 *
 * Copyright:     (c) 1997-2011 Lawrence Livermore National Security, LLC
 * Description:   Implicit time integration manager class.
 *
 ************************************************************************/

#ifndef included_algs_ImplicitIntegrator
#define included_algs_ImplicitIntegrator

#include "SAMRAI/SAMRAI_config.h"

#include "SAMRAI/tbox/Database.h"
#include "SAMRAI/tbox/Serializable.h"
#include "SAMRAI/algs/ImplicitEquationStrategy.h"
#include "SAMRAI/solv/NonlinearSolverStrategy.h"
#include "SAMRAI/solv/SAMRAIVectorReal.h"
#include "SAMRAI/hier/PatchHierarchy.h"
#include "SAMRAI/tbox/InputDatabase.h"
#include "SAMRAI/tbox/Serializable.h"

#include <boost/shared_ptr.hpp>
#include <string>

namespace SAMRAI {
namespace algs {

/*!
 * @brief Manage implicit time integration over an AMR patch hierarchy.
 *
 * Class ImplicitIntegrator manages implicit time integration
 * over an AMR patch hierarchy.  It maintains references
 * to ImplicitEquationStrategy and solv::NonlinearSolverStrategy
 * objects, which provide operations describing the implicit equations
 * and solving the problem at each time step, respectively.
 * When a time step is performed, data on all levels is integrated through
 * the same time increment.
 *
 * Initialization of an ImplicitIntegrator object is performed via a
 * combination of default parameters and values read from input.  Data
 * read from input is summarized as follows:
 *
 * Required input keys and data types:
 * @param initial_time double value for the initial simulation time.
 * @param final_time double value for the final simulation time.
 * @param max_integrator_steps integer value for the maximum number
 * of timesteps allowed.
 *
 * All input data items described above, except for initial_time,
 * may be overwritten by new input values when continuing from restart.
 *
 * A sample input file entry might look like:
 *
 * @code
 * initial_time = 0.0
 * final_time   = 1.0
 * max_integrator_steps = 100
 * @endcode
 *
 * @see algs::ImplicitEquationStrategy
 * @see solv::NonlinearSolverStrategy
 */
class ImplicitIntegrator:
   public tbox::Serializable
{
public:
   /**
    * The constructor for ImplicitIntegrator initializes the
    * default state of the integrator.  The integrator is configured with
    * the concrete strategy objects in the argument list that provide
    * operations related to the nonlinear solver and implicit equations
    * to solve.  Data members are initialized from the input and restart
    * databases.
    *
    * Note that no vectors are created in the constructor.  Vectors are
    * created and the nonlinear solver is initialized in the initialize()
    * member function.
    *
    * If assertion checking is turned on, an unrecoverable assertion will
    * result if any of the database, strategy, or hierarchy pointers is
    * null.  Assertions may also be thrown if any checks for consistency
    * among input parameters fail.
    */
   ImplicitIntegrator(
      const std::string& object_name,
      const boost::shared_ptr<tbox::Database>& input_db,
      ImplicitEquationStrategy * implicit_equations,
      solv::NonlinearSolverStrategy * nonlinear_solver,
      const boost::shared_ptr<hier::PatchHierarchy>& hierarchy);

   /**
    * Empty destructor for ImplicitIntegrator
    */
   virtual ~ImplicitIntegrator();

   /**
    * Initialize state of time integrator.  This includes creating
    * solution vector and initializing solver components.
    */
   void
   initialize();

   /*!
    * @brief Integrate entire patch hierarchy through the
    * specified time increment.
    *
    * Integrate entire patch hierarchy through the specified time
    * increment.  The time advance assumes the use of a nonlinear
    * solver to implicitly integrate the discrete equations.  The integer
    * return value is the return code generated by the particular solver
    * package in use.  It is the user's responsibility to interpret this
    * code in a manner consistent with the solver she is using.
    *
    * The boolean first_step argument is true when this is the very
    * first call to the advance function or if the call occurs immediately
    * after the hierarchy has changed due to regridding.  Otherwise it is
    * false.  Note that, when the argument is true, the use of extrapolation
    * to construct the initial guess for the advanced solution may not be
    * possible.
    *
    *
    * @param dt Time step size
    * @param first_step Whether this is the first step after grid change
    *
    * @return value is the return code generated by the particular solver
    * package in use
    */
   int
   advanceSolution(
      const double dt,
      const bool first_step);

   /**
    * Return time increment for next solution advance.  Timestep selection
    * is generally based on whether the nonlinear solution iteration
    * converged and, if so, whether the solution meets some user-defined
    * criteria.  This routine assumes that, before it is called, the
    * routine checkNewSolution() was called.  The boolean argument is the
    * return value from that call.  The integer argument is the return code
    * generated by the nonlinear solver package that computed the solution.
    */
   double
   getNextDt(
      const bool good_solution,
      const int solver_retcode);

   /**
    * Check time advanced solution to determine whether it is acceptable.
    * Return true if the solution is acceptable; return false otherwise.
    * The integer argument is the return code generated by the call to the
    * nonlinear solver "solve" routine.   The meaning of this value depends
    * on the particular nonlinear solver in use and must be intepreted
    * properly by the user-supplied solution checking routine.
    */
   bool
   checkNewSolution(
      const int solver_retcode) const;

   /**
    * Update solution (e.g., reset pointers for solution data, update
    * dependent variables, etc.) after time advance.  It is assumed that
    * when this routine is invoked, an acceptable new solution has been
    * computed.  The double return value is the simulation time corresponding
    * to the advanced solution.
    */
   double
   updateSolution();

   /**
    * Return initial integration time.
    */
   double
   getInitialTime() const;

   /**
    * Return final integration time.
    */
   double
   getFinalTime() const;

   /**
    * Return current integration time.
    */
   double
   getCurrentTime() const;

   /**
    * Return current timestep.
    */
   double
   getCurrentDt() const;

   /**
    * Return current integration step number.
    */
   int
   getIntegratorStep() const;

   /**
    * Return maximum number of integration steps.
    */
   int
   getMaxIntegratorSteps() const;

   /**
    * Return true if the number of integration steps performed by the
    * integrator has not reached the specified maximum; return false
    * otherwise.
    */
   bool
   stepsRemaining() const;

   /**
    * Print out all members of integrator instance to given output stream.
    */
   virtual void
   printClassData(
      std::ostream& os) const;

   /**
    * Write out state of object to given database.
    *
    * When assertion checking is active, the database pointer must be non-null.
    */
   void
   putToDatabase(
      const boost::shared_ptr<tbox::Database>& db) const;

   /**
    * Returns the object name.
    */
   const std::string&
   getObjectName() const;

private:
   /*
    * Static integer constant describing class's version number.
    */
   static const int ALGS_IMPLICIT_INTEGRATOR_VERSION;

   /*
    * Read input data from specified database and initialize class members.
    * If run is from restart, a subset of the restart values may be replaced
    * with those read from input.
    *
    * When assertion checking is active, the database pointer must be non-null.
    */
   void
   getFromInput(
      const boost::shared_ptr<tbox::Database>& db,
      bool is_from_restart);

   /*
    * Read object state from restart database and initialize class members.
    * Check that class and restart version numbers are the same.
    */
   void
   getFromRestart();

   /*
    * String used to identify specific class instantiation.
    */
   std::string d_object_name;

   /*
    * Pointers to implicit equation and solver strategy objects and patch
    * hierarchy.  The strategies provide nonlinear equation and solver
    * routines for treating the nonlinear problem on the hierarchy.
    */
   ImplicitEquationStrategy * d_implicit_equations;
   solv::NonlinearSolverStrategy * d_nonlinear_solver;
   boost::shared_ptr<hier::PatchHierarchy> d_patch_hierarchy;

   int d_finest_level;

   /*
    * Solution vector advanced during the time integration process.
    */
   boost::shared_ptr<solv::SAMRAIVectorReal<double> > d_solution_vector;

   /*
    * Data members representing integrator times, time increments,
    * and step count information.
    */
   double d_initial_time;
   double d_final_time;
   double d_current_time;

   double d_current_dt;
   double d_old_dt;

   int d_integrator_step;
   int d_max_integrator_steps;

   // The following are not implemented:
   ImplicitIntegrator(
      const ImplicitIntegrator&);
   void
   operator = (
      const ImplicitIntegrator&);

};

}
}
#ifdef SAMRAI_INLINE
#include "SAMRAI/algs/ImplicitIntegrator.I"
#endif
#endif
