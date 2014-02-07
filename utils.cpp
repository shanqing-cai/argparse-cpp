#include "utils.h"

using namespace std;

/* String utility functions */
/* Find all instances of a sub-string in a string */
std::vector<size_t> string_find(std::string s, std::string subs){
  std::vector<size_t> matches;

  size_t pos = 0;

  while ((pos = s.find(subs, pos)) != std::string::npos) {
    // cout << "pos = " << pos << endl;
    matches.push_back(pos);
    pos ++;
  }

  return matches;

}

/* Split string by any delimiter string */
std::vector<std::string> split_string(std::string s, std::string subs) {
  std::vector<std::string> tokens;
  std::vector<size_t> dp; /* Delimiter positions */

  size_t dl = subs.length(); /* Delimiter length */
  std::string str;
  size_t p = 0;

  // DEBUG
  // cout << "split_string: dl = " << dl << endl;

  if (dl == 0) {
    tokens.push_back(s);
  }
  else {
    dp = string_find(s, subs);

    if (dp.size() == 0) {
      tokens.push_back(s);
    }
    else {
      for (size_t i0 = 0; i0 <= dp.size(); i0++) {
	// DEBUG
	// cout << "split_sring: i0 = " << i0 << "; p = " << p << endl;

	if (i0 < dp.size())
	  str = s.substr(p, dp[i0] - p);
	else
	  str = s.substr(p, s.length() - p);
	
	p = dp[i0] + dl;

	tokens.push_back(str);
      }
    }
  }

  return tokens;
}

/* Interpreting boolean strings. 
 Return values: -1: uninterpretable string
                 0: false
		 1: true */
int interpret_bool_string(const std::string inStr) {
  if ( inStr == "true" || inStr == "True" || inStr == "TRUE" 
       || inStr == "T" || inStr == "t" || inStr == "1" )
    return 1;
  else if ( inStr == "false" || inStr == "False" || inStr == "FALSE" 
	    || inStr == "F" || inStr == "f" ||  inStr == "0")
    return 0;
  else
    return -1;
}

/* Test if a string is a number, 
   e.g. -1, 0.0, 3, 3.14, .025.
   Limitatin cannot hanle scientific notations such as 1e2. */
bool is_string_numeric(const std::string str) {
  if ( str.empty() )
    return false;

  if ( str == "-" )
    return false;

  /* Count the number of '-'. It should appear only in the first place */
  for (unsigned int i = 1; i < str.size(); ++i)
    if ( str[i] == '-' )
      return false;
  
  /* Scan for illegal characters */
  /* Count the number of '.'. It should be <= 1 */
  int nDot = 0;
  for (unsigned int i = 0; i < str.size(); ++i) {
    if ( str[i] == '-') 
      continue;

    if ( str[i] == '.') {
      nDot++;
      continue;
    }

    if ( str[i] < '0' || str[i] > '9' )
      return false;
  }

  if ( nDot > 1 )
    return false;

  
  return true;

}


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
		       const int lw) {
  std::istringstream iss(inStr);

  /* Sanity check */
  if ( w <= 0 || lw < 0 || lw >= w ) {
    std::cerr << "ERROR occurred during lineFormat()" << std::endl;
    return string("");
  }

  std::string remStr; /* Remnant string from the previous line */
  
  std::ostringstream oss;
  int state = 0; 
  /* 0 - new line; 
     1 - left white space filled, filling words */
  int lc = 0;
     
  std::string word;
  while ( iss.good() || !word.empty() ) {
    if ( word.empty() ) {
      iss >> word;
    }

    if ( state == 0 ) {
      oss << std::string(lw, ' ');
      lc += lw;

      state++;
    }
    
    if ( lc + static_cast<int>(word.size()) <= w ) {
      oss << word;
      lc += word.size();
      word.clear();
      
      if ( lc < w ) {
	oss << " ";
	lc++;
      }
    }
    else {
      if ( static_cast<int>(word.size()) <= w - lw ) { 
	/* No need to break up the word */
	oss << "\n";

	state = 0;
	lc = 0;
      }
      else { 
	/* Need to break up the word */
	oss << word.substr(0, w - lc) << "\n";
	word = word.substr(w - lc, word.size() - w + lc);

	/* DEBUG */
	cout << "word after break up = " << word << endl;
	
	state = 0;
	lc = 0;	
      }
    }
        

  }
  
  return oss.str();
}

