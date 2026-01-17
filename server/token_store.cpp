#include "token_store.h"

#include <Poco/Data/Session.h>
#include <Poco/Data/Statement.h>

using namespace Poco::Data;
using namespace Poco::Data::Keywords;

TokenStore::TokenStore(const std::string& db_path)
    : session_("SQLite", db_path)
{
    Poco::Data::SQLite::Connector::registerConnector();
}

void TokenStore::init()
{
    std::lock_guard<std::mutex> lock(mutex_);

    session_ << R"(
        CREATE TABLE IF NOT EXISTS tokens (
            token TEXT PRIMARY KEY,
            user_id TEXT NOT NULL
        )
    )",
    now;
}

bool TokenStore::save(const std::string& token,
                      const std::string& user_id)
{
    std::lock_guard<std::mutex> lock(mutex_);
    std::string tokenCopy = token; 
    std::string user_id_copy = user_id;
    try
    {
        session_ << R"(
            INSERT OR REPLACE INTO tokens (token, user_id)
            VALUES (?, ?)
        )",
        use(tokenCopy),
        use(user_id_copy),
        now;

        return true;
    }
    catch (...)
    {
        return false;
    }
}

bool TokenStore::exists(const std::string& token)
{
    std::lock_guard<std::mutex> lock(mutex_);

    int count = 0;
    std::string tokenCopy = token; 
    session_ << R"(
        SELECT COUNT(*) FROM tokens WHERE token = ?
    )",
    use(tokenCopy),
    into(count),
    now;

    return count > 0;
}

std::optional<TokenInfo> TokenStore::get(const std::string& token)
{
    std::lock_guard<std::mutex> lock(mutex_);

    TokenInfo info;
    std::string tokenCopy = token; 
    session_ << R"(
        SELECT token, user_id
        FROM tokens
        WHERE token = ?
        LIMIT 1
    )",
    use(tokenCopy),
    into(info.token),
    into(info.user_id),
    now;

    if (info.token.empty())
        return std::nullopt;

    return info;
}

bool TokenStore::remove(const std::string& token)
{
    std::lock_guard<std::mutex> lock(mutex_);
    std::string tokenCopy = token; 
    try
    {
        session_ << R"(
            DELETE FROM tokens WHERE token = ?
        )",
        use(tokenCopy),
        now;

        return true;
    }
    catch (...)
    {
        return false;
    }
}
