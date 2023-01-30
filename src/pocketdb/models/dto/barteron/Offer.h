// Copyright (c) 2023 The Pocketnet developers
// Distributed under the Apache 2.0 software license, see the accompanying
// https://www.apache.org/licenses/LICENSE-2.0

#ifndef POCKETTX_BARTERON_OFFER_H
#define POCKETTX_BARTERON_OFFER_H

#include "pocketdb/models/dto/content/Content.h"

namespace PocketTx
{
    using namespace std;

    class BarteronOffer : public Content
    {
    public:
        BarteronOffer();
        BarteronOffer(const CTransactionRef& tx);

        optional<string> GetPayloadLang() const;
        optional<string> GetPayloadCaption() const;
        optional<string> GetPayloadMessage() const;
        optional<string> GetPayloadTags() const;
        optional<string> GetPayloadImages() const;
        optional<string> GetPayloadSettings() const;
    };

} // namespace PocketTx

#endif // POCKETTX_BARTERON_OFFER_H