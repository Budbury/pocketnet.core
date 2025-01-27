// Copyright (c) 2018-2022 The Pocketnet developers
// Distributed under the Apache 2.0 software license, see the accompanying
// https://www.apache.org/licenses/LICENSE-2.0

#include <pocketdb/consensus/Base.h>
#include "pocketdb/web/PocketContentRpc.h"
#include "rpc/util.h"
#include "validation.h"

namespace PocketWeb::PocketWebRpc
{

    void ParseFeedRequest(const JSONRPCRequest& request, int& topHeight, string& topContentHash, int& countOut, string& lang, vector<string>& tags,
        vector<int>& contentTypes, vector<string>& txIdsExcluded, vector<string>& adrsExcluded, vector<string>& tagsExcluded, string& address)
    {
        topHeight = ::ChainActive().Height();
        if (request.params.size() > 0 && request.params[0].isNum() && request.params[0].get_int() > 0)
            topHeight = request.params[0].get_int();

        if (request.params.size() > 1 && request.params[1].isStr())
            topContentHash = request.params[1].get_str();

        countOut = 10;
        if (request.params.size() > 2 && request.params[2].isNum())
        {
            countOut = request.params[2].get_int();
            if (countOut > 10)
                countOut = 10;
        }

        lang = "en";
        if (request.params.size() > 3 && request.params[3].isStr())
            lang = request.params[3].get_str();

        // tags
        if (request.params.size() > 4)
            ParseRequestTags(request.params[4], tags);

        // content types
        ParseRequestContentTypes(request.params[5], contentTypes);

        // exclude ids
        if (request.params.size() > 6)
        {
            if (request.params[6].isStr())
            {
                txIdsExcluded.push_back(request.params[6].get_str());
            }
            else if (request.params[6].isArray())
            {
                UniValue txids = request.params[6].get_array();
                for (unsigned int idx = 0; idx < txids.size(); idx++)
                {
                    string txidEx = boost::trim_copy(txids[idx].get_str());
                    if (!txidEx.empty())
                        txIdsExcluded.push_back(txidEx);

                    if (txIdsExcluded.size() > 100)
                        break;
                }
            }
        }

        // exclude addresses
        if (request.params.size() > 7)
        {
            if (request.params[7].isStr())
            {
                adrsExcluded.push_back(request.params[7].get_str());
            }
            else if (request.params[7].isArray())
            {
                UniValue adrs = request.params[7].get_array();
                for (unsigned int idx = 0; idx < adrs.size(); idx++)
                {
                    string adrEx = boost::trim_copy(adrs[idx].get_str());
                    if (!adrEx.empty())
                        adrsExcluded.push_back(adrEx);

                    if (adrsExcluded.size() > 100)
                        break;
                }
            }
        }

        // exclude tags
        if (request.params.size() > 8)
        {
            if (request.params[8].isStr())
            {
                tagsExcluded.push_back(request.params[8].get_str());
            }
            else if (request.params[8].isArray())
            {
                UniValue tagsEx = request.params[8].get_array();
                for (unsigned int idx = 0; idx < tagsEx.size(); idx++)
                {
                    string tgsEx = boost::trim_copy(tagsEx[idx].get_str());
                    if (!tgsEx.empty())
                        tagsExcluded.push_back(tgsEx);

                    if (tagsExcluded.size() > 100)
                        break;
                }
            }
        }

        // address for person output
        if (request.params.size() > 9)
        {
            RPCTypeCheckArgument(request.params[9], UniValue::VSTR);
            address = request.params[9].get_str();
            CTxDestination dest = DecodeDestination(address);

            if (!IsValidDestination(dest))
                throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, string("Invalid Pocketcoin address: ") + address);
        }
    }

    static UniValue _getsubscribersfeed(const JSONRPCRequest& request)
    {
        string addressFrom;
        if (request.params.size() > 0 && request.params[0].isStr())
            addressFrom = request.params[0].get_str();

        string topContentHash;
        if (request.params.size() > 2 && request.params[2].isStr())
            topContentHash = request.params[2].get_str();

        int count = 10;
        if (request.params.size() > 3 && request.params[3].isNum())
        {
            count = request.params[3].get_int();
            if (count > 10)
                count = 10;
        }

        string lang;
        if (request.params.size() > 4 && request.params[4].isStr())
            lang = request.params[4].get_str();

        vector<string> tags;
        ParseRequestTags(request.params[5], tags);

        // content types
        vector<int> contentTypes;
        ParseRequestContentTypes(request.params[6], contentTypes);

        int64_t topContentId = 0;
        if (!topContentHash.empty())
        {
            auto ids = request.DbConnection()->WebRpcRepoInst->GetContentIds({topContentHash});
            if (!ids.empty())
                topContentId = ids[0];
        }

        return request.DbConnection()->WebRpcRepoInst->GetSubscribesFeed(addressFrom, topContentId, count,
            lang, tags, contentTypes);
    }

    static UniValue _getprofilefeed(const JSONRPCRequest& request)
    {
        string addressFrom;
        if (request.params.size() > 0 && request.params[0].isStr())
            addressFrom = request.params[0].get_str();
            
        string addressTo;
        if (request.params.size() > 1 && request.params[1].isStr())
            addressTo = request.params[1].get_str();

        string topContentHash;
        if (request.params.size() > 2 && request.params[2].isStr())
            topContentHash = request.params[2].get_str();

        int count = 10;
        if (request.params.size() > 3 && request.params[3].isNum())
        {
            count = request.params[3].get_int();
            if (count > 10)
                count = 10;
        }

        string lang = "";
        if (request.params.size() > 4 && request.params[4].isStr())
            lang = request.params[4].get_str();

        vector<string> tags;
        if (request.params.size() > 5)
            ParseRequestTags(request.params[5], tags);

        // content types
        vector<int> contentTypes;
        ParseRequestContentTypes(request.params[6], contentTypes);

        int64_t topContentId = 0;
        if (!topContentHash.empty())
        {
            auto ids = request.DbConnection()->WebRpcRepoInst->GetContentIds({ topContentHash });
            if (!ids.empty())
                topContentId = ids[0];
        }

        return request.DbConnection()->WebRpcRepoInst->GetProfileFeed(addressFrom, addressTo, topContentId, count, lang, tags, contentTypes);
    }

    RPCHelpMan GetContent()
    {
        return RPCHelpMan{"getcontent",
                "\nReturns contents for list of ids\n",
                {
                    {"ids", RPCArg::Type::ARR, RPCArg::Optional::NO, "",
                        {
                            {"id", RPCArg::Type::STR, RPCArg::Optional::NO, ""}   
                        }
                    }
                },
                {
                    // TODO (losty-rpc): provide return description
                },
                RPCExamples{
                    HelpExampleCli("getcontent", "ids[]") +
                    HelpExampleRpc("getcontent", "ids[]")
                },
        [&](const RPCHelpMan& self, const JSONRPCRequest& request) -> UniValue
    {
        vector<string> hashes;
        if (request.params.size() > 0)
        {
            if (request.params[0].isStr())
            {
                hashes.push_back(request.params[0].get_str());
            }
            else if (request.params[0].isArray())
            {
                UniValue txids = request.params[0].get_array();
                for (unsigned int idx = 0; idx < txids.size(); idx++)
                {
                    string txidEx = boost::trim_copy(txids[idx].get_str());
                    if (!txidEx.empty())
                        hashes.push_back(txidEx);

                    if (hashes.size() > 100)
                        break;
                }
            }
        }

        string address;
        if (request.params.size() > 1 && request.params[1].isStr())
            address = request.params[1].get_str();

        auto ids = request.DbConnection()->WebRpcRepoInst->GetContentIds(hashes);
        auto content = request.DbConnection()->WebRpcRepoInst->GetContentsData(ids, address);

        UniValue result(UniValue::VARR);
        result.push_backV(content);
        return result;
    },
        };
    }

    RPCHelpMan GetContents()
    {
        return RPCHelpMan{"getcontents",
                "\nReturns contents for address.\n",
                {
                    {"address", RPCArg::Type::STR, RPCArg::Optional::NO, "A pocketcoin addresses to filter"}
                },
                {
                    // TODO (losty-rpc): provide return description
                    // {RPCResult::Type::ARR, "", "", {}}
                },
                RPCExamples{
                    HelpExampleCli("getcontents", "a123bda213") +
                    HelpExampleRpc("getcontents", "a123bda213")
                },
        [&](const RPCHelpMan& self, const JSONRPCRequest& request) -> UniValue
    {
        string address;
        if (request.params[0].isStr())
            address = request.params[0].get_str();

        // TODO (brangr, team): add pagination

        return request.DbConnection()->WebRpcRepoInst->GetContentsForAddress(address);
    },
        };
    }

    RPCHelpMan GetProfileFeed()
    {
        return RPCHelpMan{"getprofilefeed",
                "\nReturns contents for list of ids\n", // TODO (team): description, args and examples really need to be fixed
                {
                    {"count", RPCArg::Type::NUM, RPCArg::Optional::OMITTED_NAMED_ARG, ""}
                },
                {
                    // TODO (losty-rpc): provide return description
                },
                RPCExamples{
                    HelpExampleCli("getprofilefeed", "") +
                    HelpExampleRpc("getprofilefeed", "")
                },
        [&](const RPCHelpMan& self, const JSONRPCRequest& request) -> UniValue
    {
        return _getprofilefeed(request);
    },
        };
    }

    RPCHelpMan GetHotPosts()
    {
        return RPCHelpMan{"GetHotPosts",
                // TODO (team): provide description, args and examples
                "",
                {},
                {
                    // TODO (losty-rpc): provide return description
                },
                RPCExamples{
                    ""
                },
        [&](const RPCHelpMan& self, const JSONRPCRequest& request) -> UniValue
    {
        int count = 30;
        if (request.params.size() > 0)
        {
            if (request.params[0].isNum())
                count = request.params[0].get_int();
            else if (request.params[0].isStr())
                // TODO (losty-fur): do not ignore result
                bool res = ParseInt32(request.params[0].get_str(), &count);
        }

        // Depth in blocks (default about 3 days)
        int dayInBlocks = 24 * 60;
        int depthBlocks = 3 * dayInBlocks;
        if (request.params.size() > 1)
        {
            if (request.params[1].isNum())
                depthBlocks = request.params[1].get_int();
            else if (request.params[1].isStr())
                // TODO (losty-fur): do not ignore result
                bool res = ParseInt32(request.params[1].get_str(), &depthBlocks);

            // for old version electron
            if (depthBlocks == 259200)
                depthBlocks = 3 * dayInBlocks;

            // Max 3 months
            depthBlocks = min(depthBlocks, 90 * dayInBlocks);
        }

        int nHeightOffset = ::ChainActive().Height();
        int nOffset = 0;
        if (request.params.size() > 2)
        {
            if (request.params[2].isNum())
            {
                if (request.params[2].get_int() > 0)
                    nOffset = request.params[2].get_int();
            }
            else if (request.params[2].isStr())
            {
                // TODO (losty-fur): do not ignore result
                bool res = ParseInt32(request.params[2].get_str(), &nOffset);
            }
            nHeightOffset -= nOffset;
        }

        string lang = "";
        if (request.params.size() > 3)
            lang = request.params[3].get_str();

        vector<int> contentTypes;
        ParseRequestContentTypes(request.params[4], contentTypes);

        string address = "";
        if (request.params.size() > 5)
            address = request.params[5].get_str();

        auto reputationConsensus = ReputationConsensusFactoryInst.Instance(ChainActive().Height());
        auto badReputationLimit = reputationConsensus->GetConsensusLimit(ConsensusLimit_bad_reputation);

        return request.DbConnection()->WebRpcRepoInst->GetHotPosts(count, depthBlocks, nHeightOffset, lang,
            contentTypes, address, badReputationLimit);
    },
        };
    }

    RPCHelpMan GetHistoricalFeed()
    {
        return RPCHelpMan{"GetHistoricalFeed",
                // TODO (team): provide description
                "",
                {
                    {"topHeight", RPCArg::Type::NUM, RPCArg::Optional::NO, ""},
                    {"topContentHash", RPCArg::Type::STR, RPCArg::Optional::OMITTED_NAMED_ARG, ""},
                    {"countOut", RPCArg::Type::NUM, RPCArg::Optional::OMITTED_NAMED_ARG, ""},
                    {"lang", RPCArg::Type::STR, RPCArg::Optional::OMITTED_NAMED_ARG, ""},
                    {"tags", RPCArg::Type::ARR, RPCArg::Optional::OMITTED_NAMED_ARG, "",
                        {
                            {"tag", RPCArg::Type::STR, RPCArg::Optional::NO, ""}   
                        }
                    },
                    {"contentTypes", RPCArg::Type::ARR, RPCArg::Optional::OMITTED_NAMED_ARG, "",
                        {
                            {"contentType", RPCArg::Type::STR, RPCArg::Optional::NO, ""}   
                        }
                    },
                    {"txIdsExcluded", RPCArg::Type::ARR, RPCArg::Optional::OMITTED_NAMED_ARG, "",
                        {
                            {"txIdExcluded", RPCArg::Type::STR, RPCArg::Optional::NO, ""}   
                        }
                    },
                    {"adrsExcluded", RPCArg::Type::ARR, RPCArg::Optional::OMITTED_NAMED_ARG, "",
                        {
                            {"adrExcluded", RPCArg::Type::STR, RPCArg::Optional::NO, ""}   
                        }
                    },
                    {"tagsExcluded", RPCArg::Type::ARR, RPCArg::Optional::OMITTED_NAMED_ARG, "",
                        {
                            {"tagExcluded", RPCArg::Type::STR, RPCArg::Optional::NO, ""}   
                        }
                    },
                    {"address", RPCArg::Type::STR, RPCArg::Optional::OMITTED_NAMED_ARG, ""},
                },
                {
                    // TODO (losty-rpc): provide return description
                },
                RPCExamples{
                    // TODO (losty-rpc): more examples
                    HelpExampleCli("GetHistoricalFeed", "123123123123") +
                    HelpExampleRpc("GetHistoricalFeed", "123123123123")
                },
        [&](const RPCHelpMan& self, const JSONRPCRequest& request) -> UniValue
    {
        int topHeight;
        string topContentHash;
        int countOut;
        string lang;
        vector<string> tags;
        vector<int> contentTypes;
        vector<string> txIdsExcluded;
        vector<string> adrsExcluded;
        vector<string> tagsExcluded;
        string address;
        ParseFeedRequest(request, topHeight, topContentHash, countOut, lang, tags, contentTypes, txIdsExcluded,
            adrsExcluded, tagsExcluded, address);

        int64_t topContentId = 0;
        if (!topContentHash.empty())
        {
            auto ids = request.DbConnection()->WebRpcRepoInst->GetContentIds({topContentHash});
            if (!ids.empty())
                topContentId = ids[0];
        }

        auto reputationConsensus = ReputationConsensusFactoryInst.Instance(ChainActive().Height());
        auto badReputationLimit = reputationConsensus->GetConsensusLimit(ConsensusLimit_bad_reputation);

        UniValue result(UniValue::VOBJ);
        UniValue content = request.DbConnection()->WebRpcRepoInst->GetHistoricalFeed(
            countOut, topContentId, topHeight, lang, tags, contentTypes,
            txIdsExcluded, adrsExcluded, tagsExcluded,
            address, badReputationLimit);

        result.pushKV("height", topHeight);
        result.pushKV("contents", content);
        return result;
    },
        };
    }

    RPCHelpMan GetHierarchicalFeed()
    {
        return RPCHelpMan{"GetHierarchicalFeed",
                // TODO (team): provide description
                "",
                {
                    {"topHeight", RPCArg::Type::NUM, RPCArg::Optional::NO, ""},
                    {"topContentHash", RPCArg::Type::STR, RPCArg::Optional::OMITTED_NAMED_ARG, ""},
                    {"countOut", RPCArg::Type::NUM, RPCArg::Optional::OMITTED_NAMED_ARG, ""},
                    {"lang", RPCArg::Type::STR, RPCArg::Optional::OMITTED_NAMED_ARG, ""},
                    {"tags", RPCArg::Type::ARR, RPCArg::Optional::OMITTED_NAMED_ARG, "",
                        {
                            {"tag", RPCArg::Type::STR, RPCArg::Optional::NO, ""}   
                        }
                    },
                    {"contentTypes", RPCArg::Type::ARR, RPCArg::Optional::OMITTED_NAMED_ARG, "",
                        {
                            {"contentType", RPCArg::Type::STR, RPCArg::Optional::NO, ""}   
                        }
                    },
                    {"txIdsExcluded", RPCArg::Type::ARR, RPCArg::Optional::OMITTED_NAMED_ARG, "",
                        {
                            {"txIdExcluded", RPCArg::Type::STR, RPCArg::Optional::NO, ""}   
                        }
                    },
                    {"adrsExcluded", RPCArg::Type::ARR, RPCArg::Optional::OMITTED_NAMED_ARG, "",
                        {
                            {"adrExcluded", RPCArg::Type::STR, RPCArg::Optional::NO, ""}   
                        }
                    },
                    {"tagsExcluded", RPCArg::Type::ARR, RPCArg::Optional::OMITTED_NAMED_ARG, "",
                        {
                            {"tagExcluded", RPCArg::Type::STR, RPCArg::Optional::NO, ""}   
                        }
                    },
                    {"address", RPCArg::Type::STR, RPCArg::Optional::OMITTED_NAMED_ARG, ""},
                },
                {
                    // TODO (losty-rpc): provide return description
                },
                RPCExamples{
                    // TODO (losty-rpc): more examples
                    HelpExampleCli("GetHierarchicalFeed", "1231231414") +
                    HelpExampleRpc("GetHierarchicalFeed", "1231231414")
                },
        [&](const RPCHelpMan& self, const JSONRPCRequest& request) -> UniValue
    {
        int topHeight;
        string topContentHash;
        int countOut;
        string lang;
        vector<string> tags;
        vector<int> contentTypes;
        vector<string> txIdsExcluded;
        vector<string> adrsExcluded;
        vector<string> tagsExcluded;
        string address;
        ParseFeedRequest(request, topHeight, topContentHash, countOut, lang, tags, contentTypes, txIdsExcluded,
            adrsExcluded, tagsExcluded, address);

        int64_t topContentId = 0;
        if (!topContentHash.empty())
        {
            auto ids = request.DbConnection()->WebRpcRepoInst->GetContentIds({topContentHash});
            if (!ids.empty())
                topContentId = ids[0];
        }

        auto reputationConsensus = ReputationConsensusFactoryInst.Instance(ChainActive().Height());
        auto badReputationLimit = reputationConsensus->GetConsensusLimit(ConsensusLimit_bad_reputation);

        UniValue result(UniValue::VOBJ);
        UniValue content = request.DbConnection()->WebRpcRepoInst->GetHierarchicalFeed(
            countOut, topContentId, topHeight, lang, tags, contentTypes,
            txIdsExcluded, adrsExcluded, tagsExcluded,
            address, badReputationLimit);

        result.pushKV("height", topHeight);
        result.pushKV("contents", content);
        return result;
    },
        };
    }

    RPCHelpMan GetSubscribesFeed()
    {
        return RPCHelpMan{"getsubscribesfeed",
                "\nReturns contents from subscribers\n",
                {
                    // TODO (team): provide arguments description
                },
                {
                    // TODO (losty-rpc): provide return description
                },
                RPCExamples{
                    HelpExampleCli("getsubscribesfeed", "") +
                    HelpExampleRpc("getsubscribesfeed", "")
                },
        [&](const RPCHelpMan& self, const JSONRPCRequest& request) -> UniValue
    {
        return _getsubscribersfeed(request);
    },
        };
    }

    RPCHelpMan FeedSelector()
    {
        return RPCHelpMan{"getrawtransactionwithmessage",
                // TODO (team): provide description
                "",
                {
                    // Args
                },
                {
                    // Returns
                },
                RPCExamples{
                    // Examples (HelpExampleCli() and HelpExampleRpc())
                    ""
                },
        [&](const RPCHelpMan& self, const JSONRPCRequest& request) -> UniValue
    {
        if (request.params.size() > 1 && request.params[1].isStr() && request.params[1].get_str() == "1")
            return _getsubscribersfeed(request);

        return _getprofilefeed(request);
    },
        };
    }

    RPCHelpMan GetContentsStatistic()
    {
        return RPCHelpMan{"getcontentsstatistic",
                "\nGet contents statistic.\n",
                {
                    {"address", RPCArg::Type::STR, RPCArg::Optional::NO, "Address - сontent author"},
                    {"contenttypes", RPCArg::Type::ARR, RPCArg::Optional::OMITTED_NAMED_ARG, "type(s) of content posts/video",
                        {
                            {"contenttype", RPCArg::Type::STR, RPCArg::Optional::NO, ""}   
                        }
                    },
                    {"height", RPCArg::Type::NUM, RPCArg::Optional::OMITTED_NAMED_ARG, "Maximum content height. Default is current chain height"},
                    {"depth", RPCArg::Type::NUM, RPCArg::Optional::OMITTED_NAMED_ARG, "Depth of content history for statistics. Default is all history"},
                },
                {
                    // TODO (losty-rpc): provide return description
                },
                RPCExamples{
                    HelpExampleCli("getcontentsstatistic", "") +
                    HelpExampleRpc("getcontentsstatistic", "")
                },
        [&](const RPCHelpMan& self, const JSONRPCRequest& request) -> UniValue
    {
        string address;
        vector<string> addresses;
        if (request.params.size() > 0) {
            if (request.params[0].isStr()) {
                address = request.params[0].get_str();
                CTxDestination dest = DecodeDestination(address);
                if (!IsValidDestination(dest)) {
                    throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, string("Invalid Pocketnet address: ") + address);
                }
                addresses.emplace_back(address);
            } else if (request.params[0].isArray()) {
                UniValue addrs = request.params[0].get_array();
                if (addrs.size() > 10) {
                    throw JSONRPCError(RPC_INVALID_PARAMS, "Too large array");
                }
                if(addrs.size() > 0) {
                    for (unsigned int idx = 0; idx < addrs.size(); idx++) {
                        address = addrs[idx].get_str();
                        CTxDestination dest = DecodeDestination(address);
                        if (!IsValidDestination(dest)) {
                            throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, string("Invalid Pocketnet address: ") + address);
                        }
                        addresses.emplace_back(address);
                    }
                }
            }
        }

        vector<int> contentTypes;
        ParseRequestContentTypes(request.params[1], contentTypes);

        return request.DbConnection()->WebRpcRepoInst->GetContentsStatistic(addresses, contentTypes);
    },
        };
    }

    RPCHelpMan GetRandomContents()
    {
        return RPCHelpMan{"GetRandomPost",
                "\nGet contents statistic.\n",
                {
                    // TODO (losty-rpc): provide args description
                },
                {
                    // TODO (losty-rpc): provide return description
                },
                RPCExamples{
                    // TODO (losty-rpc)
                    HelpExampleCli("GetRandomPost", "") +
                    HelpExampleRpc("GetRandomPost", "")
                },
        [&](const RPCHelpMan& self, const JSONRPCRequest& request) -> UniValue
    {   
        // if (request.fHelp)
        // {
        //     UniValue help(UniValue::VOBJ);
        //     help.pushKV("Method", "GetRandomPost");

        //     UniValue args(UniValue::VARR);

        //     UniValue argLang(UniValue::VOBJ);
        //     argLang.pushKV("Name", "lang");
        //     argLang.pushKV("Type", "String");
        //     argLang.pushKV("Default", "en");
        //     args.push_back(argLang);

        //     help.pushKV("Arguments", args);

        //     UniValue examples(UniValue::VARR);
        //     help.pushKV("Examples", examples);
        // }

        string lang = "en";
        if (request.params[0].isStr())
            lang = request.params[0].get_str();

        const int count = 1;
        const int height = ::ChainActive().Height() - 150000;

        auto ids = request.DbConnection()->WebRpcRepoInst->GetRandomContentIds(lang, count, height);
        auto content = request.DbConnection()->WebRpcRepoInst->GetContentsData(ids, "");

        UniValue result(UniValue::VARR);
        result.push_backV(content);

        return result;
    },
        };
    }
}