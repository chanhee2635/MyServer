#pragma once

using SessionFactory = std::function<SessionRef()>;

class Service : public std::enable_shared_from_this<Service>
{
public:
    Service(NetAddress address, SessionFactory factory, int32 maxSessionCount);
    virtual ~Service() = default;

    virtual bool Start() = 0;

    SessionRef CreateSession();
    void       AddSession(SessionRef session);
    void       ReleaseSession(SessionRef session);

    int32      GetCurrentSessionCount() const { return _sessionCount; }
    int32      GetMaxSessionCount()     const { return _maxSessionCount; }
    NetAddress GetAddress()             const { return _address; }

protected:
    std::mutex         _lock;
    NetAddress         _address;
    SessionFactory     _sessionFactory;
    Set<SessionRef>    _sessions;
    std::atomic<int32> _sessionCount = 0;
    int32              _maxSessionCount = 0;
};

class ServerService : public Service
{
public:
    ServerService(NetAddress address, SessionFactory factory, int32 maxSessionCount);
    virtual bool Start() override;

private:
    ListenerRef _listener;
};