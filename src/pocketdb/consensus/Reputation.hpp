// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2018 Bitcoin developers
// Copyright (c) 2018-2021 Pocketnet developers
// Distributed under the Apache 2.0 software license, see the accompanying
// https://www.apache.org/licenses/LICENSE-2.0

#ifndef POCKETCONSENSUS_REPUTATION_HPP
#define POCKETCONSENSUS_REPUTATION_HPP

#include "pocketdb/consensus/Base.hpp"

namespace PocketConsensus
{
    /*******************************************************************************************************************
    *
    *  Reputation consensus base class
    *
    *******************************************************************************************************************/
    class ReputationConsensus : public BaseConsensus
    {
    protected:
        virtual int64_t GetThresholdReputationScore() = 0;

        virtual int64_t GetThresholdLikersCount() = 0;

        virtual int64_t GetScoresOneToOne() = 0;

        virtual int64_t GetScoresOneToOneDepth() = 0;

        virtual int64_t GetScoresOneToOneOverComment() = 0;

        virtual int64_t GetScoresToPostModifyReputationDepth() = 0;


        virtual bool AllowModifyReputation(int addressId, int height)
        {
            // Ignore scores from users with rating < Antibot::Limit::threshold_reputation_score
            auto minUserReputation = GetThresholdReputationScore();
            auto minLikersCount = GetThresholdLikersCount();

            auto[userRepOk, userReputation] = PocketDb::RatingsRepoInst.GetUserReputation(addressId, height);
            if (!userRepOk || userReputation < minUserReputation)
                return false;

            auto[userLikersOk, userLikers] = PocketDb::RatingsRepoInst.GetUserLikersCount(addressId, height);
            if (!userLikersOk || userLikers < minLikersCount)
                return false;

            // All is OK
            return true;
        }

        virtual tuple<int, string> SelectAddressScorePost(shared_ptr <ScoreDataDto> scoreData, bool lottery)
        {
            if (lottery)
                return make_tuple(scoreData->ScoreAddressId, scoreData->ScoreAddressHash);

            return make_tuple(scoreData->ContentAddressId, scoreData->ContentAddressHash);
        }

        virtual bool AllowModifyReputationOverPost(shared_ptr <ScoreDataDto> scoreData, int height,
            const CTransactionRef& tx, bool lottery)
        {
            auto[checkScoreAddressId, checkScoreAddressHash] = SelectAddressScorePost(scoreData, lottery);

            // Check user reputation
            if (!AllowModifyReputation(checkScoreAddressId, height))
                return false;

            // Disable reputation increment if from one address to one address > 2 scores over day
            int64_t _max_scores_one_to_one = GetScoresOneToOne();
            int64_t _scores_one_to_one_depth = GetScoresOneToOneDepth();

            std::vector<int> values;
            if (lottery)
            {
                values.push_back(4);
                values.push_back(5);
            }
            else
            {
                values.push_back(1);
                values.push_back(2);
                values.push_back(3);
                values.push_back(4);
                values.push_back(5);
            }

            auto[scoreCountOk, scores_one_to_one_count] = PocketDb::RatingsRepoInst.GetScoreContentCount(
                PocketTxType::ACTION_SCORE_POST, checkScoreAddressHash, scoreData->ContentAddressHash,
                height, tx, values, _scores_one_to_one_depth);

            //LogPrintf("=== 2.1.2 AllowModifyReputationOverPost - tx:%s sAddr:%s pAddr:%s height:%d diff:%d < %d\n",
            //    tx->GetHash().GetHex(), scoreAddress, postAddress, height, scores_one_to_one_count, _max_scores_one_to_one);

            if (!scoreCountOk || scores_one_to_one_count >= _max_scores_one_to_one)
                return false;

            // All its Ok!
            return true;
        }

        virtual bool AllowModifyReputationOverComment(shared_ptr <ScoreDataDto> scoreData, int height,
            const CTransactionRef& tx, bool lottery)
        {
            // Check user reputation
            if (!AllowModifyReputation(scoreData->ScoreAddressId, height))
                return false;

            // Disable reputation increment if from one address to one address > Limit::scores_one_to_one scores over Limit::scores_one_to_one_depth
            int64_t _max_scores_one_to_one = GetScoresOneToOneOverComment();
            int64_t _scores_one_to_one_depth = GetScoresOneToOneDepth();

            std::vector<int> values;
            if (lottery)
            {
                values.push_back(1);
            }
            else
            {
                values.push_back(-1);
                values.push_back(1);
            }

            auto[scoreCountOk, scores_one_to_one_count] = PocketDb::RatingsRepoInst.GetScoreContentCount(
                PocketTx::ACTION_SCORE_COMMENT, scoreData->ScoreAddressHash, scoreData->ContentAddressHash,
                height, tx, values, _scores_one_to_one_depth);

            //LogPrintf("=== 2.1.2 AllowModifyReputationOverComment - tx:%s sAddr:%s pAddr:%s height:%d diff:%d < %d\n",
            //    tx->GetHash().GetHex(), scoreAddress, commentAddress, height, scores_one_to_one_count, _max_scores_one_to_one);

            if (!scoreCountOk || scores_one_to_one_count >= _max_scores_one_to_one)
                return false;

            // All its Ok!
            return true;
        }

    public:
        ReputationConsensus(int height) : BaseConsensus(height) {}

        virtual bool AllowModifyReputation(shared_ptr <ScoreDataDto> scoreData, const CTransactionRef& tx, int height,
            bool lottery)
        {
            if (scoreData->ScoreType == PocketTxType::ACTION_SCORE_POST)
                return AllowModifyReputationOverPost(scoreData, height, tx, lottery);

            if (scoreData->ScoreType == PocketTxType::ACTION_SCORE_COMMENT)
                return AllowModifyReputationOverComment(scoreData, height, tx, lottery);

            return false;
        }

        // TODO (brangr) v0.21.0: we need to limit the depth for all content types
        virtual bool AllowModifyOldPosts(int64_t scoreTime, int64_t contentTime, PocketTxType contentType)
        {
            if (contentType == PocketTxType::CONTENT_POST)
                return (scoreTime - contentTime) < GetScoresToPostModifyReputationDepth();

            return true;
        }
    };

    /*******************************************************************************************************************
    *
    *  Start checkpoint
    *
    *******************************************************************************************************************/
    class ReputationConsensus_checkpoint_0 : public ReputationConsensus
    {
    protected:
        int64_t GetThresholdLikersCount() override { return 0; }
        int64_t GetThresholdReputationScore() override { return -10000; }
        int64_t GetScoresOneToOneOverComment() override { return 20; }
        int64_t GetScoresOneToOne() override { return 99999; }
        int64_t GetScoresOneToOneDepth() override { return 336 * 24 * 3600; }
        int64_t GetScoresToPostModifyReputationDepth() override { return 336 * 24 * 3600; }
    public:
        ReputationConsensus_checkpoint_0(int height) : ReputationConsensus(height) {}
    };

    /*******************************************************************************************************************
    *
    *  Consensus checkpoint at 108300 block
    *
    *******************************************************************************************************************/
    class ReputationConsensus_checkpoint_108300 : public ReputationConsensus_checkpoint_0
    {
    protected:
        int64_t GetThresholdReputationScore() override { return 500; }
        int CheckpointHeight() override { return 108300; }
    public:
        ReputationConsensus_checkpoint_108300(int height) : ReputationConsensus_checkpoint_0(height) {}
    };

    /*******************************************************************************************************************
    *
    *  Consensus checkpoint at 151600 block
    *
    *******************************************************************************************************************/
    class ReputationConsensus_checkpoint_151600 : public ReputationConsensus_checkpoint_108300
    {
    protected:
        int CheckpointHeight() override { return 151600; }

        tuple<int, string> SelectAddressScorePost(shared_ptr <ScoreDataDto> scoreData, bool lottery) override
        {
            return make_tuple(scoreData->ScoreAddressId, scoreData->ScoreAddressHash);
        }

    public:
        ReputationConsensus_checkpoint_151600(int height) : ReputationConsensus_checkpoint_108300(height) {}
    };

    /*******************************************************************************************************************
    *
    *  Consensus checkpoint at 225000 block
    *
    *******************************************************************************************************************/
    class ReputationConsensus_checkpoint_225000 : public ReputationConsensus_checkpoint_151600
    {
    protected:
        int64_t GetScoresOneToOne() override { return 2; }
        int64_t GetScoresOneToOneDepth() override { return 1 * 24 * 3600; }
        int CheckpointHeight() override { return 225000; }
    public:
        ReputationConsensus_checkpoint_225000(int height) : ReputationConsensus_checkpoint_151600(height) {}
    };

    /*******************************************************************************************************************
    *
    *  Consensus checkpoint at 292800 block
    *
    *******************************************************************************************************************/
    class ReputationConsensus_checkpoint_292800 : public ReputationConsensus_checkpoint_225000
    {
    protected:
        int64_t GetThresholdReputationScore() override { return 1000; }
        int64_t GetScoresOneToOneDepth() override { return 7 * 24 * 3600; }
        int CheckpointHeight() override { return 292800; }
    public:
        ReputationConsensus_checkpoint_292800(int height) : ReputationConsensus_checkpoint_225000(height) {}
    };

    /*******************************************************************************************************************
    *
    *  Consensus checkpoint at 322700 block
    *
    *******************************************************************************************************************/
    class ReputationConsensus_checkpoint_322700 : public ReputationConsensus_checkpoint_292800
    {
    protected:
        int CheckpointHeight() override { return 322700; }
        int64_t GetScoresOneToOneDepth() override { return 2 * 24 * 3600; }
        int64_t GetScoresToPostModifyReputationDepth() override { return 30 * 24 * 3600; }
    public:
        ReputationConsensus_checkpoint_322700(int height) : ReputationConsensus_checkpoint_292800(height) {}
    };

    /*******************************************************************************************************************
    *
    *  Consensus checkpoint at 1124000 block
    *
    *******************************************************************************************************************/
    class ReputationConsensus_checkpoint_1124000 : public ReputationConsensus_checkpoint_322700
    {
    protected:
        int64_t GetThresholdLikersCount() override { return 100; }
        int CheckpointHeight() override { return 1124000; }
    public:
        ReputationConsensus_checkpoint_1124000(int height) : ReputationConsensus_checkpoint_322700(height) {}
    };

    /*******************************************************************************************************************
    *
    *  Factory for select actual rules version
    *  Каждая новая перегрузка добавляет новый функционал, поддерживающийся с некоторым условием - например высота
    *
    *******************************************************************************************************************/
    class ReputationConsensusFactory
    {
    private:
        inline static std::vector<std::pair<int, std::function<ReputationConsensus*(int height)>>> m_rules
            {
                {889524, [](int height) { return new ReputationConsensus_checkpoint_1124000(height); }},
                {322700, [](int height) { return new ReputationConsensus_checkpoint_322700(height); }},
                {292800, [](int height) { return new ReputationConsensus_checkpoint_292800(height); }},
                {225000, [](int height) { return new ReputationConsensus_checkpoint_225000(height); }},
                {151600, [](int height) { return new ReputationConsensus_checkpoint_151600(height); }},
                {108300, [](int height) { return new ReputationConsensus_checkpoint_108300(height); }},
                {0,      [](int height) { return new ReputationConsensus_checkpoint_0(height); }},
            };
    public:
        shared_ptr <ReputationConsensus> Instance(int height)
        {
            for (const auto& rule : m_rules)
                if (height >= rule.first)
                    return shared_ptr<ReputationConsensus>(rule.second(height));
        }
    };
}

#endif // POCKETCONSENSUS_REPUTATION_HPP