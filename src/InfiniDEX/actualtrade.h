// Copyright (c) 2017-2018 The Infinex Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef ACTUALTRADE_H
#define ACTUALTRADE_H

#include "hash.h"
#include "net.h"
#include "utilstrencodings.h"

class CActualTrade;
class CActualTradeManager;

extern std::map<int, std::vector<CActualTrade>> mapActualTrade; //trade pair ID & actual trade list
extern CActualTradeManager actualTradeManager;

class CActualTrade
{
public:
    std::string nUserPubKey1;
    std::string nUserPubKey2;
    int nTradePairID;
    uint64_t nTradePrice;
    uint64_t nTradeQty;
    uint64_t nTradeAmount;
    int64_t nFee1;
    int64_t nFee2;
	uint64_t nTradeTime;

	CActualTrade(std::string nUserPubKey1, std::string nUserPubKey2, int nTradePairID, uint64_t nTradePrice, uint64_t nTradeQty, uint64_t nTradeAmount, int64_t nFee1, int64_t nFee2, uint64_t nTradeTime) :
		nUserPubKey1(nUserPubKey1),
        nUserPubKey2(nUserPubKey2),
        nTradePairID(nTradePairID),
        nTradePrice(nTradePrice),
        nTradeQty(nTradeQty),
        nTradeAmount(nTradeAmount),
        nFee1(nFee1),
        nFee2(nFee2),
        nTradeTime(nTradeTime)
	{}

	CActualTrade() :
		nUserPubKey1(""),
        nUserPubKey2(""),
        nTradePairID(0),
        nTradePrice(0),
        nTradeQty(0),
        nTradeAmount(0),
        nFee1(0),
        nFee2(0),
        nTradeTime(0)
	{}
};

class CActualTradeManager
{
public:
    CActualTradeManager() {}
};

#endif
