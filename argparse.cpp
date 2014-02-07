#include <iostream> /* DEBUG */
#include <map>
#include <vector>
#include <string>
#include <iterator>
#include <sstream>
#include <algorithm>
#include <cstdlib>

#include "utils.h"
#include "argparse.h"

using namespace std;

/* Class Argument: Member functions */
Argument :: Argument(const string t_argName,
		     const valueType_t t_valType,
		     const string t_help, 
		     const action_t t_action, 
		     const vector<string> & altOptSwitches, 
		     const int t_nargs) :
  argName(t_argName), 
  nargs(t_nargs), 
  help(t_help), 
  valType(t_valType), 
  action(t_action)
{
  // cout << "Argument: action = " << action << endl; /* DEBUG */

  /* argName sanity check */
  if ( argName == "-" || argName == "--" )
    throw illegalArgName();

  /* Test if this is an optional argument */
  if ( (argName.size() >= 2 && argName[0] == '-'
	&& argName.find_first_not_of("-") == 1) 
       ||
       (argName.size() >= 3 && argName[0] == '-' && argName[1] == '-' 
	&& argName.find_first_not_of("-") == 2) )
    bOpt = true;
  else
    bOpt = false;

  /* Process action type */
  if ( !bOpt && (action != DEFAULT_ACTION) )
      /* DEFAULT_ACTION is the only action allowed for non-optional args */
      throw illegalActionType();

  if ( (action == STORE_TRUE || action == STORE_FALSE) 
       && (valType != VAL_TYPE_BOOL) )
    throw illegalActionType();

  if ( (action == STORE_TRUE || action == STORE_FALSE) &&  nargs > 1 )
    /* Binary switches are allowed to have only one argument value */
    throw illegalNArgs();

  /* Process nargs */
  if ( !bOpt && (nargs > 1) )
    /* Positional arguments cannot have >1 argument values */
    throw illegalNArgs();

  if ( nargs <= 0 )
    throw illegalNArgs();

  /* Reserve space for values */
  if ( valType == VAL_TYPE_BOOL )
    v_b.reserve(nargs);
  else if ( valType == VAL_TYPE_INT )
    v_i.reserve(nargs);
  else if ( valType == VAL_TYPE_FLOAT )
    v_f.reserve(nargs);
    else if ( valType == VAL_TYPE_STRING )
    v_s.reserve(nargs);

  /* Process bSet */
  if ( action == STORE_TRUE || action == STORE_FALSE ) {
    bSet = true;

    for (int i = 0; i < nargs; i++)
      v_i.push_back(action == STORE_FALSE);
  }
  else {
    bSet = false;
  }

  /* Process altOptSwitches */
  if ( !altOptSwitches.empty()) {
      if ( !bOpt )
	/* No alternative switches are allowed under non-optional args */
	throw illegalAltOptSwitches();

      optSwitches = altOptSwitches;
  }
}

/* Default constructor */
Argument :: Argument() : 
  bSet(false), 
  argName(""), 
  nargs(1), 
  bOpt(false), 
  optSwitches(vector<string>()), 
  help(""), 
  valType(VAL_TYPE_STRING), 
  action(DEFAULT_ACTION) {
}

/* Set the acceptance set (acceptSet) */
/* If nargs > 1 and only one aSet is supplied (as in this case), then 
   copies of aSet will copied into all positions of acceptSet, i.e., 
   all arguments will have the same acceptance set. */
void Argument :: setAcceptSet(const string aSet) {

  if ( !acceptSet.empty() ) acceptSet.clear();
  acceptSet.reserve(nargs);
  for (int i = 0; i < nargs; ++i)
    acceptSet.push_back(aSet);
}

void Argument :: setAcceptSet(const vector<string> aSets) {
  if ( aSets.size() != static_cast<unsigned int>(nargs) )
    throw unexpectedAcceptSetSizeErr();

  if ( !acceptSet.empty() ) acceptSet.clear();
  acceptSet = aSets;

}



/* Overloaded functions for setting default values.
   Checks on valType will be performed. */
/* 1. Boolean. For nargs == 1 only. */
void Argument :: setDefaultVal(const bool b) {
  if ( nargs != 1 )
    throw setDefaultValFailure();

  if ( !bOpt )
    /* Positional arguments cannot have default values */
    throw setDefaultValFailure();

  if ( valType != VAL_TYPE_BOOL )
    throw valTypeErr();

  if ( (action == STORE_TRUE && b) || (action == STORE_FALSE && !b) )
    throw valLogicalErr();

  if ( !dv_b.empty() ) dv_b.clear();
  dv_b.push_back(b);

  if ( !bSet ) { 
    v_b.clear();
    v_b.push_back(b);
    bSet = true;
  }
}

/* 1. Boolean. For all nargs >= 1 */
void Argument :: setDefaultVals(const vector<bool> bs) {
  if ( !bOpt )
    /* Positional arguments cannot have default values */
    throw setDefaultValFailure();

  if ( valType != VAL_TYPE_BOOL )
    throw valTypeErr();

  for (unsigned int i = 0; i < bs.size(); ++i)
    if ( (action == STORE_TRUE && bs[i]) || (action == STORE_FALSE && !bs[i]) )
      throw valLogicalErr();

  if ( !dv_b.empty() ) dv_b.clear();
  dv_b = bs;

  if ( !bSet ) { 
    v_b.clear();
    v_b = bs;

    bSet = true;
  }
}

/* 2. Integer */
void Argument :: setDefaultVal(const int i) {
  if ( !bOpt )
    throw setDefaultValFailure();

  if ( valType != VAL_TYPE_INT )
    throw valTypeErr();
  
  if ( !dv_i.empty() ) dv_i.clear();
  dv_i.push_back(i);

  if ( !bSet ) { 
    v_i.clear();
    v_i.push_back(i);
    bSet = true;

    if ( !valAccept() ) throw valOutsideAcceptanceSet();
  }
}

/* 2. Integer. For all nargs >= 1 */
void Argument :: setDefaultVals(const vector<int> is) {
  if ( !bOpt )
    /* Positional arguments cannot have default values */
    throw setDefaultValFailure();

  if ( valType != VAL_TYPE_INT )
    throw valTypeErr();

  if ( !dv_b.empty() ) dv_b.clear();
  dv_i = is;

  if ( !bSet ) { 
    v_i.clear();
    v_i = is;

    bSet = true;
    
    if ( !valAccept() ) throw valOutsideAcceptanceSet();
  }
}

/* 3. Float */
void Argument :: setDefaultVal(const float f) {
  if ( !bOpt )
    throw setDefaultValFailure();

  if ( valType != VAL_TYPE_FLOAT )
    throw valTypeErr();

  if ( !dv_f.empty() ) dv_f.clear();
  dv_f.push_back(f);

  if ( !bSet ) { 
    v_f.clear();
    v_f.push_back(f);
    bSet = true; 

    if ( !valAccept() ) throw valOutsideAcceptanceSet();
  }

}

void Argument :: setDefaultVal(const double d) {
  setDefaultVal(static_cast<float>(d));
}

/* 3. Float. For all nargs >= 1 */
void Argument :: setDefaultVals(const vector<float> fs) {
  if ( !bOpt )
    /* Positional arguments cannot have default values */
    throw setDefaultValFailure();

  if ( valType != VAL_TYPE_FLOAT )
    throw valTypeErr();

  if ( !dv_b.empty() ) dv_b.clear();
  dv_f = fs;

  if ( !bSet ) { 
    v_f.clear();
    v_f = fs;

    bSet = true;
    
    if ( !valAccept() ) throw valOutsideAcceptanceSet();
  }
}

/* 4. String */
void Argument :: setDefaultVal(const std::string s) {
  if ( !bOpt )
    throw setDefaultValFailure();

  if ( valType != VAL_TYPE_STRING )
    throw valTypeErr();

  if ( !dv_s.empty() ) dv_s.clear();
  dv_s.push_back(s);

  if ( !bSet ) {
    v_s.clear();
    v_s.push_back(s);
    bSet = true;

    if ( !valAccept() ) throw valOutsideAcceptanceSet();
  }
}

/* 4. String. For all nargs >= 1 */
void Argument :: setDefaultVals(const vector<string> ss) {
  if ( !bOpt )
    /* Positional arguments cannot have default values */
    throw setDefaultValFailure();

  if ( valType != VAL_TYPE_STRING )
    throw valTypeErr();

  if ( !dv_s.empty() ) dv_s.clear();
  dv_s = ss;

  if ( !bSet ) { 
    v_s.clear();
    v_s = ss;

    bSet = true;

    if ( !valAccept() ) throw valOutsideAcceptanceSet();
  }
}



/* Test if the set values are all in the acceptance set.
 Return value: true: all arguments accepted. 
               false: one or more of the arguments are not accepted. */
bool Argument :: valAccept() const {
  if ( !bSet )
    return false; /* Undefined behavior */

  if ( acceptSet.empty() ) /* Null acceptance set --> Accept all values. */
    return true;

  if ( acceptSet.size() != static_cast<unsigned int>(nargs) )
    throw unexpectedAcceptSetSizeErr();

  bool bAccept = true;
  for (int i = 0; i < nargs; ++i) {
    if ( acceptSet[i].empty() )
      /* Null acceptance set for this arg position --> Accept all values */
      continue; 

    vector<string> vas = split_string(acceptSet[i], string(","));
    
    /* TODO: VAL_TYPE_BOOL */
    bool bFound = false;
    for (vector<string>::iterator ivas = vas.begin(); 
	 ivas != vas.end(); ++ivas) {
      if ( valType == VAL_TYPE_BOOL ) {
	bool inSetVal;
	if ( interpret_bool_string(*ivas) == 0 )
	  inSetVal = false;
	else if ( interpret_bool_string(*ivas) == 1 )
	  inSetVal = true;
	else
	  throw valLogicalErr();
	    
	if ( inSetVal == v_b[i] ) {
	  bFound = true;
	  break;
	}
      }
      else if ( valType == VAL_TYPE_INT ) {	
	//cout << "Set member: " << atoi((*ivas).c_str()) << endl; // DEBUG
	if ( is_string_numeric(*ivas) ) {
	  if ( atoi((*ivas).c_str()) == v_i[i] ) {
	    // cout << "Match found: " << endl; // DEBUG
	    bFound = true;
	    break;
	  } 
	}
	else {
	  try {
	    /* Ranges such as >10<=20,>30<=40 */
	    if ( check_number<int>(v_i[i], *ivas) ) {
	      // cout << "Match found: " << *ivas << endl; // DEBUG
	      bFound = true;
	      break;
	    }
	  }
	  catch (checkNumberErr) {
	    cerr << "ERROR: Unrecognized predicate string: " << *ivas;
	    throw valLogicalErr();
	  }
	  
	}

      }
      else if ( valType == VAL_TYPE_FLOAT ) {
	if ( is_string_numeric(*ivas) ) {
	  if ( atof((*ivas).c_str()) == v_f[i] ) {
	    bFound = true;
	    break;
	  }
	}
	else {
	  try {
	    /* Ranges such as >1.5<=2.5,>-2.5<=-1.5 */
	    if ( check_number<float>(v_f[i], *ivas) ) {
	      // cout << "Match found: " << *ivas << endl; //DEBUG
	      bFound = true;
	      break;
	    }
	  }
	  catch (checkNumberErr) {
	    cerr << "ERROR: Unrecognized predicate string: " << *ivas;
	    throw valLogicalErr();
	  }

	}

      }
      else if ( valType == VAL_TYPE_STRING ) {
	// cout << "\tvalAccept(): *ivas = " << *ivas << endl; // DEBUG
	if ( *ivas == v_s[i] ) {
	  bFound = true;
	  break;
	}
      }
      
    }

    // if ( valType == VAL_TYPE_STRING ) // DEBUG
    //   cout << "\tvalAccept(): bFound = " << bFound << endl;
    
    if ( !bFound ) {
      bAccept = false;
      break;
    }
    
  }

  return bAccept;
  
}


/* Set argument value, through string. This is applicable to nargs == 1 only */
void Argument :: setVal(const string inStr) {
  if ( nargs > 1 )
    /* setVal(string) should be used only for nargs == 1. 
       For nargs > 1, use setVal(vector<string>) (see below) */
    throw setValFailure();
  
  if ( valType == VAL_TYPE_BOOL ) {
    if ( !v_b.empty() ) v_b.clear();
    if ( interpret_bool_string(inStr) == 1 )
      v_b.push_back(true);
    else if ( interpret_bool_string(inStr) == 1 )
      v_b.push_back(false);
    else
      throw unrecognizedBooleanValErr();

  }
  else if ( valType == VAL_TYPE_INT ) {
    if ( !is_string_numeric(inStr) )
      throw unrecognizedNumberErr();

    if ( !v_i.empty() ) v_i.clear();
    v_i.push_back(atoi(inStr.c_str()));
  }
  else if ( valType == VAL_TYPE_FLOAT) {
    if ( !is_string_numeric(inStr) )
      throw unrecognizedNumberErr();

    if ( !v_f.empty() ) v_f.clear();
    v_f.push_back(atof(inStr.c_str()));
  }
  else if ( valType == VAL_TYPE_STRING ) {
    if ( !v_s.empty() ) v_s.clear();
    v_s.push_back(inStr);
  }

  bSet = true;

  // if ( valType == VAL_TYPE_STRING ) { // DEBUG
  //   cout << "\tsetVal(): valAccept() = " << valAccept() << endl;
  // }
  
  //cout << "accepted = " << accepted << endl; /* DEBUG */

  if ( !valAccept() ) {
    throw valOutsideAcceptanceSet();
  }

  /* TODO: acceptSet check */
}

/* Set value, through a vector of strings. This applies to both narg == 1 and 
   narg > 1 */
void Argument :: setVal(const vector<string> inStrs) {
  // cout << "\tIn setVal(vector<string>): inStrs.size() = " 
  //      << inStrs.size() << endl; // DEBUG
  
  /* Input length check */
  if ( inStrs.size() != static_cast<unsigned int>(nargs) )
    throw setValFailure();

  if ( nargs == 1)
    setVal(inStrs[0]);

  /* Clear arg value */
  if ( valType == VAL_TYPE_BOOL ) {
    if ( !v_b.empty() ) v_b.clear();
  }
  else if ( valType == VAL_TYPE_INT ) {
    if ( !v_i.empty() ) v_i.clear();    
  }
  else if ( valType == VAL_TYPE_FLOAT ) {
    if ( !v_f.empty() ) v_f.clear();
  }
  else if ( valType == VAL_TYPE_STRING ) {
    if ( !v_s.empty() ) v_s.clear();
  }
	
  /* Set values */
  for (unsigned int i = 0; i < inStrs.size(); ++i) {
    // cout << "\tSetting " << i + 1 << " of " 
    // 	 << inStrs.size() << " argument values" << endl; // DEBUG

    string inStr = inStrs[i];

    if ( valType == VAL_TYPE_BOOL ) {
      if ( interpret_bool_string(inStr) == 1 )
    	v_b.push_back(true);
      else if ( interpret_bool_string(inStr) == 0 )
    	v_b.push_back(false);
      else
    	throw unrecognizedBooleanValErr();

    }
    else if ( valType == VAL_TYPE_INT ) {
      if ( !is_string_numeric(inStr) )
	throw unrecognizedNumberErr();

      v_i.push_back(atoi(inStr.c_str()));
    }
    else if ( valType == VAL_TYPE_FLOAT) {
      if ( !is_string_numeric(inStr) )
	throw unrecognizedNumberErr();

      v_f.push_back(atof(inStr.c_str()));
    }
    else if ( valType == VAL_TYPE_STRING ) {
      v_s.push_back(inStr);
    }
  }

  bSet = true;

  if ( !valAccept() ) throw valOutsideAcceptanceSet();
}

/* Get value */
void * Argument :: getVal() const {
  if ( nargs > 1 )
    /* getVal() is usable only under nargs == 1. 
       For nargs > 1, use _TODO_ */
    throw getValFailure();

  if ( !bSet ) throw valNotSetErr();

  /* TODO: bool */
  if ( valType == VAL_TYPE_INT )
    return (void *) &(v_i[0]);
  else if ( valType == VAL_TYPE_FLOAT )
    return (void *) &(v_f[0]);
  else if ( valType == VAL_TYPE_STRING )
    return (void *) &(v_s[0]);
  else 
    return 0;
  // else if  ( 
  //   return (void
}

/* Automatic type conversion operators for getting values out. 
 These operators are for nargs == 1 only. */

/* Boolean */
Argument :: operator bool() const {
  if ( nargs > 1 )
    throw getValFailure(); /* Should use getBoolVals instead */

  if ( !bSet ) throw valNotSetErr();

  if ( valType != VAL_TYPE_BOOL )
    throw getValFailure();

  return v_b[0];
}


vector<bool> Argument :: getBoolVals() const {
  if ( !bSet ) throw valNotSetErr();

  if ( valType != VAL_TYPE_BOOL )
    throw getValFailure();

  return v_b;
}

/* Integer */
Argument :: operator int() const {
  if ( nargs > 1 )
    throw getValFailure(); /* Should use operator vector<int> instead */

  if ( !bSet ) throw valNotSetErr();

  if ( valType != VAL_TYPE_INT )
    throw getValFailure();

  return v_i[0];
}

vector<int> Argument :: getIntVals() const {
  if ( !bSet ) throw valNotSetErr();

  if ( valType != VAL_TYPE_INT )
    throw getValFailure();

  return v_i;
}

/* Float */
Argument :: operator float() const {
  if ( nargs > 1 )
    throw getValFailure(); /* Should use operator vector<float> instead */

  if ( !bSet ) throw valNotSetErr();

  if ( valType != VAL_TYPE_FLOAT )
    throw getValFailure();

  return v_f[0];
}

vector<float> Argument :: getFloatVals() const {
  if ( !bSet ) throw valNotSetErr();

  if ( valType != VAL_TYPE_FLOAT )
    throw getValFailure();

  return v_f;
}

/* String */
Argument :: operator string() const {
  if ( nargs > 1 )
    throw getValFailure(); /* Should use operator vector<string> instead */

  if ( !bSet ) throw valNotSetErr();

  if ( valType != VAL_TYPE_STRING )
    throw getValFailure();

  return v_s[0];
}

vector<string> Argument :: getStringVals() const {
  if ( !bSet ) throw valNotSetErr();

  if ( valType != VAL_TYPE_STRING )
    throw getValFailure();

  return v_s;
}

/* Argument: generate help string */
string Argument :: getHelpString(const int cw, 
				 const int lw, 
				 const std::string destName) const {
  const int lPad = 2;
  const int mPad = 1;

  ostringstream oss;
  if ( cw <= 0 || lw <= 0 || lw >= cw )
    throw getHelpStringErr();

  /* Left padding */
  for (int i = 0; i < lPad; ++i)
    oss << " ";

  if ( !bOpt ) { /* Positional argument */
    oss << argName << " ";
   
    if ( static_cast<int>(argName.size()) + 1 >= lw )
      oss << "\n";
    else
      oss << string(lw - argName.size() - 1 - lPad, ' ');

  }
  else { /* Optional argument */
    oss << argName;
    for (int i = 0; i < nargs; ++i) {
      oss << " " << destName;
      if ( nargs > 1 )
	oss << (i + 1);
    }
    /* Optional switches */
    if ( optSwitches.size() > 0 ) {
      oss << "   (";
      for (unsigned int i = 0; i < optSwitches.size(); ++i) {
      	oss << optSwitches[i];
      	if (i < optSwitches.size() - 1)
      	  oss << ", ";
      	else 
      	  oss << ")";
      }
    }

    if ( oss.str().size() < static_cast<unsigned int>(lw) - mPad ) {
      for (unsigned int i = 0; 
	   i < static_cast<unsigned int>(lw) - mPad - oss.str().size(); ++i)
	oss << " ";      
    }
    else {
      oss << "\n";
      for (int i = 0; i < lw; ++i)
	oss << " ";
      /* TODO */
    }
  }


  /* Show type and number of arguments */
  std::ostringstream ossType;
  ossType << "[";
  if ( valType == VAL_TYPE_BOOL )
    ossType << "Boolean";
  else if ( valType == VAL_TYPE_INT )
    ossType << "integer";
  else if ( valType == VAL_TYPE_FLOAT )
    ossType << "float";
  else if ( valType == VAL_TYPE_STRING )
    ossType << "string";
  
  if ( nargs > 1 )
    ossType << " x " << nargs;
  
  ossType << "] ";

  /* Show help info */
  std::string helpStr = ossType.str() + help;

  /* Special case: STORE_TRUE and STORE_FALSE Boolean types */
  if ( (valType == VAL_TYPE_BOOL) && 
       ((action == STORE_TRUE) || (action == STORE_FALSE)) ) {
    helpStr += " (Default: ";
    helpStr += ((action == STORE_TRUE) ? "FALSE" : "TRUE");
    helpStr += ")";
  }

  helpStr = lineFormat(helpStr, cw, lw);  
  oss << helpStr.substr(lw, helpStr.size() - lw);

  /* Acceptance sets */
  if ( !acceptSet.empty() ) {
    std::ostringstream ossAcc;
    ossAcc << "Range";
    if ( nargs > 1 )
      ossAcc << "s";
    ossAcc << ": ";

    oss << "\n" << lineFormat(ossAcc.str(), cw, lw);
    for (int i = 0; i < nargs; ++i) {
      ossAcc.str("");
      ossAcc << destName;
      if (nargs > 1)
	ossAcc << i + 1;
      ossAcc << ": " << acceptSet[i];
      oss << "\n" << lineFormat(ossAcc.str(), cw, lw + 2);
    }
  }

  if ( bOpt ) {
    /* Default values */
    std::ostringstream ossDef;
    if ( valType == VAL_TYPE_BOOL && ! dv_b.empty() ) {
      oss << "\n" << lineFormat(string("Default: "), cw, lw);
      for (int i = 0; i < nargs; ++i) {
	ossDef.clear();
	ossDef << "  " << destName;
	if ( nargs > 1 ) 
	  ossDef << i + 1;
	ossDef << " = " << dv_b[i];
	oss << "\n" << lineFormat(ossDef.str(), cw + 2, lw);
      }
    }
    else if ( valType == VAL_TYPE_INT && ! dv_i.empty() ) {
      oss << "\n" << lineFormat(string("Default: "), cw, lw);
      for (int i = 0; i < nargs; ++i) {
	ossDef.str("");
	ossDef << "  " << destName;
	if ( nargs > 1 ) 
	  ossDef << i + 1;
	ossDef << " = " << dv_i[i];
	oss << "\n" << lineFormat(ossDef.str(), cw, lw + 2);
      }      
    }
    else if ( valType == VAL_TYPE_FLOAT && ! dv_f.empty() ) {
      oss << "\n" << lineFormat(string("Default: "), cw, lw);
      for (int i = 0; i < nargs; ++i) {
	ossDef.str("");
	ossDef << "  " << destName;
	if ( nargs > 1 ) 
	  ossDef << i + 1;
	ossDef << " = " << dv_f[i];
	oss << "\n" << lineFormat(ossDef.str(), cw, lw + 2);
      }      
    }
    else if ( valType == VAL_TYPE_STRING && ! dv_s.empty() ) {
      oss << "\n" << lineFormat(string("Default: "), cw, lw);
      for (int i = 0; i < nargs; ++i) {
	ossDef.str("");
	ossDef << "  " << destName;
	if ( nargs > 1 ) 
	  ossDef << i + 1;
	ossDef << " = " << dv_s[i];
	oss << "\n" << lineFormat(ossDef.str(), cw, lw + 2);
      }
    }
  }

  return oss.str();
}


/* Class ArgumentParser */
/* Default constructor */
ArgumentParser :: ArgumentParser() :
  bParsed(false), 
  bCombineSwitches(false), 
  command(""), 
  description("") {
  nPosArgs = 0;
  nOptArgs = 0;
  nSLS = 0;
}

/* Constrcutor with arguments */
ArgumentParser :: ArgumentParser(const string t_cmd, 
				 const string t_description, 
				 const bool t_bCombineSwitches) : 
  bParsed(false), 
  bCombineSwitches(t_bCombineSwitches), 
  command(t_cmd), 
  description(t_description) {
  nPosArgs = 0;
  nOptArgs = 0;
  nSLS = 0;
}

/* Private member function: addSwitch */
void ArgumentParser :: addSwitch(const std::string s) {
  if ( s.find_first_not_of("-") == 1 ) { /* Single dash */
    if ( find(sdSwitches.begin(), sdSwitches.end(), s) 
	 != sdSwitches.end() ) {
      throw duplicateSwitchesErr();
    }
    else {
      sdSwitches.push_back(s);
    }
  }
  else if ( s.find_first_not_of("-") == 2 ) { /* Double dash */
    if ( find(ddSwitches.begin(), ddSwitches.end(), s) 
	 != ddSwitches.end() ) {
      throw duplicateSwitchesErr();
    }
    else {
	  ddSwitches.push_back(s);
    }
  }

}

/* ArgumentParser: add_argument */
void ArgumentParser :: 
add_argument(const std::string t_dest, /* Destination variable name */
	     const std::string t_argName, /* Argument name */
	     const std::string t_help, /* Help information */
	     const Argument::valueType_t t_valType, /* Argument type */
	     const Argument::action_t t_action, /* Action type */
	     const std::vector<std::string> & altOptSwitches,  
	     const int t_nargs) {
  if ( t_dest.empty() )
    throw emptyDestNameErr(); /* Destination name cannot be empty */

  if ( mArgs.count(t_dest) > 0 )
    throw duplicateArgsErr(); /* Destination names must be unique */

  // cout << "t_dest = " << t_dest << endl; // DEBUG
  // cout << "t_action = " << t_action << endl;

  mArgs[t_dest] = Argument(t_argName, t_valType, t_help, t_action,  
			   altOptSwitches, t_nargs);

  if ( mArgs[t_dest].isPositional() ) {
    nPosArgs++;

    pArgs.push_back(t_dest); /* Add to the list of positional arguments */
  }
  else {
    nOptArgs++;

    /* Check the duplication of switches */
    addSwitch(t_argName);
    for ( std::vector<std::string>::const_iterator aosIt = altOptSwitches.begin(); 
    	  aosIt != altOptSwitches.end(); ++aosIt)
      addSwitch(*aosIt);
  }
  
  // cout << "nPosArgs = " << nPosArgs << endl; // DEBUG

  if ( !checkSwitches() )
    throw incompatibleSwitchesErr();
}


/* Set bCombineSwitches */
void ArgumentParser :: setCombineSwitches(const bool bcs) {
  bCombineSwitches = bcs;
}

/* Check the legality of the switches under bCombineSwitches. 
 Return value: true - OK
               false - incompatibility found */
const bool ArgumentParser :: checkSwitches() {
  if ( bCombineSwitches ) {
    /* Single-dash BOOL-type switches will be combined */
    for (AMapIt it = mArgs.begin(); it != mArgs.end(); ++it) {
      if ( (*it).second.getValType() != Argument::VAL_TYPE_BOOL )
	continue;
      
      // cout << (*it).second.getArgName() << endl; // DEBUG
      if ( (*it).second.getArgName().find_first_not_of("-") != 1 ) {
	/* Under the combine mode, all boolean switches must be single-dashed 
	   in its primary argName */
	throw incompatibleSwitchesErr();
      }

      
    }
    
    return true;
  }
  else {
    return true;
  }

}

/* Operator for allowing access to member arguments */
Argument & ArgumentParser :: operator[](const std::string t_dest) {
  /* Test if argument exists */
  if ( mArgs.count(t_dest) == 0 )
    throw argNotFoundErr();

  return mArgs[t_dest];
}

/* Key operation of ArgumentParser: parse the arguments */
void ArgumentParser :: parse_args(int argc, char ** argv) {
  const bool bDebug = false;
  
  ostringstream ossErr; /* For holding error information */

  unsigned int ip = 0; /* index to positional arguments */
  vector<int> bPosSet(pArgs.size(), 0); /* Flags for indicating whether the positional arguments have been set */
  set<string> setOpts; 
  /* A set of optional arguments already set. This is used for preventing
     repeated setting of the same optional argument. */  

  int i = 1;  /* Skip the initial program name */
  while (i < argc) {
    string arg(argv[i]);

    if (bDebug)
      cout << "Processing argument: " << arg << endl;

    if ( arg.size() > 1 && arg[0] == '-' ) { 
      /* Optional argument */
      /* Figure out what optinal argument this is */
      vector<string> t_argNames;
      vector<string> t_destNames;
      int nArgs = 0;

      /* Look in the primary argNames */
      for (AMapIt it = mArgs.begin(); it != mArgs.end(); ++it) {
	if ( (*it).second.getArgName() == arg ) {	  
	  nArgs = (*it).second.getNArgs();
	  t_argNames.push_back( (*it).second.getArgName() );
	  t_destNames.push_back( (*it).first );
	  if (bDebug)
	    cout << "\tOptional argument: " << (*it).first << "("
		 << (*it).second.getArgName() 
		 << "): nargs = " << nArgs << endl;
	  
	  break;
	}
      }
      
      if ( t_argNames.empty() ) {
	/* Look in the secondary argNames */
	for (AMapIt it = mArgs.begin(); it != mArgs.end(); ++it) {
	  vector<string> optSwitches = (*it).second.getOptSwitches();
	  for (vector<string>::iterator osIt = optSwitches.begin();
	       osIt != optSwitches.end(); ++osIt) {
	    if ( (*osIt) == arg ) {
	      nArgs = (*it).second.getNArgs();
	      t_argNames.push_back( (*it).second.getArgName() );
	      t_destNames.push_back( (*it).first );
	      if (bDebug) 
		cout << "\tOptional argument (optional switch): " 
		     << (*it).first << "("
		     << (*it).second.getArgName() 
		     << "): nargs = " << nArgs << endl;

	      break;
	    }
	  }

	}
      }

      if ( t_argNames.empty() && bCombineSwitches ) {
	/* Try to figure out whether this is a combined switch and if so, 
	   which switches these are */
	if ( arg.size() < 3 ) {
	  ossErr << "ERROR: Unrecognized optional argument switch: " << arg;
	  print_help();
	  cerr << ossErr.str() << endl;
	  throw argParsingErr(ossErr.str());
	}
	if ( arg.find_first_not_of("-") != 1 ) {
	  ossErr << "ERROR: Unrecognized optional argument switch: " << arg;
	  print_help();
	  cerr << ossErr.str() << endl;
	  throw argParsingErr(ossErr.str());
	}

	string cs = string(arg, 1, arg.size() - 1);
	if (bDebug) cout << "\tcs = " << cs << endl;
	vector<bool> parsed(cs.size(), false);

	/* Look for the individual switches in the primary argNames */
	for (unsigned int j = 0; j < cs.size(); ++j) {
	  string t_arg = string("-") + string(cs, j, 1);
	  if (bDebug) cout << "\tt_arg = " << t_arg << endl;

	  for (AMapIt it = mArgs.begin(); it != mArgs.end(); ++it) {
	    if ( (*it).second.getArgName() == t_arg ) {
	      if ((*it).second.getAction() != Argument::STORE_TRUE && 
		  (*it).second.getAction() != Argument::STORE_FALSE) {
	      	ossErr << "ERROR: Non-binary option \"" 
	      	       << (*it).second.getArgName() 
	      	       << "\" is included in the combined switch \"" 
	      	       << arg << "\"";
	      	print_help();
	      	cerr << ossErr.str() << endl;
	      	throw argParsingErr(ossErr.str());
	      }

	      nArgs = (*it).second.getNArgs();
	      t_argNames.push_back( (*it).second.getArgName() );
	      t_destNames.push_back( (*it).first );



	      parsed[j] = true;

	      if (bDebug)
		cout << "\tOptional argument (combined): " << (*it).first << "("
		     << (*it).second.getArgName() 
		     << "): nargs = " << nArgs << endl;
	      
	      break;
	    }
	  }

	}

	/* Stipulate that there is no unmatched letters in the 
	   combined switch */
	if ( find(parsed.begin(), parsed.end(), false) != parsed.end() ) {
	  ossErr << "ERROR: Combined binary switches \"" << arg
	       << "\" contains some unrecognized individual switches";
	  print_help();
	  cerr << ossErr.str() << endl;
	  throw argParsingErr(ossErr.str());
	}
	
      }
      
      /* Check for repeated setting of an argument */
      for (vector<string>::iterator ait = t_argNames.begin();
	   ait != t_argNames.end(); ++ait) {
	if ( setOpts.count(*ait) == 1 ) {
	  ossErr << "ERROR: Repeated setting of optional argument: " << *ait;
	  print_help();
	  cerr << ossErr.str() << endl;
	  throw argParsingErr(ossErr.str());
	}
	else {
	  setOpts.insert(*ait);
	}
      }
      
      if ( t_argNames.empty() ) {
	ossErr << "ERROR: Unrecognized optional argument switch: " << arg;
	print_help();
	cerr << ossErr.str() << endl;
	throw argParsingErr(ossErr.str());
      }

      for (vector<string>::iterator dit = t_destNames.begin(); 
	   dit != t_destNames.end(); ++dit) {
	if (bDebug) cout << "\taction = " 
			 << mArgs[*dit].getAction() << endl;
      }

      /* Set the values */
      if ( t_destNames.size() > 1 ) {
	/* Combined binary switches */
	for (vector<string>::iterator dit = t_destNames.begin(); 
	     dit != t_destNames.end(); ++dit) {
	  if ( mArgs[*dit].getAction() == Argument::STORE_TRUE )
	    mArgs[*dit].setVal("true");
	  else if ( mArgs[*dit].getAction() == Argument::STORE_FALSE )
	    mArgs[*dit].setVal("false");
	  else
	    throw unexpectedActionErr();

	}

      }
      else if ( mArgs[t_destNames[0]].getAction() == Argument::STORE_TRUE || 
		mArgs[t_destNames[0]].getAction() == Argument::STORE_FALSE ) {
	/* Set binary switch */
	if ( mArgs[t_destNames[0]].getAction() == Argument::STORE_TRUE )
	  mArgs[t_destNames[0]].setVal("true");
	else
	  mArgs[t_destNames[0]].setVal("false");

      }
      else {
	/* TODO: Check to make sure that the following arguments aren't 
	   switches */
	if ( nArgs == 1 ) {
	  if ( i + 1 >= argc ) {
	    ossErr << "ERROR: Too few arguments";
	    print_help();
	    cerr << ossErr.str() << endl;
	    throw argParsingErr(ossErr.str());
	  }

	  if (bDebug) cout << "\tSetting val using string \"" 
			   << string(argv[i + 1]) << "\"" << endl;
	  mArgs[t_destNames[0]].setVal(string(argv[i + 1]));
	}
	else {
	  vector<string> valStrs;
	  valStrs.reserve(nArgs);
	  for (int k = 0; k < nArgs; k++) {
	    if (i + k + 1 >= argc) {
	      ossErr << "ERROR: Too few arguments";
	      print_help();
	      cerr << ossErr.str() << endl;
	      throw argParsingErr(ossErr.str());
	    }

	    valStrs.push_back(string(argv[i + k + 1]));	    
	  }

	  mArgs[t_destNames[0]].setVal(valStrs);
	}
      }

      if ( mArgs[t_destNames[0]].getAction() == Argument::STORE_TRUE || 
	   mArgs[t_destNames[0]].getAction() == Argument::STORE_FALSE ) {
	if ( bDebug ) cout << "\tSkipping 0 arg" << endl;
	i += 1;
      }
      else {
	if ( bDebug ) cout << "\tSkipping " << nArgs << " args" << endl;
	i += (nArgs + 1); /* The switch itself needs to be skipped as well */
      }
    }
    else {
      /* Positional argument */
      if ( ip >= pArgs.size() ) {
	ossErr << "ERROR: Too many positional arguments (" 
	     << (ip + 1) << " > " << pArgs.size() << ")" << endl;
	print_help();
	cerr << ossErr.str() << endl;
	throw argParsingErr(ossErr.str());
      }

      if (bDebug)
	cout << "\tPositional argument: " << pArgs[ip] << endl;

      try {
	mArgs[pArgs[ip]].setVal(arg);
      }
      catch (Argument::setValFailure) {
	ossErr << "ERROR: Failed to set value of positional argument " 
	     << pArgs[ip] << " to: " << arg;
	cerr << ossErr.str() << endl;
	throw argParsingErr();
      }
      catch (Argument::valOutsideAcceptanceSet) {
	ossErr << "ERROR: Value outside acceptance set of positional argument "
	       << pArgs[ip] << ": " << arg;
	cerr << ossErr.str() << endl;
	throw argParsingErr();
      }

      if (bDebug)
	cout << "\tValue set" << endl;

      ip++;
      i++;
    }

  }

  /* Make sure that all positional arguments are set */
  if ( static_cast<int>(ip) < nPosArgs ) {
    ossErr << "ERROR: one or more positional arguments are not set.";
    print_help();
    cerr << ossErr.str() << endl;
    throw argParsingErr(ossErr.str());
  }

}

/* ArgumentParser: Get help string 
 Input arguments: cw - console width 
                  lw - left white-space width */
string ArgumentParser :: getHelpString(const int cw, const int lw) const {
  ostringstream oss;

  oss << "Usage: " << command << " ";
  int lw0 = oss.str().size();

  ostringstream ossArgs;
  for (AMapItC amit = mArgs.begin(); amit != mArgs.end(); ++amit) {
    /* Print all optinal arguments */
    if ( !((*amit).second.isPositional()) ) {
      ossArgs << "[" << (*amit).second.getArgName() << " ";
      
      int t_nargs = (*amit).second.getNArgs();
      for (int i = 0; i < t_nargs; ++i) {
	ossArgs << (*amit).first;

	if ( t_nargs > 1 )
	  ossArgs << (i + 1);

	if ( i == t_nargs - 1)
	  ossArgs << "] ";
	else
	  ossArgs << " ";
      }

    }
  }

  if ( lw0 >= cw ) {
    oss << ossArgs.str();
  }
  else {
    string strArgsFmt = lineFormat(ossArgs.str(), cw, lw0);
    oss << strArgsFmt.substr(lw0, strArgsFmt.size() - lw0);
  }

  /* Print all positional arguments */
  for (deque<string>::const_iterator pait = pArgs.begin();
       pait != pArgs.end(); ++pait)
    oss << *pait << " ";

  /* Print description */
  oss << "\n\n" << lineFormat(description, cw, 0) << "\n\n";

  /* Detailed help info for all positional arguments */
  if (nPosArgs > 0) {
    oss << "Positional arguments: \n";

    for (AMapItC amit = mArgs.begin(); amit != mArgs.end(); ++amit)
      if ( (*amit).second.isPositional() )
	oss << (*amit).second.getHelpString(cw, lw, (*amit).first) << "\n";
    
    oss << "\n";
  }

  /* Detailed help info for all optional arguments */
  if (nOptArgs > 0) {
    oss << "Optional arguments: \n";

    for (AMapItC amit = mArgs.begin(); amit != mArgs.end(); ++amit)
      if ( !(*amit).second.isPositional() )
	oss << (*amit).second.getHelpString(cw, lw, (*amit).first) << "\n";

    oss << "\n";
  }

  return oss.str();
} 

/* ArgumentParser: Print help */
void ArgumentParser :: print_help() const {
  cout << getHelpString(80, 24) << "\n";
}

