#include "prefix_tree.h"
#include <algorithm>
#include <cctype>
PrefixTreeNode::PrefixTreeNode() {
    height = 0;
    end = false;
    type = WordType::Undefined;
}
PrefixTreeNode::PrefixTreeNode(const char &c) {
    value = c;
    height = 0;
    end = false;
    type = WordType::Undefined;
}
PrefixTreeNode::~PrefixTreeNode() {
    for (auto node: childs)
        delete node;
}
int PrefixTreeNode::FindChild(char value) {
    for (int i = 0; i < childs.size(); i++) {
        if (value == childs[i]->value)
            return i;
    }
    return -1;
}
void PrefixTreeNode::Desc(std::ostream &os) {
    os << "[node_info] value:" << value << " height:" << height << " childs_num:" << childs.size() << " end:" << end
       << std::endl;
}
PrefixTree::PrefixTree() {
    root_ = new PrefixTreeNode('\0');
}
void PrefixTree::Insert(char *value, int length, WordType word_type) {
    PrefixTreeNode *ptr = root_;
    int begin_pos = SkipPrefix(ptr, value, length);
    for (int i = begin_pos; i < length; i++) {
        PrefixTreeNode *new_node = new PrefixTreeNode(value[i]);
        ptr->childs.push_back(new_node);
        new_node->height = ptr->height + 1;
        ptr = new_node;
    }
    if (ptr->end == false) {
        ptr->end = true;
        ptr->type = word_type;
    }
}
void PrefixTree::Insert(std::string value, WordType word_type) {
    Insert(const_cast<char *>(value.c_str()), value.length(), word_type);
}
Suffixes *PrefixTree::GetSuffixes(std::string value) {
    return GetSuffixes(const_cast<char *>(value.c_str()), value.length());
}
void PrefixTree::Delete(char *value, int length) {
    PrefixTreeNode *ptr = root_, *delete_parent_node = nullptr;
    int delete_node_index = -1;
    for (int i = 0; i < length; i++) {
        if (ptr->childs.empty())
            return;
        else {
            int res = ptr->FindChild(value[i]);
            if (res == -1)
                return;
            if (ptr->childs.size() > 1) {
                delete_parent_node = ptr;
                delete_node_index = res;
            }
            ptr = ptr->childs[i];
        }
    }
    if (ptr->end == false)
        return;
    if (!ptr->childs.empty()) {
        ptr->end = false;
    } else {
        PrefixTreeNode *delete_node = ptr->childs[delete_node_index];
        ptr->childs.erase(ptr->childs.begin() + delete_node_index);
        delete delete_node;
    }
}
Suffixes *PrefixTree::GetSuffixes(char *prefix, int length) {
    PrefixTreeNode *ptr = root_;
    int skip_chars = SkipPrefix(ptr, prefix, length);
    if (skip_chars != length)
        return nullptr;
    Suffixes ses;
    if (ptr->childs.empty()) {
        ses.suffixes.push_back("");
    } else if (ptr->childs.size() == 1) {
    } else if (ptr->c)
        return nullptr;
}
void PrefixTree::Delete(std::string value) {
    Delete(const_cast<char *>(value.c_str()), value.length());
}
int PrefixTree::SkipPrefix(PrefixTreeNode *&ptr, char *value, int length) {
    for (int i = 0; i < length; i++) {
        // ptr->Desc(std::cout);
        if (ptr->childs.empty())
            return ptr->height;
        else {
            int res_index = ptr->FindChild(value[i]);
            if (res_index == -1)
                return ptr->height;
            ptr = ptr->childs[res_index];
        }
    }
    return ptr->height;
}
int PrefixTree::FindWord(std::string value) {
    return FindWord(const_cast<char *>(value.c_str()), value.length());
}
int PrefixTree::FindWord(char *value, int length) {
    PrefixTreeNode *ptr = root_;
    for (int i = 0; i < length; i++) {
        if (ptr->childs.empty())
            return 0;
        else {
            int res_index = ptr->FindChild(value[i]);
            if (res_index == -1)
                return 0;
            ptr = ptr->childs[res_index];
        }
    }
    return 1;
}
void Suffixes::Desc() {
    int index = 0;
    for (const auto &str: suffixes_)
        printf("suffix %d:%s\n", index++, str);
}