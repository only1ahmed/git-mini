#include "StringUtil.h"
#include <string>
bool startsWith(std::string s, std::string prefix){
    int sCnt = 0;
    for (int i = 0; i < s.size() && i < prefix.size(); ++i) {
        if(s[i] != prefix[i]){
            break;
        }
        sCnt++;
    }
    if(sCnt != prefix.size()){
        return false;
    }
    return true;
}

std::string toLowerMask(std::string s) {
    for (char &c : s) {
        if (c >= 'A' && c <= 'Z') {   // only mask uppercase letters
            c |= 0x20;
        }
    }
    return s;
}