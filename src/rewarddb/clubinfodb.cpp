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

CClubInfoDB::CClubInfoDB(size_t nCacheSize, bool fMemory, bool fWipe) :
    CDBWrapper(GetDataDir() / CLUBINFODBPATH, nCacheSize, fMemory, fWipe),
    currentHeight(-1)
{
    batch = new CDBBatch(*this);
}

CClubInfoDB::CClubInfoDB(size_t nCacheSize, CRewardRateViewDB *prewardratedbview,
                         bool fMemory, bool fWipe) :
    CDBWrapper(GetDataDir() / CLUBINFODBPATH, nCacheSize, fMemory, fWipe),
    _prewardratedbview(prewardratedbview),
    currentHeight(-1)
{
    batch = new CDBBatch(*this);
}

CClubInfoDB::~CClubInfoDB()
{
    delete batch;
    batch = NULL;
    _prewardratedbview = NULL;
}

bool CClubInfoDB::WriteDB(const std::string& address, const std::vector<CMemberInfo>& value)
{
    return Write(address, value);
}

bool CClubInfoDB::ReadDB(const std::string& address, vector<CMemberInfo>& value) const
{
    return Read(address, value);
}

bool CClubInfoDB::DeleteDB(const std::string& address)
{
    return Erase(address);
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

bool CClubInfoDB::InitGenesisDB(const std::vector<std::string>& addresses)
{
    for(size_t i = 0; i < addresses.size(); i++)
    {
        vector<CMemberInfo> vmemberInfo;
        CMemberInfo memberInfo(addresses[i], 1, 0);
        vmemberInfo.push_back(memberInfo);
        cacheRecord[addresses[i]] = vmemberInfo;
        if (!WriteDB(addresses[i], cacheRecord[addresses[i]]))
            return false;
    }

    return true;
}

void CClubInfoDB::ClearCache()
{
    cacheRecord.clear();
}

bool CClubInfoDB::Commit()
{
    CommitDB();
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

bool CClubInfoDB::LoadDBToMemory()
{
    boost::scoped_ptr<CDBIterator> pcursor(NewIterator());
    pair<int, string> keyNewestH;
    string key;
    int nHeight = -1;
    pcursor->Seek(make_pair(-1, string()));
    if (pcursor->Valid() && pcursor->GetKey(keyNewestH))
    {
        nHeight = atoi(keyNewestH.second);
        Erase(keyNewestH, true);
    }
    pcursor->SeekToFirst();

    LogPrintf("%s: loading newest records from clubInfodb...\n", __func__);
    while (pcursor->Valid() && pcursor->GetKey(key))
    {
        boost::this_thread::interruption_point();
        try
        {
            //LogPrintf("%s read %c type from coindb...\n", __func__, chType);
            vector<CMemberInfo> vmemberInfo;
            if (!pcursor->GetValue(vmemberInfo)) {
                LogPrintf("%s: unable to read value\n", __func__);
                break;
            }
            cacheRecord[key] = vmemberInfo;

            pcursor->Next();
        }
        catch (const std::exception& e) {
            return error("%s: Deserialize or I/O error - %s\n", __func__, e.what());
        }
    }

    LogPrintf("%s: loaded %d newest records from clubInfodb\n", __func__, cacheRecord.size());
    SetCurrentHeight(nHeight);
    return true;
}

bool CClubInfoDB::WriteDataToDisk(int newestHeight, bool fSync)
{
    LogPrintf("%s: writing newest records to clubInfodb...\n", __func__);
    stringstream ss;
    string heightStr;
    ss << newestHeight;
    ss >> heightStr;
    WriteToBatch(make_pair(-1, heightStr), CMemberInfo());

    for(map<string, vector<CMemberInfo> >::const_iterator it = cacheRecord.begin();
        it != cacheRecord.end(); it++)
    {
        string address = it->first;
        vector<CMemberInfo> value = it->second;
        WriteToBatch(address, value);
    }

    if (CommitDB(fSync))
    {
        LogPrintf("%s: wrote %d newest records to clubInfodb\n", __func__, cacheRecord.size());
        return true;
    }
    else
        return false;
}

bool CClubInfoDB::CacheRecordIsExist(const std::string& address)
{
    return cacheRecord.find(address) != cacheRecord.end();
}

vector<CMemberInfo> CClubInfoDB::GetCacheRecord(const string& address)
{
    return cacheRecord[address];
}

string CClubInfoDB::UpdateMembersByFatherAddress(const string& fatherAddress, const CMemberInfo& memberinfo,
                                                 uint64_t& index, int nHeight, bool add)
{
    LOCK(cs_clubinfo);
    if (add)
    {
        CMemberInfo memberinfoNew = memberinfo;
        if (memberinfo.address.compare(fatherAddress) != 0)
        {
            if ((cacheRecord.find(fatherAddress) == cacheRecord.end()) ||
                (cacheRecord[fatherAddress].size() == 0))
                cacheRecord[fatherAddress].push_back(CMemberInfo(NOT_VALID_RECORD, 0, 0));
            cacheRecord[fatherAddress].push_back(memberinfoNew);
            index = cacheRecord[fatherAddress].size() - 1;
        }
        else
        {
            if ((cacheRecord.find(fatherAddress) == cacheRecord.end()) ||
                (cacheRecord[fatherAddress].size() == 0))
                cacheRecord[fatherAddress].push_back(memberinfoNew);
            else
                cacheRecord[fatherAddress][0] = memberinfoNew;
            index = 0;
        }
        LogPrint("clubinfo", "%s, father: %s, add an address: %s, h:%d\n", __func__, fatherAddress,
                 memberinfo.address, nHeight);
        return NO_MOVED_ADDRESS;
    }
    else
    {
        size_t length = cacheRecord[fatherAddress].size();
        string addressMoved = NO_MOVED_ADDRESS;
        if (index > 0 && index < length-1)
        {
            cacheRecord[fatherAddress][index] = cacheRecord[fatherAddress][length-1];
            addressMoved = cacheRecord[fatherAddress][index].address;
        }
        cacheRecord[fatherAddress].pop_back();
        LogPrint("clubinfo", "%s, father: %s, remove an address: %s, h:%d\n", __func__, fatherAddress,
                 memberinfo.address, nHeight);
        if ((cacheRecord[fatherAddress].size() == 0) ||
            (cacheRecord[fatherAddress].size() == 1 &&
             cacheRecord[fatherAddress][0].address.compare(NOT_VALID_RECORD) == 0))
        {
            cacheRecord.erase(fatherAddress);
            return NO_MOVED_ADDRESS;
        }
        return addressMoved;
    }
}

void CClubInfoDB::GetTotalMembers(const string& fatherAddress, vector<string>& vmembers)
{
    AssertLockHeld(cs_clubinfo);
    const vector<CMemberInfo> &vmemberInfo = cacheRecord[fatherAddress];
    for(size_t i = 1; i < vmemberInfo.size(); i++)
    {
        vmembers.push_back(vmemberInfo[i].address);
        GetTotalMembers(vmemberInfo[i].address, vmembers);
    }
}

void CClubInfoDB::GetTotalMembersByAddress(const string& fatherAddress, vector<string>& vmembers)
{
    LOCK(cs_clubinfo);
    GetTotalMembers(fatherAddress, vmembers);
}

bool CClubInfoDB::ComputeMemberReward(const uint64_t& MP, const uint64_t& totalMP,
                                      const CAmount& totalRewards, CAmount& memberReward)
{
    if ((totalMP < MP) || (totalMP == 0))
    {
        LogPrintf("%s, some of inputs are error, totalMP: %d, miming power: %d\n", __func__, totalMP, MP);
        return false;
    }
    if ((totalRewards >= MAX_MONEY)  || (totalRewards < 0))
    {
        LogPrintf("%s, the rewards are overrange: %d\n", __func__, totalRewards);
        return false;
    }

    arith_uint256 tmp = totalMP;
    arith_uint256 mp = MP;
    arith_uint256 tRwd_1 = totalRewards / (CENT*CENT);
    arith_uint256 tRwd_2 = totalRewards % (CENT*CENT) / CENT;
    arith_uint256 tRwd_3 = totalRewards % (CENT*CENT) % CENT;
    double ratio = mp.getdouble() / tmp.getdouble();
    CAmount memberReward_1 = ratio * tRwd_1.getdouble() * (CENT*CENT);
    CAmount memberReward_2 = ratio * tRwd_2.getdouble() * CENT;
    CAmount memberReward_3 = ratio * tRwd_3.getdouble();
    memberReward = memberReward_1 + memberReward_2 + memberReward_3;
    if (memberReward >= 0)
    {
        if (memberReward > totalRewards)
            memberReward = totalRewards;
        return true;
    }
    else
    {
        LogPrintf("%s, memberReward < 0, miming power: %d, totalMP: %d, totalRewards: %d, memberReward: %d\n",
                  __func__, MP, totalMP, totalRewards, memberReward);
        return false;
    }

    return true;
}

bool CClubInfoDB::UpdateRewards(const string& minerAddress, CAmount memberRewards,
                                uint64_t memberTotalMP, CAmount& distributedRewards, bool isUndo)
{
    AssertLockHeld(cs_clubinfo);
    const vector<CMemberInfo> &vmemberInfo = cacheRecord[minerAddress];
    for(size_t i = 1; i < vmemberInfo.size(); i++)
    {
        CAmount reward = 0;
        if (!ComputeMemberReward(vmemberInfo[i].MP, memberTotalMP, memberRewards, reward))
        {
            LogPrintf("%s, ComputeMemberReward() error, fatherAddress: %s, totalRewards: %d, totalMP: %d\n",
                      __func__, minerAddress, memberRewards, memberTotalMP);
            return false;
        }
        if (isUndo)
            cacheRecord[minerAddress][i].rwd -= reward;
        else
            cacheRecord[minerAddress][i].rwd += reward;
        distributedRewards += reward;
        UpdateRewards(vmemberInfo[i].address, memberRewards, memberTotalMP, distributedRewards, isUndo);
    }

    return true;
}

bool CClubInfoDB::UpdateRewardsByMinerAddress(const string& minerAddress, CAmount memberRewards,
                                              uint64_t memberTotalMP, CAmount& distributedRewards, bool isUndo)
{
    LOCK(cs_clubinfo);
    distributedRewards = 0;
    if (!UpdateRewards(minerAddress, memberRewards, memberTotalMP, distributedRewards, isUndo))
        return false;

    CAmount remainedReward = memberRewards - distributedRewards;
    if (remainedReward < 0)
    {
        LogPrintf("%s, memberRewards < distributedRewards, memberRewards: %d, distributedRewards: %d\n",
                  __func__, memberRewards, distributedRewards);
        return false;
    }

    if (isUndo)
        cacheRecord[minerAddress][0].rwd -= remainedReward;
    else
        cacheRecord[minerAddress][0].rwd += remainedReward;

    return true;
}

bool CClubInfoDB::UpdateMpByChange(string fatherAddr, uint64_t index, bool isUndo,
                                   uint64_t amount, bool add)
{
    if (cacheRecord.find(fatherAddr) == cacheRecord.end())
    {
        LogPrintf("%s, The input father address is : %s, which is not exist\n", __func__, fatherAddr);
        return false;
    }
    if (index >= cacheRecord[fatherAddr].size())
    {
        LogPrintf("%s, The input index is : %d, which is overrange, the max size is: %d\n",
                  __func__, index, cacheRecord[fatherAddr].size());
        return false;
    }

    if (isUndo)
        add = !add;
    if (add)
        cacheRecord[fatherAddr][index].MP += amount;
    else
        cacheRecord[fatherAddr][index].MP -= amount;

    return true;
}

void CClubInfoDB::UpdateRewardByChange(std::string fatherAddr, uint64_t index, CAmount rewardChange, bool isUndo)
{
    if (isUndo)
        rewardChange = 0 - rewardChange;
    cacheRecord[fatherAddr][index].rwd += rewardChange;
}

vector<string> CClubInfoDB::GetAllFathers()
{
    vector<string> fathers;
    LOCK(cs_clubinfo);
    for(map<string, vector<CMemberInfo> >::const_iterator it = cacheRecord.begin();
        it != cacheRecord.end(); it++)
        fathers.push_back(it->first);

    return fathers;
}
