#ifndef UTILS_H
#define UTILS_H

#include <string>
#include <vector>
#include <iostream>
#include <sstream>
#include <cstdlib>

/* String utility functions */
/* Find all instances of a sub-string in a string */
std::vector<size_t> string_find(std::string s, std::string subs);

/* Split string by any delimiter string */
std::vector<std::string> split_string(std::string s, std::string subs);

/* Interpreting boolean strings. 
 Return values: -1: uninterpretable string
                 0: false
		 1: true */
int interpret_bool_string(const std::string inStr);

/* Test if a string is a number, 
   e.g. -1, 0.0, 3, 3.14, .025.
   Limitatin cannot hanle scientific notations such as 1e2. */
bool is_string_numeric(const std::string str);

/* Test if a number (int or float or other types) satisfies a certain
   condition specified in a string.
   Can justapose predicates for "AND" relations.
   
   Currently does not support "OR" relations.
   
   e.g., 
         check_num(10, "==10") = true
	 check_num(9, "!=9") = false
         check_num(-3, ">=0") == false
	 check_num(-1.2, ">-2<-1") = true 
*/
class checkNumberErr {};

template<class NT> 
bool check_number(NT n, std::string conds) {
  const bool bDebug=false;

  if ( conds.empty() )
    return true;

  std::vector<std::string> ops;
  ops.push_back(std::string("=="));
  ops.push_back(std::string("!="));
  ops.push_back(std::string("<="));
  ops.push_back(std::string(">="));
  ops.push_back(std::string("<"));
  ops.push_back(std::string(">"));

  /* Extract predicates */
  std::vector<std::string> pred;

  bool bPass = true;

  std::string cs = conds;
  while ( !cs.empty() ) {
    /* The string must begin with an operator in ops */
    int bi = -1;
    int opn = -1;
    for (unsigned int i = 0; i < ops.size(); ++i) {
      if ( cs.size() > ops[i].size() && 
  	   cs.substr(0, ops[i].size()) == ops[i] ) {
  	bi = ops[i].size();
	opn = i;
	break;
      }
    }

    if ( bi == -1 ) {
      std::cerr << "Illegal predicate string: " << conds << std::endl;
      throw checkNumberErr();
    }

    std::string subcs = cs.substr(bi, cs.size() - bi);

    int ci = -1;
    for (unsigned int i = 0; i < ops.size(); ++i) {
      if ( subcs.find(ops[i]) != std::string::npos ) {
  	ci = subcs.find(ops[i]);
	break;
      }
    }

    std::string ps = (ci == -1) ? cs : cs.substr(0, bi + ci);
    std::string ns = ps.substr(bi, ps.size() - bi);
        
    if (bDebug)
      std::cout << "Predicate: " << ps 
		<< "; number = " << ns << std::endl;

    if ( !is_string_numeric(ns) ) {
      std::cerr << "ERROR: Unrecognized number string: " << ns << std::endl;
      throw checkNumberErr();
    }

    /* Perform the actual tests */
    bool bOkay;
    if ( ops[opn] == "==" )
      bOkay = (n == static_cast<NT>(atof(ns.c_str())));
    else if ( ops[opn] == "!=" )
      bOkay = (n != static_cast<NT>(atof(ns.c_str())));
    else if ( ops[opn] == "<=" )
      bOkay = (n <= static_cast<NT>(atof(ns.c_str())));
    else if ( ops[opn] == ">=" )
      bOkay = (n >= static_cast<NT>(atof(ns.c_str())));
        else if ( ops[opn] == "<" )
      bOkay = (n < static_cast<NT>(atof(ns.c_str())));
    else if ( ops[opn] == ">" )
      bOkay = (n > static_cast<NT>(atof(ns.c_str())));
    
    if ( !bOkay ) {
      bPass = false;
      break;
    }      
        
    cs = cs.substr(ps.size(), cs.size() - ps.size());
  }

  return bPass;
}
#endif

/* lineFormat():
      Format a long string with multiple words into a line-broken string, 
   automatically taking care of the placement of line breaks to prevent
   breaking up words whenever possible. 
   Input arguments:
       inStr: input string, unformatted
       w:     string width 
       lw:    left white-space width */
std::string lineFormat(const std::string inStr, 
		       const int w, 
		       const int lw);
