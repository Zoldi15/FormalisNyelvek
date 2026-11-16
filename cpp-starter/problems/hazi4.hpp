//
// Created by User on 16-Nov-26.
//

#ifndef PROJECT_HAZI4_H
#define PROJECT_HAZI4_H

#include "../problem.hpp"

class Hazi4Problem : public Problem {
public:
    void initialize_parser(cxxopts::Options &options) override;
    bool is_chosen_problem(const cxxopts::ParseResult &args) override;
    int run(const cxxopts::ParseResult &args) override;
};


#endif //PROJECT_HAZI4_H
