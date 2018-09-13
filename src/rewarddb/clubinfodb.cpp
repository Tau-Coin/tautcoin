#include "addrtrie.h"
#include "clubinfodb.h"
#include <time.h>

CCriticalSection cs_clubinfo;

using namespace std;

bool CRewardRateViewDB::WriteDB(int nHeight, std::string address, double value)
{
    std::stringstream ssVal;
    ssVal << value;
    std::string strValue;
    ssVal >> strValue;

    std::stringstream ssHeight;
    std::string strHeight;
    ssHeight << nHeight;
    ssHeight >> strHeight;

    leveldb::Status status = pdb->Put(leveldb::WriteOptions(), strHeight, address+"_"+strValue);
    if(!status.ok())
    {
        LogPrintf("LevelDB write failure in reward rate module: %s\n", status.ToString());
        dbwrapper_private::HandleError(status);
        return false;
    }

    return true;
}

bool CRewardRateViewDB::ReadDB(int nHeight, std::string& address_value)
{
    std::stringstream ssHeight;
    std::string strHeight;
    ssHeight << nHeight;
    ssHeight >> strHeight;

    leveldb::Status status = pdb->Get(leveldb::ReadOptions(), strHeight, &address_value);
    if(!status.ok())
    {
        LogPrintf("LevelDB read failure in reward rate module: %s\n", status.ToString());
        dbwrapper_private::HandleError(status);
        return false;
    }

    return true;
}

CRewardRateViewDB::CRewardRateViewDB()
{
    options.create_if_missing = true;

    std::string db_path = GetDataDir(true).string() + std::string(RWDBALDBRATEPATH);
    LogPrintf("Opening LevelDB in %s\n", db_path);

    leveldb::Status status = leveldb::DB::Open(options, db_path, &pdb);
    dbwrapper_private::HandleError(status);
    assert(status.ok());
    LogPrintf("Opened LevelDB successfully\n");
}

CRewardRateViewDB::~CRewardRateViewDB()
{
    delete pdb;
    pdb = NULL;
}

bool CRewardRateViewDB::GetRewardRate(int nHeight, string& addr_rate)
{
    if (!ReadDB(nHeight, addr_rate))
        return false;

    return true;
}

bool CRewardRateViewDB::UpdateRewardRate(std::string leaderAddress, double val, int nHeight)
{
    if (!CClubInfoDB::AddressIsValid(leaderAddress))
    {
        LogPrintf("%s, The input address is : %s, which is not valid\n", __func__, leaderAddress);
        return false;
    }


    if ((val < 0 || val > 1.0) && val != -1)
        return false;
    if (!WriteDB(nHeight, leaderAddress, val))
        return false;

    return true;
}

bool CClubInfoDB::WriteDB(std::string key, int nHeight, string strValue)
{
    std::stringstream ssHeight;
    std::string strHeight;
    ssHeight << nHeight;
    ssHeight >> strHeight;

    leveldb::Status status = pdb->Put(leveldb::WriteOptions(), key+DBSEPECTATOR+strHeight, strValue);
    if(!status.ok())
    {
        LogPrintf("LevelDB write failure in clubinfo module: %s\n", status.ToString());
        dbwrapper_private::HandleError(status);
        return false;
    }

    return true;
}

bool CClubInfoDB::ReadDB(std::string key, int nHeight, std::string& strValue)
{
    std::stringstream ssHeight;
    std::string strHeight;
    ssHeight << nHeight;
    ssHeight >> strHeight;

    leveldb::Status status = pdb->Get(leveldb::ReadOptions(), key+DBSEPECTATOR+strHeight, &strValue);
    if(!status.ok())
    {
        if (status.IsNotFound())
            strValue = " ";
        else
        {
            strValue = "#";
            LogPrintf("LevelDB read failure in clubinfo module: %s\n", status.ToString());
            dbwrapper_private::HandleError(status);
        }
        return false;
    }

    return true;
}

bool CClubInfoDB::DeleteDB(std::string key, int nHeight)
{
    std::stringstream ssHeight;
    std::string strHeight;
    ssHeight << nHeight;
    ssHeight >> strHeight;

    leveldb::Status status = pdb->Delete(leveldb::WriteOptions(), key+DBSEPECTATOR+strHeight);
    if(!status.ok() && !status.IsNotFound())
    {
        LogPrintf("LevelDB write failure in clubinfo module: %s\n", status.ToString());
        dbwrapper_private::HandleError(status);
        return false;
    }

    return true;
}

CClubInfoDB::CClubInfoDB() : currentHeight(-1)
{
    options.create_if_missing = true;

    std::string db_path = GetDataDir(true).string() + std::string(CLUBINFOPATH);
    LogPrintf("Opening LevelDB in %s\n", db_path);

    leveldb::Status status = leveldb::DB::Open(options, db_path, &pdb);
    dbwrapper_private::HandleError(status);
    assert(status.ok());
    LogPrintf("Opened LevelDB successfully\n");

    pclubleaderdb = new CClubLeaderDB();
}

CClubInfoDB::CClubInfoDB(CRewardRateViewDB *prewardratedbview) : _prewardratedbview(prewardratedbview),  currentHeight(-1)
{
    options.create_if_missing = true;

    std::string db_path = GetDataDir(true).string() + std::string(CLUBINFOPATH);
    LogPrintf("Opening LevelDB in %s\n", db_path);

    leveldb::Status status = leveldb::DB::Open(options, db_path, &pdb);
    dbwrapper_private::HandleError(status);
    assert(status.ok());
    LogPrintf("Opened LevelDB successfully\n");

    pclubleaderdb = new CClubLeaderDB();
}

CClubInfoDB::~CClubInfoDB()
{
    delete pdb;
    pdb = NULL;
    _prewardratedbview = NULL;

    delete pclubleaderdb;
    pclubleaderdb = NULL;
}

CRewardRateViewDB* CClubInfoDB::GetRewardRateDBPointer() const
{
    return _prewardratedbview;
}

bool CClubInfoDB::AddressIsValid(string address)
{
    CBitcoinAddress addr = CBitcoinAddress(address);
    if (!addr.IsValid())
        return false;
    return true;
}

void CClubInfoDB::ClearCache()
{
    cacheRecord.clear();
}

void CClubInfoDB::ClearReadCache()
{
    cacheForRead.clear();
}

bool CClubInfoDB::Commit(int nHeight)
{
    pclubleaderdb->Commit();

    if (cacheRecord.size() == 0)
        return true;

    for(std::map<string, string>::const_iterator it = cacheRecord.begin();
        it != cacheRecord.end(); it++)
    {
        string address = it->first;
        string strValue = it->second;
        if (!WriteDB(address, nHeight, strValue))
            return false;

        // Add to cache for accelerating
//        TAUAddrTrie::Trie trie;
//        trie.BuildTreeFromStr(strValue, false);
//        vector<string> members = trie.ListAll();
//        cacheForRead[address] = members;

        // =====Temporary program
        vector<string> splitedStr;
        boost::split(splitedStr, strValue, boost::is_any_of(DBSEPECTATOR));
        if (splitedStr.size() > 0)
            cacheForRead[address] = splitedStr;
    }

    return true;
}

void CClubInfoDB::SetCurrentHeight(int nHeight)
{
    currentHeight = nHeight;
}


int CClubInfoDB::GetCurrentHeight() const
{
    return currentHeight;
}

bool CClubInfoDB::UpdateMembersByFatherAddress(std::string fatherAddress, bool add, std::string address,
                                               int nHeight, bool isUndo)
{
    if (!CClubInfoDB::AddressIsValid(fatherAddress))
    {
        LogPrintf("%s, The input address is : %s, which is not valid\n", __func__, fatherAddress);
        return false;
    }
    if (!CClubInfoDB::AddressIsValid(address))
    {
        LogPrintf("%s, The input address is : %s, which is not valid\n", __func__, address);
        return false;
    }

    if (isUndo)
    {
        if (!DeleteDB(fatherAddress, nHeight+1))
            return false;
        return true;
    }

//    TAUAddrTrie::Trie trie;
//    trie.BuildTreeFromStr(GetTrieStrByFatherAddress(fatherAddress, nHeight-1), false);
//    if (add)
//    {
//        trie.Insert(address);
//        LogPrint("clubinfo", "%s, father: %s, add an address to trie: %s, h:%d\n", __func__, fatherAddress,
//            address, nHeight);
//    }
//    else
//    {
//        trie.Remove(address);
//        LogPrint("clubinfo", "%s, father: %s, delete an address from trie: %s, h:%d\n", __func__, fatherAddress,
//            address, nHeight);
//    }
//    string strUncompressed = "";
//    trie.OuputTree(strUncompressed);
//    //string strCompressed = trie.CompressTrieOutput(strUncompressed);
//    cacheRecord[fatherAddress] = strUncompressed;

    // =====Temporary program
    string strUncompressed = " ";
    TRY_LOCK(cs_clubinfo, cachelock);
    if (cachelock && (cacheRecord.find(fatherAddress) != cacheRecord.end()))
        strUncompressed = cacheRecord[fatherAddress];
    else
    {
        for (int h = nHeight; h >= 0; h--)
        {
            if (ReadDB(fatherAddress, h, strUncompressed))
                break;
        }
    }
    if (add)
    {
        if (strUncompressed.compare(" ") == 0)
            strUncompressed = address;
        else
            strUncompressed = strUncompressed + DBSEPECTATOR + address;
    }
    else
    {
        if (strUncompressed.compare(" ") != 0)
        {
            vector<string> splitedStr;
            boost::split(splitedStr, strUncompressed, boost::is_any_of(DBSEPECTATOR));
            strUncompressed = " ";
            if (address.compare(splitedStr[0]) != 0)
                strUncompressed = splitedStr[0];
            for(size_t i = 1; i < splitedStr.size(); i++)
            {
                if (address.compare(splitedStr[i]) != 0)
                {
                    if (strUncompressed.compare(" ") == 0)
                        strUncompressed = splitedStr[i];
                    else
                        strUncompressed = strUncompressed + DBSEPECTATOR + splitedStr[i];
                }
            }
        }
    }
    cacheRecord[fatherAddress] = strUncompressed;

    return true;
}

string CClubInfoDB::GetTrieStrByFatherAddress(std::string fatherAddress, int nHeight)
{
    TRY_LOCK(cs_clubinfo, cachelock);
    if (cachelock && (cacheRecord.find(fatherAddress) != cacheRecord.end()))
        return cacheRecord[fatherAddress];
    else
    {
        for (int h = nHeight; h >= 0; h--)
        {
            string strCompressed = "";
            if (ReadDB(fatherAddress, h, strCompressed))
                return strCompressed;
        }
    }

    return "";
}

vector<string> CClubInfoDB::GetTotalMembersByAddress(std::string fatherAddress, int nHeight, bool dbOnly)
{
    vector<string> members;
    TAUAddrTrie::Trie trie;
    if (!dbOnly)
    {
        TRY_LOCK(cs_clubinfo, cachelock);
        if (cachelock && (cacheRecord.find(fatherAddress) != cacheRecord.end()))
        {
//            trieCache.BuildTreeFromStr(cacheRecord[fatherAddress], false);
//            members = trieCache.ListAll();
            // =====Temporary program
            vector<string> splitedStr;
            boost::split(splitedStr, cacheRecord[fatherAddress], boost::is_any_of(DBSEPECTATOR));

            for (vector<string>::iterator it = splitedStr.begin();
                 it != splitedStr.end(); it++)
            {
                if (it->compare(" ") && !it->empty())
                    members.push_back(*it);
            }

            for(size_t i = 0; i < members.size(); i++)
            {
                vector<string> childMembers = GetTotalMembersByAddress(members[i], nHeight, dbOnly);
                for(size_t k = 0; k < childMembers.size(); k++)
                {
                    if (childMembers[k].compare(" ") && !childMembers[k].empty())
                    {
                        members.push_back(childMembers[k]);
                        LogPrint("clubinfo", "%s, cache father: %s, get address added: %s, h:%d\n", __func__, members[i],
                            childMembers[k], nHeight);
                    }
                }
            }

            return members;
        }

        if (nHeight == currentHeight && cacheForRead.find(fatherAddress) != cacheForRead.end())
        {
            vector<string> tempVec = cacheForRead[fatherAddress];
            for (vector<string>::iterator it = tempVec.begin();
                 it != tempVec.end(); it++)
            {
                if (it->compare(" ") && !it->empty())
                    members.push_back(*it);
            }

            for(size_t i = 0; i < members.size(); i++)
            {
                vector<string> childMembers = GetTotalMembersByAddress(members[i], nHeight, dbOnly);
                for(size_t k = 0; k < childMembers.size(); k++)
                {
                    if (childMembers[k].compare(" ") && !childMembers[k].empty())
                    {
                        members.push_back(childMembers[k]);
                        LogPrint("clubinfo", "%s, cacheRead father: %s, get address added: %s, h:%d\n", __func__, members[i],
                            childMembers[k], nHeight);
                    }
                }
            }

            return members;
        }
    }

    for (int h = nHeight; h >= 0; h--)
    {
        string strCompressed;
        if (ReadDB(fatherAddress, h, strCompressed))
        {
//            trie.BuildTreeFromStr(strCompressed, false);
//            members = trie.ListAll();
            // =====Temporary program
            vector<string> splitedStr;
            boost::split(splitedStr, strCompressed, boost::is_any_of(DBSEPECTATOR));

            for (vector<string>::iterator it = splitedStr.begin();
                 it != splitedStr.end(); it++)
            {
                if (it->compare(" ") && !it->empty())
                    members.push_back(*it);
            }

            for(size_t i = 0; i < members.size(); i++)
            {
                vector<string> childMembers = GetTotalMembersByAddress(members[i], nHeight, dbOnly);
                for(size_t k = 0; k < childMembers.size(); k++)
                {
                    if (childMembers[k].compare(" ") && !childMembers[k].empty())
                    {
                        members.push_back(childMembers[k]);
                        LogPrint("clubinfo", "%s, db father: %s, get address added: %s, h:%d\n", __func__, members[i],
                            childMembers[k], nHeight);
                    }
                }
            }

            return members;
        }
        else if(strCompressed.compare("#") == 0)
        {
            LogPrintf("%s recursion exception on %d %s have no child or read db fail\n", __func__, nHeight, fatherAddress);
            //assert(strCompressed == "");
        }
    }

    if (nHeight == currentHeight && !dbOnly)
        cacheForRead[fatherAddress] = members;// Add to cache for accelerating

    return members;
}

bool CClubInfoDB::AddClubLeader(std::string address, int height)
{
    if (!CClubInfoDB::AddressIsValid(address))
    {
        LogPrintf("%s, The input address is : %s, which is not valid\n", __func__, address);
        return false;
    }


    return pclubleaderdb->AddClubLeader(address, height);
}

bool CClubInfoDB::RemoveClubLeader(std::string address, int height)
{
    if (!CClubInfoDB::AddressIsValid(address))
    {
        LogPrintf("%s, The input address is : %s, which is not valid\n", __func__, address);
        return false;
    }


    return pclubleaderdb->RemoveClubLeader(address, height);
}

bool CClubInfoDB::DeleteClubLeader(std::string address, int height)
{
    if (!CClubInfoDB::AddressIsValid(address))
    {
        LogPrintf("%s, The input address is : %s, which is not valid\n", __func__, address);
        return false;
    }

    return pclubleaderdb->DeleteClubLeader(address, height);
}

bool CClubInfoDB::GetAllClubLeaders(std::vector<std::string>& leaders, int height)
{
    return pclubleaderdb->GetAllClubLeaders(leaders, height);
}
