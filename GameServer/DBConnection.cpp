#include "pch.h"
#include "DBConnection.h"

// ═════════════════════════════════════════════════════════════════════════════
// DBStmt
// ═════════════════════════════════════════════════════════════════════════════

bool DBStmt::Prepare(MYSQL* mysql, const char* sql)
{
    Close();
    _stmt = mysql_stmt_init(mysql);
    if (!_stmt) { LOG_ERROR("mysql_stmt_init failed"); return false; }

    if (mysql_stmt_prepare(_stmt, sql, static_cast<unsigned long>(strlen(sql))) != 0)
    {
        LOG_ERROR("mysql_stmt_prepare failed: " + std::string(mysql_stmt_error(_stmt))
            + "\nSQL: " + sql);
        Close();
        return false;
    }
    return true;
}

void DBStmt::Close()
{
    if (_stmt) { mysql_stmt_close(_stmt); _stmt = nullptr; }
}

// ── 입력 바인딩 ──────────────────────────────────────────────────────────────

void DBStmt::BindInUInt64(int idx, uint64_t val)
{
    _inU64[idx]             = val;
    _inBinds[idx]           = {};
    _inBinds[idx].buffer_type  = MYSQL_TYPE_LONGLONG;
    _inBinds[idx].buffer       = &_inU64[idx];
    _inBinds[idx].is_unsigned  = 1;
    _inCount = std::max(_inCount, idx + 1);
}

void DBStmt::BindInInt32(int idx, int32_t val)
{
    _inI32[idx]             = val;
    _inBinds[idx]           = {};
    _inBinds[idx].buffer_type = MYSQL_TYPE_LONG;
    _inBinds[idx].buffer      = &_inI32[idx];
    _inCount = std::max(_inCount, idx + 1);
}

void DBStmt::BindInString(int idx, const std::string& val)
{
    _inStr   [idx]            = val;
    _inStrLen[idx]            = static_cast<unsigned long>(val.size());
    _inBinds [idx]            = {};
    _inBinds[idx].buffer_type   = MYSQL_TYPE_STRING;
    _inBinds[idx].buffer        = const_cast<char*>(_inStr[idx].c_str());
    _inBinds[idx].buffer_length = static_cast<unsigned long>(val.size());
    _inBinds[idx].length        = &_inStrLen[idx];
    _inCount = std::max(_inCount, idx + 1);
}

// ── 결과 바인딩 ──────────────────────────────────────────────────────────────

void DBStmt::BindOutUInt64(int idx, uint64_t& out)
{
    _outBinds[idx]              = {};
    _outBinds[idx].buffer_type  = MYSQL_TYPE_LONGLONG;
    _outBinds[idx].buffer       = &out;
    _outBinds[idx].is_unsigned  = 1;
    _outBinds[idx].is_null      = &_outIsNull[idx];
    _outCount = std::max(_outCount, idx + 1);
}

void DBStmt::BindOutInt64(int idx, int64_t& out)
{
    _outBinds[idx]              = {};
    _outBinds[idx].buffer_type  = MYSQL_TYPE_LONGLONG;
    _outBinds[idx].buffer       = &out;
    _outBinds[idx].is_null      = &_outIsNull[idx];
    _outCount = std::max(_outCount, idx + 1);
}

void DBStmt::BindOutInt32(int idx, int32_t& out)
{
    _outBinds[idx]              = {};
    _outBinds[idx].buffer_type  = MYSQL_TYPE_LONG;
    _outBinds[idx].buffer       = &out;
    _outBinds[idx].is_null      = &_outIsNull[idx];
    _outCount = std::max(_outCount, idx + 1);
}

void DBStmt::BindOutString(int idx, char* buf, unsigned long cap)
{
    _outBinds[idx]              = {};
    _outBinds[idx].buffer_type   = MYSQL_TYPE_STRING;
    _outBinds[idx].buffer        = buf;
    _outBinds[idx].buffer_length = cap;
    _outBinds[idx].is_null       = &_outIsNull[idx];
    _outCount = std::max(_outCount, idx + 1);
}

void DBStmt::BindOutBool(int idx, bool& out)
{
    _outBinds[idx]              = {};
    _outBinds[idx].buffer_type  = MYSQL_TYPE_TINY;
    _outBinds[idx].buffer       = &out;
    _outBinds[idx].is_null      = &_outIsNull[idx];
    _outCount = std::max(_outCount, idx + 1);
}

// ── 실행 ─────────────────────────────────────────────────────────────────────

bool DBStmt::Execute()
{
    if (!_stmt) return false;

    // 이전 실행의 결과가 남아있으면 정리 (Commands out of sync 방지)
    mysql_stmt_free_result(_stmt);

    if (_inCount  > 0) mysql_stmt_bind_param (_stmt, _inBinds);
    if (_outCount > 0) mysql_stmt_bind_result(_stmt, _outBinds);

    if (mysql_stmt_execute(_stmt) != 0)
    {
        LOG_ERROR("mysql_stmt_execute failed: " + std::string(mysql_stmt_error(_stmt)));
        return false;
    }

    // SELECT 결과가 있으면 클라이언트 버퍼에 저장
    // → 버퍼링 완료 후에는 같은 커넥션에서 다른 쿼리 즉시 실행 가능
    if (mysql_stmt_field_count(_stmt) > 0)
        mysql_stmt_store_result(_stmt);

    return true;
}

bool DBStmt::Fetch()
{
    if (!_stmt) return false;
    int ret = mysql_stmt_fetch(_stmt);
    return ret == 0;          // MYSQL_NO_DATA(100) 이면 false
}

uint64 DBStmt::AffectedRows() const
{
    return _stmt ? static_cast<uint64>(mysql_stmt_affected_rows(_stmt)) : 0;
}

uint64 DBStmt::LastInsertId() const
{
    return _stmt ? static_cast<uint64>(mysql_stmt_insert_id(_stmt)) : 0;
}

bool DBConnection::Connect(const char* host, const char* user,
    const char* password, const char* database,
    uint32 port)
{
    _mysql = mysql_init(nullptr);
    if (!_mysql)
    {
        LOG_ERROR("mysql_init failed");
        return false;
    }

    if (!mysql_real_connect(_mysql, host, user, password, database, port, nullptr, 0))
    {
        LOG_ERROR("MySQL connect failed: " + std::string(mysql_error(_mysql)));
        mysql_close(_mysql);
        _mysql = nullptr;
        return false;
    }

    // �ѱ� ����
    mysql_set_character_set(_mysql, "utf8mb4");
    LOG_INFO("MySQL connected: " + std::string(host) + "/" + std::string(database));
    return true;
}

void DBConnection::Disconnect()
{
    if (_mysql)
    {
        mysql_close(_mysql);
        _mysql = nullptr;
    }
}

bool DBConnection::Execute(const std::string& query)
{
    if (!_mysql) return false;
    if (mysql_query(_mysql, query.c_str()) != 0)
    {
        LOG_ERROR("MySQL Execute failed: " + std::string(mysql_error(_mysql))
            + "\nQuery: " + query);
        return false;
    }
    return true;
}

DBResult DBConnection::Query(const std::string& query)
{
    if (!_mysql) return DBResult(nullptr);
    if (mysql_query(_mysql, query.c_str()) != 0)
    {
        LOG_ERROR("MySQL Query failed: " + std::string(mysql_error(_mysql))
            + "\nQuery: " + query);
        return DBResult(nullptr);
    }
    return DBResult(mysql_store_result(_mysql));
}

uint64 DBConnection::GetLastInsertId() const
{
    return _mysql ? static_cast<uint64>(mysql_insert_id(_mysql)) : 0;
}

uint64 DBConnection::GetAffectedRows() const
{
    return _mysql ? static_cast<uint64>(mysql_affected_rows(_mysql)) : 0;
}

std::string DBConnection::GetLastError() const
{
    return _mysql ? std::string(mysql_error(_mysql)) : "Not connected";
}

std::string DBConnection::Escape(const std::string& str)
{
    if (!_mysql) return str;
    std::string escaped(str.size() * 2 + 1, '\0');
    uint64 len = mysql_real_escape_string(_mysql, escaped.data(),
        str.c_str(), str.size());
    escaped.resize(len);
    return escaped;
}