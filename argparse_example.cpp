#include <iostream>
#include <vector>

#include "argparse.h"

/* Commands for testing:

Show help:
   ./argparse_example

Correct calling:
   ./argparse_example battlecruiser Excaliber 3
   ./argparse_example battlecruiser Excaliber 3 -s 3000
   ./argparse_example battlecruiser Excaliber 3 --speed 3000
   ./argparse_example battlecruiser Excaliber 3 --speed 3000 -c 
   ./argparse_example battlecruiser Excaliber 3 --speed 3000 -c --drone-bay
   ./argparse_example battlecruiser Excaliber 3 --speed 3000 -cd
   ./argparse_example battlecruiser Excaliber 3 --speed 3000 -cd --armor-thick 80  
   ./argparse_example battlecruiser Excaliber 3 --speed 3000 -cd --armor-thick 280 --alliance-corp UFP AmaDyne


To demostrate errors:
   ./argparse_example battlecruiser Excaliber -3 
   (wrong quantity)
   
   ./argparse_example battlecruiser Excaliber 3 --s 3000 
   (wrong option switch)
   
   ./argparse_example battlecruiser Excaliber 3 -s 400000000 
   (speed too high)
   
   ./argparse_example battlecruiser Excaliber 3 --speed 3000 -cd --cloak
   (repeated setting of a switch)
   
   ./argparse_example battlecruiser Excaliber 3 --speed 3000 -cd -z
   (unrecognized switch -z)

   ./argparse_example battlecruiser Excaliber 3 --speed 3000 -cd --armor-thick 160
   (float value outside acceptance set)

*/

using namespace std;


int main(int argc, char ** argv) {
  ArgumentParser ap(string("argparse_example"), 
		    string("Create new space ship(s)"), 
		    true);

  /* Create 1st positional argument */
  ap.add_argument("class", "class", 
		  "Ship class (e.g., frigate, titan)", 
		  Argument::VAL_TYPE_STRING);
  /* Supply the acceptance set, i.e., range of valid values, to ship class */
  ap["class"].setAcceptSet("frigate,destroyer,cruiser,battlecruiser,"
			   "battleship,carrier,transporter,recon");

  /* Create 2nd positoinal argument */
  ap.add_argument("shipName", "shipName", 
		  "Name of the ship (e.g., Enterprise)", 
		  Argument::VAL_TYPE_STRING);

  /* Add 3rd positional argument: integer type */
  ap.add_argument("quantity", "quantity", 
		  "Number of ships",
		  Argument::VAL_TYPE_INT);

  /* Add 1st optoinal argumennt: ship speed */
  ap.add_argument("speed", "-s", 
		  "Ship speed (m/s)", 
		  Argument::VAL_TYPE_FLOAT, Argument::DEFAULT_ACTION, 
		  vector<string>(1, "--speed"), 1);
  /* Set default value */
  ap["speed"].setDefaultVal(1000.0);
  ap["speed"].setAcceptSet(">0<299792458");

  /* Add 2nd optional argument: cloaking ability: Boolean */
  ap.add_argument("cloak", "-c", 
		  "Enable invisibility cloak", 
		  Argument::VAL_TYPE_BOOL, Argument::STORE_TRUE, 
		  vector<string>(1, "--cloak"), 1);

  ap.add_argument("droneBay", "-d", 
		  "Equip drone bay",
		  Argument::VAL_TYPE_BOOL, Argument::STORE_TRUE, 
		  vector<string>(1, "--drone-bay"), 1);

  /* Add 3rd optional argument: armor thickness 
                                (float within two discontinuous ranges) */
  ap.add_argument("armorThick", "-a", 
		  "Armor thickness (cm)", 
		  Argument::VAL_TYPE_FLOAT, Argument::DEFAULT_ACTION, 
		  vector<string>(1, "--armor-thick"), 1);
  ap["armorThick"].setAcceptSet(">10<100,>210<300");
  ap["quantity"].setAcceptSet(">0");
  

  /* Add 4rd optional argument: alliance and coroporation assignment
     Demonstrates multiple arg values */
  ap.add_argument("alliCorp", "--alliance-corp", 
		  "Assign to alliance and corporation", 
		  Argument::VAL_TYPE_STRING, Argument::DEFAULT_ACTION, 
		  vector<string>(), 2);
		  
  if ( argc <= 1) {
    ap.print_help();
    return 0;
  }
		    
  ap.parse_args(argc, argv);

  cout << "Input information: " << endl;
  cout << "\tShip class = " << string(ap["class"]) << endl;
  cout << "\tShip name = " << string(ap["shipName"]) << endl;
  cout << "\tQuantity = " << int(ap["quantity"]) << endl;
  cout << "\tMax speed = " << float(ap["speed"]) << " m/s" << endl;
  cout << "\tInvisibility cloak = " << bool(ap["cloak"]) << endl;
  cout << "\tDrone bay = " << bool(ap["droneBay"]) << endl;
  if ( ap["armorThick"].set() )
    cout << "\tArmor thickness = " << float(ap["armorThick"]) << " cm" <<endl;

  if ( ap["alliCorp"].set() ) {
    vector<string> alliCorp = ap["alliCorp"].getStringVals();
      cout << "\tAssigne to\n\t\talliance: " << alliCorp[0] << endl;
      cout << "\t\tcorporation: " << alliCorp[1] << endl;
    

  }

  return 0;
}
