
/* Command for testing:

   ./test_argparse 485 TSC --n-forms 43 -v --summary --rel-date-range -8 7

*/

#include <iostream>
#include "argparse.h"

using namespace std;

int main(int argc, char ** argv) {
  // Argument arg1("subjID", Argument::VAL_TYPE_STRING, 
  // 		string("Subject ID"));
  // arg1.setVal("S01,S02,S03,S04");

  // arg1.setVal("S04");
  // cout << "arg1.getVal() = " << *((string *)arg1.getVal()) << endl;
  // /* A more elegant way */
  // cout << "arg1.getVal() = " + string(arg1) << endl;

  // Argument arg2("--depth", Argument::VAL_TYPE_INT, 
  // 		string("White-matter depth"));
  // arg2.setAcceptSet("0,1,2,3");
  // arg2.setDefaultVal(2);

  // arg2.setVal("1");
  // cout << "arg2.getVal() = " << *((int *)arg2.getVal()) << endl;
  // /* A more elegant way */
  // cout << "arg2.getVal() = " << int(arg2) << endl;
  
  ArgumentParser ap("scan_forms", "Form selection", true);
  ap.setCombineSwitches(true);
  
  ap.add_argument("formType", "formType", "Type of form (e.g., 485)", 
		  Argument::VAL_TYPE_STRING, 
		  Argument::DEFAULT_ACTION, vector<string>(), 1);
  ap.add_argument("center", "center", "Processing center (e.g., TSC)", 
		  Argument::VAL_TYPE_STRING, 
		  Argument::DEFAULT_ACTION, vector<string>(), 1);
  ap["center"].setAcceptSet("NSC,TSC");
  cout << ap["center"].getHelpString(80, 24, "center") << endl;

  ap.add_argument("dateRange", "-d", 
		  "Date range (e.g., -d 2013-09-01 2014-01-31)", 
		  Argument::VAL_TYPE_STRING, 
		  Argument::DEFAULT_ACTION, vector<string>(), 2);
  vector<string> defaultDateRange;
  defaultDateRange.push_back("2013-01-01");
  defaultDateRange.push_back("2013-01-31");    
  ap["dateRange"].setDefaultVals(defaultDateRange);

  ap.add_argument("relDateRange", "--rel-date-range", 
		  "Relative date range, to central date (--central-date), inclusive", 
		  Argument::VAL_TYPE_INT, 
		  Argument::DEFAULT_ACTION, vector<string>(), 2);
  vector<int> defRelDateRange(2, 0);
  ap["relDateRange"].setDefaultVals(defRelDateRange);
  vector<string> rdrSets;
  rdrSets.push_back("<=0");
  rdrSets.push_back(">=0");
  ap["relDateRange"].setAcceptSet(rdrSets);
  cout << ap["relDateRange"].getHelpString(80, 24, "relDateRange") << endl;

  vector<string> altSwitches;
  altSwitches.push_back("--verbose");
  ap.add_argument("bVerbose", "-v", "Verbose mode", 
		  Argument::VAL_TYPE_BOOL, 		  
		  Argument::STORE_TRUE, altSwitches, 1);
  cout << ap["bVerbose"].getHelpString(80, 24, "bVerbose") << endl;

  altSwitches.clear();
  altSwitches.push_back("--summary");
  ap.add_argument("bSummary", "-s", "Provide summary of scanned cases", 
  		  Argument::VAL_TYPE_BOOL, 
  		  Argument::STORE_TRUE, altSwitches, 1);

  ap.add_argument("nForms", "--n-forms", "Number of forms to scan", 
		  Argument::VAL_TYPE_INT, 
		  Argument::DEFAULT_ACTION, vector<string>(), 1);
  
  cout << ap["nForms"].getArgName() << endl; // DEBUG
  ap["nForms"].setDefaultVal(100);

  ap.print_help();

  ap.parse_args(argc, argv);

  /* Print set values */
  cout << "\nArgument parsing results:" << endl;
  cout << "\tformType = " << string(ap["formType"]) << endl;
  cout << "\tcenter = " << string(ap["center"]) << endl;
  cout << "\tnForms = " << int(ap["nForms"]) << endl;

  vector<string> dateRange = ap["dateRange"].getStringVals();
  cout << "\tdateRange[0] = " << dateRange[0] << endl;
  cout << "\tdateRange[1] = " << dateRange[1] << endl;

  vector<int> relDateRange = ap["relDateRange"].getIntVals();
  cout << "\trelDateRange[0] = " << relDateRange[0] << endl;
  cout << "\trelDateRnage[1] = " << relDateRange[1] << endl;

  cout << "\tbSummary = " << bool(ap["bSummary"]) << endl;
  cout << "\tbVerbose = " << bool(ap["bVerbose"]) << endl;

  return 0;
}
