// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2018 Bitcoin developers
// Copyright (c) 2018-2021 Pocketnet developers
// Distributed under the Apache 2.0 software license, see the accompanying
// https://www.apache.org/licenses/LICENSE-2.0

#include "PocketCommentsRpc.h"

#include <pos.h>
#include <validation.h>
#include <logging.h>

UniValue PocketWeb::PocketCommentsRpc::GetComments(const JSONRPCRequest& request)
{
    std::vector<std::string> cmnIds;
    if (request.params.size() > 3)
    {
        if (request.params[3].isArray())
        {
            UniValue cmntid = request.params[3].get_array();
            for (unsigned int id = 0; id < cmntid.size(); id++)
            {
                cmnIds.push_back(cmntid[id].get_str());
            }
        }
        else
        {
            throw JSONRPCError(RPC_INVALID_PARAMS, "Invalid inputs params");
        }
    }

    if (!cmnIds.empty())
    {
        return GetCommentsByIds(request);
    }
    else
    {
        return GetCommentsByPost(request);
    }
}

UniValue PocketWeb::PocketCommentsRpc::GetCommentsByIds(const JSONRPCRequest& request)
{
    if (request.fHelp)
        throw std::runtime_error(
            "getcomments (\"postid\", \"parentid\", \"address\", [\"commend_id\",\"commend_id\",...])\n"
            "\nGet Pocketnet comments.\n");

    std::string addressHash;
    if (request.params.size() > 2) {
        addressHash = request.params[2].get_str();
    }

    std::vector<std::string> cmnIds;
    if (request.params.size() > 3)
    {
        if (request.params[3].isArray())
        {
            UniValue cmntid = request.params[3].get_array();
            for (unsigned int id = 0; id < cmntid.size(); id++)
            {
                cmnIds.push_back(cmntid[id].get_str());
            }
        }
        else
        {
            throw JSONRPCError(RPC_INVALID_PARAMS, "Invalid inputs params");
        }
    }

    auto aResult = PocketDb::WebRepoInst.GetCommentsByIds(addressHash, cmnIds);

    return aResult;
}

UniValue PocketWeb::PocketCommentsRpc::GetCommentsByPost(const JSONRPCRequest& request)
{
    if (request.fHelp)
        throw std::runtime_error(
            "getcomments (\"postid\", \"parentid\", \"address\", [\"commend_id\",\"commend_id\",...])\n"
            "\nGet Pocketnet comments.\n");

    std::string postHash;
    if (!request.params.empty()) {
        postHash = request.params[0].get_str();
        if (postHash.length() == 0 && request.params.size() < 3)
            throw JSONRPCError(RPC_INVALID_PARAMS, "Invalid postid");
    }

    std::string parentHash;
    if (request.params.size() > 1) {
        parentHash = request.params[1].get_str();
    }

    std::string addressHash;
    if (request.params.size() > 2) {
        addressHash = request.params[2].get_str();
    }

    auto aResult = PocketDb::WebRepoInst.GetCommentsByPost(postHash, parentHash, addressHash);

    return aResult;
}

UniValue PocketWeb::PocketCommentsRpc::GetLastComments(const JSONRPCRequest& request)
{
    if (request.fHelp)
        throw std::runtime_error(
            "getlastcomments (count)\n"
            "\nGet Pocketnet last comments.\n");

    int resultCount = 10;
    if (request.params.size() > 0) {
        if (request.params[0].isNum()) {
            resultCount = request.params[0].get_int();
        }
    }

    std::string address = "";
    if (request.params.size() > 1) {
        address = request.params[1].get_str();
    }

    std::string lang = "";
    if (request.params.size() > 2) {
        lang = request.params[2].get_str();
    }

    int nHeight = chainActive.Height();

    auto aResult = PocketDb::WebRepoInst.GetLastComments(resultCount, nHeight, lang);

    return aResult;
}