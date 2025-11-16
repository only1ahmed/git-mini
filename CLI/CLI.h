#pragma once
#include <string>
#include <vector>
#include <map>


enum class Operation{
    init,
    status,
    add,
    commit,
    push,
    pull,
    dif,
    ext,
    ongoing
};
class CLI{
private:
    std::string commandsPrefix = "gitmini ";
    const std::map<std::string, Operation> stringToOp = {
            {"init", Operation::init},
            {"status", Operation::status},
            {"add", Operation::add},
            {"commit", Operation::commit},
            {"push", Operation::push},
            {"pull", Operation::pull},
            {"diff", Operation::dif}
    };


public:

    // It shall return codes to navigate to the right GIT function. it shall whatsoever not modify any member variables.
    std::pair<Operation, std::vector<std::string>> parse(std::string ) const;
    void navigate(std::pair<Operation, std::vector<std::string>>);
    void run();

    std::string getCommandsPrefix(){
        return this->commandsPrefix;
    }

};



