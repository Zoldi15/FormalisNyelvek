
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
    int count1 = 0, count2 = 0, count3 = 0, count4 = 0, count5 = 0;
    int count6 = 0, count7 = 0, count8 = 0, count9 = 0, count10 = 0;

    regex dateRegex(R"(^10-15(?:\s|$))");
    regex exactPidTimeRegex(R"(^[0-9-]+ 10:18:51(?:\.[0-9]+)?$)");
    regex secondsRegex(R"(\b[0-9]{2}:[0-9]{2}:51(?:\.|\s|$))");

    regex lowMemoryRegex(R"(lowmemorykiller)", regex_constants::icase);
    regex pid221Regex(R"(^10-15 10:18:51(?:\.[0-9]+)?\s+221\s+)");

    regex javaPositionRegex(R"(\.java:[0-9]+)");
    regex exoPlayerRegex(R"(ExoPlayer)");
    regex threadRegex(R"(Thread)");

    for (const string& currentLine : lines) {
        LogEntry entry;
        bool validLog = parseLogLine(currentLine, entry);

        // 1. Október 15-i sorok
        if (regex_search(currentLine, dateRegex))
            count1++;

        // 2. Október 15. 10:18:51-kor keletkezett sorok
        if (regex_search(currentLine, exactPidTimeRegex))
            count2++;

        // 3. Minden 51. másodpercben keletkezett sor
        if (regex_search(currentLine, secondsRegex))
            count3++;

        if (!validLog)
            continue;

        // 4. lowmemorykiller taget tartalmazó sorok
        if (regex_search(entry.tag, lowMemoryRegex))
            count4++;

        // 5. 10:18:51-kor keletkezett, 221-es PID-jű sorok
        if (entry.pid == "221" && regex_match(entry.datetime, exactPidTimeRegex))
            count5++;

        // 6. Error (E) sorok
        if (entry.level == "E")
            count6++;

        // 7. PackageManager warningok
        if (entry.level == "W" &&
            entry.tag == "PackageManager") {
            count7++;
        }

        // 8. ExoPlayer debug sorok
        if (entry.level == "D" &&
            regex_search(entry.tag, exoPlayerRegex)) {
            count8++;
        }

        // 9. Java fájlpozíciót tartalmazó error és warning sorok
        if ((entry.level == "E" || entry.level == "W") &&
            regex_search(currentLine, javaPositionRegex)) {
            count9++;
        }

        // 10. Thread szó a message részben
        if (regex_search(entry.message, threadRegex))
            count10++;
    }

    output << count1 << '\n'
           << count2 << '\n'
           << count3 << '\n'
           << count4 << '\n'
           << count5 << '\n'
           << count6 << '\n'
           << count7 << '\n'
           << count8 << '\n'
           << count9 << '\n'
           << count10 << '\n';

    break;
    }

    case 2: {
        regex javaFileRegex(R"(([A-Za-z_$][A-Za-z0-9_$]*\.java):[0-9]+)");

        bool insideErrorStackTrace = false;

        for (const string& currentLine : lines) {
            LogEntry entry;

            // Ha új logcat sor következik, ellenőrizzük a szintjét.
            if (parseLogLine(currentLine, entry)) {
                insideErrorStackTrace = (entry.level == "E");
                continue;
            }

            // Csak az E sorokhoz tartozó stack trace-t dolgozzuk fel.
            if (!insideErrorStackTrace)
                continue;

            // Az összes Java fájlnevet kigyűjtjük.
            // Az ismétlődéseket is megtartjuk, eredeti sorrendben.
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
