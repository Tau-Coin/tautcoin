#include "addrtrie.h"
#include "clubinfodb.h"

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
            strValue = "";
        else
        {
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

CClubInfoDB::CClubInfoDB()
{
    options.create_if_missing = true;

    std::string db_path = GetDataDir(true).string() + std::string(CLUBINFOPATH);
    LogPrintf("Opening LevelDB in %s\n", db_path);

    leveldb::Status status = leveldb::DB::Open(options, db_path, &pdb);
    dbwrapper_private::HandleError(status);
    assert(status.ok());
    LogPrintf("Opened LevelDB successfully\n");
}

CClubInfoDB::CClubInfoDB(CRewardRateViewDB *prewardratedbview) : _prewardratedbview(prewardratedbview)
{
    options.create_if_missing = true;

    std::string db_path = GetDataDir(true).string() + std::string(CLUBINFOPATH);
    LogPrintf("Opening LevelDB in %s\n", db_path);

    leveldb::Status status = leveldb::DB::Open(options, db_path, &pdb);
    dbwrapper_private::HandleError(status);
    assert(status.ok());
    LogPrintf("Opened LevelDB successfully\n");
}

CClubInfoDB::~CClubInfoDB()
{
    delete pdb;
    pdb = NULL;
    _prewardratedbview = NULL;
}

CRewardRateViewDB* CClubInfoDB::GetRewardRateDBPointer() const
{
    return _prewardratedbview;
}

void CClubInfoDB::ClearCache()
{
    cacheRecord.clear();
}

bool CClubInfoDB::Commit(int nHeight)
{
    if (cacheRecord.size() == 0)
        return true;

    for(std::map<string, string>::const_iterator it = cacheRecord.begin();
        it != cacheRecord.end(); it++)
    {
        string address = it->first;
        string strValue = it->second;
        if (!WriteDB(address, nHeight, strValue))
            return false;
    }

    return true;
}

bool CClubInfoDB::UpdateMembersByFatherAddress(std::string fatherAddress, bool add, std::string address,
                                               int nHeight, bool isUndo)
{
    if (fatherAddress.compare("0") == 0)
        return true;
    else if(fatherAddress.compare(" ") == 0)
        return false;// Error in fatherAddress

//    if (isUndo)
//    {
//        if (!DeleteDB(address, nHeight+1))
//            return false;
//        return true;
//    }

    TAUAddrTrie::Trie trie;
    trie.BuildTreeFromStr(GetTrieStrByFatherAddress(fatherAddress, nHeight-1));
    if (add)
        trie.Insert(address);
    else
        trie.Remove(address);
    string strUncompressed = "";
    trie.OuputTree(strUncompressed);
    string strCompressed = trie.CompressTrieOutput(strUncompressed);
    cacheRecord[fatherAddress] = strCompressed;

    return true;
}

string CClubInfoDB::GetTrieStrByFatherAddress(std::string fatherAddress, int nHeight)
{
    if (cacheRecord.find(fatherAddress) != cacheRecord.end())
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

vector<string> CClubInfoDB::GetTotalMembersByAddress(std::string fatherAddress, int nHeight)
{

    vector<string> members;
    TAUAddrTrie::Trie trie;
    for (int h = nHeight; h >= 0; h--)
    {
        string strCompressed;
        if (ReadDB(fatherAddress, h, strCompressed))
        {
            trie.BuildTreeFromStr(strCompressed);
            members = trie.ListAll();
            for(size_t i = 0; i < members.size(); i++)
            {
                vector<string> childMembers = GetTotalMembersByAddress(members[i], nHeight);
                for(size_t k = 0; k < childMembers.size(); k++)
                    members.push_back(childMembers[k]);
            }

            return members;
        }
    }

    return members;
}
