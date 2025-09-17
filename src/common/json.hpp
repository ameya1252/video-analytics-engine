#pragma once
#include <string>
#include <sstream>
#include <unordered_map>

// Extremely tiny JSON builder for key/primitive pairs to avoid heavy deps.
namespace mini_json {
    inline std::string escape(const std::string& s) {
        std::ostringstream o; o << '"';
        for (auto c : s) {
            switch (c) {
                case '"':  o << "\\" << '"'; break;
                case '\\': o << "\\\\"; break;
                case '\n': o << "\\n"; break;
                case '\r': o << "\\r"; break;
                case '\t': o << "\\t"; break;
                default:    o << c; break;
            }
        }
        o << '"'; return o.str();
    }

    inline std::string obj(const std::unordered_map<std::string, std::string>& kv) {
        std::ostringstream o; o << '{';
        bool first = true;
        for (auto& [k, v] : kv) {
            if (!first) o << ',';
            first = false;
            o << escape(k) << ':' << v;
        }
        o << '}';
        return o.str();
    }

    inline std::string str(const std::string& s){ return escape(s); }
    inline std::string num(long long v){ return std::to_string(v); }
    inline std::string num(double v){ std::ostringstream o; o << v; return o.str(); }
    inline std::string boolean(bool b){ return b ? "true" : "false"; }
}
