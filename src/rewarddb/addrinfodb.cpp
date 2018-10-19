#include "addrinfodb.h"
#include <boost/algorithm/string.hpp>
#include <boost/function.hpp>
#include <sstream>

CCriticalSection cs_addrinfo;

using namespace std;

CAddrInfoDB::CAddrInfoDB(size_t nCacheSize, CClubInfoDB *pclubinfodb, bool fMemory, bool fWipe) :
    CDBWrapper(GetDataDir() / ADDRINFODBPATH, nCacheSize, fMemory, fWipe),
    _pclubinfodb(pclubinfodb),
    currentHeight(-1)
{
    batch = new CDBBatch(*this);
}

CAddrInfoDB::~CAddrInfoDB()
{
    delete batch;
    batch = NULL;
    _pclubinfodb = NULL;
}

void CAddrInfoDB::WriteToBatch(const std::string& address, int nHeight, const CTAUAddrInfo& value)
{
    WriteToBatch(make_pair(nHeight, address), value);
}

void CAddrInfoDB::WriteNewestToBatch(const std::string& address, const CTAUAddrInfo& value)
{
    WriteToBatch(make_pair(NEWESTHEIGHFLAG, address), value);
}

bool CAddrInfoDB::WriteDB(const std::string& address, int nHeight, const CTAUAddrInfo& value)
{
    return Write(make_pair(nHeight, address), value);
}

bool CAddrInfoDB::ReadDB(const std::string& address, int nHeight, CTAUAddrInfo& value) const
{
    return Read(make_pair(nHeight, address), value);
}

void CAddrInfoDB::DeleteToBatch(const std::string& address, int nHeight)
{
    DeleteToBatch(make_pair(nHeight, address));
}

bool CAddrInfoDB::DeleteDB(const std::string& address, int nHeight)
{
    return Erase(make_pair(nHeight, address));
}

void CAddrInfoDB::ClearCache()
{
    cacheRecord.clear();
}

void CAddrInfoDB::ClearUndoCache()
{
    cacheForUndo.clear();
    cacheForClubRm.clear();
    cacheForClubAdd.clear();
    cacheForErs.clear();
    cacheForUndoRead.clear();
    cacheForUndoHeight.clear();
}

void CAddrInfoDB::ClearReadCache()
{
    cacheForRead.clear();
}

bool CAddrInfoDB::UpdateCacheRecord(string address, int inputHeight, string newFather,
                                    string newMiner, uint64_t newIdx)
{
    CTAUAddrInfo addrInfo = GetAddrInfo(address, inputHeight-1);
    addrInfo.father = newFather;
    addrInfo.miner = newMiner;
    addrInfo.index = newIdx;

    cacheRecord[address] = addrInfo;

    return true;
}

bool CAddrInfoDB::UpdateCacheMpAddOne(string address, int nHeight, bool isUndo)
{
    int nHeightQuery = isUndo ? nHeight : nHeight - 1;
    CTAUAddrInfo addrInfo = GetAddrInfo(address, nHeightQuery);
    string father = addrInfo.father;
    uint64_t idx = addrInfo.index;
    if (father.compare(" ") != 0)
    {
        string actualFather = (father.compare("0") == 0) ? address : father;
        if (!_pclubinfodb->UpdateMpByChange(actualFather, idx, isUndo))
            return false;
    }
    else
    {
        LogPrintf("%s, The father of input address: %s, is not exist, which is not allowed\n", __func__, address);
        return false;
    }

    return true;
}

bool CAddrInfoDB::Commit(int nHeight, bool isUndo)
{
    _pclubinfodb->Commit(nHeight);

    for(std::map<string, CTAUAddrInfo>::const_iterator it = cacheRecord.begin();
        it != cacheRecord.end(); it++)
    {
        string address = it->first;
        CTAUAddrInfo value = it->second;
        CTAUAddrInfo valueOrig = cacheForRead[address];
        cacheForRead[address] = value;
        if ((valueOrig.father.compare(value.father) == 0) &&
            (valueOrig.miner.compare(value.miner) == 0) &&
            (valueOrig.totalMP == value.totalMP))
            continue;
        if (!isUndo)
        {
            cacheForRead[address].lastHeight = nHeight;
            if (!WriteDB(address, nHeight, value))
                return false;
        }
    }

    SetCurrentHeight(nHeight);
    _pclubinfodb->SetCurrentHeight(nHeight);

    return true;
}

void CAddrInfoDB::SetCurrentHeight(int nHeight)
{
    currentHeight = nHeight;
}


int CAddrInfoDB::GetCurrentHeight() const
{
    return currentHeight;
}

bool CAddrInfoDB::LoadNewestDBToMemory()
{
    boost::scoped_ptr<CDBIterator> pcursor(NewIterator());
    pair<int, string> key;
    int nHeight = -1;
    pcursor->Seek(make_pair(-1, string()));
    if (pcursor->Valid() && pcursor->GetKey(key))
    {
        nHeight = atoi(key.second);
        if (nHeight != _pclubinfodb->GetCurrentHeight())
            return false;
        Erase(key, true);
    }
    pcursor->Seek(make_pair(NEWESTHEIGHFLAG, string()));

    int keyHeight = 0;
    LogPrintf("%s: loading newest records from addrInfodb...\n", __func__);
    while (pcursor->Valid() && pcursor->GetKey(key))
    {
        boost::this_thread::interruption_point();
        try
        {
            keyHeight = key.first;
            if (keyHeight == NEWESTHEIGHFLAG)
            {
                CTAUAddrInfo addrInfo;
                if (!pcursor->GetValue(addrInfo)) {
                    LogPrintf("%s: unable to read value in height %d\n", __func__, keyHeight);
                    break;
                }
                cacheForRead[key.second] = addrInfo;
                DeleteToBatch(key);
            }
//            else if(keyHeight > nHeight && nHeight >= 0)
//            {
//                LogPrintf("%s: load value in height: %d, which is wrong, the newest height is: %d\n",
//                          __func__, keyHeight, nHeight);
//                return false;
//            }
            else
                break;

            pcursor->Next();
        }
        catch (const std::exception& e) {
            return error("%s: Deserialize or I/O error - %s\n", __func__, e.what());
        }
    }

    LogPrintf("%s: loaded %d newest records from addrInfodb\n", __func__, cacheForRead.size());
    SetCurrentHeight(nHeight);
    return CommitDB();
}

bool CAddrInfoDB::WriteNewestDataToDisk(int newestHeight, bool fSync)
{
    LogPrintf("%s: writing newest records to addrInfodb...\n", __func__);
    stringstream ss;
    string heightStr;
    ss << newestHeight;
    ss >> heightStr;
    WriteToBatch(make_pair(-1, heightStr), CTAUAddrInfo());

    LOCK(cs_clubinfo);
    for(map<string, CTAUAddrInfo>::const_iterator it = cacheForRead.begin();
        it != cacheForRead.end(); it++)
    {
        string address = it->first;
        CTAUAddrInfo value = it->second;
        WriteNewestToBatch(address, value);
    }

    if (CommitDB(fSync))
    {
        LogPrintf("%s: wrote %d newest records to addrInfodb\n", __func__, cacheForRead.size());
        return true;
    }
    else
        return false;
}

bool CAddrInfoDB::InitGenesisDB(const std::vector<string>& addresses)
{
    for(size_t i = 0; i < addresses.size(); i++)
    {
        CTAUAddrInfo addrInfo("0", "0", 0, 1);
        if (!WriteDB(addresses[i], 0, addrInfo))
            return false;
        addrInfo.lastHeight = 0;
        cacheForRead[addresses[i]] = addrInfo;// Add to cache for accelerating
    }

    if (!_pclubinfodb->InitGenesisDB(addresses))
        return false;
    return true;
}

string CAddrInfoDB::GetMiner(string address, int nHeight)
{
    return GetAddrInfo(address, nHeight).miner;
}

string CAddrInfoDB::GetFather(string address, int nHeight)
{
    return GetAddrInfo(address, nHeight).father;
}

bool CAddrInfoDB::GetMiningPower(string address, int nHeight, uint64_t& miningPower)
{
    CTAUAddrInfo addrInfo = GetAddrInfo(address, nHeight);
    string father = addrInfo.father;
    uint64_t idx = addrInfo.index;
    if (father.compare(" ") != 0)
    {
        string actualFather = (addrInfo.father.compare("0") == 0) ? address : father;
        const vector<CMemberInfo> &memberInfo = _pclubinfodb->GetCacheRecord(actualFather);
        if (idx < memberInfo.size())
        {
            miningPower = (_pclubinfodb->GetCacheRecord(actualFather))[idx].MP;
            return true;
        }
        else
        {
            LogPrintf("%s, The input address: %s, whose memberInfo is not exist\n", __func__, address);
            return false;
        }
    }
    else
        miningPower = 0;

    return true;
}

uint64_t CAddrInfoDB::GetTotalMP(string address, int nHeight)
{
    return GetAddrInfo(address, nHeight).totalMP;
}

CAmount CAddrInfoDB::GetRwdBalance(std::string address, int nHeight)
{
    CTAUAddrInfo addrInfo = GetAddrInfo(address, nHeight);
    string father = addrInfo.father;
    uint64_t idx = addrInfo.index;
    if (father.compare(" ") != 0)
    {
        string actualFather = (addrInfo.father.compare("0") == 0) ? address : father;
        return (_pclubinfodb->GetCacheRecord(actualFather))[idx].rwd;
    }
    else
        return 0;
}

CTAUAddrInfo CAddrInfoDB::GetAddrInfo(string address, int nHeight)
{
    CTAUAddrInfo addrInfo(" ", " ", 0, 0);
    if (nHeight < currentHeight)
    {
        if (cacheForUndoRead.find(address) != cacheForUndoRead.end())
            return cacheForUndoRead[address];
        int h = -1;
        if (cacheForRead.find(address) != cacheForRead.end())
            h = cacheForRead[address].lastHeight;
        while (h >= 0)
        {
            if (h <= nHeight)
            {
                if (ReadDB(address, h, addrInfo))
                {
                    cacheForUndoHeight[address] = h;
                    cacheForUndoRead[address] = addrInfo;
                    return addrInfo;
                }
                else
                    break;
            }
            else
            {
                if (ReadDB(address, h, addrInfo))
                    h = addrInfo.lastHeight;
                else
                    break;
            }
        }

        return CTAUAddrInfo(" ", " ", 0, 0);
    }

    {
        TRY_LOCK(cs_addrinfo, cachelock);
        if (cachelock && (cacheRecord.find(address) != cacheRecord.end()))
            return cacheRecord[address];
    }

    if (cacheForRead.find(address) != cacheForRead.end())
        return cacheForRead[address];

    return addrInfo;
}

bool CAddrInfoDB::RewardChangeUpdateByPubkey(CAmount rewardChange, string pubKey, int nHeight, bool isUndo)
{
    string address;

    if (pubKey.empty())
        return false;
    const CScript script = CScript() << ParseHex(pubKey) << OP_CHECKSIG;
    CBitcoinAddress addr;
    if (!addr.ScriptPub2Addr(script, address))
        return false;
    if (addr.IsScript())
    {
        LogPrintf("%s, The input address is a script: %s, which is not allowed to be spent\n", __func__, address);
        return false;
    }

    CTAUAddrInfo addrInfo = GetAddrInfo(address, nHeight);
    string father = addrInfo.father;
    uint64_t idx = addrInfo.index;
    string actualFather = (father.compare("0") == 0) ? address : father;
    _pclubinfodb->UpdateRewardByChange(actualFather, idx, rewardChange, isUndo);

    return true;
}

uint64_t CAddrInfoDB::GetHarvestPowerByAddress(std::string address, int nHeight)
{
    if (!CClubInfoDB::AddressIsValid(address))
    {
        LogPrintf("%s, The input address is : %s, which is not valid\n", __func__, address);
        return false;
    }

    CTAUAddrInfo addrInfo = GetAddrInfo(address, nHeight);
    if ((addrInfo.miner.compare("0") == 0) && (addrInfo.father.compare("0") == 0))
        return addrInfo.totalMP;
    else
        return 0;
}

bool CAddrInfoDB::UpdateRewardsByTX(const CTransaction& tx, CAmount blockReward, int nHeight, bool isUndo)
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

    CAmount distributedRewards = 0;
    CBitcoinAddress addr;
    string clubMinerAddress;
    if (!addr.ScriptPub2Addr(tx.vout[0].scriptPubKey, clubMinerAddress))
        return false;
    CAmount memberTotalRewards = (nHeight < 45000) ? blockReward - tx.vout[0].nValue : 0;
    if (blockReward > 0)
    {
        int nHeightQuery = isUndo ? nHeight : nHeight - 1;
        uint64_t MP = 0;
        if (!GetMiningPower(clubMinerAddress, nHeightQuery, MP))
            return false;
        uint64_t memberTotalMP = GetHarvestPowerByAddress(clubMinerAddress, nHeightQuery) - MP;
        if (!_pclubinfodb->UpdateRewardsByMinerAddress(clubMinerAddress, memberTotalRewards,
                                                       memberTotalMP, distributedRewards, isUndo))
            return false;
    }

    //Update the reward rate dataset(if required)
    RewardRateUpdate(blockReward, distributedRewards, clubMinerAddress, nHeight);

    return true;
}

void CAddrInfoDB::UpdateMembersByFatherAddress(const string& fatherAddress, const CMemberInfo& memberinfo,
                                               uint64_t& addrIndex, int nHeight, bool add, bool isUndo)
{
    int nHeightQuery = isUndo ? nHeight : nHeight - 1;
    string addressMove = _pclubinfodb->UpdateMembersByFatherAddress(fatherAddress, memberinfo,
                                                                    addrIndex, nHeight, add);

    CTAUAddrInfo addrInfo = GetAddrInfo(memberinfo.address, nHeightQuery);
    addrInfo.index = addrIndex;
    cacheRecord[memberinfo.address] = addrInfo;
    if (!add)
    {
        if (addressMove.compare(NO_MOVED_ADDRESS) != 0)
        {
            addrInfo = GetAddrInfo(addressMove, nHeightQuery);
            addrInfo.index = addrIndex;
            cacheRecord[addressMove] = addrInfo;
        }

        cacheRecord[memberinfo.address].index = 0;
        addrIndex = 0;
    }
}

bool CAddrInfoDB::EntrustByAddress(string inputAddr, string voutAddress, int nHeight)
{
    int nHeightQuery = nHeight - 1;
    string fatherOfVin, minerOfVin, fatherOfVout, minerOfVout;

    CTAUAddrInfo inputAddrInfo = GetAddrInfo(inputAddr, nHeightQuery);
    CTAUAddrInfo voutAddrInfo = GetAddrInfo(voutAddress, nHeightQuery);
    fatherOfVin = inputAddrInfo.father;
    minerOfVin = inputAddrInfo.miner;
    fatherOfVout = voutAddrInfo.father;
    minerOfVout = voutAddrInfo.miner;
    string actualFather = (fatherOfVin.compare("0") == 0) ? inputAddr : fatherOfVin;
    vector<CMemberInfo> vinputMBRInfo = _pclubinfodb->GetCacheRecord(actualFather);
    CMemberInfo inputMBRInfo;
    if (vinputMBRInfo.size() > inputAddrInfo.index)
        inputMBRInfo = vinputMBRInfo[inputAddrInfo.index];
    else
    {
        LogPrintf("%s, The input index is : %d, which is overrange, the max size is: %d\n",
                  __func__, inputAddrInfo.index, vinputMBRInfo.size());
        return false;
    }
    uint64_t inputAddrIndex = inputAddrInfo.index;

    string newMinerAddr = voutAddress;
    // The address is a new one on the chain and do not change anything
    if ((fatherOfVout.compare(" ") == 0) && (minerOfVout.compare(" ") == 0))
        return true;

    // Address of vout must have both father and miner of "0" in database or entrust itself
    bool changeRelationship = false;

    // Update club info and miner db
    // When the vout address is a club miner, the vin entrust the vout(not vin himself)
    if ((voutAddress.compare(inputAddr) != 0) && (fatherOfVin.compare(voutAddress) != 0) &&
        (fatherOfVout.compare("0") == 0) && (minerOfVout.compare("0") == 0))
    {
        // Update club info
        if ((fatherOfVin.compare("0") != 0) && (minerOfVin.compare("0") != 0))// The vin is not a miner
            UpdateMembersByFatherAddress(fatherOfVin, inputMBRInfo, inputAddrIndex, nHeight, false);
        UpdateMembersByFatherAddress(voutAddress, inputMBRInfo, inputAddrIndex, nHeight, true);

        newMinerAddr = voutAddress;
        changeRelationship = true;
    }
    // When the vin address is not a miner and the vin entrust himself
    else if((voutAddress.compare(inputAddr) == 0) &&
            (fatherOfVin.compare("0") != 0) && (minerOfVin.compare("0") != 0))
    {
        UpdateMembersByFatherAddress(fatherOfVin, inputMBRInfo, inputAddrIndex, nHeight, false);
        UpdateMembersByFatherAddress(inputAddr, inputMBRInfo, inputAddrIndex, nHeight, true);

        newMinerAddr = "0";
        if (!UpdateCacheTotalMPByChange(voutAddress, nHeight, 0, false))
            return false;
        changeRelationship = true;
    }

    if (changeRelationship)
    {
        // Update the father, miner and index of the vin address
        if (!UpdateCacheRecord(inputAddr, nHeight, newMinerAddr, newMinerAddr, inputAddrIndex))
            return false;

        // Compute the total MP of the vin address and update the miner of the these members
        vector<string> totalMembers;
        uint64_t totalMPOfVin = 0;
        if (!GetMiningPower(inputAddr, nHeightQuery, totalMPOfVin))
            return false;
        _pclubinfodb->GetTotalMembersByAddress(inputAddr, totalMembers);
        string actualFather;
        for(size_t i = 0; i < totalMembers.size(); i++)
        {
            CTAUAddrInfo memberAddrInfo = GetAddrInfo(totalMembers[i], nHeightQuery);
            memberAddrInfo.miner = voutAddress;
            cacheRecord[totalMembers[i]] = memberAddrInfo;

            string &father = memberAddrInfo.father;
            uint64_t &idx = memberAddrInfo.index;
            actualFather = (father.compare("0") == 0) ? totalMembers[i] : father;
            totalMPOfVin += (_pclubinfodb->GetCacheRecord(actualFather))[idx].MP;
        }

        // Update the total MP of the vin's miner address
        string oldMinerOfVin = (minerOfVin.compare("0") == 0) ? inputAddr : minerOfVin;
        if (!UpdateCacheTotalMPByChange(oldMinerOfVin, nHeight, totalMPOfVin, false))
            return false;

        // Update the total MP of the vout address
        if (!UpdateCacheTotalMPByChange(voutAddress, nHeight, totalMPOfVin, true))
            return false;
    }

    // Mining power of vout add one
    if (!UpdateCacheMpAddOne(voutAddress, nHeight))
        return false;

    // Total MP of vout's miner add one
    string newMinerOfVout = (GetMiner(voutAddress, nHeight).compare("0") == 0) ? voutAddress : minerOfVout;
    if (!UpdateCacheTotalMPByChange(newMinerOfVout, nHeight, 1, true))
        return false;

    return true;
}

bool CAddrInfoDB::UpdateCacheTotalMPByChange(std::string address, int nHeight, uint64_t amount, bool isAdd)
{
    int nHeightQuery = nHeight - 1;
    CTAUAddrInfo addrInfo = GetAddrInfo(address, nHeightQuery);
    if (isAdd)
        addrInfo.totalMP += amount;
    else
    {
        if (amount == 0)
            addrInfo.totalMP = 0;
        else if(addrInfo.totalMP >= amount)
            addrInfo.totalMP -= amount;
        else
        {
            LogPrintf("%s, TotalMP - miming_power error, address:%s, %d - %d\n", __func__, address, addrInfo.totalMP, amount);
            return false;
        }
    }

    // Update cache
    cacheRecord[address] = addrInfo;

    //LogPrintf("%s, member:%s, rw:%d\n", __func__, member, rewardbalance_old);

    return true;
}

bool CAddrInfoDB::UpdateMpAndTotalMPByAddress(string address, int nHeight, string fatherInput)
{
    int nHeightQuery = nHeight - 1;
    CTAUAddrInfo addrInfo = GetAddrInfo(address, nHeightQuery);
    if ((addrInfo.father.compare(" ") == 0) && (addrInfo.miner.compare(" ") == 0))
    {
        // It's a new address on the chain
        // Update clubInfo
        CMemberInfo memberInfo(address, 1, 0);
        UpdateMembersByFatherAddress(fatherInput, memberInfo, addrInfo.index, nHeight, true);

        // Update addrInfo
        string minerOfFather = GetMiner(fatherInput, nHeightQuery);
        addrInfo.miner = (minerOfFather.compare("0") == 0) ? fatherInput : minerOfFather;
        addrInfo.father = fatherInput;

        // Total MP of the address' miner add one
        if (!UpdateCacheTotalMPByChange(addrInfo.miner, nHeight, 1, true))
            return false;
    }
    else if((addrInfo.father.compare(" ") != 0) && (addrInfo.miner.compare(" ") != 0))
    {
        // It's an existed address on the chain
        // Total MP of the address' miner add one
        if (addrInfo.miner.compare("0") == 0)
            addrInfo.totalMP += 1;
        else
        {
            if (!UpdateCacheTotalMPByChange(addrInfo.miner, nHeight, 1, true))
                return false;
        }

        // Mp of the address add one
        string actualFather = (addrInfo.father.compare("0") == 0) ? address : addrInfo.father;
        if (!_pclubinfodb->UpdateMpByChange(actualFather, addrInfo.index, false, 1, true))
            return false;
    }

    // Check if father or miner is null
    if ((addrInfo.father.compare(" ") == 0) || (addrInfo.miner.compare(" ") == 0))
    {
        LogPrintf("%s, The address: %s, whose father or miner is null\n", __func__, address);
        return false;
    }

    cacheRecord[address] = addrInfo;

    //LogPrintf("%s, member:%s, rw:%d\n", __func__, member, rewardbalance_old);

    return true;
}

bool CAddrInfoDB::GetBestFather(const CTransaction& tx, const CCoinsViewCache& view, string& bestFather,
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

    if (!_pclubinfodb->AddressIsValid(bestFather))
        return false;

    return true;
}

bool CAddrInfoDB::UpdateFatherAndMpByTX(const CTransaction& tx, const CCoinsViewCache& view, int nHeight,
                                          map<string, CAmount> vin_val)
{
    if (!tx.IsCoinBase())
    {
        // Get best father
        string bestFather = " ";
        if (!GetBestFather(tx, view, bestFather, vin_val))
        {
            LogPrintf("%s, GetBestFather() failed\n", __func__);
            return false;
        }
        CBitcoinAddress addrFt = CBitcoinAddress(bestFather);
        if (addrFt.IsScript())
            return true;

        // Update miner, father, tmp and tc
        for(unsigned int i = 0; i < tx.vout.size(); i++)
        {
            CBitcoinAddress addr;
            string voutAddress;
            if (!addr.ScriptPub2Addr(tx.vout[i].scriptPubKey, voutAddress))
                return false;
            CBitcoinAddress addrVout = CBitcoinAddress(voutAddress);
            if (addrVout.IsScript())
                continue;

            if (tx.vout[i].nValue == 0)
            {
                if (!EntrustByAddress(bestFather, voutAddress, nHeight))
                    return false;
            }
            else
            {
                if (!UpdateMpAndTotalMPByAddress(voutAddress, nHeight, bestFather))
                    return false;
            }
        }
    }

    return true;
}

bool CAddrInfoDB::UndoMiningPowerByTX(const CTransaction& tx, const CCoinsViewCache& view, int nHeight,
                                      map<string, CAmount> vin_val)
{
    if (!tx.IsCoinBase())
    {
        // Get best father
        string bestFather = " ";
        if (!GetBestFather(tx, view, bestFather, vin_val, true))
        {
            LogPrintf("%s, GetBestFather() failed\n", __func__);
            return false;
        }
        CBitcoinAddress addrFt = CBitcoinAddress(bestFather);
        if (addrFt.IsScript())
            return true;

        // Undo miner, father, total MP and mining power
        for(unsigned int i = tx.vout.size(); i-- > 0;)
        {
            CBitcoinAddress addr;
            string voutAddress;
            if (!addr.ScriptPub2Addr(tx.vout[i].scriptPubKey, voutAddress))
                return false;
            CBitcoinAddress addrVout = CBitcoinAddress(voutAddress);
            if (addrVout.IsScript())
                continue;

            CTAUAddrInfo pastInputInfo = GetAddrInfo(bestFather, nHeight-1);
            CTAUAddrInfo curVoutInfo = GetAddrInfo(voutAddress, nHeight);
            CTAUAddrInfo curInputInfo = GetAddrInfo(bestFather, nHeight);
            string curActualVoutFather =
                    (curVoutInfo.father.compare("0") == 0) ? voutAddress : curVoutInfo.father;
            string curActualVoutMiner =
                    (curVoutInfo.miner.compare("0") == 0) ? voutAddress : curVoutInfo.miner;
            string pastActualInputFather =
                    (pastInputInfo.father.compare("0") == 0) ? bestFather : pastInputInfo.father;
            string curActualInputFather =
                    (curInputInfo.father.compare("0") == 0) ? bestFather : curInputInfo.father;

            // In the entrust TX, the address is a new one on the chain and do anything
            if ((tx.vout[i].nValue == 0) &&
                (curActualVoutFather.compare(" ") == 0) && (curActualVoutMiner.compare(" ") == 0))
                continue;

            // Undo mining power
            if (!_pclubinfodb->UpdateMpByChange(curActualVoutFather, curVoutInfo.index, true))
                return false;

            // If the address is a new one on the chain
            if ((_pclubinfodb->GetCacheRecord(curActualVoutFather))[curVoutInfo.index].MP == 0)
            {
                cacheForErs.insert(voutAddress);
                if (cacheForClubRm.find(voutAddress) == cacheForClubRm.end())
                    cacheForClubRm[voutAddress] = curActualVoutFather;
            }

            // Undo entrust TX if it is
            if (tx.vout[i].nValue == 0)
            {
                if (pastInputInfo.father.compare(curInputInfo.father) != 0)
                {
                    // Undo clubInfo
                    if (cacheForClubRm.find(bestFather) == cacheForClubRm.end())
                    {
                        if (curInputInfo.father.compare("0") != 0)
                            cacheForClubRm[bestFather] = curActualInputFather;
                    }
                    if ((cacheForClubAdd.find(bestFather) == cacheForClubAdd.end()) &&
                        (pastActualInputFather.compare(" ") != 0))
                        cacheForClubAdd[bestFather] = pastActualInputFather;
                }
            }
        }
    }

    return true;
}

bool CAddrInfoDB::UndoClubMembers(int nHeight)
{
    set<string> alreadyAddedAddr;
    for(map<string, string>::const_iterator itRm = cacheForClubRm.begin();
        itRm != cacheForClubRm.end(); itRm++)
    {
        const string &address = itRm->first;
        CTAUAddrInfo addrInfo = GetAddrInfo(address, nHeight);
        const string &father = itRm->second;
        uint64_t idx = addrInfo.index;
        CMemberInfo memberInfo = _pclubinfodb->GetCacheRecord(father)[idx];
        if (memberInfo.address.empty() || (memberInfo.address.compare(address) != 0))
        {
            if (memberInfo.MP <= 0)
                continue;
            LogPrintf("%s: unable to read record in clubInfo db to be removed, father: %s, index: %d\n",
                      __func__, father, idx);
            return false;
        }
        UpdateMembersByFatherAddress(father, memberInfo, idx, nHeight, false, true);
        if (cacheForClubAdd.find(address) != cacheForClubAdd.end())
        {
            UpdateMembersByFatherAddress(cacheForClubAdd[address], memberInfo, idx, nHeight, true, true);
            alreadyAddedAddr.insert(address);
        }
    }
    for(map<string, string>::const_iterator itAdd = cacheForClubAdd.begin();
        itAdd != cacheForClubAdd.end(); itAdd++)
    {
        if (alreadyAddedAddr.find(itAdd->first) != alreadyAddedAddr.end())
            continue;

        const string &address = itAdd->first;
        CTAUAddrInfo addrInfo = GetAddrInfo(address, nHeight);
        const string &father = itAdd->second;
        uint64_t idx = addrInfo.index;
        string actualFather =
                (addrInfo.father.compare("0") == 0) ? address : addrInfo.father;
        CMemberInfo memberInfo = _pclubinfodb->GetCacheRecord(actualFather)[idx];
        if (memberInfo.address.empty() || (memberInfo.address.compare(address) != 0))
        {
            LogPrintf("%s: unable to read record in clubInfo db to be added, father: %s, index: %d\n",
                      __func__, actualFather, idx);
            return false;
        }
        UpdateMembersByFatherAddress(father, memberInfo, idx, nHeight, true, true);
        alreadyAddedAddr.insert(address);
    }

    return true;
}


void CAddrInfoDB::UndoCacheRecords(int nHeight)
{
    boost::scoped_ptr<CDBIterator> pcursor(NewIterator());
    pair<int, string> key;
    CTAUAddrInfo addrInfo;
    pcursor->Seek(make_pair(nHeight, string()));
    while (pcursor->Valid() && pcursor->GetKey(key))
    {
        boost::this_thread::interruption_point();
        if (key.first == nHeight)
        {
            if (!pcursor->GetValue(addrInfo)) {
                LogPrintf("%s: unable to read value in height %d\n", __func__, key.first);
                break;
            }
            cacheForUndo[key.second] = GetAddrInfo(key.second, nHeight-1);
            DeleteDB(key.second, key.first);
        }
        else
            break;

        pcursor->Next();
    }

    // Erase cacheRecord which is not exist before
    for(set<string>::const_iterator itErs = cacheForErs.begin();
        itErs != cacheForErs.end(); itErs++)
    {
        const string &addrErs = *itErs;
        map<string, CTAUAddrInfo>::iterator it = cacheRecord.find(addrErs);
        if (it != cacheRecord.end())
            cacheRecord.erase(it);
        it = cacheForRead.find(addrErs);
        if (it != cacheForRead.end())
            cacheForRead.erase(it);
        it = cacheForUndo.find(addrErs);
        if (it != cacheForUndo.end())
            cacheForUndo.erase(it);
        DeleteDB(addrErs, nHeight);
    }

    // Update undo cache records to cacheRecord(except the index)
    for(map<string, CTAUAddrInfo>::const_iterator it = cacheForUndo.begin();
        it != cacheForUndo.end(); it++)
    {
        if (cacheRecord.find(it->first) == cacheRecord.end())
            cacheRecord[it->first].index = cacheForRead[it->first].index;
        cacheRecord[it->first].father = cacheForUndo[it->first].father;
        cacheRecord[it->first].miner = cacheForUndo[it->first].miner;
        cacheRecord[it->first].totalMP = cacheForUndo[it->first].totalMP;
        if (cacheForUndoHeight.find(it->first) != cacheForUndoHeight.end())
            cacheRecord[it->first].lastHeight = cacheForUndoHeight[it->first];
        else///////////////////////////////////////////////
        {
            //if (cacheForErs.find(it->first) == cacheForErs.end())
                cout<<"haha: "<<it->first<<endl;
        }

        DeleteDB(it->first, nHeight);
        //DeleteToBatch(it->first, nHeight);
    }
}

bool CAddrInfoDB::RewardRateUpdate(CAmount blockReward, CAmount distributedRewards, string clubLeaderAddress, int nHeight)
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

vector<string> CAddrInfoDB::GetAllClubMiners()
{
    vector<string> miners;
    const vector<string> &allFathers = _pclubinfodb->GetAllFathers();
    CTAUAddrInfo addrInfo;
    for(size_t i = 0; i < allFathers.size(); i++)
    {
        addrInfo = GetAddrInfo(allFathers[i], currentHeight);
        if ((addrInfo.father.compare("0") == 0) &&
            (addrInfo.miner.compare("0") == 0))
            miners.push_back(allFathers[i]);
    }

    return miners;
}
