
#include <algorithm>
#include <fstream>
#include <map>
#include <set>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>
#include <regex>

#include "hazi4.hpp"

using namespace std;

// [2] dátum és idő
// [3] PID
// [4] TID
// [5] logszint
// [6] tag
// [7] message

const regex logRegex(
    R"(([0-9-]+ [0-9:.]+) +([0-9]+) +([0-9]+) ([IDVEFAW]) (.+?):(?: (.+))?)"
);

struct LogEntry {
    string datetime;
    string pid;
    string tid;
    string level;
    string tag;
    string message;
};


bool parseLogLine(const string& line, LogEntry& entry) {
    smatch match;

    if (!regex_search(line, match, logRegex))  return false;

    entry.datetime = match[1].str();
    entry.pid = match[2].str();
    entry.tid = match[3].str();
    entry.level = match[4].str();
    entry.tag = match[5].str();
    entry.message = match[6].str();

    size_t first = entry.tag.find_first_not_of(" \t");
    size_t last = entry.tag.find_last_not_of(" \t");

    if (first == string::npos) entry.tag.clear();
    else entry.tag = entry.tag.substr(first, last - first + 1);

    return true;
}


void Hazi4Problem::initialize_parser(cxxopts::Options& options) {
    options.add_options()
        ("rgx", "Regex task number", cxxopts::value<int>());
}

bool Hazi4Problem::is_chosen_problem(const cxxopts::ParseResult& args) {
    return args.count("rgx") > 0;
}

int Hazi4Problem::run(const cxxopts::ParseResult& args) {
    int task = args["rgx"].as<int>();

    if (task < 1 || task > 5) throw runtime_error("Invalid regex task number");

    string inputFilename = args["input"].as<string>();
    string outputFilename = args["output"].as<string>();

    ifstream input(inputFilename);

    if (!input.is_open()) throw runtime_error("Cannot open input file: " + inputFilename);

    ofstream output(outputFilename);

    if (!output.is_open())  throw runtime_error("Cannot open output file: " + outputFilename);

    vector<string> lines;
    string line;

    while (getline(input, line)) {
        lines.push_back(line);
    }

    input.close();


switch (task) {

case 1: {
    int count1 = 0, count2 = 0, count3 = 0, count4 = 0;
    int count5 = 0, count6 = 0, count7 = 0, count8 = 0;
    int count9 = 0, count10 = 0;

    regex dateRegex(R"(^10-15)");
    regex exactTimeRegex(R"(^10-15 10:18:51)");
    regex secondsRegex(R"(^[0-9]{2}-[0-9]{2} [0-9]{2}:[0-9]{2}:51)");

    regex lowMemoryRegex(
        R"(^[0-9-]+ [0-9:.]+ +[0-9]+ +[0-9]+ [IDVEFAW] +[^:]*lowmemorykiller[^:]*:)",
        regex_constants::icase
    );

    regex pid221Regex(R"(^[0-9-]+ 10:18:51\.[0-9]+ +221 +)");

    regex exoPlayerRegex(
        R"(^[0-9-]+ [0-9:.]+ +[0-9]+ +[0-9]+ D .*ExoPlayer)"
    );

    regex packageManagerRegex(
        R"(^[0-9-]+ [0-9:.]+ +[0-9]+ +[0-9]+ W +[^:]*PackageManager[^:]*:)",
        regex_constants::icase
    );

    regex javaPositionRegex(
        R"(^[0-9-]+ [0-9:.]+ +[0-9]+ +[0-9]+ [EW] .*\.java:)"
    );

    regex threadRegex(
        R"(^[0-9-]+ [0-9:.]+ +[0-9]+ +[0-9]+ [IDVEFAW] +[^:]+: .*Thread)"
    );

    for (const string& currentLine : lines) {
        LogEntry entry;

        bool validLog = parseLogLine(currentLine, entry);

        // 1. October 15 entries
        if (regex_search(currentLine, dateRegex))
            count1++;

        // 2. October 15 entries at 10:18:51
        if (regex_search(currentLine, exactTimeRegex))
            count2++;

        // 3. Entries occurring at second 51
        if (regex_search(currentLine, secondsRegex))
            count3++;

        // Python regexes 4-10 operate on the complete log line.
        if (!validLog)
            continue;

        // 4. lowmemorykiller
        if (regex_search(currentLine, lowMemoryRegex))
            count4++;

        // 5. 10:18:51 and PID 221
        if (regex_search(currentLine, pid221Regex))
            count5++;

        // 6. Error entries
        if (entry.level == "E")
            count6++;

        // 7. PackageManager warnings
        if (regex_search(currentLine, packageManagerRegex))
            count7++;

        // 8. ExoPlayer debug
        if (regex_search(currentLine, exoPlayerRegex))
            count8++;

        // 9. Error and Warning entries containing Java positions
        if (regex_search(currentLine, javaPositionRegex))
            count9++;

        // 10. Thread in the log message
        if (regex_search(currentLine, threadRegex))
            count10++;
    }

    output << count1 << '\n';
    output << count2 << '\n';
    output << count3 << '\n';
    output << count4 << '\n';
    output << count5 << '\n';
    output << count6 << '\n';
    output << count7 << '\n';
    output << count8 << '\n';
    output << count9 << '\n';
    output << count10 << '\n';

    break;
}



    case 2: {
        regex javaFileRegex(R"(([A-Za-z0-9_$]+\.java):[0-9]+)");

        for (const string& currentLine : lines) {
            LogEntry entry;

            if (!parseLogLine(currentLine, entry))
                continue;

            if (entry.level != "E")
                continue;

            sregex_iterator it(
                currentLine.begin(),
                currentLine.end(),
                javaFileRegex
            );

            sregex_iterator end;

            for (; it != end; ++it) {
                output << (*it)[1].str() << '\n';
            }
        }

        break;
    }


    case 3: {
        set<string> seenPids;

        for (const string& currentLine : lines) {
            LogEntry entry;

            if (!parseLogLine(currentLine, entry)) continue;
            if (seenPids.insert(entry.pid).second)  output << entry.pid << '\n';
        }

        break;
    }

    case 4: {
        map<string, int> errorCounts;
        vector<string> pidOrder;

        for (const string& currentLine : lines) {
            LogEntry entry;

            if (!parseLogLine(currentLine, entry)) continue;

            if (entry.level == "E") {
                if (errorCounts.find(entry.pid) == errorCounts.end()) {
                    errorCounts[entry.pid] = 0;
                    pidOrder.push_back(entry.pid);
                }

                errorCounts[entry.pid]++;
            }
        }

        string maxPid;
        int maxErrors = 0;

        for (const string& pid : pidOrder) {
            if (errorCounts[pid] > maxErrors) {
                maxErrors = errorCounts[pid];
                maxPid = pid;
            }
        }

        if (!maxPid.empty()) {
            output << "PID=" << maxPid << " (" << maxErrors << " errors)";
        }

        break;
    }

    case 5: {
        int mainThreadErrors = 0;

        for (const string& currentLine : lines) {
            LogEntry entry;

            if (!parseLogLine(currentLine, entry)) continue;

            if (entry.level == "E" &&
                entry.pid == entry.tid) {
                mainThreadErrors++;
            }
        }

        output << mainThreadErrors << '\n';

        break;
    }

    default: throw runtime_error("Invalid regex task number");
}


    return 0;
}
