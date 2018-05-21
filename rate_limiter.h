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

    virtual void set_max_permits(int permits) { max_permits_ = permits; };  // todp
    virtual int get_max_permits() const { return max_permits_; };  // todo:

    virtual double get_rate() const;
    virtual void set_rate(double rate);
private:
    void sync(unsigned long long now);
    std::chrono::microseconds claim_next(double permits);
private:
    double interval_;               ///< �������Ƶ����ʣ������ٵ�����ֵ����λ����/��
    double max_permits_;            ///< ����Ͱ����
    double stored_permits_;         ///< ��ǰ����Ͱ�ڵ�������

    unsigned long long next_free_;  ///< �������ƵĿ�ʼʱ��

    std::mutex mut_;
};


#endif
