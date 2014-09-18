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
    Automata( std::string alphabet, int start, int numStates, 
              std::vector<int> finalStates )
    { 
        Init(alphabet, start, numStates, finalStates);
    }

    // Destructor
    virtual ~Automata() {};

    /**
     * void Init
     * usage: fa.Init(alphabet, startState, numStates);
     * -------------------------------------------------------------------------
     * Initialize the automata with an alphabet, start state and total number of
     * states.
     */
    void Init( std::string _alphabet, int _start, int _numStates, 
               std::vector<int> _finalStates )
    {
        numStates = _numStates;
        finalStates = _finalStates;
        alphabet = _alphabet;
        start = _start;
        states.assign(numStates, State());

        for (int i = 0; i<numStates; i++)
            for (int j = 0; j<alphabet.length(); j++)
                states[i][alphabet.at(j)].assign(numStates, 0);
    };

    /**
     * void AddTrans
     * usage: fa.AddTrans(ida, idb, c);
     * -------------------------------------------------------------------------
     * Add a transition from state with id ida to state with id idb on symbol c.
     */
    virtual void AddTrans(int ida, int idb, char c) { states[ida][c][idb] = 1; }

    /**
     * void DelTrans
     * usage: fa.AddTrans(ida, idb, c);
     * -------------------------------------------------------------------------
     * Remove the transition from state with id ida to state with id idb on 
     * symbol c.
     */
    virtual void DelTrans(int ida, int idb, char c) { states[ida][c][idb] = 0; }

    /**
     * vector<int> SymClosure
     * usage: vector<int> toStates = fa.SymClosure(fromStates, sym);
     * ------------------------------------------------------------------------- 
     * Get the complete list of transition states reachable from fromStates on 
     * the alphabet symbol sym.
     */
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

    /**
     * vector<int> EClosure
     * usage: toStates = fa.EClosure(fromStates, sym);
     * -------------------------------------------------------------------------
     * Get the complete list of transition states reachable from fromStates on 
     * the epsilon transition.
     */
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

    /**
     * int Start
     * usage: startState = fa.Start();
     * -------------------------------------------------------------------------
     * Get the id of the start state of the finite acceptor.
     */
    int Start() const { return start; }

    /**
     * string Alphabet
     * usage: alphabet = fa.Alphabet();
     * -------------------------------------------------------------------------
     * returns a copy of the input alphabet of the finite acceptor.
     */
    std::string Alphabet() const { return alphabet; }

    /**
     * string Alphabet
     * usage: alphabet = fa.Alphabet(i);
     * -------------------------------------------------------------------------
     * returns a copy of the input alphabet character at position i.
     */
    char Alphabet(int i) const { return alphabet.at(i); }

    /**
     * int NumStates
     * usage: numStates = fa.NumStates();
     * -------------------------------------------------------------------------
     * Get the total number of states in the finite acceptor.
     */
    int NumStates() const { return numStates; }

    /** 
     * vector<int> FinalStates()
     * usage: finalStates = fa.FinalStates();
     * -------------------------------------------------------------------------
     * Get a list of the final states in this automata.
     */
    std::vector<int> FinalStates() const { return finalStates; }

    /**
     * void Print
     * usage: fa.Print();
     * -------------------------------------------------------------------------
     * Print the finite acceptor in a formatted style to stdout.
     */
    void Print()
    {
        // start and final states
        std::cout << "\nInitial State: {" << start+1 << "}\n";
        std::cout << "Final States: ";
        std::string buf = "";
        for (int i = 0; i<finalStates.size(); i++) {
            std::stringstream convert;
            convert << finalStates[i]+1;
            buf += convert.str();
            buf += ',';
        }
        std::cout << '{' << buf.substr(0, buf.length()-1) << "}\n";

        // header
        std::cout << "State\t";
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
    std::vector<int> finalStates;

    // Input alphabet
    std::string alphabet;

    // Vertex count
    int numStates;

    // Start state
    int start;
};

#endif//_Automata_cc