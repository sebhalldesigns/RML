//
//  main.cpp
//  RML-Parser
//
//  Created by Seb on 17/01/2024.
//

#include <cstdio>
#include <cstdint>
#include <fstream>
#include <string>
#include <vector>
#include <map>

enum ParseState {
    ROOT, // 0
    READY, // 1
    CONTENTS, // 2
    BEGIN_TAG, // 3
    CLASS, //4
    TAG_READY, // 5
    KEY, // 6
    EQUALS, // 7
    VALUE, // 8
    PARTIAL_COMPLETE, // 9
    SINGLE_TAG_CLOSING, // 10
    CLOSING_TAG, // 11
    CLOSING_CLASS, // 12
    COMPLETE, // 13
    SYNTAX_ERROR // 14
};

struct Tag {
    std::string className;
    std::map<std::string, std::string> parameters;
    std::vector<Tag*> children;
    std::string contents;
};

ParseState state;
int level;
Tag* rootTag;

std::string workingClass;
std::string workingKey;
std::string workingValue;
std::string workingContents;

bool IsText(char character);

int main(int argc, const char * argv[]) {

    std::ifstream file(argv[1]);
    
    if (!file.is_open()) {
        printf("file invalid or unreachable!\n");
        return -1;
    }
    
    std::string contents;
    std::string line;
    
    while (getline(file,line)) {
        contents.append(line);
    }
    
    file.close();
    
    state = READY;
    
    uint64_t length = contents.size();
    uint64_t i = 0;
    uint64_t row = 0;
    uint64_t column = 0;
    
    printf("file found with %llu lines\n", length);
    
    while (i < length) {
        
        
        
        char current = contents[i];
        
        printf("i:%llu %c", i, current);
        
        /*if (current == '\n') {
            current++;
            row++;
            column = 0;
            break;
        } else if (current == '\t' || current == '\0' || current == '\r') {
            current++; // ignore some whitespace characters
            break;
        }*/
        
        switch (state) {
            case ROOT:
                state = READY;
                break;
            case READY:
                if (IsText(current)) {
                    state = CONTENTS;
                } else if (current == '<') {
                    state = BEGIN_TAG;
                    workingClass = "";
                    workingContents = "";
                } else {
                    printf("SYNTAX ERROR AT %llu %llu, unexpected character in READY state\n", row, column);
                    return -1;
                }
                break;
            case CONTENTS:
                if (current == '<') {
                    state = BEGIN_TAG;
                } else {
                    workingContents += current; // should allow non-ASCII characters
                }
                break;
            case BEGIN_TAG:
                if (IsText(current)) {
                    state = CLASS;
                } else if (current == '/') {
                    state = CLOSING_TAG;
                } else {
                    printf("SYNTAX ERROR AT %llu %llu, unexpected character in BEGIN_TAG state\n", row, column);
                    return -1;
                }
                break;
            case CLASS:
                if (IsText(current)) {
                    workingClass += current;
                } else if (current == ' ') {
                    state = TAG_READY;
                } else if (current == '/') {
                    state = SINGLE_TAG_CLOSING;
                } else if (current == '>') {
                    state = PARTIAL_COMPLETE;
                } else {
                    printf("SYNTAX ERROR AT %llu %llu, unexpected character in CLASS state\n", row, column);
                    return -1;
                }
                break;
            case TAG_READY:
                if (IsText(current)) {
                    state = KEY;
                } else if (current == ' ') {
                    state = READY;
                } else if (current == '>') {
                    state = PARTIAL_COMPLETE;
                } else if (current == '/') {
                    state = SINGLE_TAG_CLOSING;
                } else {
                    printf("SYNTAX ERROR AT %llu %llu, unexpected character in TAG_READY state\n", row, column);
                    return -1;
                }
                break;
            case KEY:
                if (IsText(current)) {
                    workingKey += current;
                } else if (current == '=') {
                    state = EQUALS;
                } else {
                    printf("SYNTAX ERROR AT %llu %llu, unexpected character in KEY state\n", row, column);
                    return -1;
                }
                break;
            case EQUALS:
                if (current == ' ') {
                    state = EQUALS;
                } else if (current == '\"') {
                    state = VALUE;
                } else {
                    printf("SYNTAX ERROR AT %llu %llu, unexpected character in EQUALS state\n", row, column);
                    return -1;
                }
                break;
            case VALUE:
                if (current == '\"') {
                    state = TAG_READY;
                } else {
                    workingValue += current; // should allow non ASCII characters
                }
                break;
            case PARTIAL_COMPLETE:
                printf("completed tag with class %s\n", workingClass.c_str());
                level++;
                state = READY;
                break;
            case SINGLE_TAG_CLOSING:
                if (current == '>') {
                    state = COMPLETE;
                } else {
                    printf("SYNTAX ERROR AT %llu %llu, unexpected character in SINGLE_TAG_CLOSING state\n", row, column);
                    return -1;
                }
                break;
            case CLOSING_TAG:
                if (IsText(current)) {
                    state = CLOSING_CLASS;
                } else {
                    printf("SYNTAX ERROR AT %llu %llu, unexpected character in CLOSING_TAG state\n", row, column);
                    return -1;
                }
                break;
            case CLOSING_CLASS:
                if (IsText(current)) {
                    workingClass += current;
                } else if (current == '>') {
                    state = COMPLETE;
                }
                break;
            case COMPLETE:
                if (level < 1) {
                    printf("PARSING COMPLETE at %llu %llu\n", row, column);
                    return 0;
                } else {
                    level--;
                    state = READY;
                }
                break;
            case SYNTAX_ERROR:
                printf("SYNTAX ERROR AT %llu %llu, unexpected character in SYNTAX_ERROR state\n", row, column);
                return -1;
                break;
        }
        
        column++;
        i++;
        
        printf(" %d\n", state);
        
    }
    
    
    
    return 0;
}

bool IsText(char character) {
    return
    (character > 47 && character < 58) || // numbers
    (character > 64 && character < 91) || // upper case
    (character > 96 && character < 123);  // lower case
}
