#include "memberinfodb.h"
#include <boost/algorithm/string.hpp>
#include <boost/function.hpp>

using namespace std;

CMemberInfoDB::CMemberInfoDB(CClubInfoDB *pclubinfodb) : _pclubinfodb(pclubinfodb), currentHeight(-1)
{
    options.create_if_missing = true;

    std::string db_path = GetDataDir(true).string() + std::string(MEMBERINFODBPATH);
    LogPrintf("Opening LevelDB in %s\n", db_path);

    leveldb::Status status = leveldb::DB::Open(options, db_path, &pdb);
    dbwrapper_private::HandleError(status);
    assert(status.ok());
    LogPrintf("Opened LevelDB successfully\n");
}

CMemberInfoDB::~CMemberInfoDB()
{
    delete pdb;
    pdb = NULL;
    _pclubinfodb = NULL;
}

bool CMemberInfoDB::WriteDB(std::string key, int nHeight, string strValue)
{
    std::stringstream ssHeight;
    std::string strHeight;
    ssHeight << nHeight;
    ssHeight >> strHeight;

    leveldb::Status status = pdb->Put(leveldb::WriteOptions(), key+DBSEPECTATOR+strHeight, strValue);
    if(!status.ok())
    {
        LogPrintf("LevelDB write failure in rwdbalance module: %s\n", status.ToString());
        dbwrapper_private::HandleError(status);
        return false;
    }

    return true;
}

bool CMemberInfoDB::WriteDB(std::string key, int nHeight, string packer, string father,
                            uint64_t tc, uint64_t ttc, CAmount value)
{
    string newRecord = " ";
    if (!GenerateRecord(packer, father, tc, ttc, value, newRecord))
    {
        LogPrintf("%s, GenerateRecord error, packer: %s, father: %s, tc: %d, ttc: %d, rwdbal: %d\n",
                  __func__, packer, father, tc, ttc, value);
        return false;
    }
    return WriteDB(key, nHeight, newRecord);
}

bool CMemberInfoDB::ReadDB(std::string key, int nHeight, string &packer, string& father,
                           uint64_t& tc, uint64_t &ttc, CAmount& value, bool dbOnly)
{
    if (!dbOnly && (cacheForRead.find(key) != cacheForRead.end()))
    {
        if (!ParseRecord(cacheForRead[key], packer, father, tc, ttc, value))
            return false;
        return true;
    }

    std::stringstream ssHeight;
    std::string strHeight;
    ssHeight << nHeight;
    ssHeight >> strHeight;

    std::string strValue;
    leveldb::Status status = pdb->Get(leveldb::ReadOptions(), key+DBSEPECTATOR+strHeight, &strValue);
    if(!status.ok())
    {
        if (status.IsNotFound())
        {
            packer = " ";
            father = " ";
            tc = 0;
            ttc = 0;
            value = 0;
        }
        else
        {
            LogPrintf("LevelDB read failure in rwdbalance module: %s\n", status.ToString());
            dbwrapper_private::HandleError(status);
        }
        return false;
    }

    if (!dbOnly)
        cacheForRead[key] = strValue;// Add to cache for accelerating
    if (!ParseRecord(strValue, packer, father, tc, ttc, value))
        return false;

    return true;
}

bool CMemberInfoDB::ReadDB(std::string key, int nHeight, std::string& strValue)
{
    if (cacheForRead.find(key) != cacheForRead.end())
    {
        strValue = cacheForRead[key];
        return true;
    }

    std::stringstream ssHeight;
    std::string strHeight;
    ssHeight << nHeight;
    ssHeight >> strHeight;

    leveldb::Status status = pdb->Get(leveldb::ReadOptions(), key+DBSEPECTATOR+strHeight, &strValue);
    if(!status.ok())
    {
        if (status.IsNotFound())
            strValue = " _ _0_0_0";
        else
        {
            LogPrintf("LevelDB read failure in rwdbalance module: %s\n", status.ToString());
            dbwrapper_private::HandleError(status);
        }
        return false;
    }


    cacheForRead[key] = strValue;// Add to cache for accelerating

    return true;
}

bool CMemberInfoDB::DeleteDB(std::string key, int nHeight)
{
    std::stringstream ssHeight;
    std::string strHeight;
    ssHeight << nHeight;
    ssHeight >> strHeight;

    leveldb::Status status = pdb->Delete(leveldb::WriteOptions(), key+DBSEPECTATOR+strHeight);
    if(!status.ok() && !status.IsNotFound())
    {
        LogPrintf("LevelDB write failure in rwdbalance module: %s\n", status.ToString());
        dbwrapper_private::HandleError(status);
        return false;
    }

    return true;
}

void CMemberInfoDB::ClearCache()
{
    cacheRecord.clear();
    //cacheForRead.clear();
}

void CMemberInfoDB::ClearReadCache()
{
    cacheForRead.clear();
}

bool CMemberInfoDB::UpdateCacheFather(string address, int inputHeight, string newFather, bool isUndo)
{
    if (!CClubInfoDB::AddressIsValid(address))
    {
        LogPrintf("%s, The input address is : %s, which is not valid\n", __func__, address);
        return false;
    }
    if (!CClubInfoDB::AddressIsValid(newFather))
    {
        if (newFather.compare("0") != 0)
        {
            LogPrintf("%s, The input address of newFather is : %s, which is not valid\n", __func__, newFather);
            return false;
        }
    }

    if (isUndo)
    {
        if (!DeleteDB(address, inputHeight+1))
            return false;
        return true;
    }

    CAmount rwdbalance = 0;
    string packer = " ";
    string ftInput = " ";
    uint64_t tc = 0;
    uint64_t ttc = 0;
    GetFullRecord(address, inputHeight-1, packer, ftInput, tc, ttc, rwdbalance);
    ftInput = newFather;
    string newRecordInput = " ";//GenerateRecord(packer, ftInput, tc, ttc, rwdbalance);
    if (!GenerateRecord(packer, ftInput, tc, ttc, rwdbalance, newRecordInput))
    {
        LogPrintf("%s, GenerateRecord error, packer: %s, father: %s, tc: %d, ttc: %d, rwdbal: %d\n",
                  __func__, packer, ftInput, tc, ttc, rwdbalance);
        return false;
    }
    cacheRecord[address] = newRecordInput;

    return true;
}

bool CMemberInfoDB::UpdateCacheFatherAndPacker(string address, int inputHeight, string newAddr, bool isUndo)
{
    if (!CClubInfoDB::AddressIsValid(address))
    {
        LogPrintf("%s, The input address is : %s, which is not valid\n", __func__, address);
        return false;
    }
    if (!CClubInfoDB::AddressIsValid(newAddr))
    {
        if (newAddr.compare("0") != 0)
        {
            LogPrintf("%s, The input new address is : %s, which is not valid\n", __func__, newAddr);
            return false;
        }
    }

    if (isUndo)
    {
        if (!DeleteDB(address, inputHeight+1))
            return false;
        return true;
    }

    CAmount rwdbalance = 0;
    string packerInput = " ";
    string ftInput = " ";
    uint64_t tc = 0;
    uint64_t ttc = 0;
    GetFullRecord(address, inputHeight-1, packerInput, ftInput, tc, ttc, rwdbalance);
    ftInput = newAddr;
    packerInput = newAddr;
    string newRecordInput = " ";
    if (!GenerateRecord(packerInput, ftInput, tc, ttc, rwdbalance, newRecordInput))
    {
        LogPrintf("%s, GenerateRecord error, packer: %s, father: %s, tc: %d, ttc: %d, rwdbal: %d\n",
                  __func__, packerInput, ftInput, tc, ttc, rwdbalance);
        return false;
    }
    cacheRecord[address] = newRecordInput;

    return true;
}

bool CMemberInfoDB::UpdateCachePacker(std::string address, int inputHeight, std::string newPacker, bool isUndo)
{
    if (!CClubInfoDB::AddressIsValid(address))
    {
        LogPrintf("%s, The input address is : %s, which is not valid\n", __func__, address);
        return false;
    }
    if (!CClubInfoDB::AddressIsValid(newPacker))
    {
        if (newPacker.compare("0") != 0)
        {
            LogPrintf("%s, The input address of newPacker is : %s, which is not valid\n", __func__, newPacker);
            return false;
        }
    }

    if (isUndo)
    {
        if (!DeleteDB(address, inputHeight+1))
            return false;
        return true;
    }

    CAmount rwdbalance = 0;
    string packerInput = " ";
    string ft = " ";
    uint64_t tc = 0;
    uint64_t ttc = 0;
    GetFullRecord(address, inputHeight-1, packerInput, ft, tc, ttc, rwdbalance);
    packerInput = newPacker;
    string newRecordInput = " ";//GenerateRecord(packerInput, ft, tc, ttc, rwdbalance);
    if (!GenerateRecord(packerInput, ft, tc, ttc, rwdbalance, newRecordInput))
    {
        LogPrintf("%s, GenerateRecord error, packer: %s, father: %s, tc: %d, ttc: %d, rwdbal: %d\n",
                  __func__, packerInput, ft, tc, ttc, rwdbalance);
        return false;
    }
    cacheRecord[address] = newRecordInput;

    return true;
}

bool CMemberInfoDB::UpdateCacheTcAddOne(string address, int inputHeight, bool isUndo)
{
    if (!CClubInfoDB::AddressIsValid(address))
    {
        LogPrintf("%s, The input address is : %s, which is not valid\n", __func__, address);
        return false;
    }

    if (isUndo)
    {
        if (!DeleteDB(address, inputHeight+1))
            return false;
        return true;
    }

    CAmount rwdbalance = 0;
    string packer = " ";
    string ft = " ";
    uint64_t tcVout = 0;
    uint64_t ttc = 0;
    GetFullRecord(address, inputHeight-1, packer, ft, tcVout, ttc, rwdbalance);
    tcVout++;
    string newRecordVout = " ";//GenerateRecord(packer, ft, tcVout, ttc, rwdbalance);
    if (!GenerateRecord(packer, ft, tcVout, ttc, rwdbalance, newRecordVout))
    {
        LogPrintf("%s, GenerateRecord error, packer: %s, father: %s, tc: %d, ttc: %d, rwdbal: %d\n",
                  __func__, packer, ft, tcVout, ttc, rwdbalance);
        return false;
    }
    cacheRecord[address] = newRecordVout;

    return true;
}

bool CMemberInfoDB::UpdateCacheRewardChange(string address, int inputHeight, CAmount rewardChange, bool isUndo)
{
    if (!CClubInfoDB::AddressIsValid(address))
    {
        LogPrintf("%s, The input address is : %s, which is not valid\n", __func__, address);
        return false;
    }

    if (isUndo)
    {
        if (!DeleteDB(address, inputHeight+1))
            return false;
        return true;
    }

    CAmount rewardbalance_old = 0;
    string packer = " ";
    string ft = " ";
    uint64_t tc = 0;
    uint64_t ttc = 0;
    GetFullRecord(address, inputHeight-1, packer, ft, tc, ttc, rewardbalance_old);
    CAmount newValue = rewardbalance_old + rewardChange;
    string newRecord = " ";
    if (!GenerateRecord(packer, ft, tc, ttc, newValue, newRecord))
    {
        LogPrintf("%s, GenerateRecord error, packer: %s, father: %s, tc: %d, ttc: %d, rwdbal: %d\n",
                  __func__, packer, ft, tc, ttc, newValue);
        return false;
    }
    cacheRecord[address] = newRecord;

    return true;
}

bool CMemberInfoDB::Commit(int nHeight)
{
    _pclubinfodb->Commit(nHeight);

    if (cacheRecord.size() == 0)
        return true;

    for(std::map<string, string>::const_iterator it = cacheRecord.begin();
        it != cacheRecord.end(); it++)
    {
        string address = it->first;
        string strValue = it->second;
        cacheForRead[address] = strValue;
        if (!WriteDB(address, nHeight, strValue))
            return false;
    }

    return true;
}

void CMemberInfoDB::SetCurrentHeight(int nHeight)
{
    currentHeight = nHeight;
}


int CMemberInfoDB::GetCurrentHeight() const
{
    return currentHeight;
}

bool CMemberInfoDB::InitGenesisDB(std::vector<std::string> addresses)
{
    for(uint i = 0; i < addresses.size(); i++)
    {
        CAmount rewardbalance = 0;
        string packer = "0";
        string ft = "0";
        uint64_t tc = 1;
        uint64_t ttc = 1;
        if (!WriteDB(addresses[i], 0, packer, ft, tc, ttc, rewardbalance))
            return false;

        if (!_pclubinfodb->AddClubLeader(addresses[i], 0))
            return false;

        _pclubinfodb->Commit(0);
    }
    return true;
}

bool CMemberInfoDB::ParseRecord(string inputStr, string& packer, string& father,
                                uint64_t& tc, uint64_t& ttc, CAmount& value) const
{
    vector<string> splitedStr;
    boost::split(splitedStr, inputStr, boost::is_any_of(DBSEPECTATOR));
    if (splitedStr.size() != 5)
    {
        LogPrintf("%s, splitedStr.size() != 5, inputStr:%s\n", __func__, inputStr);
        return false;
    }
    packer = splitedStr[0];
    father = splitedStr[1];
    std::istringstream ssVal2(splitedStr[2]);
    ssVal2 >> tc;
    std::istringstream ssVal3(splitedStr[3]);
    ssVal3 >> ttc;
    std::istringstream ssVal4(splitedStr[4]);
    ssVal4 >> value;

    return true;
}

bool CMemberInfoDB::GenerateRecord(string packer, string father, uint64_t tc, uint64_t ttc,
                                   CAmount value, string& outputStr) const
{
    outputStr = "";
    std::stringstream ssVal;
    if ((packer.compare(" ") == 0) || (father.compare(" ") == 0))
        return false;
    ssVal << packer;
    ssVal << DBSEPECTATOR;
    ssVal << father;
    ssVal << DBSEPECTATOR;
    ssVal << tc;
    ssVal << DBSEPECTATOR;
    ssVal << ttc;
    ssVal << DBSEPECTATOR;
    ssVal << value;
    ssVal >> outputStr;

    return true;
}

string CMemberInfoDB::GetPacker(string address, int nHeight)
{
    string packer = " ";
    string ft = " ";
    uint64_t tc = 0;
    uint64_t ttc = 0;
    CAmount value;
    GetFullRecord(address, nHeight, packer, ft, tc, ttc, value);
    return packer;
}

string CMemberInfoDB::GetFather(string address, int nHeight)
{
    string packer = " ";
    string ft = " ";
    uint64_t tc = 0;
    uint64_t ttc = 0;
    CAmount value;
    GetFullRecord(address, nHeight, packer, ft, tc, ttc, value);
    return ft;
}

uint64_t CMemberInfoDB::GetTXCnt(string address, int nHeight)
{
    string packer = " ";
    string ft = " ";
    uint64_t tc = 0;
    uint64_t ttc = 0;
    CAmount value;
    GetFullRecord(address, nHeight, packer, ft, tc, ttc, value);
    return tc;
}

uint64_t CMemberInfoDB::GetTotalTXCnt(string address, int nHeight)
{
    string packer = " ";
    string ft = " ";
    uint64_t tc = 0;
    uint64_t ttc = 0;
    CAmount value;
    GetFullRecord(address, nHeight, packer, ft, tc, ttc, value);
    return ttc;
}

CAmount CMemberInfoDB::GetRwdBalance(std::string address, int nHeight)
{
    string packer = " ";
    string ft = " ";
    uint64_t tc = 0;
    uint64_t ttc = 0;
    CAmount value;
    GetFullRecord(address, nHeight, packer, ft, tc, ttc, value);
    return value;
}

void CMemberInfoDB::GetFullRecord(string address, int nHeight, string& packer, string& father,
                                  uint64_t& tc, uint64_t& ttc, CAmount& value, bool dbOnly)
{
    if (!dbOnly)
    {
        if (cacheRecord.find(address) != cacheRecord.end())
        {
            ParseRecord(cacheRecord[address], packer, father, tc, ttc, value);
            return;
        }
    }

    for (int h = nHeight; h >= 0; h--)
    {
        if (ReadDB(address, h, packer, father, tc, ttc, value, dbOnly))
            return;
    }
}

string CMemberInfoDB::GetFullRecord(std::string address, int nHeight)
{
    string strValue;
    if (cacheRecord.find(address) != cacheRecord.end())
    {
        strValue = cacheRecord[address];
        return strValue;
    }
    else
    {
        for (int h = nHeight; h >= 0; h--)
        {
            if (ReadDB(address, h, strValue))
                return strValue;
        }
    }

    return strValue;
}

bool CMemberInfoDB::RewardChangeUpdate(CAmount rewardChange, string address, int nHeight, bool isUndo)
{
    if (!CClubInfoDB::AddressIsValid(address))
    {
        LogPrintf("%s, The input address is : %s, which is not valid\n", __func__, address);
        return false;
    }

    if (rewardChange >= MAX_MONEY || rewardChange <= -MAX_MONEY)
    {
        LogPrintf("%s, The reward is overrange : %d, which is not valid\n", __func__, rewardChange);
        return false;
    }

    if (isUndo)
    {
        if (!DeleteDB(address, nHeight+1))
            return false;
        return true;
    }

    CBitcoinAddress addr = CBitcoinAddress(address);
    if (addr.IsScript())
        return true;

    if (!UpdateCacheRewardChange(address, nHeight, rewardChange, isUndo))
        return false;

    //LogPrintf("%s, member:%s, rw:%d\n", __func__, member, rewardbalance_old);

    return true;
}

bool CMemberInfoDB::RewardChangeUpdateByPubkey(CAmount rewardChange, string pubKey, int nHeight, bool isUndo)
{
    string address;

    if (pubKey.empty())
        return false;
    const CScript script = CScript() << ParseHex(pubKey) << OP_CHECKSIG;
    CBitcoinAddress addr;
    if (!addr.ScriptPub2Addr(script, address))
        return false;
    if (addr.IsScript())
        return true;

    if (!RewardChangeUpdate(rewardChange, address, nHeight, isUndo))
        return false;

    return true;
}

bool CMemberInfoDB::ComputeMemberReward(const uint64_t& txCnt, const uint64_t& totalTXCnt,
                                        const CAmount& totalRewards, CAmount& memberReward) const
{
    if ((totalTXCnt < txCnt) || (totalTXCnt == 0))
    {
        LogPrintf("%s, some of inputs are error, totalTXCnt: %d, txCnt: %d\n", __func__, totalTXCnt, txCnt);
        return false;
    }
    if ((totalRewards >= MAX_MONEY)  || (totalRewards < 0))
    {
        LogPrintf("%s, the rewards are overrange: %d\n", __func__, totalRewards);
        return false;
    }

    arith_uint256 ttc = totalTXCnt;
    arith_uint256 tc = txCnt;
    arith_uint256 tRwd_1 = totalRewards / (CENT*CENT);
    arith_uint256 tRwd_2 = totalRewards % (CENT*CENT) / CENT;
    arith_uint256 tRwd_3 = totalRewards % (CENT*CENT) % CENT;
    double ratio = tc.getdouble() / ttc.getdouble();
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
        LogPrintf("%s, memberReward < 0, txCnt: %d, totalTXCnt: %d, totalRewards: %d, memberReward: %d\n",
                  __func__, txCnt, totalTXCnt, totalRewards, memberReward);
        return false;
    }
}

bool CMemberInfoDB::InitRewardsDist(CAmount memberTotalRewards, const CScript& scriptPubKey, int nHeight, string& clubLeaderAddress,
                                    CAmount& distributedRewards, map<string, CAmount>& memberRewards)
{
    if (memberTotalRewards < 0)
        return false;

    memberRewards.clear();
    CBitcoinAddress addr;
    map<string, uint64_t> addrToTC;
    uint64_t totalmemberTXCnt = 0;
    if (!addr.ScriptPub2Addr(scriptPubKey, clubLeaderAddress))
        return false;

    uint64_t harvestPower = GetHarvestPowerByAddress(clubLeaderAddress, nHeight-1);
    vector<string> members = _pclubinfodb->GetTotalMembersByAddress(clubLeaderAddress, nHeight-1);
    for(size_t i = 0; i < members.size(); i++)
    {
        uint64_t tXCnt = GetTXCnt(members[i], nHeight-1);
        addrToTC.insert(pair<string, uint64_t>(members[i], tXCnt));
    }
    totalmemberTXCnt = harvestPower - GetTXCnt(clubLeaderAddress, nHeight-1);

    distributedRewards = 0;
    if (totalmemberTXCnt > 0)
    {
        for(std::map<string, uint64_t>::const_iterator it = addrToTC.begin(); it != addrToTC.end(); it++)
        {
            uint64_t TXCnt = it->second;
            CAmount memberReward = 0;
            if (!ComputeMemberReward(TXCnt, totalmemberTXCnt, memberTotalRewards, memberReward))
            {
                LogPrintf("%s, ComputeMemberReward() error, address is: %s\n", __func__, it->first);
                return false;
            }
            if (memberReward > 0)
                memberRewards.insert(pair<string, CAmount>(it->first, memberReward));
            distributedRewards += memberReward;
        }
        CAmount remainedReward = memberTotalRewards - distributedRewards;
        if (remainedReward < 0)
        {
            LogPrintf("%s, memberTotalRewards < distributedRewards, memberTotalRewards: %d, distributedRewards: %d\n",
                      __func__, memberTotalRewards, distributedRewards);
            return false;
        }
    }

    return true;
}

uint64_t CMemberInfoDB::GetHarvestPowerByAddress(std::string address, int nHeight)
{
    if (!CClubInfoDB::AddressIsValid(address))
    {
        LogPrintf("%s, The input address is : %s, which is not valid\n", __func__, address);
        return false;
    }

    string packer;
    string ft;
    uint64_t tc = 0;
    uint64_t ttc = 0;
    CAmount value;
    GetFullRecord(address, nHeight, packer, ft, tc, ttc, value);
    if ((packer.compare("0") == 0) && (ft.compare("0") == 0))
        return ttc;
    else
        return 0;
}

bool CMemberInfoDB::UpdateRewardsByTX(const CTransaction& tx, CAmount blockReward, int nHeight, bool isUndo)
{
    if (!tx.IsCoinBase())
    {
        for(unsigned int i = 0; i < tx.vreward.size(); i++)
        {
            if (!RewardChangeUpdateByPubkey(0-tx.vreward[i].rewardBalance,
                                            tx.vreward[i].senderPubkey, nHeight, isUndo))
                return false;
        }
        return true;
    }

    if (isUndo)
    {
        bool ret = true;
        CBitcoinAddress addr;
        string clubLeaderAddress;
        if (!addr.ScriptPub2Addr(tx.vout[0].scriptPubKey, clubLeaderAddress))
            return false;
        vector<string> members = _pclubinfodb->GetTotalMembersByAddress(clubLeaderAddress, nHeight);
        for(size_t i = 0; i < members.size(); i++)
            ret &= RewardChangeUpdate(0, members[i], nHeight, isUndo);
        return ret;
    }

    // Init rewards distribution and check if valid
    if (tx.vout.size() != 1)
    {
        LogPrintf("Error: The TX's vout size is 0\n");
        return false;
    }
    bool ret = true;
    CAmount distributedRewards = 0;
    string clubLeaderAddress;
    map<string, CAmount> memberRewards;
    CAmount memberTotalRewards = blockReward-tx.vout[0].nValue;
    if (!InitRewardsDist(memberTotalRewards, tx.vout[0].scriptPubKey, nHeight, clubLeaderAddress,
                         distributedRewards, memberRewards))
        return false;

    // Distribute rewards to member and return remained rewards back to club leader
    for(std::map<string, CAmount>::const_iterator it = memberRewards.begin(); it != memberRewards.end(); it++)
        ret &= RewardChangeUpdate(it->second, it->first, nHeight, isUndo);
    CAmount remainedReward = memberTotalRewards - distributedRewards;
    ret &= RewardChangeUpdate(remainedReward, clubLeaderAddress, nHeight, isUndo);

    // Update the reward rate dataset(if required)
    RewardRateUpdate(blockReward, distributedRewards, clubLeaderAddress, nHeight);

    return ret;
}

bool CMemberInfoDB::EntrustByAddress(string inputAddr, string voutAddress, int nHeight, bool isUndo)
{
    if (!CClubInfoDB::AddressIsValid(inputAddr))
    {
        LogPrintf("%s, The input address is : %s, which is not valid\n", __func__, inputAddr);
        return false;
    }
    if (!CClubInfoDB::AddressIsValid(voutAddress))
    {
        LogPrintf("%s, The input address is : %s, which is not valid\n", __func__, voutAddress);
        return false;
    }

    string fatherOfVin, packerOfVin, fatherOfVout, packerOfVout;
    int nHeightQuery = 0;
    if (!isUndo)
        nHeightQuery = nHeight-1;
    else
        nHeightQuery = nHeight;
    fatherOfVin = GetFather(inputAddr, nHeightQuery);
    packerOfVin = GetPacker(inputAddr, nHeightQuery);
    fatherOfVout = GetFather(voutAddress, nHeightQuery);
    packerOfVout = GetPacker(voutAddress, nHeightQuery);

    string newPackerAddr = voutAddress;
    // The address is a new one on the chain
    if ((fatherOfVout.compare(" ") == 0) && (packerOfVout.compare(" ") == 0))
        return true;

    // Address of vout must have both father and packer of "0" in database or entrust itself
    bool changeRelationship = false;

    // Update club info
    // When the vout address is a club leader, the vin entrust the vout(not vin himself)
    if ((voutAddress.compare(inputAddr) != 0) && (fatherOfVin.compare(voutAddress) != 0) &&
        (fatherOfVout.compare("0") == 0) && (packerOfVout.compare("0") == 0))
    {
        if ((fatherOfVin.compare("0") != 0) && (packerOfVin.compare("0") != 0))// The father of vin is not a packer
            _pclubinfodb->UpdateMembersByFatherAddress(fatherOfVin, false, inputAddr, nHeight, isUndo);
        else if(!(fatherOfVin.compare("0") == 0) && (packerOfVin.compare("0") == 0))
        {
            LogPrintf("%s, The input address is error, fatherOfVin: %s, packerOfVin: %s\n",
                      __func__, fatherOfVin, packerOfVin);
            return false;
        }
        _pclubinfodb->UpdateMembersByFatherAddress(voutAddress, true, inputAddr, nHeight, isUndo);
        newPackerAddr = voutAddress;
        changeRelationship = true;

        // Remove this address from leader db
        if ((fatherOfVin.compare("0") == 0) && (packerOfVin.compare("0") == 0))
        {
            if (!isUndo)
                _pclubinfodb->RemoveClubLeader(inputAddr, nHeight + 1);
            else
                _pclubinfodb->AddClubLeader(inputAddr, nHeight);
        }

    }
    // When the vin address is not a club leader and the vin entrust himself
    else if((voutAddress.compare(inputAddr) == 0) &&
            (fatherOfVin.compare("0") != 0) && (packerOfVin.compare("0") != 0))
    {
        _pclubinfodb->UpdateMembersByFatherAddress(fatherOfVin, false, inputAddr, nHeight, isUndo);
        newPackerAddr = "0";
        if (!UpdateCacheTtcByChange(voutAddress, nHeight, 0, false, isUndo))
            return false;
        changeRelationship = true;

        if (!isUndo)
            _pclubinfodb->AddClubLeader(inputAddr, nHeight);
        else
            _pclubinfodb->RemoveClubLeader(inputAddr, nHeight + 1);

    }

    if (changeRelationship)
    {
        // Compute the ttc of the vin address and update the packer of the these members
        vector<string> totalMembers = _pclubinfodb->GetTotalMembersByAddress(inputAddr, nHeightQuery);
        uint64_t ttcOfVin = GetTXCnt(inputAddr, nHeightQuery);
        for(size_t i = 0; i < totalMembers.size(); i++)
        {
            CAmount rwdbalance = 0;
            string packerInput = " ";
            string ft = " ";
            uint64_t tc = 0;
            uint64_t ttc = 0;
            GetFullRecord(totalMembers[i], nHeightQuery, packerInput, ft, tc, ttc, rwdbalance);
            packerInput = voutAddress;
            string newRecordInput = " ";
            if (!GenerateRecord(packerInput, ft, tc, ttc, rwdbalance, newRecordInput))
            {
                LogPrintf("%s, GenerateRecord error, packer: %s, father: %s, tc: %d, ttc: %d, rwdbal: %d\n",
                          __func__, packerInput, ft, tc, ttc, rwdbalance);
                return false;
            }
            cacheRecord[totalMembers[i]] = newRecordInput;
            if (isUndo)
            {
                if (!DeleteDB(totalMembers[i], nHeight+1))
                    return false;
            }

            ttcOfVin += tc;
        }

        // Update the father and packer of the vin address
        if (!UpdateCacheFatherAndPacker(inputAddr, nHeight, newPackerAddr, isUndo))
            return false;

//        // Update the packer of the vin address
//        if (!UpdateCachePacker(inputAddr, nHeight, newPackerAddr, isUndo))
//            return false;

        // Update the ttc of the vin's packer address
        if (packerOfVin.compare("0") != 0)
        {
            if (!UpdateCacheTtcByChange(packerOfVin, nHeight, ttcOfVin, false, isUndo))
                return false;
        }
        else
        {
            if (!UpdateCacheTtcByChange(inputAddr, nHeight, ttcOfVin, false, isUndo))
                return false;
        }

        // Update the ttc of the vout address
        if (!UpdateCacheTtcByChange(voutAddress, nHeight, ttcOfVin, true, isUndo))
            return false;
    }

    // TX count add one, including vout address and packer of vout
    if (!UpdateCacheTcAddOne(voutAddress, nHeight, isUndo))
        return false;
    string newPackerOfVout = GetPacker(voutAddress, nHeight);
    if (newPackerOfVout.compare("0") != 0)
    {
        if (!UpdateCacheTtcByChange(packerOfVout, nHeight, 1, true, isUndo))
            return false;
    }
    else
    {
        if (!UpdateCacheTtcByChange(voutAddress, nHeight, 1, true, isUndo))
            return false;
    }

    return true;
}

bool CMemberInfoDB::UpdateCacheTtcByChange(std::string address, int nHeight, uint64_t count, bool isAdd, bool isUndo)
{
    if (isUndo)
    {
        if (!DeleteDB(address, nHeight+1))
            return false;
        return true;
    }

    if (!CClubInfoDB::AddressIsValid(address))
    {
        LogPrintf("%s, The input address is : %s, which is not valid\n", __func__, address);
        return false;
    }

    string packer = " ";
    string ft = " ";
    uint64_t tc = 0;
    uint64_t ttcInput = 0;
    CAmount rewardbalance = 0;
    GetFullRecord(address, nHeight-1, packer, ft, tc, ttcInput, rewardbalance);
    if (isAdd)
        ttcInput += count;
    else
    {
        if (count == 0)
            ttcInput = 0;
        else if(ttcInput >= count)
            ttcInput -= count;
        else
        {
            LogPrintf("%s, ttc - count error, address:%s, %d - %d\n", __func__, address, ttcInput, count);
            return false;
        }
    }

    // Update cache
    string newRecord = " ";
    if (!GenerateRecord(packer, ft, tc, ttcInput, rewardbalance, newRecord))
    {
        LogPrintf("%s, GenerateRecord error, packer: %s, father: %s, tc: %d, ttc: %d, rwdbal: %d\n",
                  __func__, packer, ft, tc, ttcInput, rewardbalance);
        return false;
    }
    cacheRecord[address] = newRecord;

    //LogPrintf("%s, member:%s, rw:%d\n", __func__, member, rewardbalance_old);

    return true;
}

bool CMemberInfoDB::UpdateTcAndTtcByAddress(string address, int nHeight, string father, bool isUndo)
{
    if (!CClubInfoDB::AddressIsValid(address))
    {
        LogPrintf("%s, The input address is : %s, which is not valid\n", __func__, address);
        return false;
    }
    if (!CClubInfoDB::AddressIsValid(father))
    {
        LogPrintf("%s, The input address of the father is : %s, which is not valid\n", __func__, father);
        return false;
    }

    CAmount rewardbalance = 0;
    string packer = " ";
    string ft = " ";
    uint64_t tc = 0;
    uint64_t ttc = 0;
    bool addressUpdated = false;
    if (!isUndo)
        GetFullRecord(address, nHeight-1, packer, ft, tc, ttc, rewardbalance);
    else
        GetFullRecord(address, nHeight, packer, ft, tc, ttc, rewardbalance);
    if ((ft.compare(" ") == 0) && (packer.compare(" ") == 0))
    {
        // It's a new address on the chain
        // Update father
        ft = father;
        _pclubinfodb->UpdateMembersByFatherAddress(father, true, address, nHeight, isUndo);

        // Update packer
        string packerOfFather;
        if (!isUndo)
            packerOfFather = GetPacker(father, nHeight-1);
        else
            packerOfFather = GetPacker(father, nHeight);

        if (packerOfFather.compare("0") != 0)
            packer = packerOfFather;
        else
            packer = father;

        // Ttc of the address' packer add one
        if (!UpdateCacheTtcByChange(packer, nHeight, 1, true, isUndo))
            return false;
    }
    else if((ft.compare(" ") != 0) && (packer.compare(" ") != 0))
    {
        // It's an existed address on the chain
        // Ttc of the address' packer add one
        if (packer.compare("0") != 0)
        {
            if (!UpdateCacheTtcByChange(packer, nHeight, 1, true, isUndo))
                return false;
        }
        else
        {
            if (!UpdateCacheTtcByChange(address, nHeight, 1, true, isUndo))
                return false;
            addressUpdated = true;
        }
    }

    // Check if father or packer is null
    if ((ft.compare(" ") == 0) || (packer.compare(" ") == 0))
    {
        LogPrintf("%s, The address: %s, whose father or packer is null\n", __func__, address);
        return false;
    }

    // Tc of the address add one
    if (addressUpdated)
        GetFullRecord(address, nHeight-1, packer, ft, tc, ttc, rewardbalance);
    tc++;

    // Update cache
    string newRecord = " ";
    if (!GenerateRecord(packer, ft, tc, ttc, rewardbalance, newRecord))
    {
        LogPrintf("%s, GenerateRecord error, packer: %s, father: %s, tc: %d, ttc: %d, rwdbal: %d\n",
                  __func__, packer, ft, tc, ttc, rewardbalance);
        return false;
    }
    cacheRecord[address] = newRecord;

    //LogPrintf("%s, member:%s, rw:%d\n", __func__, member, rewardbalance_old);

    if (isUndo)
    {
        if (!DeleteDB(address, nHeight+1))
            return false;
        return true;
    }

    return true;
}

bool CMemberInfoDB::GetBestFather(const CTransaction& tx, const CCoinsViewCache& view, string& bestFather,
                                  map<string, CAmount> vin_val, bool isUndo)
{
    bestFather = " ";
    CAmount maxValue = 0;
    if (isUndo)
    {
        for(map<string, CAmount>::const_iterator it = vin_val.begin(); it != vin_val.end(); it++)
        {
            if (it->second > maxValue)
            {
                maxValue = it->second;
                bestFather = it->first;
            }
        }
    }
    else
    {
        for(unsigned int j = 0; j < tx.vin.size(); j++)
        {
            const CCoins* coins;
            coins = view.AccessCoins(tx.vin[j].prevout.hash);
            if (coins == NULL)
            {
                LogPrintf("%s, AccessCoins() failed, no specific coins\n", __func__);
                return false;
            }

            //CTxOut out = view.GetOutputFor(tx.vin[j]);
            CTxOut out;
            CAmount val = 0;
            if (coins->vout.size() > 0)
            {
                out = coins->vout[tx.vin[j].prevout.n];
                val = out.nValue;
            }
            if (val > maxValue)
            {
                maxValue = val;

                const CScript script = out.scriptPubKey;
                CBitcoinAddress addr;
                if (!addr.ScriptPub2Addr(script, bestFather))
                    return false;
            }
        }
    }
    for(unsigned int k = 0; k < tx.vreward.size(); k++)
    {
        CAmount val = tx.vreward[k].rewardBalance;
        if (val > maxValue)
        {
            maxValue = val;

            const CScript script = CScript() << ParseHex(tx.vreward[k].senderPubkey) << OP_CHECKSIG;
            CBitcoinAddress addr;
            if (!addr.ScriptPub2Addr(script, bestFather))
                return false;
        }
    }

    return true;
}

bool CMemberInfoDB::UpdateFatherAndTCByTX(const CTransaction& tx, const CCoinsViewCache& view, int nHeight,
                                          map<string, CAmount> vin_val, bool isUndo)
{
    if (tx.IsCoinBase() && isUndo)
    {
        string packer;
        CBitcoinAddress addr;
        if (!addr.ScriptPub2Addr(tx.vout[0].scriptPubKey, packer))
            return false;
        DeleteDB(packer, nHeight+1);
    }

    if (!tx.IsCoinBase())
    {
        // Get best father
        string bestFather = " ";
        if (!GetBestFather(tx, view, bestFather, vin_val, isUndo))
        {
            LogPrintf("%s, GetBestFather() failed\n", __func__);
            return false;
        }

        if (bestFather.compare(" ") == 0)
        {
            LogPrintf("%s, The bestFather address is : %s, which is not valid\n", __func__, bestFather);
            return false;
        }
        CBitcoinAddress addrFt = CBitcoinAddress(bestFather);
        if (addrFt.IsScript())
            return true;

        // Update packer, father, ttc and tc
        for(unsigned int i = 0; i < tx.vout.size(); i++)
        {
            CTxDestination dst;
            ExtractDestinationFromP2PKAndP2PKH(tx.vout[i].scriptPubKey, dst);
            string voutAddress = CBitcoinAddress(dst).ToString();
            CBitcoinAddress addrVout = CBitcoinAddress(voutAddress);
            if (addrVout.IsScript())
                return true;

            if (tx.vout[i].nValue == 0)
            {
                if (!EntrustByAddress(bestFather, voutAddress, nHeight, isUndo))
                    return false;
            }
            else
            {
                if (!UpdateTcAndTtcByAddress(voutAddress, nHeight, bestFather, isUndo))
                    return false;
            }
        }
    }

    return true;
}

bool CMemberInfoDB::RewardRateUpdate(CAmount blockReward, CAmount distributedRewards, string clubLeaderAddress, int nHeight)
{
    bool updateRewardRate = false;
    CRewardRateViewDB* prewardratedb = _pclubinfodb->GetRewardRateDBPointer();
    if (mapArgs.count("-updaterewardrate") && mapMultiArgs["-updaterewardrate"].size() > 0)
    {
        string flag = mapMultiArgs["-updaterewardrate"][0];
        if (flag.compare("true") == 0)
            updateRewardRate = true;
    }
    if (updateRewardRate && blockReward > 0)
    {
        arith_uint256 totalval = blockReward;
        arith_uint256 distributedval = distributedRewards;
        double rewardRate = distributedval.getdouble() / totalval.getdouble();
        if (!prewardratedb->UpdateRewardRate(clubLeaderAddress, rewardRate, nHeight))
        {
            LogPrintf("Warning: UpdateRewardRate failed!");
            return false;
        }
    }
    else if(updateRewardRate)
    {
        if (!prewardratedb->UpdateRewardRate(clubLeaderAddress, -1, nHeight))
        {
            LogPrintf("Warning: UpdateRewardRate failed!");
            return false;
        }
    }

    return true;
}
