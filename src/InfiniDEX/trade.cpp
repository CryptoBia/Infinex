// Copyright (c) 2017-2018 The Infinex Core developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "stdafx.h"
#include "trade.h"
#include "chartdata.h"
#include "orderbook.h"
#include "tradepair.h"
#include "userbalance.h"
#include "usertradehistory.h"

#include <boost/multiprecision/cpp_int.hpp>
#include <chrono>

class CUserTrade;
class CUserTradeSetting;
class CUserTradeManager;

class CActualTrade;
class CActualTradeSetting;
class CActualTradeManager;

std::map<int, mUTPIUTV> mapBidUserTradeByPrice;
std::map<int, mUTPIUTV> mapAskUserTradeByPrice;
std::map<int, mUTPKUTV> mapBidUserTradeByPubkey;
std::map<int, mUTPKUTV> mapAskUserTradeByPubkey;
std::map<int, CUserTradeSetting> mapUserTradeSetting;
std::map<int, std::set<std::string>> mapUserTradeHash;
std::map<int, mATIAT> mapActualTradeByActualTradeID;
std::map<int, mUTImAT> mapActualTradeByUserTradeID;
std::map<int, std::vector<CActualTrade>> mapConflictTrade;
std::map<int, CActualTradeSetting> mapActualTradeSetting;
std::map<int, std::set<std::string>> mapActualTradeHash;
CActualTradeManager actualTradeManager;
CUserTradeManager userTradeManager;

bool CUserTradeSetting::IsValidSubmissionTimeAndUpdate(uint64_t time)
{
	int diff = nLastUserTradeTime - time;
	if (diff > nMaxSubmissionTimeDiff)
		return false;

	nLastUserTradeTime = time;
	return true;
}

bool CUserTrade::VerifyUserSignature()
{
	return true;
}

bool CUserTrade::VerifyMNSignature()
{
	return true;
}

bool CUserTrade::MNSign()
{
	return true;
}

void CUserTrade::RelayTo(CUserConnection& conn)
{

}

void CUserTrade::RelayToHandler()
{

}

uint64_t CUserTradeManager::GetAdjustedTime()
{
	return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
}

uint64_t CUserTradeManager::GetBidRequiredAmount(uint64_t Price, uint64_t Qty, int TradeFee)
{
	//overflow prevention
	boost::multiprecision::uint128_t amount = Price * Qty * 10000 / (10000 + TradeFee);
	return (uint64_t)amount;
}

uint64_t CUserTradeManager::GetAskExpectedAmount(uint64_t Price, uint64_t Qty, int TradeFee)
{
	//overflow prevention
	boost::multiprecision::uint128_t amount = Price * Qty * 10000 / (10000 - TradeFee);
	return (uint64_t)amount;
}

bool CUserTradeManager::IsSubmittedBidAmountValid(std::shared_ptr<CUserTrade>& userTrade, int nTradeFee)
{
	uint64_t ExpectedAmount = GetBidRequiredAmount(userTrade->nPrice, userTrade->nQuantity, nTradeFee);
	if ((ExpectedAmount - 1) <= userTrade->nAmount <= (ExpectedAmount + 1))
		return true;
	return false;
}

bool CUserTradeManager::IsSubmittedAskAmountValid(std::shared_ptr<CUserTrade>& userTrade, int nTradeFee)
{
	uint64_t ExpectedAmount = GetAskExpectedAmount(userTrade->nPrice, userTrade->nQuantity, nTradeFee);
	if ((ExpectedAmount - 1) <= userTrade->nAmount <= (ExpectedAmount + 1))
		return true;
	return false;
}

//change to enum for more return info
bool CUserTradeManager::IsSubmittedBidValid(std::shared_ptr<CUserTrade>& userTrade, CTradePair TradePair)
{
	if (TradePair.nTradePairID != userTrade->nTradePairID)
		return false;

	if (userTrade->nAmount < TradePair.nMaximumTradeAmount)
		return false;

	if (userTrade->nAmount > TradePair.nMaximumTradeAmount)
		return false;

	if (!IsSubmittedBidAmountValid(userTrade, TradePair.nBidTradeFee))
		return false;

	return true;
}

//change to enum for more return info
bool CUserTradeManager::IsSubmittedAskValid(std::shared_ptr<CUserTrade>& userTrade, CTradePair TradePair)
{
	if (TradePair.nTradePairID != userTrade->nTradePairID)
		return false;

	if (userTrade->nAmount < TradePair.nMaximumTradeAmount)
		return false;

	if (userTrade->nAmount > TradePair.nMaximumTradeAmount)
		return false;

	if (!IsSubmittedAskAmountValid(userTrade, TradePair.nAskTradeFee))
		return false;

	return true;
}

bool CUserTradeManager::IsUserTradeInList(int TradePairID, std::string UserHash)
{
	return mapUserTradeHash[TradePairID].count(UserHash);
}

void CUserTradeManager::AddToUserTradeList(int TradePairID, std::string UserHash)
{
	mapUserTradeHash[TradePairID].insert(UserHash);
}

bool CUserTradeManager::InputUserBuyTrade(std::shared_ptr<CUserTrade>& userTrade)
{
	userbalance_to_exchange_enum_t result = userBalanceManager.BalanceToExchange(userTrade->nTradePairID, userTrade->nUserPubKey, userTrade->nAmount);
	if (result != USER_BALANCE_DEDUCTED)
	{
		//we should not be here
		//for future user banning
		return false;
	}

	if (!mapBidUserTradeByPrice[userTrade->nTradePairID].count(userTrade->nPrice))
	{
		mUTIUT temp;
		temp.insert(std::make_pair(userTrade->nUserTradeID, userTrade));
		mapBidUserTradeByPrice[userTrade->nTradePairID].insert(std::make_pair(userTrade->nPrice, temp));
	}
	else
		mapBidUserTradeByPrice[userTrade->nTradePairID][userTrade->nPrice].insert(std::make_pair(userTrade->nUserTradeID, userTrade));

	if (!mapBidUserTradeByPubkey[userTrade->nTradePairID].count(userTrade->nUserPubKey))
	{
		mUTIUT temp;
		temp.insert(std::make_pair(userTrade->nUserTradeID, userTrade));
		mapBidUserTradeByPubkey[userTrade->nTradePairID].insert(std::make_pair(userTrade->nUserPubKey, temp));
	}
	else
		mapBidUserTradeByPubkey[userTrade->nTradePairID][userTrade->nUserPubKey].insert(std::make_pair(userTrade->nUserTradeID, userTrade));
	return true;
}

bool CUserTradeManager::InputUserSellTrade(std::shared_ptr<CUserTrade>& userTrade)
{
	userbalance_to_exchange_enum_t result = userBalanceManager.BalanceToExchange(userTrade->nTradePairID, userTrade->nUserPubKey, userTrade->nAmount);
	if (result != USER_BALANCE_DEDUCTED)
	{
		//we should not be here
		//for future user banning
		return false;
	}

	if (!mapAskUserTradeByPrice[userTrade->nTradePairID].count(userTrade->nPrice))
	{
		mUTIUT temp;
		temp.insert(std::make_pair(userTrade->nUserTradeID, userTrade));
		mapAskUserTradeByPrice[userTrade->nTradePairID].insert(std::make_pair(userTrade->nPrice, temp));
	}
	else
		mapAskUserTradeByPrice[userTrade->nTradePairID][userTrade->nPrice].insert(std::make_pair(userTrade->nUserTradeID, userTrade));

	if (!mapAskUserTradeByPubkey[userTrade->nTradePairID].count(userTrade->nUserPubKey))
	{
		mUTIUT temp;
		temp.insert(std::make_pair(userTrade->nUserTradeID, userTrade));
		mapAskUserTradeByPubkey[userTrade->nTradePairID].insert(std::make_pair(userTrade->nUserPubKey, temp));
	}
	else
		mapAskUserTradeByPubkey[userTrade->nTradePairID][userTrade->nUserPubKey].insert(std::make_pair(userTrade->nUserTradeID, userTrade));
	return true;
}

void CUserTradeManager::ProcessUserBuyRequest(std::shared_ptr<CUserTrade>& userTrade)
{
	if (userTrade->nQuantity <= 0)
		return;

	CTradePair tradePair = tradePairManager.GetTradePair(userTrade->nTradePairID);
	if (tradePair.nTradePairID != userTrade->nTradePairID)
		return;

	if (tradePair.nBidTradeFee > userTrade->nTradeFee)
		return;

	CUserTradeSetting setting = mapUserTradeSetting[userTrade->nTradePairID];
	if (setting.nTradePairID != userTrade->nTradePairID)
		return;

	if (setting.nSyncInProgress)
		return;

	if (!setting.nIsInChargeOfProcessUserTrade)
		return;

	if (!setting.IsValidSubmissionTimeAndUpdate(userTrade->nTimeSubmit))
		return;

	if (!userTrade->VerifyUserSignature())
		return;

	if (IsUserTradeInList(userTrade->nTradePairID, userTrade->nUserHash))
		return;

	if (!IsSubmittedBidAmountValid(userTrade, tradePair.nBidTradeFee))
		return;

	userTrade->nUserTradeID = (setting.nLastUserTradeID + 1);
	userTrade->nBalanceAmount = userTrade->nAmount;
	userTrade->nBalanceQty = userTrade->nQuantity;
	userTrade->nMNPubKey = setting.nMNPubKey;
	userTrade->nLastUpdate = GetAdjustedTime();
	userTrade->MNSign();

	if (!InputUserBuyTrade(userTrade))
		return;

	++setting.nLastUserTradeID;
	AddToUserTradeList(userTrade->nTradePairID, userTrade->nUserHash);
	userTrade->RelayToHandler();

	if (setting.nIsInChargeOfMatchUserTrade)
		InputMatchUserBuyRequest(userTrade);
}

void CUserTradeManager::ProcessUserSellRequest(std::shared_ptr<CUserTrade>& userTrade)
{
	if (userTrade->nQuantity <= 0)
		return;

	CTradePair tradePair = tradePairManager.GetTradePair(userTrade->nTradePairID);
	if (tradePair.nTradePairID != userTrade->nTradePairID)
		return;

	if (tradePair.nAskTradeFee > userTrade->nTradeFee)
		return;

	CUserTradeSetting setting = mapUserTradeSetting[userTrade->nTradePairID];
	if (setting.nTradePairID != userTrade->nTradePairID)
		return;

	if (setting.nSyncInProgress)
		return;

	if (!setting.nIsInChargeOfProcessUserTrade)
		return;

	if (!setting.IsValidSubmissionTimeAndUpdate(userTrade->nTimeSubmit))
		return;

	if (!userTrade->VerifyUserSignature())
		return;

	if (IsUserTradeInList(userTrade->nTradePairID, userTrade->nUserHash))
		return;

	if (!IsSubmittedAskAmountValid(userTrade, tradePair.nAskTradeFee))
		return;

	userTrade->nUserTradeID = (setting.nLastUserTradeID + 1);
	userTrade->nBalanceAmount = userTrade->nAmount;
	userTrade->nBalanceQty = userTrade->nQuantity;
	userTrade->nMNPubKey = setting.nMNPubKey;
	userTrade->nLastUpdate = GetAdjustedTime();
	userTrade->MNSign();

	if (!InputUserSellTrade(userTrade))
		return;

	++setting.nLastUserTradeID;
	AddToUserTradeList(userTrade->nTradePairID, userTrade->nUserHash);
	userTrade->RelayToHandler();
}

void CUserTradeManager::InputMatchUserBuyRequest(std::shared_ptr<CUserTrade>& userTrade, bool InitialCheck)
{
	CTradePair tradePair = tradePairManager.GetTradePair(userTrade->nTradePairID);
	CUserTradeSetting& setting = mapUserTradeSetting[userTrade->nTradePairID];

	if (!InitialCheck) 
	{
		if (userTrade->nBalanceQty <= 0)
			return;
		
		if (tradePair.nTradePairID != userTrade->nTradePairID)
			return;
		
		if (setting.nTradePairID != userTrade->nTradePairID)
			return;

		if (setting.nSyncInProgress)
			return;

		if (!setting.nIsInChargeOfMatchUserTrade)
			return;

		if (!userTrade->VerifyMNSignature())
			return;

		if (IsUserTradeInList(userTrade->nTradePairID, userTrade->nUserHash))
			return;

		int sequence = userTrade->nUserTradeID - setting.nLastUserTradeID;
		if (sequence <= 0)
			return;

		if (sequence > 1)
		{
			//need to do something more here
			return;
		}

		if (!InputUserBuyTrade(userTrade))
		{
			//need to know why its here
			return;
		}
	}

	mUTPIUTV::iterator Sellers = mapAskUserTradeByPrice[userTrade->nTradePairID].begin();
	while (Sellers != mapAskUserTradeByPrice[userTrade->nTradePairID].end() && Sellers->first <= userTrade->nPrice)
	{
		mUTIUT::iterator Seller = Sellers->second.begin();
		while (Seller != Sellers->second.end())
		{
			std::shared_ptr<CUserTrade> ExistingTrade = Seller->second;
			if (ExistingTrade->nBalanceQty <= 0)
				continue;

			int qty = 0;
			if (userTrade->nBalanceQty <= ExistingTrade->nBalanceQty)
			{
				qty = userTrade->nBalanceQty;
			}
			else
			{
				qty = ExistingTrade->nBalanceQty;
			}

			ExistingTrade->nBalanceQty -= qty;
			userTrade->nBalanceQty -= qty;
			int bidTradeFee = (userTrade->nTradeFee < tradePair.nAskTradeFee) ? userTrade->nTradeFee : tradePair.nAskTradeFee;
			int askTradeFee = (ExistingTrade->nTradeFee < tradePair.nBidTradeFee) ? ExistingTrade->nTradeFee : tradePair.nBidTradeFee;
			uint64_t bidAmount = GetBidRequiredAmount(userTrade->nPrice, qty, bidTradeFee);
			uint64_t askAmount = GetAskExpectedAmount(ExistingTrade->nPrice, qty, askTradeFee);
			uint64_t actualAmount = userTrade->nPrice*qty;
			uint64_t tradeTime = GetAdjustedTime();
			//userBalanceManager.UpdateAfterTradeBalance(ExistingTrade->nUserPubKey, cuserTrade->nUserPubKey, tradePair.nCoinInfoID1, tradePair.nCoinInfoID2, -bidAmount, qty, -qty, askAmount);
			if (setting.nIsInChargeOfOrderBook)
				orderBookManager.AdjustAskQuantity(tradePair.nTradePairID, ExistingTrade->nPrice, qty);
			//actualTradeManager.InputNewCompletedTrade(CActualTrade(ExistingTrade->nUserPubKey, cut->nUserPubKey, tradePair.nTradePairID, ExistingTrade->nPrice, qty, actualAmount, bidAmount - actualAmount, tradePair.nBidTradeFeeCoinID, actualAmount - askAmount, tradePair.nAskTradeFeeCoinID, "", GetAdjustedTime()));

			if (setting.nIsInChargeOfChartData)
				ChartDataManager.InputNewTrade(userTrade->nTradePairID, userTrade->nPrice, qty, tradeTime);
			if (setting.nIsInChargeOfMarketTradeHistory)
				userTradeHistoryManager.InputNewUserTradeHistory(CUserTradeHistory(userTrade->nTradePairID, userTrade->nUserPubKey, ExistingTrade->nUserPubKey, userTrade->nPrice, qty, actualAmount, true, tradeTime));

			if (userTrade->nBalanceQty == 0)
				return;

			++Seller;
		}
		++Sellers;
	}
}

void CUserTradeManager::InputMatchUserSellRequest(std::shared_ptr<CUserTrade>& userTrade, bool InitialCheck)
{
	CTradePair tradePair = tradePairManager.GetTradePair(userTrade->nTradePairID);
	CUserTradeSetting& setting = mapUserTradeSetting[userTrade->nTradePairID];

	if (!InitialCheck)
	{
		if (userTrade->nBalanceQty <= 0)
			return;
		
		if (tradePair.nTradePairID != userTrade->nTradePairID)
			return;
				
		if (setting.nTradePairID != userTrade->nTradePairID)
			return;

		if (setting.nSyncInProgress)
			return;

		if (!setting.nIsInChargeOfMatchUserTrade)
			return;

		if (!userTrade->VerifyMNSignature())
			return;

		if (IsUserTradeInList(userTrade->nTradePairID, userTrade->nUserHash))
			return;

		int sequence = userTrade->nUserTradeID - setting.nLastUserTradeID;
		if (sequence <= 0)
			return;

		if (sequence > 1)
		{
			//need to do something more here
			return;
		}

		if (!InputUserSellTrade(userTrade))
		{
			//need to know why its here
			return;
		}
	}

	mUTPIUTV::reverse_iterator Buyers = mapBidUserTradeByPrice[userTrade->nTradePairID].rbegin();
	while (Buyers != mapBidUserTradeByPrice[userTrade->nTradePairID].rend() && Buyers->first >= userTrade->nPrice)
	{
		mUTIUT::iterator Buyer = Buyers->second.begin();
		while (Buyer != Buyers->second.end())
		{
			std::shared_ptr<CUserTrade> ExistingTrade = Buyer->second;
			if (ExistingTrade->nBalanceQty <= 0)
				continue;

			int qty = 0;
			if (userTrade->nBalanceQty <= ExistingTrade->nBalanceQty)
			{
				qty = userTrade->nBalanceQty;
			}
			else
			{
				qty = ExistingTrade->nBalanceQty;
			}

			ExistingTrade->nBalanceQty -= qty;
			userTrade->nBalanceQty -= qty;
			int bidTradeFee = (ExistingTrade->nTradeFee < tradePair.nBidTradeFee) ? ExistingTrade->nTradeFee : tradePair.nBidTradeFee;
			int askTradeFee = (userTrade->nTradeFee < tradePair.nAskTradeFee) ? userTrade->nTradeFee : tradePair.nAskTradeFee;
			uint64_t bidAmount = GetBidRequiredAmount(ExistingTrade->nPrice, qty, bidTradeFee);
			uint64_t askAmount = GetAskExpectedAmount(userTrade->nPrice, qty, askTradeFee);
			uint64_t actualAmount = userTrade->nPrice*qty;
			uint64_t tradeTime = GetAdjustedTime();
			CActualTrade actualTrade(tradePair.nTradePairID, ExistingTrade->nUserTradeID, userTrade->nUserTradeID, userTrade->nPrice, qty, ExistingTrade->nUserPubKey, userTrade->nUserPubKey, bidTradeFee, 0, askTradeFee, 0, GetAdjustedTime());
			if (actualTradeManager.GenerateActualTrade(actualTrade))
				userBalanceManager.UpdateAfterTradeBalance(ExistingTrade->nUserPubKey, userTrade->nUserPubKey, tradePair.nCoinID1, tradePair.nCoinID2, 0 - bidAmount, qty, 0 - qty, askAmount);
			if (setting.nIsInChargeOfOrderBook)
				orderBookManager.AdjustBidQuantity(tradePair.nTradePairID, ExistingTrade->nPrice, qty);
			if (setting.nIsInChargeOfChartData)
				ChartDataManager.InputNewTrade(userTrade->nTradePairID, userTrade->nPrice, qty, tradeTime);
			if (setting.nIsInChargeOfMarketTradeHistory)
				userTradeHistoryManager.InputNewUserTradeHistory(CUserTradeHistory(userTrade->nTradePairID, ExistingTrade->nUserPubKey, userTrade->nUserPubKey, userTrade->nPrice, qty, actualAmount, false, tradeTime));
			if (userTrade->nBalanceQty == 0)
				return;

			++Buyer;
		}
		++Buyers;
	}
}

bool CActualTradeManager::GenerateActualTrade(CActualTrade& ActualTrade)
{
	ActualTrade.nActualTradeID = ++mapActualTradeSetting[ActualTrade.nTradePairID].nLastActualTradeID;
	ActualTrade.nMasternodeInspector = ""; //to fill with MN public key
	mapActualTradeSetting[ActualTrade.nTradePairID].nLastHash = ActualTrade.nCurrentHash = ActualTrade.GetHash();
	ActualTrade.Sign();
	return true;
}

std::string CActualTrade::GetHash()
{
	return "";
}

bool CActualTrade::Sign()
{
	std::string strError = "";
	std::string strMessage = boost::lexical_cast<std::string>(nTradePairID) + boost::lexical_cast<std::string>(nTradePrice) +
		boost::lexical_cast<std::string>(nTradeQty) + boost::lexical_cast<std::string>(nTradeAmount) + nUserPubKey1 + nUserPubKey2 + boost::lexical_cast<std::string>(nFee1) +
		boost::lexical_cast<std::string>(nFee2) + nMasternodeInspector + boost::lexical_cast<std::string>(nTradeTime);
	return true;
}

bool CActualTrade::VerifySignature()
{
	return true;
}

bool CActualTrade::InformActualTrade()
{
	if (!Sign())
	{
		return false;
	}
	return true;
}

bool CActualTrade::InformConflictTrade(CNode* node)
{
	return true;
}

void CActualTradeManager::InputNewTradePair(int TradePairID)
{
	if (!mapActualTradeByActualTradeID.count(TradePairID))
	{
		mapActualTradeByActualTradeID.insert(std::make_pair(TradePairID, mATIAT()));
		mapActualTradeByUserTradeID.insert(std::make_pair(TradePairID, mUTImAT()));		
		mapConflictTrade.insert(std::make_pair(TradePairID, std::vector<CActualTrade>()));
		mapActualTradeSetting.insert(std::make_pair(TradePairID, CActualTradeSetting(TradePairID)));
		mapActualTradeHash.insert(std::make_pair(TradePairID, std::set<std::string>()));
	}
}

bool CActualTradeManager::GetActualTrade(CNode* node, int ActualTradeID, int TradePairID)
{
	if (!mapActualTradeByActualTradeID.count(TradePairID))
	{
		//inform other node that we don't carry this trade pair
		return false;
	}

	if (!mapActualTradeByActualTradeID[TradePairID].count(ActualTradeID))
	{
		//inform other node that we don't have this trade data		
		return false;
	}

	//send this trade data to other node
	CActualTrade temp = *mapActualTradeByActualTradeID[TradePairID][ActualTradeID];

	return true;
}

bool CActualTradeManager::AddNewActualTrade(CActualTrade ActualTrade)
{
	if (!IsTradePairInList(ActualTrade.nTradePairID))
	{
		//its not possible to be here
		//if here, we would need to switch this node to passive & resync
		return false;
	}
	
	std::string tempHash(ActualTrade.GetHash());
	if (mapActualTradeHash[ActualTrade.nTradePairID].count(tempHash))
		return false;
	mapActualTradeHash[ActualTrade.nTradePairID].insert(tempHash);	
	ActualTrade.nCurrentHash = tempHash;	
	ActualTrade.nActualTradeID = ++mapActualTradeSetting[ActualTrade.nTradePairID].nLastActualTradeID;
	std::pair<int, std::shared_ptr<CActualTrade>> temp(std::make_pair(ActualTrade.nActualTradeID, &ActualTrade));
	mapActualTradeByActualTradeID[ActualTrade.nTradePairID].insert(temp);
	
	if (ChartDataManager.IsInChargeOfChartData(ActualTrade.nTradePairID))
		ChartDataManager.InputNewTrade(ActualTrade.nTradePairID, ActualTrade.nTradePrice, ActualTrade.nTradeQty, userTradeManager.GetAdjustedTime());

	if (mapActualTradeByUserTradeID[ActualTrade.nTradePairID].count(ActualTrade.nUserTrade1))
		mapActualTradeByUserTradeID[ActualTrade.nTradePairID][ActualTrade.nUserTrade1].insert(temp);
	else
	{
		mATIAT temp2;
		temp2.insert(temp);		
		mapActualTradeByUserTradeID[ActualTrade.nTradePairID].insert(std::make_pair(ActualTrade.nUserTrade1, temp2));
	}

	if (mapActualTradeByUserTradeID[ActualTrade.nTradePairID].count(ActualTrade.nUserTrade2))
		mapActualTradeByUserTradeID[ActualTrade.nTradePairID][ActualTrade.nUserTrade2].insert(temp);
	else
	{
		mATIAT temp2;
		temp2.insert(temp);
		mapActualTradeByUserTradeID[ActualTrade.nTradePairID].insert(std::make_pair(ActualTrade.nUserTrade2, temp2));
	}

	ActualTrade.InformActualTrade();
	return true;
}

uint64_t CActualTradeManager::GetTotalTradedQuantity(int TradePairID, int UserTradeID)
{
	mATIAT* temp = &mapActualTradeByUserTradeID[TradePairID][UserTradeID];
	mATIAT::iterator it = temp->begin();
	uint64_t qty = 0;
	while (it != temp->end())
	{
		qty += it->second->nTradeQty;
		it++;
	}
	return qty;
}

bool CActualTradeManager::IsActualTradeInList(CActualTrade ActualTrade)
{
	if (!mapActualTradeByActualTradeID[ActualTrade.nTradePairID].count(ActualTrade.nActualTradeID))
		return false;

	if (mapActualTradeByActualTradeID[ActualTrade.nTradePairID][ActualTrade.nActualTradeID]->nCurrentHash != ActualTrade.nCurrentHash)
	{
		//inform of differences
		return false;
	}

	return true;
}

bool CActualTradeManager::AddNewActualTrade(CNode* node, CConnman& connman, CActualTrade ActualTrade)
{
	return true;
}

std::vector<std::string> CActualTradeManager::FindDuplicateTrade(int TradePairID)
{
	std::vector<std::string> DuplicatedTrade;
	std::vector<std::string> HashContainer;
	for (auto &a : mapActualTradeByActualTradeID[TradePairID])
	{
		bool Conflict = false;
		for (auto &b : HashContainer)
		{
			if (a.second->nCurrentHash == b)
			{
				DuplicatedTrade.push_back(b);
				Conflict = true;
				break;
			}
		}
		if (!Conflict)
			HashContainer.push_back(a.second->nCurrentHash);
	}
	return DuplicatedTrade;
}

bool CActualTradeManager::ToVerifyActualTrade(int TradePairID)
{
	return mapActualTradeSetting[TradePairID].ToVerifyActualTrade;
}

int CActualTradeManager::IsActualTradeInSequence(CActualTrade ActualTrade)
{
	return ActualTrade.nActualTradeID - mapActualTradeSetting[ActualTrade.nTradePairID].nLastActualTradeID;
}

void CActualTradeManager::ProcessActualTrade(CActualTrade ActualTrade)
{
	if (!IsTradePairInList(ActualTrade.nTradePairID))
		return;

	if (!ActualTrade.VerifySignature())
		return;

	int sequence = IsActualTradeInSequence(ActualTrade);
	if (sequence <= 0)
		return;

	if (sequence > 1)
	{
		//need to do something more here
		return;
	}

	if (IsActualTradeInList(ActualTrade.nTradePairID, ActualTrade.nActualTradeID))
		return;

	if (ToVerifyActualTrade(ActualTrade.nTradePairID))
	{

	}
}

bool CActualTradeManager::IsTradePairInList(int TradePairID)
{
	return true;
}

bool CActualTradeManager::IsActualTradeInList(int TradePairID, int ActualTradeID)
{
	return true;
}