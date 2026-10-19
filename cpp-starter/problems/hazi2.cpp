
#include <algorithm>
#include <fstream>
#include <map>
#include <queue>
#include <set>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#include "hazi2.hpp"

using namespace std;

struct Route {
    string start;
    char letter;
    string end;
};

void Hazi2Problem::initialize_parser(cxxopts::Options& options) {
    options.add_options()
        ("det", "Convert automaton to deterministic automaton");
}

bool Hazi2Problem::is_chosen_problem(
    const cxxopts::ParseResult& args
) {
    return args.count("det") > 0;
}

int Hazi2Problem::run(const cxxopts::ParseResult& args) {
    const string inputFilename = args["input"].as<string>();
    const string outputFilename = args["output"].as<string>();

    ifstream input(inputFilename);
    if (!input.is_open()) throw runtime_error("Cannot open input file: " + inputFilename);
    ofstream output(outputFilename);
    if (!output.is_open()) throw runtime_error("Cannot open output file: " + outputFilename);

    //Beolvasas
    string line;
    vector<string> states;
    vector<char> alphabet;
    set<string> startStates;
    set<string> finalStates;
    vector<Route> routes;

    auto readWords = [](const string& text) {
        istringstream stream(text);
        vector<string> words;
        string word;

        while (stream >> word) words.push_back(word);

        return words;
    };

    if (!getline(input, line)) throw runtime_error("Empty input file");
    states = readWords(line);

    if (!getline(input, line)) throw runtime_error("Missing alphabet");

    for (const string& symbol : readWords(line)) {
        if (symbol.size() != 1)throw runtime_error("Alphabet symbols must be single characters");
        alphabet.push_back(symbol[0]);
    }

    if (!getline(input, line)) throw runtime_error("Missing start state");

    for (const string& state : readWords(line)) startStates.insert(state);

    if (!getline(input, line)) throw runtime_error("Missing final states");

    for (const string& state : readWords(line)) finalStates.insert(state);

    while (getline(input, line)) {
        vector<string> words = readWords(line);

        if (words.empty()) continue;

        if (words.size() != 3 || words[1].size() != 1) {
            throw runtime_error("Invalid transition: " + line);
        }

        routes.push_back({words[0], words[1][0], words[2]});
    }

    if (startStates.empty()) {
        throw runtime_error("The automaton has no start state");
    }

    // Az állapotok és az ábécé ellenőrzése.
    set<string> knownStates(states.begin(), states.end());
    set<char> knownSymbols(alphabet.begin(), alphabet.end());

    for (const string& state : startStates) {
        if (!knownStates.count(state)) {
            throw runtime_error("Unknown start state: " + state);
        }
    }

    for (const string& state : finalStates) {
        if (!knownStates.count(state)) {
            throw runtime_error("Unknown final state: " + state);
        }
    }

    for (const Route& route : routes) {
        if (!knownStates.count(route.start) ||
            !knownStates.count(route.end)) {
            throw runtime_error("Transition contains an unknown state");
        }

        if (!knownSymbols.count(route.letter)) {
            throw runtime_error("Transition contains an unknown symbol");
        }
    }

    map<pair<string, char>, set<string>> transitions;

    for (const Route& route : routes) {
        transitions[{route.start, route.letter}].insert(route.end);
    }

    // Részhalmaz
    using StateSet = set<string>;

    map<StateSet, string> stateNames;
    queue<StateSet> pendingStates;
    vector<StateSet> dfaStates;
    vector<Route> dfaRoutes;
    set<string> dfaFinalStates;

    StateSet initialState = startStates;

    stateNames[initialState] = "s0";
    pendingStates.push(initialState);
    dfaStates.push_back(initialState);

    auto getStateName = [&](const StateSet& stateSet) -> string {
        auto it = stateNames.find(stateSet);

        if (it != stateNames.end()) return it->second;

        string name = "s" + to_string(stateNames.size());
        stateNames[stateSet] = name;
        pendingStates.push(stateSet);
        dfaStates.push_back(stateSet);

        return name;
    };

    while (!pendingStates.empty()) {
        StateSet current = pendingStates.front();
        pendingStates.pop();

        string currentName = stateNames.at(current);

        for (const string& state : current) {
            if (finalStates.count(state)) {
                dfaFinalStates.insert(currentName);
                break;
            }
        }

        for (char symbol : alphabet) {
            StateSet destinations;

            for (const string& state : current) {
                auto it = transitions.find({state, symbol});

                if (it != transitions.end()) destinations.insert(it->second.begin(),it->second.end());
            }

            // Üres célhalmaz esetén nem írunk ki átmenetet.
            if (destinations.empty()) continue;

            string destinationName = getStateName(destinations);

            dfaRoutes.push_back({currentName,symbol,destinationName});
        }
    }

    // Az állapotnevek természetes sorrendje: s0, s1, ..., s10.
    auto stateNumber = [](const string& name) {
        return stoi(name.substr(1));
    };

    sort(dfaStates.begin(), dfaStates.end(),
        [&](const StateSet& a, const StateSet& b) {
            return stateNumber(stateNames.at(a)) <
                   stateNumber(stateNames.at(b));
        });

    sort(dfaRoutes.begin(), dfaRoutes.end(),
        [&](const Route& a, const Route& b) {
            if (a.start != b.start) {
                return stateNumber(a.start) < stateNumber(b.start);
            }
            if (a.letter != b.letter) {
                return a.letter < b.letter;
            }
            return stateNumber(a.end) < stateNumber(b.end);
        });

    // Kimenet
    for (size_t i = 0; i < dfaStates.size(); ++i) {
        if (i > 0) output<<' ';
        output << stateNames.at(dfaStates[i]);
    }
    output<<endl;

    sort(alphabet.begin(), alphabet.end());
    alphabet.erase(unique(alphabet.begin(), alphabet.end()), alphabet.end());

    for (size_t i = 0; i < alphabet.size(); ++i) {
        if (i > 0) output << ' ';
        output << alphabet[i];
    }

    output<<endl<<"s0"<<endl;

    vector<string> sortedFinalStates(
        dfaFinalStates.begin(), dfaFinalStates.end()
    );

    sort(sortedFinalStates.begin(), sortedFinalStates.end(),
        [&](const string& a, const string& b) {
            return stateNumber(a) < stateNumber(b);
        });

    for (size_t i = 0; i < sortedFinalStates.size(); ++i) {
        if (i > 0) output << ' ';
        output << sortedFinalStates[i];
    }
    output << endl;

    for (const Route& route : dfaRoutes) {
        output << route.start << ' ' << route.letter << ' ' << route.end << '\n';
    }

    return 0;
}
