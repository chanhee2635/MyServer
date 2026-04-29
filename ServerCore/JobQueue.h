#pragma once

using Job = std::function<void()>;

class JobQueue : public std::enable_shared_from_this<JobQueue>
{
public:
    void Push(Job job);
    void Execute();

protected:
    template<typename T, typename Ret, typename... Args>
    void DoAsync(Ret(T::* memfn)(Args...), Args... args)
    {
        auto self = std::static_pointer_cast<T>(shared_from_this());
        auto tuple = std::make_tuple(std::move(args)...);
        Push([self, memfn, t = std::move(tuple)]() mutable {
            std::apply([&](auto&&... a) {
                ((*self).*memfn)(std::move(a)...);
                }, t);
            });
    }

private:
    std::mutex _lock;
    Queue<Job>  _jobs;
    bool        _pending = false; 
};
