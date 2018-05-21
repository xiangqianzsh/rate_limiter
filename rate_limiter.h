#ifndef _rate_limiter_h_
#define _rate_limiter_h_

#include <mutex>

class RateLimiter {
public:
    RateLimiter();
    virtual ~RateLimiter() {};
    virtual long aquire();
    virtual long aquire(int permits);

    virtual bool try_aquire(int timeouts);
    virtual bool try_aquire(int permits, int timeout);

    virtual void set_max_permits(double permits) { max_permits_ = permits; };  // todp
    virtual int get_max_permits() const { return max_permits_; };  // todo:

    virtual double get_rate() const;
    virtual void set_rate(double rate);
private:
    void sync(unsigned long long now);
    std::chrono::microseconds claim_next(double permits);
private:
    double stable_interval_micros_;               // 生成令牌的速率，生成一个令牌需要多少微秒. 单位 microseconds/个, 1s = 10^6 microseconds
    double max_permits_;            // 令牌桶容量
    double stored_permits_;         // 当前令牌桶内的令牌数

    unsigned long long next_free_;  // 下次生成令牌的开始时间, 是一个未来时间

    std::mutex mut_;
};

#endif
