//
// Created by User on 19-Oct-26.
//

#ifndef PROJECT_HAZI2_H
#define PROJECT_HAZI2_H

#include "../problem.hpp"

class Hazi2Problem : public Problem {
public:
    void initialize_parser(cxxopts::Options &options) override;
    bool is_chosen_problem(const cxxopts::ParseResult &args) override;
    int run(const cxxopts::ParseResult &args) override;
};


#endif //PROJECT_HAZI2_H
