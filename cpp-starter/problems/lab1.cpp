#include <string>
#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>
#include <map>
#include <algorithm>

#include "lab1.hpp"

using namespace std;

void Lab1Problem::initialize_parser(cxxopts::Options &options) {options.add_options()("check", "Word check", cxxopts::value<string>());}

bool Lab1Problem::is_chosen_problem(const cxxopts::ParseResult &args) {return args.count("check") > 0;}

struct Route {
    string start;
    char letter;
    string end;
};

void checker(const string& inputFilename, const string& word) {
    ifstream inputFile(inputFilename);

    if (!inputFile) {
        cerr << "Error opening input file: " << inputFilename << endl;
    }

    // States read
    vector<string> states;
    string line;

    getline(inputFile, line);
    {
        istringstream iss(line);
        string state;

        while (iss >> state) {
            states.push_back(state);
        }
    }

    // Alphabet read
    vector<char> alphabet;

    getline(inputFile, line);
    {
        istringstream iss(line);
        char letter;

        while (iss >> letter) {
            alphabet.push_back(letter);
        }
    }

    // Start read
    string checkStart;
    getline(inputFile, checkStart);

    // End read
    string checkEnd;
    getline(inputFile, checkEnd);

    // Routes read
    map<string, vector<Route>> routes;

    while (getline(inputFile, line)) {
        istringstream iss(line);

        string start, end;
        char letter;

        if (iss >> start >> letter >> end) {
            routes[start].push_back({start, letter, end});
        }
    }

    inputFile.close();
    // Input end

    // Check
    string currentState = checkStart;

    for (char letter : word) {
        if (find(alphabet.begin(), alphabet.end(), letter) == alphabet.end()) {
            cout << "NEM";
        }

        // Find route
        bool found = false;

        for (const Route& route : routes[currentState]) {
            if (route.letter == letter) {
                currentState = route.end;
                found = true;
                break;
            }
        }

        if (!found) cout << "NEM";
    }


    if (currentState == checkEnd) cout << "IGEN";
    else cout << "NEM";
}

int Lab1Problem::run(const cxxopts::ParseResult &args) {
    string inputFilename = args["input"].as<string>();
    string checkInput = args["check"].as<string>();

    stringstream ss(checkInput);
    string word;

    while (getline(ss, word, ',')) {
        checker(inputFilename, word);
        cout<<endl;
    }

    return 0;
}