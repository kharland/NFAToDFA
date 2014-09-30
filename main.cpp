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
#include <algorithm> //for remove_if
#include "Automata.cc"


/* Forward declarations ------------------------------------------------------*/
void read_final_states( std::vector<int>* );
void read_alphabet( std::string* );
void read_automata( Automata* );
void convert_nfa_dfa( Automata* );


/* Main Implementation -------------------------------------------------------*/
int main( int argc, char *argv[] )
{
    if (argc == 2 && strcmp(argv[1], "--help") == 0) {
        std::cout << "Input Format:\n\n"
        << "Initial State: {3}\n"
        << "Final States:  {12,...}\n"
        << "Total States:  15\n"
        << "State    a     b     E\n"
        << "1      {...} {...} {...}\n"
        << "...    ...\n";
        exit(0);
    }

    int initState=0, totalStates=0;
    std::vector<int> finalStates;
    std::string alphabet;

    scanf("Initial State: {%d}\n", &initState); // read initial state
    read_final_states(&finalStates);            // read final states 
    scanf("Total States: %d\n", &totalStates);  // read total no. of states
    read_alphabet(&alphabet);                   // read alphabet

    // Create NFA in memory and read transition table from stdin
    Automata fa(alphabet, initState-1, totalStates, finalStates);
    read_automata(&fa);

    // Convert to dfa
    convert_nfa_dfa(&fa);
    fa.Print();

    return 0;
}


/* Helpers -------------------------------------------------------------------*/

/**  
 * void read_final_states( std::vector<int> *states );
 * Read the final states of the NFA. We use a vector here to avoid having to
 * make one pass over the data to count the final states before initializing
 * the array and loading the states in a second pass.
 */
void read_final_states( std::vector<int> *states )
{
    char nChar; int nDig;

    scanf("Final States: {");
    do 
    {
        scanf("%d%c", &nDig, &nChar);
        states->push_back(nDig-1);
    } 
    while (nChar != '}');
    scanf("\n");
}

/**
 * void read_alphabet( std::string *alphabet );
 * Read the alphabet to be used for the given NFA
 */
void read_alphabet( std::string *alphabet )
{
    scanf("State");
    getline(std::cin, *alphabet);
    auto newEnd = std::remove_if(alphabet->begin(), alphabet->end(), isspace);
    alphabet->erase(newEnd, alphabet->end());
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
 * Temporary state representation for use during NFA to DFA conversion. We
 * can't create our dfa as an Automata instance and modify it in place because
 * the Automata state transition table is not resizable and we don't know how
 * many states will be required until the conversion is complete.  Given a 
 * different implementation style, a different approach might be more space
 * efficient.
 */
struct TempState {
    std::vector<int> anchors; // original nfa states of this dfa state
    std::map<char, int> next; // mapping of alphabet symbols to next dfa state
};


/**
 * Print formatted output for a template vector.
 */
template<typename T>
void _print_vec(std::vector<T> v) 
{
	std::string buf = "";
	for (int i = 0; i<v.size(); i++) {
		std::stringstream convert;
		convert << v[i]+1;
		buf += convert.str() + ',';
	}
	std::cout << '{' << buf.substr(0, buf.length()-1) << '}';
}


/**
 * Find the dfa-converted state needle in the list of dfa-converted-states 
 * haystack.
 */
int _find_state( std::vector<int> needle, std::vector<TempState> haystack ) 
{
	std::sort(needle.begin(), needle.end());
	for (int i = 0; i<haystack.size(); i++) {
		std::vector<int> hsi = haystack[i].anchors;
		std::sort(hsi.begin(), hsi.end());
		if (needle == hsi) {
			return i;
        }
	}
	return -1;
}


/**
 * Convert an nfa to a dfa
 */
void convert_nfa_dfa( Automata* nfa ) 
{
    std::vector<bool> marked;
    std::vector<TempState> dfaStates;

    /**
     * -- Algorithm --
     * 
     * while unmarked states exist:
     *    if empty ( dfaStates ):
     *       curTempState.anchors = e-closure(nfa.Start)
     *       dfaStates.addState(curTempState)
     *       markedState = curTempState
     *    else:
     *       foreach a in Alphabet:
     *          newState_i.anchors = e-closure(a-closure(markedState))
     *          if not dfaStates.contains(newState_i):
     *             dfaStates.addState(newState_i)
     *             dfaStates.addTrans(markedState, newstate_i)
     *          else:
     *             dfaStates.addTrans(markedState, dfaStates.get(newState_i))
     *     
     *       if state was not last:
     *          mark dfa.nextUnmarkedState
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
            std::cout << " = " << curMarked+1 << "\n";
            std::cout << "\nMark " << curMarked+1 << std::endl;
            // -- End output check --
        } 
        else
        {
            for (int i = 0; i < nfa->Alphabet().length()-1; i++) {
                TempState cmSymClosure, newState;
                cmSymClosure.anchors = nfa->SymClosure(dfaStates[curMarked].anchors, 
                                                       nfa->Alphabet(i)),
                newState.anchors = nfa->EClosure({cmSymClosure.anchors});

                // make sure the new state doens't already exist
                int spos = _find_state(newState.anchors, dfaStates);
                if (newState.anchors.size() > 0 && spos < 0) {
                    dfaStates.push_back(newState);
                    marked.push_back(false);
                    dfaStates[curMarked].next[nfa->Alphabet(i)] = dfaStates.size()-1;
                } else {
                    dfaStates[curMarked].next[nfa->Alphabet(i)] = spos;
                }

                // -- Output check --
                if (newState.anchors.size() != 0) {                    
                    _print_vec(dfaStates[curMarked].anchors);
                    std::cout << "--" << nfa->Alphabet(i) << "--> ";
                    _print_vec(cmSymClosure.anchors);
                    std::cout << "\nE-closure"; 
                    _print_vec(cmSymClosure.anchors);
                    std::cout << "= ";
                    _print_vec(newState.anchors);
                    std::cout << " = " << (spos >= 0 ? spos+1 : dfaStates.size()) << "\n";
                }
                // -- End output check --
            }
            
            marked[curMarked++] = true;

            // -- Output check
            if (curMarked < dfaStates.size()) {
                std::cout << "\nMark " << curMarked+1 << std::endl;
            }
            // -- End output check --
        }
    } 
    while (std::find(marked.begin(),marked.end(), false) != marked.end());

    // Trim Epsilon from alphabet
    std::string dfaAlpha = nfa->Alphabet().substr(0,nfa->Alphabet().length()-1);
    
    // Load DFA
    int dfaStart = 0;
    std::vector<int> dfaFinalStates;
    for (int i = 0; i<nfa->FinalStates().size(); i++) {
        for (int j = 0; j<dfaStates.size(); j++) {
            // auto: because typing iterator types sucks sometimes
            auto start = dfaStates[j].anchors.begin(), 
                 end = dfaStates[j].anchors.end();
            if (find(start, end, nfa->FinalStates()[i]) != end) {
                dfaFinalStates.push_back(j);
            }
        }
    }

    // Add transitions
    Automata dfa(dfaAlpha, dfaStart, dfaStates.size(), dfaFinalStates);
    for (int i = 0; i<dfaStates.size(); i++) {
        for (int j = 0; j<dfa.Alphabet().length(); j++) {
            if (dfaStates[i].next[dfa.Alphabet().at(j)] >= 0)
                dfa.AddTrans(i, dfaStates[i].next[dfa.Alphabet().at(j)], dfa.Alphabet().at(j));
        }
    }
    *nfa = dfa;
}