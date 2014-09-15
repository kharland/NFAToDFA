/**
 * File:   NFA to DFA Converter
 * Author: Kendal Harland (kendaljharland@gmail.com)
 *
 * TODO:
 * [x] Parse headers
 * [ ] Parse initial nfa
 * [ ] Implement nfa to dfa conversion
 * [ ] Ensure 'E' transition specified
 * [ ] Verify character parsing makes correct checks in this file at all stages
 * [ ] Error check for duplicate symbol transitions
 * [ ] Enable states to be listed in any order in input file
 * [ ] Handle all edge-cases in input parsing.
 * -----------------------------------------------------------------------------
 */
#include <cstdlib>
#include <iostream>
#include <string.h>
#include <sstream>
#include <vector>
#include "Automata.cc"

/* Forward declarations ------------------------------------------------------*/
void read_init_state( int* );
void read_final_states( std::vector<int>* );
void read_total_states( int* );
void read_alphabet( std::string* );
void read_automata( Automata* );

/* Main Implementation -------------------------------------------------------*/
int main( int argc, char *argv[] )
{
	if (argc == 2 && strcmp(argv[1], "h") == 0) {
		std::cout << "Input Format:\n\n"
        << "Initial State: {1}\n"
        << "Final States:  {11,...}\n"
        << "Total States:  11\n"
        << "State    a     b     E\n"
        << "1      {...} {...} {...}\n"
        << "...\n\n";
	}

	int initState=0, totalStates=0;
	std::vector<int> finalStates;
  std::string alphabet;

	// Read Initial State, Final State and Total States
  read_init_state(&initState);
  read_final_states(&finalStates);
  read_total_states(&totalStates);
  // Get alphabet
  read_alphabet(&alphabet);
  Automata nfa(alphabet, totalStates);
  // Get inital edges
  read_automata(&nfa);
  nfa.Print();

	return 0;
}

/* Helpers -------------------------------------------------------------------*/

/** 
 * void read_init( int *state );
 * Read the start state of the NFA.
 */
void read_init_state( int *state )
{
	int startSeq=0;
	std::string line, buf;

	std::getline(std::cin, line);
	if ((startSeq = line.find('{')) != std::string::npos) {
	  char c = line.at(++startSeq);
      while(c != '}') {
      	if (iswspace(c) || isalnum(c)) {
      	  buf += line.at(startSeq);
          c = line.at(++startSeq);
        } else {
          std::cerr << "Invalid char " << c << " found parsing Initial State";
          exit(-1);
        }
      }
      *state = atoi(buf.c_str());
  } else {
	  std::cerr << "Malformed input while reading Initial State\n";
	  exit(-1);
	}
}

/**  
 * void read_final_states( std::vector<int> *states );
 * Read the final states of the NFA. We use a vector here to avoid having to
 * make one pass over the data to count the final states before initializing
 * the array and loading the states in a second pass.
 */
void read_final_states( std::vector<int> *states )
{
    int startSeq=0;
    std::string line, buf;

    std::getline(std::cin, line);
    if ((startSeq = line.find('{')) != std::string::npos) {
      char c = line.at(++startSeq);
      while(1) {
      	if (c == ',') {
            states->push_back(atoi(buf.c_str()));
            buf = "";
        } else if (c == '}') {
        	states->push_back(atoi(buf.c_str()));
            buf = "";
            break;
        } else {
      	    buf += line.at(startSeq);	
      	}
      	c = line.at(++startSeq);
      }
    } else {
			std::cerr << "Malformed input while reading Final States\n";
	    exit(-1);
    }
}


/**
 * void read_total_states( int *statec );
 * Read the total # of states in the NFA
 */
void read_total_states( int *statec )
{
	int startSeq=0;
	std::string line, buf;

	std::getline(std::cin, line);
	if ((startSeq = line.find(':')) != std::string::npos)
      *statec = atoi(line.substr(startSeq+1, line.length()-startSeq-1).c_str());

    if (*statec == 0) {
		std::cerr << "Malformed input while reading Initial State\n";
		exit(-1);
	}
}

/**
 * void read_alphabet( std::string *alphabet );
 * Read the alphabet to be used for the given NFA
 */
void read_alphabet( std::string *alphabet )
{
	std::string line, nextSym; // declare nextSym as string so ss>> ignores '\n'
	std::cin.ignore(6,' ');    // discard 'States[ ]+'
	getline(std::cin, line);
	std::stringstream ss(line);
	 
  while (!ss.eof()) {
  	ss >> nextSym;
  	*alphabet += nextSym;
  }
}

/**
 * void read_automata( Automata *nfa );
 */
void read_automata( Automata *nfa ) 
{
	std::string line, buf;
	std::vector<std::string> states;
  int lastBrack;
  char c;

  for (int i = 0; i<nfa->NumStates(); i++) {
    getline(std::cin, line);
    lastBrack=0;

    // Parse each state
    for (int j = 0; j<nfa->Alphabet().length(); j++) {
      if ((lastBrack = line.find('{', lastBrack)) == std::string::npos) {
      	std::cerr << "Malformed input in state list\n";
      	exit(-1);
      }
      
      // Parse all edges for each input symbol from this state
      c = line.at(++lastBrack);
      while (1) {
        if (c == ',') {
          nfa->AddTrans(i+1, atoi(buf.c_str()), nfa->Alphabet().at(j));
          buf = "";
        } else if (c == '}') {
        	if (buf.length() > 0)
        	  nfa->AddTrans(i+1, atoi(buf.c_str()), nfa->Alphabet().at(j));
          buf = "";
          break;
        } else if (isalnum(c)) {
        	buf += c;
        } else if (iswspace(c)) {;}
        else { /* error */ }
        c = line.at(++lastBrack);
      }
    }
  }

}