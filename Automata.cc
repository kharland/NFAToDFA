/**
 * File: automata.cc
 * Author: Kendal Harland (kendaljharland@gmail.com)
 * 
 * 3d Adjacency matrix implementation of an Automata class
 */

#ifndef _Automata_cc
#define _Automata_cc

#include <iostream>
#include <map>       // for std::map
#include <vector>    // for std::vector
#include <sstream>   // for stringstream
#include <algorithm> // for std::find

// Represent the automata as a list of states
typedef std::map<char, std::vector<bool> > State;

/**
 * 3d adjacency matrix implementation of an automata class. Each state stored 
 */
class Automata
{
public:

  // Default constructor
  Automata() {};
  
  // Initializing constructor
  Automata( std::string alphabet, int start, int numStates )
  { 
    Init(alphabet,start,numStates);
  }

  // Destructor
  virtual ~Automata() {};

  /**/
  void Init( std::string _alphabet, int _start, int _numStates )
  {
    numStates = _numStates;
    alphabet = _alphabet;
    start = _start;
    states.assign(numStates, State());

    for (int i = 0; i<numStates; i++)
      for (int j = 0; j<alphabet.length(); j++)
        states[i][alphabet.at(j)].assign(numStates, 0);
  };

  /**/
  virtual void AddTrans(int ida, int idb, char c) { states[ida][c][idb] = 1; }

  /**/
  virtual void DelTrans(int ida, int idb, char c) { states[ida][c][idb] = 0; }

  /**/
  virtual std::vector<int> SymClosure(std::vector<int> anchors, char a) 
  {
    std::vector<int> dest;

    for (int i = 0; i < anchors.size(); i++) {
      std::vector<bool> d = states[anchors[i]][a];
      for (int j = 0; j < d.size(); j++) {
        if (d[j] && std::find(dest.begin(),dest.end(), j) == dest.end()) 
          dest.push_back(j);
      }
    }
    return dest;
  }

  /**/
  virtual std::vector<int> EClosure(std::vector<int> anchors)   
  {
    bool added = false;
    std::vector<int> dest(anchors);
    
    for (int i = 0; i < anchors.size(); i++) {
      std::vector<bool> d = states[anchors[i]]['E'];
      for (int j = 0; j < d.size(); j++) {
        if (d[j] && std::find(dest.begin(),dest.end(), j) == dest.end()) {
          dest.push_back(j);
          added = true;
        }
      }
    }

    if (!added) return dest;
    return EClosure(dest);
  }

  /**/
  int Start() const { return start; }

  /**/
  std::string Alphabet() const { return alphabet; }

  /**/
  int NumStates() const { return numStates; }

  /**/
  void Print()
  {
    // header
    std::cout << " \t";
    for (int i = 0; i<alphabet.length(); i++) 
      std::cout << alphabet.at(i) << '\t';
    std::cout << '\n';

    // transition table
    for (int i = 0; i<numStates; i++) {
      std::cout << i+1 << ":\t";
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
  std::vector<State> states;

  // Input alphabet
  std::string alphabet;

  // Vertex count
  int numStates;

  // Start state
  int start;
};

#endif//_Automata_cc