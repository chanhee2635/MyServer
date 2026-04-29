#pragma once
#include <mysql.h>

// ─────────────────────────────────────────────────────────────────────────────
// DBResult  : mysql_store_result RAII 래퍼 (동적 쿼리용)
// ─────────────────────────────────────────────────────────────────────────────
class DBResult
{
public:
    explicit DBResult(MYSQL_RES* res) : _res(res) {}
    ~DBResult() { if (_res) mysql_free_result(_res); }

    DBResult(const DBResult&) = delete;
    DBResult& operator=(const DBResult&) = delete;
    DBResult(DBResult&& o) noexcept : _res(o._res) { o._res = nullptr; }

    bool      IsValid()  const { return _res != nullptr; }
    uint64    NumRows()  const { return _res ? mysql_num_rows(_res) : 0; }
    MYSQL_ROW FetchRow() { return _res ? mysql_fetch_row(_res) : nullptr; }

private:
    MYSQL_RES* _res = nullptr;
};

// ─────────────────────────────────────────────────────────────────────────────
// DBStmt  : Prepared Statement RAII 래퍼
//   - 같은 쿼리를 반복 실행할 때 매번 파싱·플랜 수립을 생략
//   - DBConnection 당 쿼리 종류별로 하나씩 보관
//
// 사용 예)
//   stmt.Prepare(mysql, "SELECT level FROM players WHERE player_id = ?");
//   stmt.BindInUInt64(0, playerId);
//   int32 level = 0;
//   stmt.BindOutInt32(0, level);
//   if (stmt.Execute() && stmt.Fetch()) { ... }
// ─────────────────────────────────────────────────────────────────────────────
class DBStmt
{
public:
    static constexpr int MAX_BINDS = 16;

    DBStmt() = default;
    ~DBStmt() { Close(); }

    DBStmt(const DBStmt&) = delete;
    DBStmt& operator=(const DBStmt&) = delete;

    bool Prepare(MYSQL* mysql, const char* sql);
    void Close();
    bool IsPrepared() const { return _stmt != nullptr; }

    // ── 입력 파라미터 (? 순서대로 0-based) ──────────────────────────────────
    void BindInUInt64(int idx, uint64_t val);
    void BindInInt32 (int idx, int32_t  val);
    void BindInString(int idx, const std::string& val);

    // ── 결과 컬럼 (SELECT 컬럼 순서대로 0-based) ────────────────────────────
    void BindOutUInt64(int idx, uint64_t& out);
    void BindOutInt64 (int idx, int64_t&  out);
    void BindOutInt32 (int idx, int32_t&  out);
    void BindOutString(int idx, char* buf, unsigned long cap);
    void BindOutBool  (int idx, bool& out);

    // ── 실행·조회 ────────────────────────────────────────────────────────────
    bool    Execute();          // INSERT / UPDATE / DELETE / SELECT
    bool    Fetch();            // 결과 한 행씩
    uint64  AffectedRows() const;
    uint64  LastInsertId() const;

    // 다음 Execute 전에 바인딩 인덱스 초기화 (재사용 시 호출)
    void ResetBindings() { _inCount = 0; _outCount = 0; }

private:
    MYSQL_STMT* _stmt = nullptr;

    MYSQL_BIND _inBinds [MAX_BINDS] = {};
    MYSQL_BIND _outBinds[MAX_BINDS] = {};

    int _inCount  = 0;   // 실제 등록된 입력 파라미터 수
    int _outCount = 0;   // 실제 등록된 출력 컬럼 수

    // 입력값 임시 저장 (MYSQL_BIND 은 포인터로 참조)
    uint64_t      _inU64   [MAX_BINDS] = {};
    int32_t       _inI32   [MAX_BINDS] = {};
    unsigned long _inStrLen[MAX_BINDS] = {};
    std::string   _inStr   [MAX_BINDS];

    // 출력 임시 저장 (MySQL 8.0에서 my_bool 제거됨 → bool 사용)
    bool _outIsNull[MAX_BINDS] = {};
};

// ─────────────────────────────────────────────────────────────────────────────
// DBConnection  : 커넥션 하나 = 워커 스레드 하나가 독점 사용
// ─────────────────────────────────────────────────────────────────────────────
class DBConnection
{
public:
    DBConnection() = default;
    ~DBConnection() { Disconnect(); }

    DBConnection(const DBConnection&) = delete;
    DBConnection& operator=(const DBConnection&) = delete;

    bool Connect(const char* host, const char* user,
        const char* password, const char* database,
        uint32 port = 3306);
    void Disconnect();

    bool        IsConnected()    const { return _mysql != nullptr; }
    MYSQL*      GetHandle()      const { return _mysql; }
    uint64      GetLastInsertId() const;
    uint64      GetAffectedRows() const;
    std::string GetLastError()   const;

    // 일회성 동적 쿼리 (INSERT IGNORE 같은 특수 케이스용)
    bool     Execute(const std::string& query);
    DBResult Query  (const std::string& query);

    std::string Escape(const std::string& str);

private:
    MYSQL* _mysql = nullptr;
};