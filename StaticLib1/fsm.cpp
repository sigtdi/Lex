#include "fsm.h"
#include "pch.h"
#include <iostream>
#include <vector>
#include <string>
#include <map>
using namespace std;

Lexem lex = {"", ""};
string value_str = "";
map<char, string> dict1 = { {'(', "lpar"}, {')', "rpar"}, {'{', "lbrace"}, {'}', "rbrace"}, {'>', "opgt"}, {'*', "opmul"}, {';', "semicolon"}, {':', "colon"}, {',', "comma"} };
map<string, string> dict2 = { {"int", "kwint"}, {"char", "kwchar"}, {"if", "kwif"}, {"else", "kwelse"}, {"break", "kwbreak"}, {"switch", "kwswitch"}, {"case", "kwcase"}, {"while", "kwwhile"}, {"for", "kwfor"}, {"return", "kwreturn"}, {"in", "kwin"}, {"out", "kwout"}, {"default", "kwdefault"} };

void initFSM() {
     lex = { "", "" };
     value_str = "";
}

void read(char& cache, istream& stream) {
    if (stream) {
        stream.get(cache);
    }
    if (!stream) {
        cache = 0;
    }
}

pair<int, Lexem> tick(int state, istream& stream, char& cache) {

    vector<pair<int, Lexem>(*)(char&, istream&)> states = {
        [](char& cache, istream& stream) -> pair<int, Lexem> {
            if (cache == 0) {
                return { -1, LEX_EOF };
            }
            else if (cache == -1) {
                return { -1, LEX_ERROR };
            }
            else if (isalpha(cache)) {
                value_str += cache;
                read(cache, stream);
                return { 21, LEX_EMPTY };
            }
            else if (isdigit(cache)) {
                value_str += cache;
                read(cache, stream);
                return { 23, LEX_EMPTY };
            }
            else if (dict1.find(cache) != dict1.end()) {
                lex = { dict1[cache], "" };
                read(cache, stream);
                return { 1, LEX_EMPTY };
            }
            switch (cache) {
                case '+':
                    read(cache, stream);
                    return { 8, LEX_EMPTY };
                case '=':
                    read(cache, stream);
                    return { 6, LEX_EMPTY };
                case '!':
                    read(cache, stream);
                    return { 4, LEX_EMPTY };
                case '<':
                    read(cache, stream);
                    return { 2, LEX_EMPTY };
                case '|':
                    read(cache, stream);
                    return { 10, LEX_EMPTY };
                case '&':
                    read(cache, stream);
                    return { 10, LEX_EMPTY };
                case '-':
                    read(cache, stream);
                    return { 22, LEX_EMPTY };
                case '"':
                    value_str = "";
                    read(cache, stream);
                    return { 18, LEX_EMPTY };
                case '\'':
                    read(cache, stream);
                    return { 14, LEX_EMPTY };
                case ' ':
                case '\t':
                case '\n':
                    read(cache, stream);
                    return { 0, LEX_EMPTY };
         }}, //0
        [](char& cache, istream& stream) -> pair<int, Lexem> {
            return { 0, lex };
        }, //1
        [](char& cache, istream& stream)->pair<int, Lexem> {
            if (cache == '=') {
                read(cache, stream);
                return { 3, LEX_EMPTY };
            }
            return { 0, {"oplt", ""} };
        }, //2
        [](char& cache, istream& stream)->pair<int, Lexem> {
            return { 0, {"ople", ""} };
        }, //3
        [](char& cache, istream& stream)->pair<int, Lexem> {
            if (cache == '=') {
                read(cache, stream);
                return { 5, LEX_EMPTY };
            }
            return { 0, {"opnot", ""} };
        }, //4
        [](char& cache, istream& stream)->pair<int, Lexem> {
            return { 0, {"opne", ""} };
        }, //5
        [](char& cache, istream& stream)->pair<int, Lexem> {
            if (cache == '=') {
                read(cache, stream);
                return { 7, LEX_EMPTY };
            }
            return { 0, {"opassign", ""} };
        }, //6
        [](char& cache, istream& stream)->pair<int, Lexem> {
            return { 0, {"opeq", ""} };
        }, //7
        [](char& cache, istream& stream)->pair<int, Lexem> {
            if (cache == '+') {
                read(cache, stream);
                return { 9, LEX_EMPTY };
            }
            return { 0, {"opplus", ""} };
        }, //8
        [](char& cache, istream& stream)->pair<int, Lexem> {
            return { 0, {"opinc", ""} };
        }, //9
        [](char& cache, istream& stream)->pair<int, Lexem> {
            if (cache == '|') {
                read(cache, stream);
                return { 11, LEX_EMPTY };
            }
            return { -1, LEX_ERROR };
        }, //10
        [](char& cache, istream& stream)->pair<int, Lexem> {
            return { 0, {"opor", ""} };
        }, //11
        [](char& cache, istream& stream)->pair<int, Lexem> {
            if (cache == '&') {
                read(cache, stream);
                return { 13, LEX_EMPTY };
            }
            return { -1, LEX_ERROR };
        }, //12
        [](char& cache, istream& stream)->pair<int, Lexem> {
            return { 0, {"opand", ""} };
        }, //13
        [](char& cache, istream& stream)->pair<int, Lexem> {
            if (cache == '\'') {
                return { 15, LEX_EMPTY };
            }
            value_str = cache;
            read(cache, stream);
            return { 16, LEX_EMPTY };
        }, //14
        [](char& cache, istream& stream)->pair<int, Lexem> {
            return { -1, LEX_ERROR };
        }, //15
        [](char& cache, istream& stream)->pair<int, Lexem> {
            if (cache == '\'') {
                lex = { std::string("char"), value_str };
                value_str = "";
                read(cache, stream);
                return { 17, LEX_EMPTY };
            }
            read(cache, stream);
            return { -1, LEX_ERROR };
        }, //16
        [](char& cache, istream& stream)->pair<int, Lexem> {
            return { 0, lex };
        }, //17
        [](char& cache, istream& stream)->pair<int, Lexem> {
            if (cache == '"') {
                read(cache, stream);
                return { 20, LEX_EMPTY };
            }
            else if (cache == 0) {
                return { 19, LEX_EMPTY };
            }
            value_str += cache;
            read(cache, stream);
            return { 18, LEX_EMPTY };
        }, //18
        [](char& cache, istream& stream)->pair<int, Lexem> {
            return { -1, LEX_ERROR };
        }, //19
        [](char& cache, istream& stream)->pair<int, Lexem> {
            lex = { std::string("str"), std::string(value_str) };
            value_str = "";
            return { 0, lex };
        }, //20
        [](char& cache, istream& stream)->pair<int, Lexem> {
            if (isalpha(cache) || isdigit(cache) || cache == '_') {
                value_str += cache;
                read(cache, stream);
                return { 21, LEX_EMPTY };
            }
            if (dict2.find(value_str) == dict2.end()) {
                lex = { std::string("id"), std::string(value_str) };
            }
            else {
                lex = { std::string(dict2[value_str]), "" };
            }
            value_str = "";
            return { 0, lex };
        }, //21
        [](char& cache, istream& stream)->pair<int, Lexem> {
            if (isdigit(cache)) {
                value_str = '-';
                value_str += cache;
                read(cache, stream);
                return { 23, LEX_EMPTY };
            }
            return { 0, {"opminus", ""} };
        }, //22
        [](char& cache, istream& stream)->pair<int, Lexem> {
            if (isdigit(cache)) {
                value_str += cache;
                read(cache, stream);
                return { 23, LEX_EMPTY };
            }
            lex = { std::string("num"), std::string(value_str) };
            value_str = "";
            return { 0, lex };
        }, //23
    };

    return states[state](cache, stream);
}