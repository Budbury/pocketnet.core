// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2018 Bitcoin developers
// Copyright (c) 2018-2021 Pocketnet developers
// Distributed under the Apache 2.0 software license, see the accompanying
// https://www.apache.org/licenses/LICENSE-2.0

#ifndef POCKETCONSENSUS_SOCIAL_BASE_HPP
#define POCKETCONSENSUS_SOCIAL_BASE_HPP

#include "univalue/include/univalue.h"

#include "pocketdb/helpers/TypesHelper.hpp"
#include "pocketdb/pocketnet.h"
#include "pocketdb/models/base/Base.hpp"
#include "pocketdb/consensus/Base.hpp"

namespace PocketConsensus
{
    using std::static_pointer_cast;

    class SocialBaseConsensus : BaseConsensus
    {
    public:
        SocialBaseConsensus(int height) : BaseConsensus(height) {}
        
        // TODO (brangr): разделить проверку для подключения к цепи и первичную? парам или разные методы
        virtual tuple<bool, SocialConsensusResult> Validate(shared_ptr<Transaction> tx, PocketBlock& block)
        {
            
        }
    protected:
        virtual tuple<bool, SocialConsensusResult> CheckRegistration(shared_ptr<string> address)
        {
            // TODO (brangr): call repo select for find account
        }
    private:
    };
}

#endif // POCKETCONSENSUS_SOCIAL_BASE_HPP