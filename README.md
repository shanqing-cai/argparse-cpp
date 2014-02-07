
============
argparse-cpp
============

   argparse-cpp is a tool for parsing command line arguments. 
   It generally follows the style of Python argparse.

   Features: 
   * Print-out of formatted help information
   * Support Boolean, integer, float and string types 
   * Support multiple arguments per optional switch
   * Can set default values for optional switches
   * Can set acceptance ranges for string and numerical values (e.g., >0<=100.3)
   * Can combine multiple Boolean switches (e.g., -c, -d --> -cd)


   See argparse_example.cpp for example usages.
   

   Author: Shanqing Cai (shanqing.cai@gmail.com)
   Date: Feb. 2014


   Example help print-out:
   
   Usage: argparse_example [--alliance-corp alliCorp1 alliCorp2] [-a armorThick] 
                        [-c cloak] [-d droneBay] [-s speed]  class shipName quantity 

Create new space ship(s) 

Positional arguments: 
  class                 [string] Ship class (e.g., frigate, titan) 
                        Range:  
                          class: frigate,destroyer,cruiser,battlecruiser,battles
                          hip,carrier,transporter,recon 
  quantity              [integer] Number of ships 
                        Range:  
                          quantity: >0 
  shipName              [string] Name of the ship (e.g., Enterprise) 

Optional arguments: 
  --alliance-corp alliCorp1 alliCorp2
                        [string x 2] Assign to alliance and corporation 
  -a armorThick   (--armor-thick)
                        [float] Armor thickness (cm) 
                        Range:  
                          armorThick: >10<100,>210<300 
  -c cloak   (--cloak) [Boolean] Enable invisibility cloak (Default: FALSE) 
  -d droneBay   (--drone-bay)
                        [Boolean] Equip drone bay (Default: FALSE) 
  -s speed   (--speed) [float] Ship speed (m/s) 
                        Range:  
                          speed: >0<299792458 
                        Default:  
                          speed = 1000

