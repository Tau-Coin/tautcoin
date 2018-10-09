#include "addrtrie.h"

namespace TAUAddrTrie
{

void Trie::Insert(std::string s)
{
    int index = 0;
    TrieNode* p = pRoot;
    for(size_t i = 0; i < s.size(); i++)
    {
        index = charToIndex(s[i]);
        if (p->GetNextBranch(index) == NULL)
            p->NewNextBranch(index);
        p = p->GetNextBranch(index);
    }

    if("" == p->GetWord()){
        p->SetWord(s);
    }
}


bool Trie::Search(std::string s)
{
    int index = 0;
    TrieNode* p = pRoot;
    for(size_t i = 0; i < s.size(); i++)
    {
        index = charToIndex(s[i]);
        if (p->GetNextBranch(index) == NULL)
            return false;
        p = p->GetNextBranch(index);
    }

    if(p->GetWord() == s)
        return true;

    return false;
}


void Trie::Remove(std::string s)
{
    int index = 0;
    TrieNode* p = pRoot;
    for(size_t i = 0; i < s.size(); i++)
    {
        index = charToIndex(s[i]);
        if (p->GetNextBranch(index) == NULL)
        {
            //cout<<"该字符串不存在"<<endl;
            return;
        }
        p = p->GetNextBranch(index);
    }
    p->SetWord("");
}


void Trie::PrintAll() const
{
    print(pRoot);
}

void Trie::print(TrieNode* pRoot) const
{
    if(NULL == pRoot)
        return;

    if(pRoot->GetWord() != "")
        std::cout << pRoot->GetWord() << std::endl;

    for(int i = 0; i < MaxBranchSize; i++)
        print(pRoot->GetNextBranch(i));
}

std::vector<std::string> Trie::ListAll() const
{
    std::vector<std::string> vec;
    list(pRoot, vec);
    return vec;
}

void Trie::list(TrieNode* pRoot, std::vector<std::string>& vec) const
{
    if(pRoot == NULL)
        return;

    if(pRoot->GetWord() != "")
        vec.push_back(pRoot->GetWord());

    for(int i = 0; i < MaxBranchSize; i++)
        list(pRoot->GetNextBranch(i), vec);
}

int Trie::charToIndex(char input)
{
    if ((int)(input - 'm') >= 0)
        return (int)(input - 'm') + 44;
    else if ((int)(input - 'a') >= 0)
        return (int)(input - 'a') + 33;
    else if ((int)(input - 'P') >= 0)
        return (int)(input - 'P') + 22;
    else if ((int)(input - 'J') >= 0)
        return (int)(input - 'J') + 17;
    else if ((int)(input - 'A') >= 0)
        return (int)(input - 'A') + 9;
    else if ((int)(input - '1') >= 0)
        return (int)(input - '1');
    else
        return MaxBranchSize;
}

char Trie::indexToChar(int idx)
{
    if (idx < 9)
        return char(idx+49);// ASCII 49 <==> '1'
    else if(idx < 17)
        return char(idx-9+65);// ASCII 65 <==> 'A'
    else if(idx < 22)
        return char(idx-17+74);// ASCII 74 <==> 'J'
    else if(idx < 33)
        return char(idx-22+80);// ASCII 80 <==> 'P'
    else if(idx < 44)
        return char(idx-33+97);// ASCII 97 <==> 'a'
    else if(idx <= MaxBranchSize)
        return char(idx-44+109);// ASCII 109 <==> 'm'
    else
        return '?';
}

void Trie::ouputTree(std::string& output, TrieNode* root)
{
    for(size_t i = 0; i < MaxBranchSize; i++)
    {
        if(root->GetNextBranch(i))
        {
            char c = indexToChar(i);
            output += c;

            if((root->GetNextBranch(i))->GetWord() != "")
                output += "0";

            output += "O";
            ouputTree(output, root->GetNextBranch(i));
            output += "l";
        }
    }
}

/*
* Trie output
*/
void Trie::OuputTree(std::string& output)
{
    output = "";
    ouputTree(output, pRoot);
}

/*
* Build tree from string
*/
bool Trie::BuildTreeFromStr(std::string output, bool isCompressed)
{
    if (output.compare("") == 0)
        return true;

    int index = 0;
    std::string s = "";
    TrieNode* p = pRoot;
    std::string strUncompressed = output;
    if (isCompressed)
        if (!UncompressTrieOutput(output, strUncompressed))
            return false;

    for(size_t i = 0; i < strUncompressed.length(); i++)
    {
        if (i == 0)
        {
            index = charToIndex(strUncompressed[i]);
            if (p->GetNextBranch(index) == NULL)
                p->NewNextBranch(index);
            s += strUncompressed[i];
            p = p->GetNextBranch(index);
        }
        else
        {
            switch(strUncompressed[i])
            {
            case '?':
                return false;
            case 'O':
                if (strUncompressed[i+1] != 'l')
                {
                    index = charToIndex(strUncompressed[i+1]);
                    if (p->GetNextBranch(index) == NULL)
                        p->NewNextBranch(index);
                    s += strUncompressed[i+1];
                    p = p->GetNextBranch(index);
                }
                break;
            case '0':
                p->SetWord(s);
                break;
            case 'l':
                if (strUncompressed[i-1] != 'O')
                {
                    p = p->GetParrent();
                    s.pop_back();
                }
                break;
            default:
                if ((i >= 1) && (strUncompressed[i-1] != 'O'))
                {
                    p = p->GetParrent();
                    if (s.length() > 0)
                        s.pop_back();

                    index = charToIndex(strUncompressed[i]);
                    if (p->GetNextBranch(index) == NULL)
                        p->NewNextBranch(index);
                    s += strUncompressed[i];
                    p = p->GetNextBranch(index);
                }
                break;
            }
        }
    }

    return true;
}

void Trie::splitString(const std::string& s, std::vector<std::string>& v, const std::string& c)
{
    std::string::size_type pos1, pos2;
    pos2 = s.find(c);
    pos1 = 0;
    while(std::string::npos != pos2)
    {
        v.push_back(s.substr(pos1, pos2-pos1));

        pos1 = pos2 + c.size();
        pos2 = s.find(c, pos1);
    }
    if(pos1 != s.length())
        v.push_back(s.substr(pos1));
}

std::string Trie::CompressTrieOutput(std::string strInput)
{
    std::string strOutput = "";
    std::vector<std::string> splitedStr;
    splitString(strInput, splitedStr, std::string("Ol"));
    for(size_t i = 0; i < splitedStr.size(); i++)
    {
        // Delete "O"
        std::string str = splitedStr[i];
        str.erase(std::remove(str.begin(), str.end(), 'O'), str.end());

        // Compress "l"
        int cnt_l = count(str.begin(), str.end(), 'l');
        if (cnt_l > 0)
        {
            std::stringstream cnt_lStream;
            cnt_lStream << cnt_l;
            std::string cnt_lStr;
            cnt_lStream >> cnt_lStr;

            str.erase(std::remove(str.begin(), str.end(), 'l'), str.end());
            if (str.length() != 0)
                str = ";" + cnt_lStr + "l" + str;
        }
        strOutput += str;

        //std::cout<<str<<std::endl;
    }

    //std::cout<<strOutput<<std::endl;
    return strOutput;
}

bool Trie::UncompressTrieOutput(std::string strInput, std::string& strOutput)
{
    strOutput = "";
    std::vector<std::string> splitedStr;
    splitString(strInput, splitedStr, std::string(";"));
    for(size_t i = 0; i < splitedStr.size(); i++)
    {
        std::string str = splitedStr[i];
        int cnt_l = count(str.begin(), str.end(), 'l');
        std::vector<std::string> splitedStr_l;
        std::string extractedStr = "";
        if (cnt_l == 1)
        {
            splitString(str, splitedStr_l, std::string("l"));
            if (splitedStr_l.size() == 2)
            {
                std::istringstream ssVal(splitedStr_l[0]);
                ssVal >> cnt_l;
                extractedStr = splitedStr_l[1];
            }
            else
                return false;
        }
        else if(cnt_l == 0)
            extractedStr = str;
        else
            return false;

        std::string strRestored;
        // Insert "O"
        for(size_t k = 0; k < extractedStr.size(); k++)
        {
            std::string c = extractedStr.substr(k, 1);
            if (k != extractedStr.size()-1)
            {
                if (c.compare("0") != 0)
                    strRestored += (c + "O");
                else
                {
                    strRestored.pop_back();
                    strRestored += "0";
                }
            }
            else if(c.compare("0") == 0)
            {
                strRestored.pop_back();
                strRestored += "0";
            }
        }
        // Restore "l"
        for(int j = 0; j < cnt_l; j++)
            strRestored = "l" + strRestored;

        //std::cout<<strRestored<<std::endl;
        strOutput += strRestored;
    }

    //std::cout<<strOutput<<std::endl;
    return true;
}

}
