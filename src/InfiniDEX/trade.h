// Copyright (c) 2017-2018 The Infinex Core developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef TRADE_H
#define TRADE_H

#include <vector>
#include <map>
#include <memory>
#include <set>
#include "hash.h"
#include "net.h"
#include "tradepair.h"
#include "utilstrencodings.h"

class CUserTrade;
class CUserTradeSetting;
class CUserTradeManager;

class CActualTrade;
class CActualTradeManager;

typedef std::map<int, std::shared_ptr<CUserTrade>> mINTUT; //int and trade details

typedef std::pair<int, mINTUT> pULTIUTC;
extern std::set<uint256> userTradesHash;
extern std::set<uint256> mnBalanceTradesHash;
extern std::vector<CUserTrade> pendingProcessUserTrades;
extern std::vector<CCancelTrade> pendingProcessCancelTrades;
extern std::map<std::string, pULTIUTC> mapUserTrades;

typedef std::map<uint64_t, mINTUT> mUTPIUTV; //price and user trade container
extern std::map<int, mUTPIUTV> mapBidUserTradeByPrice;
extern std::map<int, mUTPIUTV> mapAskUserTradeByPrice;

extern std::map<int, CUserTradeSetting> mapUserTradeSetting;
extern CUserTradeManager userTradeManager;

typedef std::map<int, std::shared_ptr<CActualTrade>> mATIAT;
typedef std::map<int, mATIAT> mUTImAT;

extern std::map<std::string, mUTImAT> mapUserActualTrades;

extern std::map<int, mATIAT> mapActualTradeByActualTradeID;
extern std::map<int, mUTImAT> mapActualTradeByUserTradeID;
extern std::map<int, std::vector<CActualTrade>> mapConflictTrade;
extern std::map<int, std::set<std::string>> mapActualTradeHash;
extern CActualTradeManager actualTradeManager;

class CCancelTrade
{
private:
	std::vector<unsigned char> userVchSig;
	std::vector<unsigned char> mnTradeVchSig;
	std::vector<unsigned char> mnBalanceVchSig;

public:
	int nUserTradeID;
	int nPairTradeID;
	int nTradePairID;
	std::string nUserPubKey;
	bool isBid;
	uint64_t nUserSubmitTime;
	uint64_t nPrice;
	uint64_t nBalanceQty;
	uint64_t nBalanceAmount;
	std::string nMNTradePubKey;
	uint64_t nMNTradeProcessTime;
	std::string nMNBalancePubKey;
	uint64_t nMNBalanceProcessTime;
	uint64_t nLastUpdateTime;

	CCancelTrade() :
		nUserTradeID(0),
		nPairTradeID(0),
		nTradePairID(0),
		nUserPubKey(""),
		isBid(true),
		nUserSubmitTime(0),
		nPrice(0),
		nBalanceQty(0),
		nBalanceAmount(0),
		nMNTradePubKey(""),
		nMNTradeProcessTime(0),
		nMNBalancePubKey(""),
		nMNBalanceProcessTime(0),
		nLastUpdateTime(0)
	{}

	ADD_SERIALIZE_METHODS;
	template <typename Stream, typename Operation>
	inline void SerializationOp(Stream& s, Operation ser_action, int nType, int nVersion) 
	{
		READWRITE(nUserTradeID);
		READWRITE(nPairTradeID);
		READWRITE(nTradePairID);
		READWRITE(nUserPubKey);
		READWRITE(isBid);
		READWRITE(nUserSubmitTime);
		READWRITE(nPrice);
		READWRITE(nBalanceQty);
		READWRITE(nBalanceAmount);
		READWRITE(nMNTradePubKey);
		READWRITE(nMNTradeProcessTime);
		READWRITE(nMNBalancePubKey);
		READWRITE(nMNBalanceProcessTime);
		READWRITE(nLastUpdateTime);
		READWRITE(userVchSig);
		READWRITE(mnTradeVchSig);
		READWRITE(mnBalanceVchSig);
	}

	bool VerifyUserSignature();
	bool VerifyTradeMNSignature();
	bool VerifyBalanceMNSignature();
	bool TradeMNSign();
	bool BalanceMNSign();
	void RelayTo(CNode* node, CConnman& connman);
	void RelayToTradeMN(CConnman& connman);
	void RelayToBalanceMN(CConnman& connman);
};

class CUserTradeSetting
{
public:
	int nTradePairID;
	int nLastPairTradeID;
	int nLastActualTradeID;
	uint32_t nMaxSubmissionTimeDiff;
	uint16_t nToStoreLowerLimit;
	uint16_t nToStoreUpperLimit;
	uint64_t nLastPairTradeTime;
	uint64_t nLastActualTradeTime;
	bool nInChargeOfBidBroadcast;
	bool nInChargeOfAskBroadcast;
	bool nInChargeOfMatchUserTrade;
	bool nInChargeOfChartData;
	bool nInChargeOfMarketTradeHistory;
	bool nInChargeOfUserTradeHistory;

	CUserTradeSetting(int nTradePairID) :
		nTradePairID(nTradePairID),
		nLastPairTradeID(0),
		nLastActualTradeID(0),
		nMaxSubmissionTimeDiff(3000),
		nToStoreLowerLimit(50),
		nToStoreUpperLimit(100),
		nLastPairTradeTime(0),
		nLastActualTradeTime(0),
		nInChargeOfBidBroadcast(false),
		nInChargeOfAskBroadcast(false),
		nInChargeOfMatchUserTrade(false),
		nInChargeOfChartData(false),
		nInChargeOfMarketTradeHistory(false),
		nInChargeOfUserTradeHistory(false)
	{}

	CUserTradeSetting() :
		nTradePairID(0),
		nLastPairTradeID(0),
		nLastActualTradeID(0),
		nMaxSubmissionTimeDiff(3000),
		nToStoreLowerLimit(50),
		nToStoreUpperLimit(100),
		nLastPairTradeTime(0),
		nLastActualTradeTime(0),		
		nInChargeOfBidBroadcast(false),
		nInChargeOfAskBroadcast(false),
		nInChargeOfMatchUserTrade(false),
		nInChargeOfChartData(false),
		nInChargeOfMarketTradeHistory(false),
		nInChargeOfUserTradeHistory(false)
	{}

	bool IsValidSubmissionTimeAndUpdate(uint64_t time);
};

class CUserTrade
{
private:
	std::vector<unsigned char> userVchSig;
	std::vector<unsigned char> mnBalanceVchSig;
	std::vector<unsigned char> mnTradeVchSig;

public:
	int nTradePairID;
	uint64_t nPrice;
	uint64_t nQuantity;
	uint64_t nAmount; //to replace with uint256 on actual implementation
	bool nIsBid;
	int nTradeFee; //this is here to compare with trade pair fee & apply to whichever lower fee (benefit user)
	std::string nUserPubKey;
	uint64_t nTimeSubmit;
	uint256 nUserHash;
	int nUserTradeID; //generated by balance MN
	std::string nMNBalancePubKey;
	uint64_t nMNBalanceProcessTime;
	uint256 nMNBalanceHash;
	int nPairTradeID; //generated by trade MN
	std::string nMNTradePubKey;
	uint64_t nMNTradeProcessTime;
	int64_t nBalanceQty;
	int64_t nBalanceAmount;
	uint64_t nLastUpdate;

	//to remove on actual implementation
	CUserTrade(int nTradePairID, uint64_t nPrice, uint64_t nQuantity, bool nIsBid, int nTradeFee, std::string nUserPubKey, uint64_t nTimeSubmit, uint256 nUserHash) :
		nTradePairID(nTradePairID),
		nPrice(nPrice),
		nQuantity(nQuantity),
		nAmount(0),
		nIsBid(nIsBid),
		nTradeFee(nTradeFee),
		nUserPubKey(nUserPubKey),
		nTimeSubmit(nTimeSubmit),
		nUserHash(nUserHash),
		nUserTradeID(0),
		nMNBalancePubKey(""),
		nMNBalanceProcessTime(0),
		nMNBalanceHash(0),
		nPairTradeID(0),
		nMNTradePubKey(""),
		nMNTradeProcessTime(0),
		nBalanceQty(nQuantity),
		nBalanceAmount(nAmount),
		nLastUpdate(0)
	{}

	CUserTrade() :
		nTradePairID(0),
		nPrice(0),
		nQuantity(0),
		nAmount(0),
		nIsBid(true),
		nTradeFee(0),
		nUserPubKey(""),
		nTimeSubmit(0),
		nUserHash(0),
		nUserTradeID(0),
		nMNBalancePubKey(""),
		nMNBalanceProcessTime(0),
		nMNBalanceHash(0),
		nPairTradeID(0),
		nMNTradePubKey(""),
		nMNTradeProcessTime(0),
		nBalanceQty(nQuantity),
		nBalanceAmount(nAmount),
		nLastUpdate(0)
	{}

	void MNBalanceHash();
	bool VerifyUserSignature();
	bool VerifyMNBalanceSignature();
	bool VerifyMNTradeSignature();
	bool MNBalanceSign();
	bool MNTradeSign();
	void RelayTo(CNode* node, CConnman& connman);
	void RelayToBalanceMN(CConnman& connman);
	void RelayToTradeMN(CConnman& connman);
	void RelayToBackupMN(CConnman& connman);
};

class CUserTradeManager
{
private:
	std::vector<unsigned char> vchSig;

public:
	void ProcessMessage(CNode* pfrom, std::string& strCommand, CDataStream& vRecv, CConnman& connman);
	void InputUserTrade(CUserTrade& userTrade, CNode* pfrom, CConnman& connman);
	void InputTradeCancel(CCancelTrade& cancelTrade, CNode* pfrom, CConnman& connman);
	bool ProcessTradeCancelRequest(CCancelTrade& cancelTrade);
	void ReturnTradeCancelBalance(CCancelTrade& cancelTrade);
	void InitTradePair(int TradePairID);
	void AssignBidBroadcastRole(int TradePairID, bool toAssign = true);
	void AssignAskBroadcastRole(int TradePairID, bool toAssign = true);
	void AssignUserHistoryProviderRole(int TradePairID, bool toAssign = true);
	void AssignMarketHistoryProviderRole(int TradePairID, bool toAssign = true);
	void AssignChartDataProviderRole(int TradePairID, bool toAssign = true);
	void AssignMatchUserTradeRole(int TradePairID, bool toAssign = true);
	uint64_t GetBidRequiredAmount(uint64_t Price, uint64_t Qty, int TradeFee);
	uint64_t GetAskExpectedAmount(uint64_t Price, uint64_t Qty, int TradeFee);
	bool IsSubmittedBidValid(const std::shared_ptr<CUserTrade>& userTrade, CTradePair& TradePair);
	bool IsSubmittedAskValid(const std::shared_ptr<CUserTrade>& userTrade, CTradePair& TradePair);
	bool IsSubmittedBidAmountValid(const std::shared_ptr<CUserTrade>& userTrade, int nTradeFee);
	bool IsSubmittedAskAmountValid(const std::shared_ptr<CUserTrade>& userTrade, int nTradeFee);	
	bool ProcessUserTradeRequest(const std::shared_ptr<CUserTrade>& userTrade, CTradePair& tradePair);
	void SaveProcessedUserTrade(const std::shared_ptr<CUserTrade>& userTrade, CTradePair& tradePair);
	void InputMatchUserBuyRequest(const std::shared_ptr<CUserTrade>& userTrade, CTradePair& tradePair);
	void InputMatchUserSellRequest(const std::shared_ptr<CUserTrade>& userTrade, CTradePair& tradePair);
	bool ReduceBalanceQty(int TradePairID, int UserTradeID1, int UserTradeID2, uint64_t Qty);
	int64_t GetBalanceAmount(int TradePairID, uint64_t Price, int UserTradeID);
	void RequestUserTradeData(std::string UserPubKey, int TradePairID, CConnman& connman);
};

class CActualTrade
{
private:
	std::vector<unsigned char> mnTradeVchSig;
	std::vector<unsigned char> mnBalance1VchSig;
	std::vector<unsigned char> mnBalance2VchSig;

public:
	int nActualTradeID;
	int nTradePairID;
	int nUserTrade1;
	int nUserTrade2;
	uint64_t nTradePrice;
	uint64_t nTradeQty;
	uint64_t nTradeAmount;
	uint64_t nBidAmount;
	uint64_t nAskAmount;
	std::string nUserPubKey1;
	std::string nUserPubKey2;
	int64_t nFee1; //during promo period, we can provide rebate instead of trade fee to user
	int64_t nFee2; //during promo period, we can provide rebate instead of trade fee to user
	bool nFromBid;
	std::string nTradeMNPubKey;
	std::string nBalance1MNPubKey;
	std::string nBalance2MNPubKey;
	std::string nCurrentHash;
	uint64_t nTradeTime;

	CActualTrade(int nTradePairID, int nUserTrade1, int nUserTrade2, uint64_t nTradePrice, uint64_t nTradeQty, uint64_t nBidAmount, uint64_t nAskAmount, std::string nUserPubKey1,
		std::string nUserPubKey2, int64_t nFee1, int64_t nFee2, bool nFromBid, uint64_t nTradeTime) :
		nActualTradeID(0),
		nTradePairID(nTradePairID),
		nUserTrade1(nUserTrade1),
		nUserTrade2(nUserTrade2),
		nTradePrice(nTradePrice),
		nTradeQty(nTradeQty),
		nTradeAmount(nTradePrice * nTradeQty),
		nBidAmount(nBidAmount),
		nAskAmount(nAskAmount),
		nUserPubKey1(nUserPubKey1),
		nUserPubKey2(nUserPubKey2),
		nFee1(nFee1),
		nFee2(nFee2),
		nFromBid(nFromBid),
		nTradeMNPubKey(""),
		nBalance1MNPubKey(""),
		nBalance2MNPubKey(""),
		nCurrentHash(""),		
		nTradeTime(nTradeTime)
	{}

	CActualTrade() :
		nActualTradeID(0),
		nTradePairID(0),
		nUserTrade1(0),
		nUserTrade2(0),
		nTradePrice(0),
		nTradeQty(0),
		nTradeAmount(0),
		nBidAmount(0),
		nAskAmount(0),
		nUserPubKey1(""),
		nUserPubKey2(""),
		nFee1(0),
		nFee2(0),
		nFromBid(true),
		nTradeMNPubKey(""),
		nBalance1MNPubKey(""),
		nBalance2MNPubKey(""),
		nCurrentHash(""),
		nTradeTime(0)
	{}

	std::string GetHash();
	bool VerifyTradeMNSignature();
	bool VerifyBalance1MNSignature();
	bool VerifyBalance2MNSignature();
	bool TradeMNSign();
	bool Balance1MNSign();
	bool Balance2MNSign();	
};

class CActualTradeManager
{
public:
	CActualTradeManager() {}
	bool GenerateActualTrade(std::shared_ptr<CActualTrade> actualTrade, CUserTradeSetting& setting);
	bool InputActualTrade(const std::shared_ptr<CActualTrade>& actualTrade, CUserTradeSetting& setting, CTradePair& tradePair);
	bool InputActualTradeFromNetwork(const std::shared_ptr<CActualTrade>& actualTrade, CUserTradeSetting& setting, CTradePair& tradePair);
	bool IsActualTradeInList(int TradePairID, int ActualTradeID, std::string Hash);
};

#endif