// Copyright (c) 2017-2018 The Infinex Core developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef TRADE_H
#define TRADE_H

#include <iostream>
#include <vector>
#include <map>
#include <memory>
#include <set>
#include "tradepair.h"
#include "userconnection.h"

class CUserTrade;
class CUserTradeSetting;
class CUserTradeManager;

class CActualTrade;
class CActualTradeSetting;
class CActualTradeManager;

typedef std::map<int, std::shared_ptr<CUserTrade>> mUTIUT; //user trade id and trade details
typedef std::map<uint64_t, mUTIUT> mUTPIUTV; //price and user trade container
typedef std::map<std::string, mUTIUT> mUTPKUTV; //user public key and user trade container
extern std::map<int, mUTPIUTV> mapBidUserTradeByPrice;
extern std::map<int, mUTPIUTV> mapAskUserTradeByPrice;
extern std::map<int, mUTPKUTV> mapBidUserTradeByPubkey;
extern std::map<int, mUTPKUTV> mapAskUserTradeByPubkey;
extern std::map<int, CUserTradeSetting> mapUserTradeSetting;
extern std::map<int, std::set<std::string>> mapUserTradeHash;
extern std::map<int, std::set<std::shared_ptr<CUserTrade>>> mapPendingUserTrade;
extern CUserTradeManager userTradeManager;

typedef std::map<int, std::shared_ptr<CActualTrade>> mATIAT;
typedef std::map<int, mATIAT> mUTImAT;
extern std::map<int, mATIAT> mapActualTradeByActualTradeID;
extern std::map<int, mUTImAT> mapActualTradeByUserTradeID;
extern std::map<int, std::vector<CActualTrade>> mapConflictTrade;
extern std::map<int, CActualTradeSetting> mapActualTradeSetting;
extern std::map<int, std::set<std::string>> mapActualTradeHash;
extern CActualTradeManager actualTradeManager;

class CUserTradeSetting
{
public:
	int nTradePairID;
	uint32_t nMaxSubmissionTimeDiff;
	uint16_t nToStoreLowerLimit;
	uint16_t nToStoreUpperLimit;
	int nLastUserTradeID;
	uint64_t nLastUserTradeTime;
	std::string nMNPubKey;
	bool nSyncInProgress;
	bool nInChargeOfBidBroadcast;
	bool nInChargeOfAskBroadcast;
	bool nIsInChargeOfProcessUserTrade;
	bool nIsInChargeOfMatchUserTrade;

	CUserTradeSetting(int nTradePairID, std::string nMNPubKey) :
		nTradePairID(nTradePairID),
		nMaxSubmissionTimeDiff(3000),
		nToStoreLowerLimit(50),
		nToStoreUpperLimit(100),
		nLastUserTradeID(0),
		nMNPubKey(nMNPubKey),
		nSyncInProgress(false),
		nInChargeOfBidBroadcast(false),
		nInChargeOfAskBroadcast(false),
		nIsInChargeOfProcessUserTrade(false),
		nIsInChargeOfMatchUserTrade(false)
	{}

	CUserTradeSetting() :
		nTradePairID(0),
		nMaxSubmissionTimeDiff(3000),
		nToStoreLowerLimit(50),
		nToStoreUpperLimit(100),
		nLastUserTradeID(0),
		nMNPubKey(""),
		nSyncInProgress(false),
		nInChargeOfBidBroadcast(false),
		nInChargeOfAskBroadcast(false),
		nIsInChargeOfProcessUserTrade(false),
		nIsInChargeOfMatchUserTrade(false)
	{}

	bool IsValidSubmissionTimeAndUpdate(uint64_t time);
};

class CUserTrade
{
private:
	std::vector<unsigned char> userVchSig;
	std::vector<unsigned char> mnVchSig;

public:
	int nTradePairID;
	uint64_t nPrice;
	uint64_t nQuantity;
	uint64_t nAmount;
	int nTradeFee; //this is here to compare with trade pair fee & apply to whichever lower fee (benefit user)
	std::string nUserPubKey;
	uint64_t nTimeSubmit;
	std::string nUserHash;
	int nUserTradeID; //generated by MN
	std::string nMNPubKey;
	int64_t nBalanceQty;
	int64_t nBalanceAmount;
	uint64_t nLastUpdate;

	CUserTrade(int nTradePairID, uint64_t nPrice, uint64_t nQuantity, int nTradeFee, std::string nUserPubKey, uint64_t nTimeSubmit, std::string nUserHash) :
		nTradePairID(nTradePairID),
		nPrice(nPrice),
		nQuantity(nQuantity),
		nAmount(nPrice * nQuantity),
		nTradeFee(nTradeFee),
		nUserPubKey(nUserPubKey),
		nTimeSubmit(nTimeSubmit),
		nUserHash(nUserHash),
		nUserTradeID(0),
		nMNPubKey(""),
		nBalanceQty(nQuantity),
		nBalanceAmount(nAmount),
		nLastUpdate(0)
	{}

	CUserTrade(int nTradePairID, uint64_t nPrice, uint64_t nQuantity, uint64_t nAmount, int nTradeFee, std::string nUserPubKey, uint64_t nTimeSubmit, std::string nUserHash, int nUserTradeID, std::string nMNPubKey, int64_t nBalanceQty, int64_t nBalanceAmount, uint64_t nLastUpdate) :
		nTradePairID(nTradePairID),
		nPrice(nPrice),
		nQuantity(nQuantity),
		nAmount(nAmount),
		nTradeFee(nTradeFee),
		nUserPubKey(nUserPubKey),
		nTimeSubmit(nTimeSubmit),
		nUserHash(nUserHash),
		nUserTradeID(nUserTradeID),
		nMNPubKey(nMNPubKey),
		nBalanceQty(nBalanceQty),
		nBalanceAmount(nBalanceAmount),
		nLastUpdate(nLastUpdate)
	{}

	CUserTrade() :
		nTradePairID(0),
		nPrice(0),
		nQuantity(0),
		nAmount(0),
		nTradeFee(0),
		nUserPubKey(""),
		nTimeSubmit(0),
		nUserHash(""),
		nUserTradeID(0),
		nMNPubKey(""),
		nBalanceQty(0),
		nBalanceAmount(0),
		nLastUpdate(0)
	{}

	bool VerifyUserSignature();
	bool VerifyMNSignature();
	bool MNSign();
	void RelayTo(CUserConnection& conn);
	void RelayToHandler();
};

class CUserTradeManager
{
private:
	std::vector<unsigned char> vchSig;

public:
	uint64_t GetAdjustedTime();
	void AssignBidBroadcastRole(int TradePairID);
	void AssignAskBroadcastRole(int TradePairID);
	void InitTradePair(int TradePairID);
	bool AssignNodeToProcessUserTrade(int TradePairID, bool toAssign = true);
	bool AssignNodeToMatchUserTrade(int TradePairID, bool toAssign = true);
	bool IsUserTradeInList(int TradePairID, std::string UserHash);
	void AddToUserTradeList(int TradePairID, std::string UserHash);
	uint64_t GetBidRequiredAmount(uint64_t Price, uint64_t Qty, int TradeFee);
	uint64_t GetAskExpectedAmount(uint64_t Price, uint64_t Qty, int TradeFee);
	bool IsSubmittedBidValid(std::shared_ptr<CUserTrade>& userTrade, CTradePair& TradePair);
	bool IsSubmittedAskValid(std::shared_ptr<CUserTrade>& userTrade, CTradePair& TradePair);
	bool IsSubmittedBidAmountValid(std::shared_ptr<CUserTrade>& userTrade, int nTradeFee);
	bool IsSubmittedAskAmountValid(std::shared_ptr<CUserTrade>& userTrade, int nTradeFee);
	void ProcessUserBuyRequest(std::shared_ptr<CUserTrade>& userTrade, CUserTradeSetting& setting, CTradePair& tradePair);
	void ProcessUserSellRequest(std::shared_ptr<CUserTrade>& userTrade, CUserTradeSetting& setting, CTradePair& tradePair);
	bool InputUserBuyTrade(std::shared_ptr<CUserTrade>& userTrade);
	bool InputUserSellTrade(std::shared_ptr<CUserTrade>& userTrade);
	void InputMatchUserBuyRequest(std::shared_ptr<CUserTrade>& userTrade, CUserTradeSetting& setting, CTradePair& tradePair, bool InitialCheck = false);
	void InputMatchUserSellRequest(std::shared_ptr<CUserTrade>& userTrade, CUserTradeSetting& setting, CTradePair& tradePair, bool InitialCheck = false);
	bool ReduceBalanceQty(int TradePairID, int UserTradeID1, int UserTradeID2, uint64_t Qty);
	int64_t GetBalanceAmount(int TradePairID, uint64_t Price, int UserTradeID);
};

class CActualTradeSetting
{
public:
	int nTradePairID;
	uint32_t nMaxSubmissionTimeDiff;
	uint32_t nLastTradeMaxPreTimeDistance;
	uint16_t nToStoreLowerLimit;
	uint16_t nToStoreUpperLimit;
	int nLastActualTradeID;
	uint64_t nLastActualTradeTime;
	std::string nMNPubKey;
	bool nSyncInProgress;
	bool ToVerifyActualTrade;
	bool nIsInChargeOfChartData;
	bool nIsInChargeOfMarketTradeHistory;
	bool nIsInChargeOfUserTradeHistory;
	bool nIsInChargeOfUserBalance;
	bool nIsInChargeOfOrderBook;

	CActualTradeSetting(int nTradePairID, std::string nMNPubKey) :
		nTradePairID(nTradePairID),
		nMaxSubmissionTimeDiff(3000),
		nLastTradeMaxPreTimeDistance(3000),
		nToStoreLowerLimit(50),
		nToStoreUpperLimit(100),
		nLastActualTradeID(0),
		nLastActualTradeTime(0),
		nMNPubKey(nMNPubKey),
		nSyncInProgress(false),
		ToVerifyActualTrade(false),
		nIsInChargeOfChartData(false),
		nIsInChargeOfMarketTradeHistory(false),
		nIsInChargeOfUserTradeHistory(false),
		nIsInChargeOfUserBalance(false),
		nIsInChargeOfOrderBook(false)
	{}

	CActualTradeSetting():
		nTradePairID(0),
		nMaxSubmissionTimeDiff(3000),
		nLastTradeMaxPreTimeDistance(3000),
		nToStoreLowerLimit(50),
		nToStoreUpperLimit(100),
		nLastActualTradeID(0),
		nLastActualTradeTime(0),
		nSyncInProgress(false),
		ToVerifyActualTrade(false),
		nIsInChargeOfChartData(false),
		nIsInChargeOfMarketTradeHistory(false),
		nIsInChargeOfUserTradeHistory(false),
		nIsInChargeOfUserBalance(false),
		nIsInChargeOfOrderBook(false)
	{}

	bool IsValidSubmissionTimeAndUpdate(uint64_t time);
};

class CActualTrade
{
private:
	std::vector<unsigned char> vchSig;

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
	std::string nMasternodeInspector;
	std::string nCurrentHash;
	uint64_t nTradeTime;

	CActualTrade(int nTradePairID, int nUserTrade1, int nUserTrade2, uint64_t nTradePrice, uint64_t nTradeQty, uint64_t nBidAmount, uint64_t nAskAmount, std::string nUserPubKey1,
		std::string nUserPubKey2, int64_t nFee1, int64_t nFee2, uint64_t nTradeTime) :
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
		nMasternodeInspector(""),
		nCurrentHash(""),
		nTradeTime(nTradeTime)
	{}
	
	CActualTrade(int nActualTradeID, int nTradePairID, int nUserTrade1, int nUserTrade2, uint64_t nTradePrice, uint64_t nTradeQty, uint64_t nTradeAmount, uint64_t nBidAmount, uint64_t nAskAmount, 
		std::string nUserPubKey1, std::string nUserPubKey2, int64_t nFee1, int64_t nFee2, std::string nMasternodeInspector, std::string nCurrentHash, uint64_t nTradeTime) :
		nActualTradeID(nActualTradeID),
		nTradePairID(nTradePairID),
		nUserTrade1(nUserTrade1),
		nUserTrade2(nUserTrade2),
		nTradePrice(nTradePrice),
		nTradeQty(nTradeQty),
		nTradeAmount(nTradeAmount),
		nBidAmount(nBidAmount),
		nAskAmount(nAskAmount),
		nUserPubKey1(nUserPubKey1),
		nUserPubKey2(nUserPubKey2),
		nFee1(nFee1),
		nFee2(nFee2),
		nMasternodeInspector(nMasternodeInspector),
		nCurrentHash(nCurrentHash),
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
		nMasternodeInspector(""),
		nCurrentHash(""),
		nTradeTime(0)
	{}

	std::string GetHash();
	bool VerifySignature();
	bool Sign();
	bool Relay();
	bool InformConflictTrade(CNode* node);
};

class CActualTradeManager
{
public:
	CActualTradeManager() {}
	bool GenerateActualTrade(std::shared_ptr<CActualTrade> actualTrade, CActualTradeSetting& actualTradeSetting);
	bool InputActualTrade(std::shared_ptr<CActualTrade> actualTrade, CActualTradeSetting& setting, CTradePair& tradePair);
	bool InputActualTradeFromNode(std::shared_ptr<CActualTrade> actualTrade, CActualTradeSetting& setting, CTradePair& tradePair);
	bool IsActualTradeInList(int TradePairID, int ActualTradeID, std::string Hash);
	bool GetActualTrade(CNode* node, int ActualTradeID, int TradePairID);
	bool AddNewActualTrade(CActualTrade ActualTrade); //process by same node
	std::vector<std::string> FindDuplicateTrade(int TradePairID);
	void InputNewTradePair(int TradePairID);	
	uint64_t GetTotalTradedQuantity(int TradePairID, int UserTradeID);
};

#endif