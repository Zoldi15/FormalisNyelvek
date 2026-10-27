
#include <algorithm>
#include <fstream>
#include <map>
#include <queue>
#include <set>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#include "hazi3.hpp"

using namespace std;

struct Route {
    string start;
    char letter;
    string end;
};

// Allapotnevek termeszetes sorrendje
static bool stateLess(const string& a, const string& b) {
    auto numberStart = [](const string& s) {
        size_t pos = s.size();
        while (pos > 0 && s[pos - 1] >= '0' && s[pos - 1] <= '9') --pos;
        return pos;
    };

    size_t pa = numberStart(a);
    size_t pb = numberStart(b);

    if (pa < a.size() && pb < b.size()) {
        string prefixA = a.substr(0, pa);
        string prefixB = b.substr(0, pb);

        if (prefixA == prefixB) {
            long long na = stoll(a.substr(pa));
            long long nb = stoll(b.substr(pb));

            if (na != nb) return na < nb;
        }
    }

    return a<b;
}

static string renameState(const string& state) {
    if (!state.empty() && state[0] == 'q') return "s" + state.substr(1);
    return "s" + state;
}

void Hazi3Problem::initialize_parser(cxxopts::Options& options) {
    options.add_options()
        ("mini", "Minimize deterministic automaton");
}

bool Hazi3Problem::is_chosen_problem(
    const cxxopts::ParseResult& args
) {
    return args.count("mini") > 0;
}

int Hazi3Problem::run(const cxxopts::ParseResult& args) {
    const string inputFilename = args["input"].as<string>();
    const string outputFilename = args["output"].as<string>();

    ifstream input(inputFilename);

    if (!input.is_open()) {
        throw runtime_error("Cannot open input file: " + inputFilename);
    }

    // Beolvasas

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

        while (stream >> word)  words.push_back(word);

        return words;
    };

    if (!getline(input, line)) {
        throw runtime_error("Empty input file");
    }

    states = readWords(line);

    if (states.empty()) {
        throw runtime_error("No states specified");
    }

    set<string> knownStates(states.begin(), states.end());

    if (knownStates.size() != states.size()) {
        throw runtime_error("Duplicate state declaration");
    }

    if (!getline(input, line)) {
        throw runtime_error("Missing alphabet");
    }

    for (const string& symbol : readWords(line)) {
        if (symbol.size() != 1) {
            throw runtime_error(
                "Alphabet symbols must be single characters"
            );
        }

        alphabet.push_back(symbol[0]);
    }

    sort(alphabet.begin(), alphabet.end());
    alphabet.erase(
        unique(alphabet.begin(), alphabet.end()),
        alphabet.end()
    );

    if (!getline(input, line)) {
        throw runtime_error("Missing start state");
    }

    for (const string& state : readWords(line)) {
        startStates.insert(state);
    }

    if (startStates.empty()) {
        throw runtime_error("The automaton has no start state");
    }

    if (startStates.size() != 1) {
        throw runtime_error(
            "A deterministic automaton must have exactly one start state"
        );
    }

    if (!getline(input, line)) {
        throw runtime_error("Missing final states");
    }

    for (const string& state : readWords(line)) {
        finalStates.insert(state);
    }

    while (getline(input, line)) {
        vector<string> words = readWords(line);

        if (words.empty()) {
            continue;
        }

        if (words.size() != 3 || words[1].size() != 1) {
            throw runtime_error("Invalid transition: " + line);
        }

        routes.push_back({words[0], words[1][0], words[2]});
    }

    const string startState = *startStates.begin();

    // Állapotok és jelek ellenőrzése.
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

    set<char> knownAlphabet(alphabet.begin(), alphabet.end());

    // Determinisztikus átmeneti függvény.
    map<pair<string, char>, string> transitions;

    for (const Route& route : routes) {
        if (!knownStates.count(route.start)) {
            throw runtime_error(
                "Unknown transition source: " + route.start
            );
        }

        if (!knownStates.count(route.end)) {
            throw runtime_error(
                "Unknown transition destination: " + route.end
            );
        }

        if (!knownAlphabet.count(route.letter)) {
            throw runtime_error(
                "Unknown transition symbol: " + string(1, route.letter)
            );
        }

        pair<string, char> key = {route.start, route.letter};

        auto it = transitions.find(key);

        if (it != transitions.end() && it->second != route.end) {
            throw runtime_error(
                "The automaton is not deterministic at state " + route.start
            );
        }

        transitions[key] = route.end;
    }

    // Allapotok meghatarozasa.
    set<string> reachable;
    queue<string> pending;

    reachable.insert(startState);
    pending.push(startState);

    while (!pending.empty()) {
        string current = pending.front();
        pending.pop();

        for (char symbol : alphabet) {
            auto it = transitions.find({current, symbol});

            if (it == transitions.end())  continue;

            const string& destination = it->second;

            if (reachable.insert(destination).second)  pending.push(destination);
        }
    }

    // Kezdeti particionalas
    vector<vector<string>> partitions;
    vector<string> accepting;
    vector<string> nonAccepting;

    for (const string& state : reachable) {
        if (finalStates.count(state)) accepting.push_back(state);
        else  nonAccepting.push_back(state);
    }

    if (!nonAccepting.empty()) partitions.push_back(nonAccepting);
    if (!accepting.empty())partitions.push_back(accepting);

    // Finomitas
    bool changed = true;

    while (changed) {
        changed = false;

        map<string, int> partitionOf;

        for (int i = 0; i < static_cast<int>(partitions.size()); ++i) {
            for (const string& state : partitions[i])  partitionOf[state] = i;
        }

        vector<vector<string>> refined;

        for (const vector<string>& group : partitions) {
            map<vector<int>, vector<string>> subgroups;

            for (const string& state : group) {
                vector<int> signature;

                for (char symbol : alphabet) {
                    auto it = transitions.find({state, symbol});

                    if (it == transitions.end())  signature.push_back(-1);
                    else  signature.push_back(partitionOf.at(it->second));
                }

                subgroups[signature].push_back(state);
            }

            if (subgroups.size() > 1)  changed = true;

            for (auto& entry : subgroups)  refined.push_back(entry.second);
        }

        partitions = refined;
    }

    // Atiras
    map<string, string> representativeOf;
    map<string, string> newNameOf;
    vector<string> minimizedStates;

    for (vector<string>& group : partitions) {
        sort(group.begin(), group.end(), stateLess);

        const string representative = group.front();
        const string newName = renameState(representative);

        minimizedStates.push_back(newName);

        for (const string& state : group) {
            representativeOf[state] = representative;
            newNameOf[state] = newName;
        }
    }

    sort(minimizedStates.begin(), minimizedStates.end(), stateLess);

    // Uj be és ki allapotok
    const string newStart = newNameOf.at(startState);

    set<string> minimizedFinalStates;

    for (const string& state : reachable) {
        if (finalStates.count(state))  minimizedFinalStates.insert(newNameOf.at(state));
    }

    // Atmenetek letrehozasa
    vector<Route> minimizedRoutes;
    set<tuple<string, char, string>> uniqueRoutes;

    for (const vector<string>& group : partitions) {
        const string representative = group.front();
        const string newSource = renameState(representative);

        for (char symbol : alphabet) {
            auto it = transitions.find({representative, symbol});

            if (it == transitions.end()) continue;

            const string newDestination = newNameOf.at(it->second);

            auto routeKey = make_tuple(newSource, symbol, newDestination);

            if (uniqueRoutes.insert(routeKey).second) {
                minimizedRoutes.push_back({newSource,symbol,newDestination });
            }
        }
    }

    // Atmenetek rendezese
    sort(
        minimizedRoutes.begin(),
        minimizedRoutes.end(),
        [](const Route& a, const Route& b) {
            if (a.start != b.start)  return stateLess(a.start, b.start);
            if (a.letter != b.letter) return a.letter < b.letter;

            return stateLess(a.end, b.end);
        }
    );

    // Kiir
    ofstream output(outputFilename);

    if (!output.is_open()) {
        throw runtime_error(
            "Cannot open output file: " + outputFilename
        );
    }

    for (size_t i = 0; i < minimizedStates.size(); ++i) {
        if (i > 0)  output << ' ';
        output << minimizedStates[i];
    }
    output << '\n';

    for (size_t i = 0; i < alphabet.size(); ++i) {
        if (i > 0) output << ' ';
        output << alphabet[i];
    }
    output << '\n';

    output << newStart << '\n';

    vector<string> sortedFinals(minimizedFinalStates.begin(), minimizedFinalStates.end());
    sort(sortedFinals.begin(), sortedFinals.end(), stateLess);

    for (size_t i = 0; i < sortedFinals.size(); ++i) {
        if (i > 0)  output << ' ';
        output << sortedFinals[i];
    }
    output << '\n';

    for (const Route& route : minimizedRoutes) {
        output << route.start << ' ' << route.letter << ' ' << route.end << '\n';
    }

    return 0;
}