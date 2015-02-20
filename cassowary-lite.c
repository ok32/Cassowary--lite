#include "c++/Cl.h"
#include "cassowary-lite.h"
#include <stdlib.h>
#include <iostream>

// Operations on ClStrengths:

cStrength cStrength_required()
{
  return &( const_cast<ClStrength &>(ClsRequired()) );
}


cStrength cStrength_strong()
{
  return &( const_cast<ClStrength &>(ClsStrong()) );
}


cStrength cStrength_medium()
{
  return &( const_cast<ClStrength &>(ClsMedium()) );
}

cStrength cStrength_weak()
{
  return &( const_cast<ClStrength &>(ClsWeak()) );
}


// Operations on ClVariables:

cVariable cVariable_new(const char* name, double initialValue)
{
  return new ClVariable(name, initialValue);
}

void cVariable_delete(cVariable var)
{
  delete var;
}

const char* cVariable_getName(cVariable var)
{
  return var->Name().c_str();
}

double cVariable_getValue(cVariable var)
{
  return var->Value();
}

// Operations on ClLinearExpressions:
enum cOperation_implementation {
  cPlus, cMinus, cTimes, cDivide
};
  
cOperation cOperation_plus()
{
  return cPlus;
}

cOperation cOperation_minus()
{
  return cMinus;
}

cOperation cOperation_times()
{
  return cTimes;
}
  
cOperation cOperation_divide()
{
  return cDivide;
}

cLinearExpression cLinearExpression_newVar(cVariable var, double coefficient, double additiveConstant)
{
  return new ClLinearExpression(*var, coefficient, additiveConstant);
}

cLinearExpression cLinearExpression_newN(double n)
{
  return new ClLinearExpression(n);
}
cLinearExpression cLinearExpression_newExpTimesExp(cLinearExpression leftExp, cLinearExpression rightExp)
{
  try {
    return new ClLinearExpression(leftExp->Times(*rightExp));
  } catch(ExCLNonlinearExpression) {
    return NULL;
  }
}

cLinearExpression cLinearExpression_newExpWithExp(cLinearExpression leftExp, cOperation op, cLinearExpression rightExp)
{
  switch(op) {
  case cPlus: return new ClLinearExpression(leftExp->Plus(*rightExp));
  case cMinus: return new ClLinearExpression(leftExp->Minus(*rightExp));
  case cTimes:
    try {
      return new ClLinearExpression(leftExp->Times(*rightExp));
    } catch(ExCLNonlinearExpression) {
      return NULL;
    }
  case cDivide:
    try {
      return new ClLinearExpression(leftExp->Divide(*rightExp));
    } catch(ExCLNonlinearExpression) {
      return NULL;
    }
  }
  return NULL;
}

cLinearExpression cLinearExpression_newExpWithN(cLinearExpression exp, cOperation op, double n)
{
  switch(op) {
  case cPlus: return new ClLinearExpression(exp->Plus(n));
  case cMinus: return new ClLinearExpression(exp->Minus(n));
  case cTimes: return new ClLinearExpression(exp->Times(n));
  case cDivide: return new ClLinearExpression(exp->Divide(n));
  }
}

void cLinearExpression_delete(cLinearExpression exp)
{
  delete exp;
}

// Operations on ClLinearInequalities:

cRelation cRelation_equals()
{
  return cnEQ;
}

cRelation cRelation_ltEquals()
{
  return cnLEQ;
}

cRelation cRelation_gtEquals()
{
  return cnGEQ;
}

// Begin a few utility definitions we'll need to implement a fairly
// clean interface to cLinearRelation:

inline cRelation cRelation_flipOperator(cRelation op)
{
  switch(op) {
  case cnEQ: return cnEQ;
  case cnLT: return cnGT;
  case cnLEQ: return cnGEQ;
  case cnGT: return cnLT;
  case cnGEQ: return cnLEQ;
  }
}

inline ClCnRelation cRelation_toClCnRelation(cRelation op)
{

  switch(op) {
  case cnEQ: return cnEQ;
  case cnLT: return cnLT;
  case cnLEQ: return cnLEQ;
  case cnGT: return cnGT;
  case cnGEQ: return cnGEQ;
  }
}

static ClLinearExpression zero_expression = ClLinearExpression(0);

// They end here, though.

cLinearRelation cLinearRelation_newExpToZero(cLinearExpression exp, cRelation op, cStrength strength)
{
  return cLinearRelation_newExpToExp(exp, op, &zero_expression, strength);
}

cLinearRelation cLinearRelation_newExpToVar(cLinearExpression exp, cRelation op, cVariable var, cStrength strength)
{
  if(op == cnEQ) {
    return new ClLinearEquation(*var, *exp, *strength);
  } else {
    return new ClLinearInequality(*var, cRelation_toClCnRelation(cRelation_flipOperator(op)), *exp, *strength);
  }
}

cLinearRelation cLinearRelation_newExpToExp(cLinearExpression leftExp, cRelation op, cLinearExpression rightExp, cStrength strength)
{
  if(op == cnEQ) {
    return new ClLinearEquation(*leftExp, *rightExp, *strength);
  } else {
    return new ClLinearInequality(*leftExp, cRelation_toClCnRelation(op), *rightExp, *strength);
  }
}

void cLinearRelation_delete(cLinearRelation eqn)
{
  delete eqn;
}


bool cLinearRelation_isSatisfied(cLinearRelation eqn)
{
  return eqn->FIsSatisfied();
}


// Operations on ClEditOrStayConstraints:

cEditOrStayConstraint cEditOrStayConstraint_new(cStrength strength, cVariable var)
{
  return new ClEditConstraint(*var, *strength);
}

void cEditOrStayConstraint_delete(cEditOrStayConstraint cnst)
{
  delete cnst;
}

// Operations on ClSimplexSolvers:


cSimplexSolver cSimplexSolver_new()
{
  cSimplexSolver toReturn = new ClSimplexSolver();
  return toReturn;
}

void cSimplexSolver_delete(cSimplexSolver solver)
{
  delete solver;
}

bool cSimplexSolver_addLinearRelationAndResolve(cSimplexSolver solver, cLinearRelation eqn)
{
  return solver->AddConstraintNoException(eqn);
}

void cSimplexSolver_removeLinearRelation(cSimplexSolver solver, cLinearRelation eqn)
{
  solver->RemoveConstraint(eqn);
}

bool cSimplexSolver_addEditOrStayConstraintAndResolve(cSimplexSolver solver, cEditOrStayConstraint eqn)
{
  return solver->AddConstraintNoException(eqn);
}

void cSimplexSolver_removeEditOrStayConstraint(cSimplexSolver solver, cEditOrStayConstraint eqn)
{
  solver->RemoveConstraint(eqn);
}

bool cSimplexSolver_addStayVar(cSimplexSolver solver, cVariable var)
{
  try {
    solver->AddStay(*var);
    return true;
  } catch(...) {
    return false;
  }
}

// The mutable edit variable annex:
bool cSimplexSolver_editVariableInEditConstraint(cSimplexSolver solver, cVariable var, double value)
{
  try {
    solver->SuggestValue(*var, value);
    return true;
  } catch(ExCLEditMisuse) {
    return false;
  }
}

bool cSimplexSolver_resolve(cSimplexSolver solver)
{
  try {
    solver->Resolve();
    return true;
  } catch(...) {
    return false;
  }
}

bool cSimplexSolver_updateVariableInEditConstraint(cSimplexSolver solver, cVariable var, double value)
{  
  cSimplexSolver_editVariableInEditConstraint(solver, var, value);
  cSimplexSolver_resolve(solver);
}

void cSimplexSolver_debug_printState(cSimplexSolver solver)
{
  solver->PrintOn(std::cout);
}
