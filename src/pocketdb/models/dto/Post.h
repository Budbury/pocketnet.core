#ifndef POCKETTX_POST_H
#define POCKETTX_POST_H

#include "pocketdb/models/base/Transaction.h"

namespace PocketTx
{
    using namespace std;

    class Post : public Transaction
    {
    public:
        Post(const string& hash, int64_t time);

        shared_ptr<UniValue> Serialize() const override;

        void Deserialize(const UniValue& src) override;
        void DeserializeRpc(const UniValue& src) override;

        shared_ptr<string> GetAddress() const;
        void SetAddress(string value);

        shared_ptr<string> GetRootTxHash() const;
        void SetRootTxHash(string value);

        shared_ptr<string> GetRelayTxHash() const;
        void SetRelayTxHash(string value);

        bool IsEdit() const;

    protected:
        void DeserializePayload(const UniValue& src) override;
        void BuildHash() override;
    };

} // namespace PocketTx

#endif // POCKETTX_POST_H