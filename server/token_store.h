#ifndef TOKEN_STORE_H
#define TOKEN_STORE_H

#include <string>
#include <optional>
#include <mutex>

#include <Poco/Data/Session.h>
#include <Poco/Data/SQLite/Connector.h>

struct TokenInfo
{
    std::string token;
    std::string user_id;
};

class TokenStore
{
public:
    explicit TokenStore(const std::string& db_path);

    void init();

    // 保存 token（已存在则覆盖 user_id）
    bool save(const std::string& token,
              const std::string& user_id);

    // 是否存在 token
    bool exists(const std::string& token);

    // 读取 token 信息
    std::optional<TokenInfo> get(const std::string& token);

    // 删除 token
    bool remove(const std::string& token);

private:
    Poco::Data::Session session_;
    std::mutex mutex_;
};
#endif