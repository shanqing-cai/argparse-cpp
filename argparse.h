#ifndef ARGPARSE_H
#define ARGPARSE_H

#include <string>
#include <vector>
#include <deque>
#include <set>
#include <map>
#include <string>

/* Main class: Argument: a single argument */
class Argument {
 private:
  bool bSet; 
  /* Status flag: whether the value has been read in from the argv or not */

  std::string argName; /* Argument name */

  int nargs; /* Number of arguments (e.g., --rgb 160 240 80) */

  bool bOpt;
  /* Whether the argument is optional or not.
     Optional arguments start have argNames that start with - or --. 
     Optional arguments can have default values, whereas non-optional ones 
     cannot. */

  std::vector<std::string> optSwitches;
  /* These are all the alternative forms of the option switch:
     e.g., --verbose, -v */

  std::string help; /* Help information */

  /* Values:
     These values are stored in vectors, so that multiple arguments (i.e., 
     nargs > 1) can be supported */
  std::vector<bool> v_b;           /* Boolean value */
  std::vector<int> v_i;            /* Integer value */
  std::vector<float> v_f;          /* Float value */
  std::vector<std::string> v_s;    /* String value */

  /* Default values */
  std::vector<bool> dv_b;        /* Default Boolean value */
  std::vector<int> dv_i;         /* Default integer value */
  std::vector<float> dv_f;       /* Default float value */
  std::vector<std::string> dv_s; /* Default string value */

  /* Acceptable set of values */
  std::vector<std::string> acceptSet;
  /* Acceptable set for the values. This field holds the acceptable set of
     all possible value types. This approach is for achieving maximal 
     flexibility in some of the value types. For example, for interger values, 
     you can use string(">=0,<=-3") to specify an acceptance range that 
     includes all integers except -2 and -1. For float values, you can use 
     string(">0") to specify a positive real-valued acceptance range. For
     the string type, you can use something like string("r,g,b,black"). 
     See functions setAcceptSet() and setVal(). 

     For multiple argument values (nargs > 1), different arguments can have 
     different acceptance sets. This is why "acceptSet" is defined as a 
     vector. */

  /* Test if the set values are all in the acceptance set */
  bool valAccept() const;
  /* TODO: nested structures */

 public:
  /* Value types */
  typedef enum {
    VAL_TYPE_BOOL, 
    VAL_TYPE_INT,
    VAL_TYPE_FLOAT,
    VAL_TYPE_STRING,
  } valueType_t;

  /* Actions: 
   non-optional arguments can only be associated with DEFAULT_ACTION */
  typedef enum {
    DEFAULT_ACTION, 
    STORE_TRUE,
    STORE_FALSE,
  } action_t;

  valueType_t valType;
  action_t action;

  /* Default constructor */
  Argument();

  /* Full constructor */
  Argument(const std::string t_argName,
	   const valueType_t t_valType=VAL_TYPE_STRING, 
	   const std::string t_help=std::string(""), 
	   const action_t t_action=DEFAULT_ACTION, 
	   const std::vector<std::string> & altOptSwitches=std::vector<std::string>(),
	   const int t_nargs=1);
	   
  /* Setting the acceptance set (acceptSet) */
  void setAcceptSet(const std::string aSet);
  void setAcceptSet(const std::vector<std::string> aSets);
  
  /* Set value, through string */
  void setVal(const std::string inStr);
  void setVal(const std::vector<std::string> inStrs);

  /* Overloaded functions for setting default value. 
     These functions are for nargs == 1 only. */
  void setDefaultVal(const bool b);
  void setDefaultVal(const int i);
  void setDefaultVal(const float f);
  void setDefaultVal(const std::string s);

  void setDefaultVal(const double d);

  /* Overloaded functions for setting default values. 
     These apply to all nargs >= 1 */
  void setDefaultVals(const std::vector<bool> bs);
  void setDefaultVals(const std::vector<int> is);
  void setDefaultVals(const std::vector<float> fs);
  void setDefaultVals(const std::vector<std::string> ss);

  /* Get value. This is one way of getting the value. 
     An alterntive way (perhaps more elegant, is by using the automatic type 
     conversion operators below. 
     getVal works only for nargs == 1. */
  void * getVal() const;

  /* A bunch of automatic type conversion operators. 
     These operators are for nargs == 1 only. */
  operator bool() const;
  operator int() const;
  operator float() const;
  operator std::string() const;

  /* Generating string (help information) */
  std::string getHelpString(const int consoleWidth, 
			    const int leftWidth, 
			    const std::string destName) const;

  /* Value getters for nargs >= 1 */
  std::vector<bool> getBoolVals() const;
  std::vector<int> getIntVals() const;
  std::vector<float> getFloatVals() const;
  std::vector<std::string> getStringVals() const;  

  /* A bunch of getters */
  const bool isPositional() const { return !bOpt; }
  const valueType_t getValType() const { return valType; }
  const std::string getArgName() const { return argName; }
  const int getNArgs() const { return nargs; }
  const int getAction() const { return action; }
  const std::vector<std::string> & getOptSwitches() const { 
    return optSwitches; }
  const bool set() const { return bSet; }

  /* Exceptions */
  class illegalArgName {};
  class illegalActionType {};
  class illegalNArgs {};
  class illegalAltOptSwitches {};
  class setValFailure {};
  class setDefaultValFailure {};
  class unexpectedAcceptSetSizeErr {};
  class valOutsideAcceptanceSet {};
  class getValFailure {};
  class valNotSetErr {};
  class valTypeErr {};
  class valLogicalErr {};
  class unrecognizedBooleanValErr {};
  class unrecognizedNumberErr {};  
  class getHelpStringErr {};
};

/* Main class: ArgumentParser */
class ArgumentParser {
 private:
  int nPosArgs; /* Number of positional arguments */
  int nOptArgs; /* Number of optional arguments */
  int nSLS; /* Number of single-letter switches */

  bool bParsed; /* Whether all the arguments has been fully parsed from argv */
  bool bCombineSwitches; 
  /* Combine single-letter boolean switches into a single argument: e.g., 
     -avz */

  typedef std::map<std::string, Argument> ArgumentMap;
  typedef std::map<std::string, Argument>::iterator AMapIt;
  typedef std::map<std::string, Argument>::const_iterator AMapItC;

  std::string command;
  std::string description;
  ArgumentMap mArgs;
  /* Map of all arguments, with destination names as keys */

  std::deque<std::string> pArgs; 
  /* List of all positional arguments */

  std::deque<std::string> sdSwitches; /* Single-dash switches */
  std::deque<std::string> ddSwitches; /* Double-dash switches */

  /* TODO: Mutual exclusion */
  /* TODO: Argument groups */

  /* Private member functions */
  void addSwitch(const std::string s);

  /* Check the legality of switches under bCombineSwitches */
  const bool checkSwitches();

 public:
  /* Default constructor */
  ArgumentParser();
 
  ArgumentParser(const std::string t_cmd, 
		 const std::string t_description, 
		 const bool t_bCombineSwithces=false);
  
  /* Add argument */
  void add_argument(const std::string t_dest, /* Destination variable name */
		    const std::string t_argName, /* Argument name */
		    const std::string t_help, /* Help information */
		    const Argument::valueType_t t_valType=Argument::VAL_TYPE_STRING, /* Argument type */
		    const Argument::action_t t_action=Argument::DEFAULT_ACTION, /* Action type */
		    const std::vector<std::string> & altOptSwitches=std::vector<std::string>(), 
		    const int t_nargs=1 /* Alternative optional switches */  
		    );



  /* Remove argument */
  void remove_argument(const std::string t_argName);

  void parse_args(int argc, char ** argv);

  void setCombineSwitches(const bool bcs);

  std::string getHelpString(const int cw, const int lw) const;
  void print_help() const;

  /* Operator for allowing access to member arguments */
  Argument & operator[](std::string t_dest);
  
  /* Exception classes */
  class addArgumentFailure {};
  class emptyDestNameErr {};
  class duplicateArgsErr {};
  class duplicateSwitchesErr {};
  class incompatibleSwitchesErr {};
  class argRemovalFailure {};
  class argNotFoundErr {};
  class unexpectedActionErr {};
  class argParsingErr {
    std::string errMsg;

  public:
  argParsingErr() : errMsg("") {}
  argParsingErr(std::string em) : errMsg(em) {}
  };


};

#endif
