//
// Created by User on 27-Oct-26.
//

#ifndef PROJECT_HAZI3_H
#define PROJECT_HAZI3_H

#include "../problem.hpp"

class Hazi3Problem : public Problem {
public:
    void initialize_parser(cxxopts::Options &options) override;
    bool is_chosen_problem(const cxxopts::ParseResult &args) override;
    int run(const cxxopts::ParseResult &args) override;
};

#endif //PROJECT_HAZI3_H
