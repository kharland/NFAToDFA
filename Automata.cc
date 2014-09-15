/**
 * File: automata.cc
 * Author: Kendal Harland (kendaljharland@gmail.com)
 * 
 * 3d Adjacency matrix implementation of an Automata class
 */

#ifndef _Automata_cc
#define _Automata_cc

#include <iostream>
#include <map>      // for std::map
#include <vector>   // for std::vector
#include <sstream>  // for stringstream

// Represent the automata as a list of states
typedef std::map<char, std::vector<bool> > State;

/**
 * 3d adjacency matrix implementation of an automata class.
 */
class Automata
{
public:

  // Default constructor
  Automata() : states(NULL) {};
  
  // Initializing constructor
  Automata(std::string alphabet, int statec) 
  : states(NULL)
  { 
  	Init(alphabet, statec);
  }

  // Destructor
  virtual ~Automata() { if (states) delete [] states; };

  /**/
  void Init(std::string _alphabet, int statec)
  {
  	numStates = statec;
  	alphabet = _alphabet;
  	states = new State[numStates];

    for (int i = 0; i<numStates; i++)
    	for (int j = 0; j<alphabet.length(); j++)
    		states[i][alphabet.at(j)].assign(numStates, 0);
  };

  /**/
  void AddTrans(int ida, int idb, char c) { states[ida-1][c][idb-1] = 1; }

  /**/
  void DelTrans(int ida, int idb, char c) { states[ida-1][c][idb-1] = 0; }

  std::string Alphabet() const { return alphabet; }

  /**/
  int NumStates() const { return numStates; }

  /**/
  void Print() const
  {
  	// header
  	std::cout << "  \t";
  	for (int i = 0; i<alphabet.length(); i++) 
  		std::cout << alphabet.at(i) << '\t';
    std::cout << '\n';

    // edges
  	for (int i = 0; i<numStates; i++) {
  		std::cout << (i+1) << ":\t";
  		for (int j = 0; j<alphabet.length(); j++) {
  			std::string set = "";
  			for (int k = 0; k<states[i][alphabet.at(j)].size(); k++) {
  				std::stringstream convert;
  			  if (states[i][alphabet.at(j)][k]) {
  			  	convert << k+1;
  		      set += convert.str();
  		      set += ',';
  		    }
  		  }
  		  std::cout << '{' << set.substr(0,set.length()-1) << "}\t";
  	  }
  	  std::cout << '\n';
  	}
  };

private:

	/**
	 * List of States in the automata. Each state is a mapping of an input symbol
	 * to a bit vector (vector<bool>) in which each bit is set to 1 if there
	 * exists an edge between the current state and state_i, else 0. This is much
	 * more space efficient than representing each edge using type `int` (1 bit
	 * to represent an edge vs 32-bits).
	 */ 
  State* states;

  // Input alphabet
  std::string alphabet;

  // Vertex count
  int numStates;
};

#endif//_Automata_cc