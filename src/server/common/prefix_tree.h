#pragma once
#include <cstring>
#include <iostream>
#include <string>
#include <vector>
enum WordType
{
    Undefined = 0,
    KeyWord,
    TableName,
    DatabaseName,
    AttrName
};
struct PrefixTreeNode {
public:
    PrefixTreeNode();
    PrefixTreeNode(const char &c);
    ~PrefixTreeNode();
    int FindChild(char value);
    void Desc(std::ostream &os);

public:
    char value;
    std::vector<PrefixTreeNode *> childs;
    int height;
    bool end;
    WordType type;
};
struct Suffix {
public:
    Suffix();
    Suffix(WordType type);

public:
    WordType type;
    std::string str;
};
struct Suffixes {
public:
    Suffixes() = default;
    void Desc();

public:
    std::vector<std::string> suffixes;
};
class PrefixTree
{
public:
    PrefixTree();
    void Insert(std::string value, WordType word_type);
    void Delete(std::string value);
    Suffixes *GetSuffixes(std::string value);
    int FindWord(std::string value);

private:
    int SkipPrefix(PrefixTreeNode *&ptr, char *value, int length);
    void Insert(char *value, int length, WordType word_type);
    void Delete(char *value, int length);
    Suffixes *GetSuffixes(char *prefix, int length);
    int FindWord(char *value, int length);

private:
    PrefixTreeNode *root_;
};