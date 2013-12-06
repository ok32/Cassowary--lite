#include "cassowary-lite.h"
#include <assert.h>
#include <string.h>
#include <stdio.h>

// Various little didactic twists and turns into the behavior of
// Cassowary - we begin with the mechanics of variables, and slowly
// build up our working set until entire systems of constraints are
// being pushed around and solved (or rejected, at that.)

// Leaks memory like hell as well, too. So don't you worry a bit about that.

int main()
{
  // Tests on cVariables - the only interesting operation with
  // properties constrained, though, are getters on state:
  cVariable foo = cVariable_new("foo", 23.0);
  assert(0 == strncmp(cVariable_getName(foo), "foo", 3));
  assert(23.0 == cVariable_getValue(foo));

  // cLinearExpression, at the very least, must begin to invoke quite
  // a bit as we try to define what operations to create compound
  // linear expressions should pass without incident:
  cLinearExpression exp = cLinearExpression_newVar(foo, 1, 0);
  cLinearExpression twice_exp = cLinearExpression_newExpWithExp(exp, cOperation_plus(), exp);
  // n * (n + n)  is most certainly not linear, of course:
  cLinearExpression invalid_exp = cLinearExpression_newExpWithExp(twice_exp, cOperation_times(), exp);
  assert(NULL == invalid_exp);
  // But what about (n + n)/n?
  cLinearExpression very_valid_exp = cLinearExpression_newExpWithExp(twice_exp, cOperation_divide(), exp);
  assert(NULL == very_valid_exp); // KNOWN BUG:
                                 // very_valid_exp isn't sarcasm; it's
                                 // in fact representative of a known
                                 // bug in Cassowary: attempts to
                                 // combine expressions that share
                                 // variables will get you fined an
                                 // exception for creating a nonlinear
                                 // expression; even if some
                                 // reduction'll leave you with a
                                 // non-linear one.
                                 // That it is impossible to
                                 // efficiently transform an equation
                                 // into a format with desired
                                 // properties - or check whether it's
                                 // otherwise possible - is a fact no
                                 // doubt quite a few people at WRI
                                 // will stare at you with hurt eyes
                                 // over.
                                 // An exercise to the reader: is it
                                 // possible to create a typing system
                                 // such that nonlinear expressions
                                 // like these do not pop up? I'd
                                 // begin by exploiting an equivalency
                                 // between typing and computing...
                                 // And yet another exercise: what
                                 // equivalencies exist between
                                 // *checking* whether an operation is
                                 // possible, and actually *doing it*?
  // Here's a more obvious example:
  cLinearExpression base =  cLinearExpression_newVar(foo, 1, 0);
  cLinearExpression failure = cLinearExpression_newExpWithExp(base, cOperation_divide(), base);
  assert(NULL == failure);

  cLinearExpression another_exp = cLinearExpression_newN(3);

  // And now - onto cLinearRelations. The only time they expose
  // behavior with properties dependent on input  though, without 
  // requiring we invoke something larger,
  // is with cLinearEquation_isSatisfied():
  cVariable three_variable = cVariable_new("three", 3.0);
  cVariable  not_three_variable = cVariable_new("three", 3.14159265);
  cLinearExpression not_three_variable_expression = cLinearExpression_newVar(not_three_variable, 1, 0);
  cLinearExpression three_expression = cLinearExpression_newN(3.0);

  // Do the initial values of not_three_variable = three_variable
  // satisfy our equation?
  cLinearRelation aRelation = cLinearRelation_newExpToVar(not_three_variable_expression, cRelation_equals(), three_variable, cStrength_required());
  assert(0 == cLinearRelation_isSatisfied(aRelation));
  // not_three_variable > three_variable should, though:
  aRelation = cLinearRelation_newExpToVar(not_three_variable_expression, cRelation_gtEquals(), three_variable, cStrength_required());
  assert(1 == cLinearRelation_isSatisfied(aRelation));

  
  // Sadly enough, ClEditConstraints don't provide  much of a handhold
  // to experiment with. 
  // Onwards, then, to the Big Kahuna - cSimplexSolver:

  // Solve a = 4 [required], where a is initially 0 (and so should end up as 4);
  cVariable a_var = cVariable_new("a", 0);
  cLinearExpression four_exp = cLinearExpression_newN(4);
  cLinearRelation a_equals_four = cLinearRelation_newExpToVar(four_exp, cRelation_gtEquals(), a_var, cStrength_required());
  cSimplexSolver solver = cSimplexSolver_new();
  cSimplexSolver_addLinearRelationAndResolve(solver, a_equals_four);
  assert(4 == cVariable_getValue(a_var));

  // Solve 3 = 4 [required], where attempts at solving generate an error:
  cLinearExpression three_exp = cLinearExpression_newN(3);
  cLinearRelation three_equals_four = cLinearRelation_newExpToExp(three_exp, cRelation_equals(), four_exp, cStrength_required());
  solver = cSimplexSolver_new();
  assert(0 == cSimplexSolver_addLinearRelationAndResolve(solver, three_equals_four));


  // Solve 3 = 4 [weak], where attempts at solving *still* generate an
  // error - this should explain what Strengths in fact do: they help
  // *fall back*, but do not ignore entirely at will:
  cLinearRelation three_equals_four_weak = cLinearRelation_newExpToExp(three_exp, cRelation_equals(), four_exp, cStrength_weak());
  solver = cSimplexSolver_new();
  assert(0 == cSimplexSolver_addLinearRelationAndResolve(solver, three_equals_four));

  // Here's one constraint falling back onto another in action:
  // a >= 4 [weak], and a <= 3 [required]; 
  cLinearRelation a_gte_four = cLinearRelation_newExpToVar(four_exp, cRelation_ltEquals(), a_var, cStrength_weak());
  cLinearRelation a_lte_three = cLinearRelation_newExpToVar(three_exp, cRelation_gtEquals(), a_var, cStrength_required());
  solver = cSimplexSolver_new();
  assert(1 == cSimplexSolver_addLinearRelationAndResolve(solver, a_gte_four));
  assert(1 == cSimplexSolver_addLinearRelationAndResolve(solver, a_lte_three));
  assert(cVariable_getValue(a_var) <= 3 && !(cVariable_getValue(a_var) >= 4));

  // I'd be remiss if I didn't point out that variables play a dual
  // role: they're all up to being changed, yes, but with an
  // edit-or-stay constraint they're directly substitutable for
  // literals (as long as our constraint heirarchies remain able to
  // specifically constrain variables):

  // stay(n), stay(m); both required, and where n = 20, m = 10;
  // o = n + m - thus, o = 20 + 10, according to our substitution rule
  // above...
  cVariable o_var = cVariable_new("o", 330);
  cVariable n_var = cVariable_new("n", 20);
  cVariable m_var = cVariable_new("m", 10);
  cLinearExpression n_plus_m = cLinearExpression_newExpWithExp(cLinearExpression_newVar(n_var, 1, 0), cOperation_plus(), cLinearExpression_newVar(m_var, 1, 0));
  cLinearRelation o_n_plus_m = cLinearRelation_newExpToVar(n_plus_m, cRelation_equals(), o_var, cStrength_required());
  solver = cSimplexSolver_new();


  assert(1 == cSimplexSolver_addLinearRelationAndResolve(solver, o_n_plus_m));
  assert(1 == cSimplexSolver_addEditOrStayConstraintAndResolve(solver, cEditOrStayConstraint_new(cStrength_required(), n_var)));
  assert(1 == cSimplexSolver_addEditOrStayConstraintAndResolve(solver, cEditOrStayConstraint_new(cStrength_required(), m_var)));
  cSimplexSolver_updateVariableInEditConstraint(solver, n_var, 20);
  cSimplexSolver_updateVariableInEditConstraint(solver, m_var, 10);
  assert(cVariable_getValue(n_var) + cVariable_getValue(m_var) == cVariable_getValue(o_var));


  return 0;
}
