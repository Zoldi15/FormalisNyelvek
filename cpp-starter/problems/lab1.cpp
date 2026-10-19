#include <string>
#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>
#include <map>
#include <algorithm>

#include "lab1.hpp"

using namespace std;

void Lab1Problem::initialize_parser(cxxopts::Options& options) {
    options.add_options()
        ("check", "Words to check", cxxopts::value<string>());
}

bool Lab1Problem::is_chosen_problem(const cxxopts::ParseResult& args) {return args.count("check") > 0;}

struct Route {
    string start;
    char letter;
    string end;
};

void checker(const string& inputFilename, const string& word, ofstream& outputFile) {
    ifstream inputFile(inputFilename);

    if (!inputFile) throw runtime_error("Error opening input file: " + inputFilename);

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

    // Start state read
    string checkStart;
    getline(inputFile, checkStart);

    // Final states read
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

    // Check word
    string currentState = checkStart;

    for (char letter : word) {
        if (find(alphabet.begin(), alphabet.end(), letter) == alphabet.end()) {
            outputFile << "NEM";
            return;
        }

        bool found = false;

        for (const Route& route : routes[currentState]) {
            if (route.letter == letter) {
                currentState = route.end;
                found = true;
                break;
            }
        }

        if (!found) {
            outputFile << "NEM";
            return;
        }
    }

    // Check whether the final state is accepting
    istringstream iss(checkEnd);
    string finalState;

    while (iss >> finalState) {
        if (currentState == finalState) {
            outputFile << "IGEN";
            return;
        }
    }

    outputFile << "NEM";
}

int Lab1Problem::run(const cxxopts::ParseResult& args) {
    string inputFilename = args["input"].as<string>();
    string outputFilename = args["output"].as<string>();
    string checkInput = args["check"].as<string>();

    ofstream outputFile(outputFilename);

    if (!outputFile) throw runtime_error("Error opening output file: " + outputFilename);

    stringstream ss(checkInput);
    string word;

    while (getline(ss, word, ',')) {
        checker(inputFilename, word, outputFile);
        outputFile << '\n';
    }

    outputFile.close();

    return 0;
}