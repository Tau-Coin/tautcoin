#include "memberinfodb.h"
#include <boost/algorithm/string.hpp>
#include <boost/function.hpp>

using namespace std;

CRwdBalanceViewDB::CRwdBalanceViewDB(CClubInfoDB *pclubinfodb) : _pclubinfodb(pclubinfodb)
{
    options.create_if_missing = true;

    std::string db_path = GetDataDir(true).string() + std::string(RWDBALDBPATH);
    LogPrintf("Opening LevelDB in %s\n", db_path);

    leveldb::Status status = leveldb::DB::Open(options, db_path, &pdb);
    dbwrapper_private::HandleError(status);
    assert(status.ok());
    LogPrintf("Opened LevelDB successfully\n");
}

CRwdBalanceViewDB::~CRwdBalanceViewDB()
{
    delete pdb;
    pdb = NULL;
    _pclubinfodb = NULL;
}

bool CRwdBalanceViewDB::WriteDB(std::string key, int nHeight, string strValue)
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

bool CRwdBalanceViewDB::WriteDB(std::string key, int nHeight, string father, uint64_t tc, CAmount value)
{
    return WriteDB(key, nHeight, GenerateRecord(father, tc, value));
}

bool CRwdBalanceViewDB::ReadDB(std::string key, int nHeight, string& father, uint64_t& tc, CAmount& value)
{
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
            father = " ";
            tc = 0;
            value = 0;
        }
        else
        {
            LogPrintf("LevelDB read failure in rwdbalance module: %s\n", status.ToString());
            dbwrapper_private::HandleError(status);
        }
        return false;
    }

    if (!ParseRecord(strValue, father, tc, value))
        return false;

    return true;
}

bool CRwdBalanceViewDB::ReadDB(std::string key, int nHeight, std::string& strValue)
{
    std::stringstream ssHeight;
    std::string strHeight;
    ssHeight << nHeight;
    ssHeight >> strHeight;

    leveldb::Status status = pdb->Get(leveldb::ReadOptions(), key+DBSEPECTATOR+strHeight, &strValue);
    if(!status.ok())
    {
        if (status.IsNotFound())
            strValue = " _0_0";
        else
        {
            LogPrintf("LevelDB read failure in rwdbalance module: %s\n", status.ToString());
            dbwrapper_private::HandleError(status);
        }
        return false;
    }

    return true;
}

bool CRwdBalanceViewDB::DeleteDB(std::string key, int nHeight)
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

void CRwdBalanceViewDB::ClearCache()
{
    cacheRecord.clear();
}

void CRwdBalanceViewDB::UpdateCacheFather(string address, int inputHeight, string newFather)
{
    CAmount rwdbalanceInput = 0;
    string ftInput = " ";
    uint64_t tcInput = 1;
    GetFullRecord(address, inputHeight, ftInput, tcInput, rwdbalanceInput);
    ftInput = newFather;
    string newRecordInput = GenerateRecord(ftInput, tcInput, rwdbalanceInput);
    cacheRecord[address] = newRecordInput;
}

void CRwdBalanceViewDB::UpdateCacheTcAddOne(string address, int inputHeight)
{
    CAmount rwdbalanceVout = 0;
    string ftVout = " ";
    uint64_t tcVout = 1;
    GetFullRecord(address, inputHeight, ftVout, tcVout, rwdbalanceVout);
    tcVout++;
    string newRecordVout = GenerateRecord(ftVout, tcVout, rwdbalanceVout);
    cacheRecord[address] = newRecordVout;
}

void CRwdBalanceViewDB::UpdateCacheRewardChange(string address, int inputHeight, CAmount rewardChange)
{
    CAmount rewardbalance_old = 0;
    string ft = " ";
    uint64_t tc = 1;
    GetFullRecord(address, inputHeight, ft, tc, rewardbalance_old);
    CAmount newValue = rewardbalance_old + rewardChange;
    string newRecord = GenerateRecord(ft, tc, newValue);
    cacheRecord[address] = newRecord;
}

bool CRwdBalanceViewDB::Commit(int nHeight)
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

bool CRwdBalanceViewDB::InitGenesisDB(std::vector<std::string> addresses)
{
    for(uint i = 0; i < addresses.size(); i++)
    {
        CAmount rewardbalance = 0;
        string ft = "0";
        uint64_t tc = 1;
        if (!WriteDB(addresses[i], 0, ft, tc, rewardbalance))
            return false;
    }
    return true;
}

bool CRwdBalanceViewDB::ParseRecord(string inputStr, string& father, uint64_t& tc, CAmount& value)
{
    vector<string> splitedStr;
    boost::split(splitedStr, inputStr, boost::is_any_of(DBSEPECTATOR));
    if (splitedStr.size() != 3)
        return false;
    father = splitedStr[0];
    std::istringstream ssVal1(splitedStr[1]);
    ssVal1 >> tc;
    std::istringstream ssVal2(splitedStr[2]);
    ssVal2 >> value;

    return true;
}

string CRwdBalanceViewDB::GenerateRecord(string father, uint64_t tc, CAmount value)
{
    string outputStr;
    std::stringstream ssVal;
    ssVal << father;
    ssVal << DBSEPECTATOR;
    ssVal << tc;
    ssVal << DBSEPECTATOR;
    ssVal << value;
    ssVal >> outputStr;

    return outputStr;
}

CAmount CRwdBalanceViewDB::GetRwdBalance(std::string address, int nHeight)
{
    string ft = " ";
    uint64_t tc;
    CAmount value = 0;
    GetFullRecord(address, nHeight, ft, tc, value);
    return value;
}

string CRwdBalanceViewDB::GetFather(string address, int nHeight)
{
    string ft = " ";
    uint64_t tc;
    CAmount value;
    GetFullRecord(address, nHeight, ft, tc, value);
    return ft;
}

uint64_t CRwdBalanceViewDB::GetTXCnt(string address, int nHeight)
{
    string ft;
    uint64_t tc = 0;
    CAmount value;
    GetFullRecord(address, nHeight, ft, tc, value);
    return tc;
}

void CRwdBalanceViewDB::GetFullRecord(string address, int nHeight, string& father, uint64_t& tc, CAmount& value)
{
    if (cacheRecord.find(address) != cacheRecord.end())
    {
        ParseRecord(cacheRecord[address], father, tc, value);
        return;
    }
    else
    {
        for (int h = nHeight; h >= 0; h--)
        {
            if (ReadDB(address, h, father, tc, value))
                return;
        }
    }
}

string CRwdBalanceViewDB::GetFullRecord(std::string address, int nHeight)
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

bool CRwdBalanceViewDB::RewardChangeUpdate(CAmount rewardChange, string address, int nHeight, bool isUndo)
{
    if (rewardChange >= MAX_MONEY || rewardChange <= -MAX_MONEY)
        return false;
    if (isUndo)
    {
        if (!DeleteDB(address, nHeight+1))
            return false;
        return true;
    }

    UpdateCacheRewardChange(address, nHeight-1, rewardChange);

    //LogPrintf("%s, member:%s, rw:%d\n", __func__, member, rewardbalance_old);

    return true;
}

bool CRwdBalanceViewDB::RewardChangeUpdateByPubkey(CAmount rewardChange, string pubKey, int nHeight, bool isUndo)
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

bool CRwdBalanceViewDB::ComputeMemberReward(const uint64_t& txCnt, const uint64_t& totalTXCnt,
                                            const CAmount& totalRewards, CAmount& memberReward)
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
        return false;
}

bool CRwdBalanceViewDB::InitRewardsDist(CAmount memberTotalRewards, const CScript& scriptPubKey, int nHeight, string& clubLeaderAddress,
                                        CAmount& distributedRewards, map<string, CAmount>& memberRewards)
{
    if (memberTotalRewards < 0)
        return false;

    memberRewards.clear();
    bool ret = true;
//    ClubManager* clubMan = ClubManager::GetInstance();
//    RewardManager* rewardMan = RewardManager::GetInstance();
    CBitcoinAddress addr;
    uint64_t clubID;
    map<string, uint64_t> addrToTC;
    uint64_t totalmemberTXCnt = 0;
    if (!addr.ScriptPub2Addr(scriptPubKey, clubLeaderAddress))
        return false;

    uint64_t harvestPower = 0;
    vector<string> members = _pclubinfodb->GetMembersByFatherAddress(clubLeaderAddress, nHeight-1);
    for(size_t i = 0; i < members.size(); i++)
    {
        uint64_t tXCnt = GetTXCnt(members[i], nHeight-1);
        addrToTC.insert(pair<string, uint64_t>(members[i], tXCnt));
        harvestPower += tXCnt;
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
            LogPrintf("Error: The club's totalRewards are less than distributedRewards\n");
            return false;
        }
    }

    return true;
}

bool CRwdBalanceViewDB::UpdateRewardsByTX(const CTransaction& tx, CAmount blockReward, int nHeight, bool isUndo)
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

    return ret;
}

bool CRwdBalanceViewDB::EntrustByAddress(string inputAddr, string voutAddress, int nHeight, bool isUndo)
{
    if (isUndo)
    {
        if (GetFather(voutAddress, nHeight).compare("0") == 0)
        {
            if (!DeleteDB(inputAddr, nHeight+1))
                return false;
            //UpdateMemberDB(voutAddress, isAdd, inputAddr, isUndo);
            //UpdateMemberDB(inputAddr, isDel, voutAddress, isUndo);
        }
        if (!DeleteDB(voutAddress, nHeight+1))
            return false;
        return true;
    }

    // Address of vout must have father of "0" in database or entrust itself
    if ((voutAddress.compare(inputAddr) != 0) && (GetFather(voutAddress, nHeight-1).compare("0") == 0))
    {
        // Input address update
        UpdateCacheFather(inputAddr, nHeight-1, voutAddress);

        //UpdateMemberDB(voutAddress, isAdd, inputAddr, isUndo);
        //UpdateMemberDB(inputAddr, isDel, voutAddress, isUndo);
    }
    else if((voutAddress.compare(inputAddr) == 0) && (GetFather(voutAddress, nHeight-1).compare("0") != 0))
    {
        // Input address update
        UpdateCacheFather(inputAddr, nHeight-1, "0");

        //UpdateMemberDB(voutAddress, isAdd, inputAddr, isUndo);
        //UpdateMemberDB(inputAddr, isDel, voutAddress, isUndo);
    }

    // Address of vout update by transaction count
    UpdateCacheTcAddOne(voutAddress, nHeight-1);

    return true;
}

bool CRwdBalanceViewDB::TcAddOneByAddress(string address, int nHeight, string father, bool isUndo)
{
    if (isUndo)
    {
        if (!DeleteDB(address, nHeight+1))
            return false;
        return true;
    }

    CAmount rewardbalance = 0;
    string ft = " ";
    uint64_t tc = 1;
    GetFullRecord(address, nHeight-1, ft, tc, rewardbalance);
    if (ft.compare(" ") == 0)
    {
        ft = father;
        //UpdateMemberDB(voutAddress, isAdd, inputAddr, isUndo);
    }
    tc++;

    string newRecord = GenerateRecord(ft, tc, rewardbalance);
    cacheRecord[address] = newRecord;

    //LogPrintf("%s, member:%s, rw:%d\n", __func__, member, rewardbalance_old);

    return true;
}

bool CRwdBalanceViewDB::UpdateFatherTCByTX(const CTransaction& tx, const CCoinsViewCache& view, int nHeight, bool isUndo)
{
    if (!tx.IsCoinBase())
    {
        // get best father
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

        // update packer, father and tc
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
                if (!TcAddOneByAddress(voutAddress, nHeight, bestFather, isUndo))
                    return false;
            }
        }
    }

    return true;
}
