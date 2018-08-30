#include "memberinfodb.h"
#include <boost/algorithm/string.hpp>
#include <boost/function.hpp>

using namespace std;

CMemberInfoDB::CMemberInfoDB(CClubInfoDB *pclubinfodb) : _pclubinfodb(pclubinfodb)
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
    return WriteDB(key, nHeight, GenerateRecord(packer, father, tc, ttc, value));
}

bool CMemberInfoDB::ReadDB(std::string key, int nHeight, string &packer, string& father,
                           uint64_t& tc, uint64_t &ttc, CAmount& value)
{
    if (cacheForRead.find(key) != cacheForRead.end())
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
    cacheForRead.clear();
}

void CMemberInfoDB::UpdateCacheFather(string address, int inputHeight, string newFather)
{
    CAmount rwdbalance = 0;
    string packer = " ";
    string ftInput = " ";
    uint64_t tc = 0;
    uint64_t ttc = 0;
    GetFullRecord(address, inputHeight-1, packer, ftInput, tc, ttc, rwdbalance);
    ftInput = newFather;
    string newRecordInput = GenerateRecord(packer, ftInput, tc, ttc, rwdbalance);
    cacheRecord[address] = newRecordInput;
}

void CMemberInfoDB::UpdateCachePacker(std::string address, int inputHeight, std::string newPacker)
{
    CAmount rwdbalance = 0;
    string packerInput = " ";
    string ft = " ";
    uint64_t tc = 0;
    uint64_t ttc = 0;
    GetFullRecord(address, inputHeight-1, packerInput, ft, tc, ttc, rwdbalance);
    packerInput = newPacker;
    string newRecordInput = GenerateRecord(packerInput, ft, tc, ttc, rwdbalance);
    cacheRecord[address] = newRecordInput;
}

void CMemberInfoDB::UpdateCacheTcAddOne(string address, int inputHeight)
{
    CAmount rwdbalance = 0;
    string packer = " ";
    string ft = " ";
    uint64_t tcVout = 0;
    uint64_t ttc = 0;
    GetFullRecord(address, inputHeight-1, packer, ft, tcVout, ttc, rwdbalance);
    tcVout++;
    string newRecordVout = GenerateRecord(packer, ft, tcVout, ttc, rwdbalance);
    cacheRecord[address] = newRecordVout;
}

void CMemberInfoDB::UpdateCacheRewardChange(string address, int inputHeight, CAmount rewardChange)
{
    CAmount rewardbalance_old = 0;
    string packer = " ";
    string ft = " ";
    uint64_t tc = 0;
    uint64_t ttc = 0;
    GetFullRecord(address, inputHeight-1, packer, ft, tc, ttc, rewardbalance_old);
    CAmount newValue = rewardbalance_old + rewardChange;
    string newRecord = GenerateRecord(packer, ft, tc, ttc, newValue);
    cacheRecord[address] = newRecord;
}

bool CMemberInfoDB::Commit(int nHeight)
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

    _pclubinfodb->Commit(nHeight);

    return true;
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

        if (!_pclubinfodb->AddClubLeader(addresses[i]))
            return false;
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

string CMemberInfoDB::GenerateRecord(string packer, string father, uint64_t tc, uint64_t ttc, CAmount value) const
{
    string outputStr;
    std::stringstream ssVal;
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

    return outputStr;
}

string CMemberInfoDB::GetPacker(string address, int nHeight)
{
    string packer;
    string ft;
    uint64_t tc = 0;
    uint64_t ttc = 0;
    CAmount value;
    GetFullRecord(address, nHeight, packer, ft, tc, ttc, value);
    return packer;
}

string CMemberInfoDB::GetFather(string address, int nHeight)
{
    string packer;
    string ft;
    uint64_t tc = 0;
    uint64_t ttc = 0;
    CAmount value;
    GetFullRecord(address, nHeight, packer, ft, tc, ttc, value);
    return ft;
}

uint64_t CMemberInfoDB::GetTXCnt(string address, int nHeight)
{
    string packer;
    string ft;
    uint64_t tc = 0;
    uint64_t ttc = 0;
    CAmount value;
    GetFullRecord(address, nHeight, packer, ft, tc, ttc, value);
    return tc;
}

uint64_t CMemberInfoDB::GetTotalTXCnt(string address, int nHeight)
{
    string packer;
    string ft;
    uint64_t tc = 0;
    uint64_t ttc = 0;
    CAmount value;
    GetFullRecord(address, nHeight, packer, ft, tc, ttc, value);
    return ttc;
}

CAmount CMemberInfoDB::GetRwdBalance(std::string address, int nHeight)
{
    string packer;
    string ft;
    uint64_t tc = 0;
    uint64_t ttc = 0;
    CAmount value;
    GetFullRecord(address, nHeight, packer, ft, tc, ttc, value);
    return value;
}

void CMemberInfoDB::GetFullRecord(string address, int nHeight, string& packer, string& father,
                                  uint64_t& tc, uint64_t& ttc, CAmount& value)
{
    if (cacheRecord.find(address) != cacheRecord.end())
    {
        ParseRecord(cacheRecord[address], packer, father, tc, ttc, value);
        return;
    }
    else
    {
        for (int h = nHeight; h >= 0; h--)
        {
            if (ReadDB(address, h, packer, father, tc, ttc, value))
                return;
        }
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
    if (rewardChange >= MAX_MONEY || rewardChange <= -MAX_MONEY)
        return false;

//    if (isUndo)
//    {
//        if (!DeleteDB(address, nHeight+1))
//            return false;
//        return true;
//    }

    UpdateCacheRewardChange(address, nHeight, rewardChange);

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

    if (!RewardChangeUpdate(rewardChange, address, nHeight, isUndo))
        return false;

    return true;
}

bool CMemberInfoDB::ComputeMemberReward(const uint64_t& txCnt, const uint64_t& totalTXCnt,
                                        const CAmount& totalRewards, CAmount& memberReward) const
{
    if (totalTXCnt < txCnt || totalTXCnt == 0 || totalRewards < 0)
        return false;
    if (totalRewards >= MAX_MONEY)
    {
        LogPrintf("Error: The rewards are too large\n");
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
                return false;
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
//    uint64_t hPower = 0;
//    vector<string> clubMembers = _pclubinfodb->GetTotalMembersByAddress(address, nHeight);
//    hPower += GetTXCnt(address, nHeight);
//    for(size_t i = 0; i < clubMembers.size(); i++)
//        hPower += GetTXCnt(clubMembers[i], nHeight);
//    return hPower;

    string packer;
    string ft;
    uint64_t tc = 0;
    uint64_t ttc = 0;
    CAmount value;
    GetFullRecord(address, nHeight, packer, ft, tc, ttc, value);
    if ((packer.compare("0") == 0) && (ft.compare("0") == 0))
        return GetTotalTXCnt(address, nHeight);
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
//    if (isUndo)
//    {
//        if (GetFather(voutAddress, nHeight).compare("0") == 0)
//        {
//            if (!DeleteDB(inputAddr, nHeight+1))
//                return false;

//            _pclubinfodb->UpdateMembersByFatherAddress(voutAddress, true, inputAddr, nHeight, isUndo);
//            _pclubinfodb->UpdateMembersByFatherAddress(GetFather(inputAddr, nHeight), false, inputAddr, nHeight, isUndo);
//        }
//        if (!DeleteDB(voutAddress, nHeight+1))
//            return false;
//        return true;
//    }

    // Address of vout must have both father and packer of "0" in database or entrust itself
    string fatherOfVin = GetFather(inputAddr, nHeight-1);
    string packerOfVin = GetPacker(inputAddr, nHeight-1);
    string fatherOfVout = GetFather(voutAddress, nHeight-1);
    string packerOfVout = GetPacker(voutAddress, nHeight-1);
    string newPackerAddr = voutAddress;
    bool changeRelationship = false;

    // Compute the total TX count of the vin address and update the packer of the these members
    vector<string> totalMembers = _pclubinfodb->GetTotalMembersByAddress(inputAddr, nHeight-1);
    uint64_t ttcOfVin = GetTXCnt(inputAddr, nHeight-1);
    for(size_t i = 0; i < totalMembers.size(); i++)
    {
        CAmount rwdbalance = 0;
        string packerInput = " ";
        string ft = " ";
        uint64_t tc = 0;
        uint64_t ttc = 0;
        GetFullRecord(totalMembers[i], nHeight-1, packerInput, ft, tc, ttc, rwdbalance);
        packerInput = voutAddress;
        string newRecordInput = GenerateRecord(packerInput, ft, tc, ttc, rwdbalance);
        cacheRecord[totalMembers[i]] = newRecordInput;

        ttcOfVin += tc;
    }

    // Update club info
    // When the vout address is a club leader, the vin entrust the vout(not vin himself)
    if ((voutAddress.compare(inputAddr) != 0) && (fatherOfVin.compare(voutAddress) != 0) &&
        (fatherOfVout.compare("0") == 0) && (packerOfVout.compare("0") == 0))
    {
        _pclubinfodb->UpdateMembersByFatherAddress(fatherOfVin, false, inputAddr, nHeight, isUndo);
        _pclubinfodb->UpdateMembersByFatherAddress(voutAddress, true, inputAddr, nHeight, isUndo);
        newPackerAddr = voutAddress;
        changeRelationship = true;

        // If input address is a club leader
        if ((fatherOfVin.compare("0") == 0) && (packerOfVin.compare("0") == 0))
        {
            if (!isUndo)
                _pclubinfodb->RemoveClubLeader(inputAddr); // Remove this address from leader db
            else
                _pclubinfodb->AddClubLeader(inputAddr);
        }
    }
    // When the vin address is not a club leader and the vin entrust himself
    else if((voutAddress.compare(inputAddr) == 0) &&
            (fatherOfVin.compare("0") != 0) && (packerOfVin.compare("0") != 0))
    {
        _pclubinfodb->UpdateMembersByFatherAddress(fatherOfVin, false, inputAddr, nHeight, isUndo);
        newPackerAddr = "0";
        changeRelationship = true;

        if (!isUndo)
            _pclubinfodb->AddClubLeader(inputAddr);
        else
            _pclubinfodb->RemoveClubLeader(inputAddr);
    }

    if (changeRelationship)
    {
        // Update the father of the vin address
        UpdateCacheFather(inputAddr, nHeight, newPackerAddr);

        // Update the packer of the vin address
        UpdateCachePacker(inputAddr, nHeight, newPackerAddr);

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
    UpdateCacheTcAddOne(voutAddress, nHeight);
    if (newPackerAddr.compare("0") != 0)
    {
        if (!UpdateCacheTtcByChange(newPackerAddr, nHeight, 1, true, isUndo))
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
//    if (isUndo)
//    {
//        if (!DeleteDB(address, nHeight+1))
//            return false;
//        return true;
//    }

    CBitcoinAddress addr = CBitcoinAddress(address);
    if (!addr.IsValid())
        return false;

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
    string newRecord = GenerateRecord(packer, ft, tc, ttcInput, rewardbalance);
    cacheRecord[address] = newRecord;

    //LogPrintf("%s, member:%s, rw:%d\n", __func__, member, rewardbalance_old);

    return true;
}

bool CMemberInfoDB::UpdateTcAndTtcByAddress(string address, int nHeight, string father, bool isUndo)
{
//    if (isUndo)
//    {
//        if (!DeleteDB(address, nHeight+1))
//            return false;
//        return true;
//    }

    CAmount rewardbalance = 0;
    string packer = " ";
    string ft = " ";
    uint64_t tc = 0;
    uint64_t ttc = 0;
    bool addressUpdated = false;
    GetFullRecord(address, nHeight-1, packer, ft, tc, ttc, rewardbalance);
    if ((ft.compare(" ") == 0) && (packer.compare(" ") == 0))
    {
        // It's a new address on chain
        // Update father
        ft = father;
        _pclubinfodb->UpdateMembersByFatherAddress(father, true, address, nHeight, isUndo);

        // Update packer
        string packerOfFather = GetPacker(father, nHeight-1);
        if (packerOfFather.compare("0") != 0)
            packer = packerOfFather;
        else
            packer = father;

        // Ttc of packer add one
        if (!UpdateCacheTtcByChange(packer, nHeight, 1, true, isUndo))
            return false;
    }
    else if((ft.compare(" ") != 0) && (packer.compare(" ") != 0))
    {
        // Total TX count of the address' packer add one
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
    else
        return false;

    // Check if father or packer is null
    if ((ft.compare(" ") == 0) || (packer.compare(" ") == 0))
    {
        //LogPrintf("%s, member:%s, rw:%d\n", __func__, member, rewardbalance_old);
        return false;
    }

    // TX count of the address add one
    if (addressUpdated)
        GetFullRecord(address, nHeight-1, packer, ft, tc, ttc, rewardbalance);
    tc++;

    // Update cache
    string newRecord = GenerateRecord(packer, ft, tc, ttc, rewardbalance);
    cacheRecord[address] = newRecord;

    //LogPrintf("%s, member:%s, rw:%d\n", __func__, member, rewardbalance_old);

    return true;
}

bool CMemberInfoDB::UpdateFatherAndTCByTX(const CTransaction& tx, const CCoinsViewCache& view, int nHeight, bool isUndo)
{
    if (!tx.IsCoinBase())
    {
        // Get best father
        string bestFather = " ";
        if (!isUndo)
        {
            CAmount maxValue = 0;
            for(unsigned int j = 0; j < tx.vin.size(); j++)
            {
                CTxOut out = view.GetOutputFor(tx.vin[j]);
                CAmount val = out.nValue;
                if (val > maxValue)
                {
                    maxValue = val;

                    const CScript script = out.scriptPubKey;
                    CBitcoinAddress addr;
                    if (!addr.ScriptPub2Addr(script, bestFather))
                        return false;
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
        }

        // Update packer, father, ttc and tc
        for(unsigned int i = 0; i < tx.vout.size(); i++)
        {
            CTxDestination dst;
            ExtractDestinationFromP2PKAndP2PKH(tx.vout[i].scriptPubKey, dst);
            string voutAddress = CBitcoinAddress(dst).ToString();

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
