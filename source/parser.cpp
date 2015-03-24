/* ***************************************************************************

Parser: a simple expression evaluator for ReferenceFinder
Originally a yacc/bison grammar, reimplemented due to portability issues.

Author: C.A.Furuti

Subject to same distribution conditions of ReferenceFinder, by R.J.Lang

Copyright (c) 2003 Carlos A. Furuti www.progonos.com/furuti

*************************************************************************** */

#include <cmath>	// sin, cos, tan
#include <cstdlib>	// strtod
#include "parser.h"

// some systems don't define those in math.h
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif
#ifndef M_E
#define M_E 2.7182818284590452354
#endif


/* parseMessage: convenience error output
   Parameters:
     error: error identifier
   Returns:
     corresponding error message
*/
const char *Parser::parseMessage (errType error) {
  switch (error) {
  case none: return "";
  case opExpected: return "operator +,-,*,/,^ expected";
  case parExpected: return "parenthesis expected";
  case unknownId: return "unknown name";
  case notAFunction: return "identifier is not a function";
  case extraInput: return "extra input at end of text";
  case emptyInput: return "empty text";
  case illegalWord: return "neither number, name nor operator";
  case zeroDivide: return "division by zero";
  case illegalParameter: return "parameter of function out of range";
  case cantHappen: return "internal error";
  default: return "illegal error code";
  }
}

// symbol table
std::map <std::string, Parser::id*> Parser::ids;
bool Parser::idsOk = false;

/* Ordinary recursive descent predictive parser */

/* value: number|var|function(expr)|(expr)
 */
Parser::errType Parser::value (double &result) {
  errType error;
  switch (nextToken) {
  case Lexer::numberTk:
    result = std::strtod (lexer -> token ().c_str (), 0);
    nextToken = lexer -> next ();
    return none;
  case Lexer::wordTk: {
    if (! idsOk) {
      setVariable ("w", 1, false);
      setVariable ("h", 1, false);
      setVariable ("d", std::sqrt (2.0), false);
      ids [std::string("phi")] = new id (num, 0.5 * (std::sqrt (5.0) - 1));
      ids [std::string("Phi")] = new id (num, 0.5 * (std::sqrt (5.0) + 1));
      ids [std::string("pi")] = new id (num, M_PI);
      ids [std::string("e")] = new id (num, M_E);
      ids [std::string ("sqrt")] = new id (fsqrt);
      ids [std::string ("sin")] = new id (fsin);
      ids [std::string ("cos")] = new id (fcos);
      ids [std::string ("tan")] = new id (ftan);
      ids [std::string ("deg2rad")] = new id (fdeg2rad);
      idsOk = true;
    }
    id *p = ids [lexer -> token ()];
    if (! p)
      return unknownId;
    if (p -> type == num) { // simple variable
      result = p -> val;
      nextToken = lexer -> next ();
      return none;
    } else { // function call
      double r1;
      if ((nextToken = lexer -> next ()) != '(')
	return parExpected;
      nextToken = lexer -> next ();
      if ((error = expression (r1)) != none)
	return error;
      if (nextToken != ')')
	return parExpected;
      nextToken = lexer -> next ();
      switch (p -> type) {
      case fsqrt: 
	if (r1 < 0)
	  return illegalParameter;
	result = std::sqrt (r1); break;
      case fsin: result = std::sin (r1); break;
      case fcos: result = std::cos (r1); break;
      case ftan: 
	if (std::cos (r1) == 0) 
	  // TODO: should test against an appropriate epsilon
	  return illegalParameter;
	result = std::tan (r1); break;
      case fdeg2rad: result = M_PI * r1 / 180; break;
      default: return cantHappen;
      }
      return none;
    }
  }
  case '(':
    nextToken = lexer -> next ();
    double r;
    if ((error = expression (r)) != none)
      return error;
    if (nextToken != ')')
      return parExpected;
    nextToken = lexer -> next ();
    result = r;
    return none;
  default:
    return illegalWord;
  }
}

/* signedExpr: [+-]?value
 */
Parser::errType Parser::signedExpr (double &result) {
  int tokOp = nextToken;
  errType error;
  if (nextToken == '-' || nextToken == '+') {
    double r1;
    nextToken = lexer -> next ();
    if ((error = value (r1)) != none)
      return error;
    result = tokOp == '-' ? -r1 : r1;
    return none;
  } else
    return value (result);
}

/* factor: signedExpr (^ factor)?
 */
Parser::errType Parser::factor (double &result) {
  double r1, r2;
  errType error;
  if ((error = signedExpr (r1)) != none)
    return error;
  int tokOp = nextToken;
  if (tokOp == '^') {
    nextToken = lexer -> next ();
    if ((error = factor (r2)) != none)
      return error;
    else { // TODO: should test for r1 < 0 and fractionary r2
      result = std::pow (r1, r2);
      return none;
    }
  } else {
    result = r1;
    return none;
  }
}

/* term: factor ((/|*) factor)*
*/
Parser::errType Parser::term (double &result) {
  double r1, r2;
  errType error;
  if ((error = factor (r1)) != none)
    return error;
  else {
    while (1)
      switch (nextToken) {
      case '*':
	nextToken = lexer -> next ();
	if ((error = factor (r2)) != none)
	  return error;
	else
	  r1 *= r2;
	break;
      case '/':
	nextToken = lexer -> next ();
	if ((error = factor (r2)) != none)
	  return error;
	if (r2 == 0) // should test agains epsilon
	  return zeroDivide;
	result = r1 /= r2;
	break;
      default:
	result = r1;
	return none;
      }
    result = r1;
    return none;
  }
}

/* expression: term ([+-] term)*
 */
Parser::errType Parser::expression (double &result) {
  double r1, r2;
  errType error;
  if ((error = term (r1)) != none)
    return error;
  while (1)
    switch (nextToken) {
    case '+':
      nextToken = lexer -> next ();
      if ((error = term (r2)) != none)
	return error;
      else
	r1 += r2;
      break;
    case '-':
      nextToken = lexer -> next ();
      if ((error = term (r2)) != none)
	return error;
      else
	r1 -= r2;
      break;
    default:
      result = r1;
      return none;
    }
  result = r1;
  return none;
}

/* evaluate: parse and evaluate simple expression
   Parameters:
     str: text to parse
     result: result if no error found
     useDefault: use default if empty text
     defaultValue: default value if text is empty
   Returns:
     <none> if no error was found, otherwise error code
*/
Parser::errType Parser::evaluate (std::string str, double &result,
				  bool useDefault, 
				  double defaultValue) {
  lexer = new Lexer (str);
  if (! (nextToken = lexer -> next ())) { // already end of text
    if (useDefault) {
      result = defaultValue;
      return none;
    }
    else
      return emptyInput;
  }  
  double r;
  errType err = expression (r);
  if (nextToken && err == none)
    err = extraInput;
  if (err == none)
    result = r;
  return err;
}

/* setVariable: create or update variable
   Parameters:
     name: symbol name
     val: symbol value
     overwrite: if false, don't change existing variable
*/
void Parser::setVariable (std::string sname, double val, bool overwrite) {
  id *p = ids [sname];
  if (! p) 
    ids [sname] = new id (num, val);
  else
    if (overwrite) {
      p -> type = num; // killing a function definition is allowed
      p -> val = val;
    }
}
