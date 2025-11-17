#include "CLI.h"
#include <string>
#include <iostream>
#include <vector>
#include <sstream>
#include "util/StringUtil.h"

const std::string EXT = "exit";


std::pair<Operation, std::vector<std::string>> CLI::parse(std::string command) const {

    if(startsWith(toLowerMask(command), EXT)){
        return {Operation::ext,{}};
    }
    if(not startsWith(toLowerMask(command), this->commandsPrefix)){
        return {Operation::ongoing,{}};
    }
    std::string gitCommand = command.substr((int)(this->commandsPrefix.size()));
    if(gitCommand.empty()){
        // TODO: add help menu here
        return {Operation::ongoing, {}};
    }


    std::istringstream iss(gitCommand);
    std::vector<std::string> args;
    std::string arg;

    iss >> arg;
    if(this->stringToOp.count(arg) < 1){
        return {Operation::ongoing, {}};
    }

    // Note: you CANNOT use operator[] with const maps cause the overloaded operator can throw a default value if doesn't find the key (which contradicts with the fact that the map is const).
    Operation op = stringToOp.at(arg);

    while (iss >> arg) {
        args.push_back(arg);
    }


    return {op, args};
}

void CLI::navigate(std::pair<Operation, std::vector<std::string>> code) {
    //TODO: replace the switch case with std::map that maps the "Operation" to the function of the operation for O(LOG(numOfOperations)) time complexity.
    switch (code.first) {
        case Operation::init:{
            break;
        }

        case Operation::add:{
            break;
        }

        case Operation::status:{
            break;
        }

        case Operation::commit:{
            break;
        }
        case Operation::push:{
            break;
        }
        case Operation::pull:{
            break;
        }
        default:
            std::cout << "Unknown command!\n";
            break;
    }
}



void CLI::run() {
    std::string cmd;

    while(true){
        std::cout << "Welcome to gitmini: ";
        getline(std::cin, cmd);
        std::pair<Operation, std::vector<std::string>> op = parse(cmd);
        if(op.first == Operation::ext){
            break;
        }
        navigate(op);
    }
}