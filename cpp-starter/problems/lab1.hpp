//
// Created by User on 22-Sep-26.
//

#ifndef PROJECT_LAB1_H
#define PROJECT_LAB1_H

#include "../problem.hpp"

class Lab1Problem : public Problem {
public:
    void initialize_parser(cxxopts::Options &options) override;
    bool is_chosen_problem(const cxxopts::ParseResult &args) override;
    int run(const cxxopts::ParseResult &args) override;
};

#endif //PROJECT_LAB1_H
