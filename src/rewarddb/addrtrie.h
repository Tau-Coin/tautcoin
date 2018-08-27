#ifndef ADDRTRIE_H
#define ADDRTRIE_H

#include <iostream>
#include <string>
#include <algorithm>
#include <sstream>
#include <boost/algorithm/string.hpp>

namespace TAUAddrTrie
{

const int MaxBranchSize = 58;
const char b58Alphabet[MaxBranchSize+1] = {'1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C',
                                         'D', 'E', 'F', 'G', 'H', 'J', 'K', 'L', 'M', 'N', 'P', 'Q',
                                         'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', 'a', 'b', 'c',
                                         'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'm', 'n', 'o', 'p',
                                         'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z', '?'};
class TrieNode
{
public:
    std::string GetWord() const { return _word; }

    void SetWord(std::string input) { _word = input; }

    TrieNode* GetNextBranch(int index) const { return _nextBranch[index]; }

    void NewNextBranch(int index) { _nextBranch[index] = new TrieNode(this); }

    TrieNode* GetParrent() const { return _parrent; }

    TrieNode() : _word("") { memset(_nextBranch, 0, sizeof(TrieNode*)*MaxBranchSize); }

    TrieNode(TrieNode* p) : _word(""), _parrent(p) { memset(_nextBranch, 0, sizeof(TrieNode*)*MaxBranchSize); }

    ~TrieNode()
    {
        for(int i = 0; i < MaxBranchSize; i++)
        {
            if (_nextBranch[i] != NULL)
                deleteNectBranch(i);
        }
    }

private:
    std::string _word;

    TrieNode* _nextBranch[MaxBranchSize];

    TrieNode* _parrent;

    void deleteNectBranch(int index) { delete _nextBranch[index]; _nextBranch[index] = NULL; }
};

class Trie
{
public:
    Trie() { pRoot = new TrieNode(); }

    ~Trie()
    {
        if (pRoot != NULL)
        {
            delete pRoot;
            pRoot = NULL;
        }
    }

    void Insert(std::string s);

    bool Search(std::string s);

    void Remove(std::string s);

    void PrintAll() const;

    std::vector<std::string> ListAll() const;

    void OuputTree(std::string& output);

    bool BuildTreeFromStr(std::string output, bool isCompressed=true);

    std::string CompressTrieOutput(std::string strInput);

    bool UncompressTrieOutput(std::string strInput, std::string& strOutput);

private:
    TrieNode* pRoot;

    void print(TrieNode* pRoot) const;

    void list(TrieNode* pRoot, std::vector<std::string>& vec) const;

    int charToIndex(char input);

    char indexToChar(int idx);

    void splitString(const std::string& s, std::vector<std::string>& v, const std::string& c);

    void ouputTree(std::string& output, TrieNode* root);
};

}

#endif //ADDRTRIE_H
