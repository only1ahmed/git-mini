#include "../gitmini.h"

void navigate() {

}

void gitmini::commit(const std::vector<std::string> &args) {

//    navigate()

    // load staged and current commit.
    gitminiHelper::loadCurrentCommit(this->commitRoot, gitmini::branchTracer);
    gitminiHelper::loadStagedChanges(this->stagedChanges, gitmini::stageTracer);
    

}