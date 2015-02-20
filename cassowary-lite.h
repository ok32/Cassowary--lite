#ifndef CCASSOWARY_H
#define CCASSOWARY_H

extern "C" {



  // # Operations on ClStrengths:
  // A primer to a strength's probably apropos as well, since the name
  // suggests quite a few policies, you could implement that involve
  // strengths - they allow for both *explicitly* contradictory
  // constraints, and suggestions of overlapping sets to be gracefully
  // to be gracefully resolved by de-emphasizing weaker constraints.
  // *Note* the requirement that an alternative exist: Cassowary does
  // not project into the dark.
  class ClStrength;
  typedef ClStrength *cStrength;
  
  cStrength cStrength_required();
  cStrength cStrength_strong();
  cStrength cStrength_medium();
  cStrength cStrength_weak();


  
  // # Operations on ClVariables:
  class ClVariable;
  typedef ClVariable *cVariable;
  
  cVariable cVariable_new(const char* name, double initialValue);
  void cVariable_delete(cVariable var);

  const char* cVariable_getName(cVariable var);
  double cVariable_getValue(cVariable var);
}



// # Operations on ClLinearExpressions.

// A brief break from the extern "C" context in order to
// forward-declare a template - why one can't declare a template with
// C linkage should be obvious: the compiled artefacts - procedures -
// *must* be mangled by default.
template <class T> class ClGenericLinearExpression;
typedef ClGenericLinearExpression<double> ClLinearExpression;

extern "C" {
  typedef ClLinearExpression *cLinearExpression;

  typedef int cOperation;
  cOperation cOperation_plus();
  cOperation cOperation_minus();
  cOperation cOperation_times();
  cOperation cOperation_divide();
  
  cLinearExpression cLinearExpression_newVar(cVariable var, double coefficient, double additiveConstant);
  cLinearExpression cLinearExpression_newN(double n);
  cLinearExpression cLinearExpression_newExpWithExp(cLinearExpression leftExp, cOperation op, cLinearExpression rightExp);
  cLinearExpression cLinearExpression_newExpWithN(cLinearExpression exp, cOperation op, double n);
  void cLinearExpression_delete(cLinearExpression exp);


  
  // # Operations on ClLinearInequalities - subsuming ClLinearEquations
  // as well:
  
  class ClLinearConstraint;
  typedef ClLinearConstraint *cLinearRelation;

  typedef int cRelation; // ISO C++ requires backwards convertibility;
  cRelation cRelation_equals();
  cRelation cRelation_ltEquals();
  cRelation cRelation_gtEquals();
  // You'll notice that not-equals and the strict inequalities are
  // missing - I paraphrase the original authors: the former cannot be
  // meaningfully expressed in a constraint solver that relaxes
  // constraints to solve the unsolvable; for the latter, though:
  /* "The issue is that if strict inequalities were allowed, it would be easy to construct
     constraint hierarchies such that there were no solutions to the hierarchy, even though the
     required constraints could be satisfied. For example, consider the constraint hierarchy
        x > 0, required;
        x = 0, weak
     for any candidate solution x -> \epsilon, the solution x ->
     (\epsilon)/2 ̄ more nearly satisfies the weak constraint—and so
     there is no solution to the hierarchy."
     - The Cassowary Linear Arithmetic Constraint Solving Algorithm
  */
  // - which is to say that the problem lies in charting the edges of a
  // closed interval (when done iteratively, an infinite process)
  // , which non-strict inequalities cannot construct...

  cLinearRelation cLinearRelation_newExpToZero(cLinearExpression exp, cRelation op, cStrength strength);
  cLinearRelation cLinearRelation_newExpToVar(cLinearExpression exp, cRelation op, cVariable var, cStrength strength);
  cLinearRelation cLinearRelation_newExpToExp(cLinearExpression leftExp, cRelation op, cLinearExpression rightExp, cStrength strength);
  void cLinearRelation_delete(cLinearRelation eqn);

  bool cLinearRelation_isSatisfied(cLinearRelation eqn);



  // # Operations on ClEditOrStayConstraints - the upshot, remember,
  // being that the variable involved *must stay the same*: whether or
  // not to reassure the user that their edits are being responded to,
  // or whether an invariant inherent to the application.
  class ClEditConstraint; // ClEditOrStayConstraint itself is abstract
                          // - for lack of a print-self
                          // implementation. We're not about to expose
                          // printer functionality, though, so on we go...
  typedef ClEditConstraint *cEditOrStayConstraint;

  cEditOrStayConstraint cEditOrStayConstraint_new(cStrength strength, cVariable var);
  void cEditOrStayConstraint_delete(cEditOrStayConstraint cnst);
 

  
  // # Operations on ClSimplexSolvers:
  class ClSimplexSolver;
  typedef ClSimplexSolver *cSimplexSolver;

  cSimplexSolver cSimplexSolver_new();
  void cSimplexSolver_delete(cSimplexSolver solver);

  bool cSimplexSolver_addLinearRelationAndResolve(cSimplexSolver solver, cLinearRelation eqn);
  // Fails when required constraints are set up in contradiction to
  // each other.
  void cSimplexSolver_removeLinearRelation(cSimplexSolver solver, cLinearRelation eqn);


  bool cSimplexSolver_addEditOrStayConstraintAndResolve(cSimplexSolver solver, cEditOrStayConstraint cst);
  // Fails when the constrained variable isn't involved elsewhere; a
  // potential deal-breaker, if decoupled client components are adding
  // constraints separately -
  // - see the failing cases above as well, though.
  void cSimplexSolver_removeEditOrStayConstraint(cSimplexSolver solver, cEditOrStayConstraint est);

  bool cSimplexSolver_addStayVar(cSimplexSolver solver, cVariable var);

  void cSimplexSolver_debug_printState(cSimplexSolver solver);

  // # And, finally, a mutable-state annex that allows variables to be
  // # updated - and so to let the Solver then re-update as required:
  bool cSimplexSolver_editVariableInEditConstraint(cSimplexSolver solver, cVariable var, double value);
  // Returns false when a variable being edited isn't within an edit
  // constraint.  
  bool cSimplexSolver_resolve(cSimplexSolver solver);
  // No failure case here - if we've edited ourselves into a bind,
  // alter the edit variable.
  
  // This function wraps a cycle of all of the four above into one:
  bool cSimplexSolver_updateVariableInEditConstraint(cSimplexSolver solver, cVariable var, double value);
}

#endif

