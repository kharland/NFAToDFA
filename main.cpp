/**
 * File:   NFA to DFA Converter
 * Author: Kendal Harland (kendaljharland@gmail.com)
 *
 * TODO:
 * [x] Parse headers
 * [x] Parse initial nfa
 * [x] Implement nfa to dfa conversion
 * [ ] Recover final states and start state after conversion
 * [ ] Improve management of memory after dfa conversion (replacment of nfa)
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
void convert_nfa_dfa( Automata* );


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

    // Read Initial State, Final State and Total States from stdin
    read_init_state(&initState);
    read_final_states(&finalStates);
    read_total_states(&totalStates);

    // Get alphabet from stdin
    read_alphabet(&alphabet);

    // Create NFA in memory and read transition table from stdin
    Automata fa(alphabet, initState, totalStates);
    read_automata(&fa);

    // Convert to dfa
    convert_nfa_dfa(&fa);
    fa.Print();

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
        *state = atoi(buf.c_str())-1;
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
                states->push_back(atoi(buf.c_str())-1);
                buf = "";
            } else if (c == '}') {
                states->push_back(atoi(buf.c_str())-1);
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
 * Read the total number of states in the NFA
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
 * Read and load the transition table for this NFA into memory
 */
void read_automata( Automata *nfa ) 
{
    std::string line, buf;
    std::vector<std::string> states;
    int lastBrack;
    char c;

    for (int i = 0; i<nfa->NumStates(); i++) {
        getline(std::cin, line);
        lastBrack = 0;

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
                    nfa->AddTrans(i, atoi(buf.c_str())-1, nfa->Alphabet().at(j));
                    buf = "";
                } else if (c == '}') {
                    if (buf.length() > 0)
                        nfa->AddTrans(i, atoi(buf.c_str())-1, nfa->Alphabet().at(j));
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


/**
 * struct TempState
 * temporary state representation for using during NFA to DFA conversion.
 * we can't create our dfa as an Automata instance and modify it inplace 
 * because the Automata state transition table is not resizable and we 
 * don't know how many states will be required until the conversion is
 * complete.
 */
struct TempState 
{
    std::vector<int> anchors;
    std::map<char, int> next;
};


/**/
template<typename T>
void _print_vec(std::vector<T> v) 
{
	std::string buf = "";
	for (int i = 0; i<v.size(); i++) {
		std::stringstream convert;
		convert << v[i];
		buf += convert.str();
		buf += ',';
	}
	std::cout << '{' << buf.substr(0, buf.length()-1) << '}';
}


/**/
int _find_state( std::vector<int> needle, std::vector<TempState> haystack ) 
{
	std::sort(needle.begin(), needle.end());
	for (int i = 0; i<haystack.size(); i++) {
		std::vector<int> hsi = haystack[i].anchors;
		std::sort(hsi.begin(), hsi.end());
		if (needle == hsi)
			return i;
	}
	return -1;
}


/**/
void convert_nfa_dfa( Automata* nfa ) 
{
    std::vector<bool> marked;
    std::vector<TempState> dfaStates;

    /**
     * -- Algorithm --
     * 
     * while there are unmarked states:
     *     if dfaStates is empty:
     *         curTempState->anchors = e-closure of nfa->Start
     *         addState curTempState to dfaStates
     *         mark curTempState
     *     else:
     *         for each symbol a in Alphabet:
     *         newState_i->anchors = e-closure of a-closure of marked state
     *         if newState_i is not in dfaStates as state_i:
     *             addState newState_i to dfaStates
     *             addTrans from marked state to newstate_i in dfaStates
     *         else:
     *             addTrans from marked state to state_i in dfaStates
     *     
     *         if state was not last:
     *             mark dfa.firstUnmarkedState
     */
    int curMarked = -1;
    do 
    {
        if (dfaStates.size() == 0) {
            TempState DFAStartState;
            DFAStartState.anchors = nfa->EClosure({nfa->Start()});
            dfaStates.push_back(DFAStartState);
            marked.push_back(false);
            curMarked = 0;

            // -- Output check --
            std::cout << "E-closure(IO) = "; _print_vec(DFAStartState.anchors);
            std::cout << " = " << curMarked << "\n";
            std::cout << "\nMark " << curMarked << std::endl;
            // -- End output check --
        } 
        else
        {
            for (int i = 0; i < nfa->Alphabet().length()-1; i++) {
                TempState cmSymClosure, newState;
                cmSymClosure.anchors = nfa->SymClosure(dfaStates[curMarked].anchors, 
                                                       nfa->Alphabet().at(i)),
                newState.anchors = nfa->EClosure({cmSymClosure.anchors});

                int spos = _find_state(newState.anchors, dfaStates);
                if (newState.anchors.size() > 0 && spos < 0) {
                    dfaStates.push_back(newState);
                    marked.push_back(false);
                    dfaStates[curMarked].next[nfa->Alphabet().at(i)] = dfaStates.size()-1;
                } else {
                    dfaStates[curMarked].next[nfa->Alphabet().at(i)] = spos;
                }
                // -- Output check --
                if (newState.anchors.size() != 0) {
                    _print_vec(dfaStates[curMarked].anchors);
                    std::cout << "--" << nfa->Alphabet().at(i) << "--> ";
                    _print_vec(cmSymClosure.anchors);
                    std::cout << "\nE-closure"; 
                    _print_vec(cmSymClosure.anchors);
                    std::cout << "= ";
                    _print_vec(newState.anchors);
                    std::cout << " = " << dfaStates.size()-1 << "\n"	;
                }// -- End output check --
            }
            
            marked[curMarked++] = true;
            // -- Output check
            if (curMarked < dfaStates.size()) {
                std::cout << "\nMark " << curMarked << std::endl;
            } // -- End output check --
            
        }
    } 
    while (std::find(marked.begin(),marked.end(),false) != marked.end());

    // Create dfa and trim Epsilon from alphabet
    std::string dfaAlpha = nfa->Alphabet().substr(0,nfa->Alphabet().length()-1);
    int dfaStart = 0;

    Automata dfa(dfaAlpha, dfaStart, dfaStates.size());
    for (int i = 0; i<dfaStates.size(); i++) {
        for (int j = 0; j<dfa.Alphabet().length(); j++) {
            if (dfaStates[i].next[dfa.Alphabet().at(j)] >= 0)
                dfa.AddTrans(i, dfaStates[i].next[dfa.Alphabet().at(j)], dfa.Alphabet().at(j));
        }
    }

    *nfa = dfa;
}