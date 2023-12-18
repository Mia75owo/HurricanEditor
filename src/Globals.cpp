#include "Globals.hpp"

void ReplaceAll(std::string &str, const std::string &from, const std::string &to) {
    if (str.empty() || from.empty())
        return;
    std::string new_str;
    new_str.reserve(str.length());
    size_t start_pos = 0, pos;
    while ((pos = str.find(from, start_pos)) != std::string::npos) {
        new_str += str.substr(start_pos, pos - start_pos);
        new_str += to;
        pos += from.length();
        start_pos = pos;
    }
    new_str += str.substr(start_pos);
    str.swap(new_str);
}
