/* ***************************************************************************

Parser: a simple expression evaluator for ReferenceFinder
Originally a yacc/bison grammar, reimplemented due to portability issues.

Author: C.A.Furuti

Subject to same distribution conditions of ReferenceFinder, by R.J.Lang

Copyright (c) 2003 Carlos A. Furuti www.progonos.com/furuti

*************************************************************************** */

#ifndef PARSER_H

#define PARSER_H

#include <string>
#include <map>
#include "lexer.h"

class Parser {
 public:
  // error condition
  enum errType {none,  // expression ok
		opExpected, // operator expected
		parExpected, // parenthesis expected
		unknownId, // unknown name
		notAFunction, // function unknown
		extraInput, // extra input at end of text
		emptyInput, // empty text
		illegalWord, // illegal input
		zeroDivide, // division by zero
		illegalParameter, // illegal function parameter
		cantHappen}; // internal error
  
  // parse string and evaluate expression
  errType evaluate (std::string text, double &result, 
		    bool useDefault = false, 
		    double defaultValue = 0);
  // map error codes to strings
  static const char *parseMessage (errType);
  // updates or creates numerical variable
  static void setVariable (std::string name, double val = 0, 
			   bool overwrite = true);

  // currently the identifier dictionary is shared
 private:
  Lexer *lexer;
  errType value (double &result);
  errType signedExpr (double &result);
  errType factor (double &);
  errType term (double &);
  errType expression (double &);
  int nextToken; // current token

  /* Identifier dictionary, shared by all instances. In ReferenceFinder
     there's at most one Parser active at any time, and there's no need to
     generalize this.
  */
  
  // identifier category
  enum idType {num, fsqrt, fsin, fcos, ftan, fdeg2rad};
  
  // identifier record
  struct id {
    idType type;	// category
    double val;		// numerical value, if any
    id (idType t, double v = 0) : type (t), val (v) {}
  };
  
  // identifier table
  static std::map <std::string, id*> ids;
  static bool idsOk;
};

#endif
