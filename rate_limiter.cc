#include <algorithm>
#include <chrono>
#include <stdexcept>
#include <thread>

#include "util/rate_limiter/rate_limiter.h"

/*
#if 0
///< Google��Դ���߰�Guava�ṩ������������RateLimiter����Ҫʵ�ִ���:

public double acquire() {
    return acquire(1);
}

public double acquire(int permits) {
    checkPermits(permits);  //�������Ƿ�Ϸ����Ƿ����0��
    long microsToWait;
    synchronized (mutex) { //Ӧ�Բ��������Ҫͬ��
        microsToWait = reserveNextTicket(permits, readSafeMicros()); //�����Ҫ�ȴ���ʱ��
    }
    ticker.sleepMicrosUninterruptibly(microsToWait); //�ȴ�����δ�ﵽ����ʱ��microsToWaitΪ0
    return 1.0 * microsToWait / TimeUnit.SECONDS.toMicros(1L);
}

private long reserveNextTicket(double requiredPermits, long nowMicros) {
    resync(nowMicros); //��������
    long microsToNextFreeTicket = nextFreeTicketMicros - nowMicros;
    double storedPermitsToSpend = Math.min(requiredPermits, this.storedPermits); //��ȡ����������ĵ�������Ŀ
    double freshPermits = requiredPermits - storedPermitsToSpend;

    long waitMicros = storedPermitsToWaitTime(this.storedPermits, storedPermitsToSpend)
            + (long) (freshPermits * stableIntervalMicros);

    this.nextFreeTicketMicros = nextFreeTicketMicros + waitMicros;
    this.storedPermits -= storedPermitsToSpend; // ��ȥ���ĵ�����
    return microsToNextFreeTicket;
}

private void resync(long nowMicros) {
    // if nextFreeTicket is in the past, resync to now
    if (nowMicros > nextFreeTicketMicros) {
        storedPermits = Math.min(maxPermits,
                                 storedPermits + (nowMicros - nextFreeTicketMicros) / stableIntervalMicros);
        nextFreeTicketMicros = nowMicros;
    }
}
#endif
*/

RateLimiter::RateLimiter()
    : interval_(0), max_permits_(0), stored_permits_(0), next_free_(0) {
}
long RateLimiter::aquire() {
    return aquire(1);
}
long RateLimiter::aquire(int permits) {
    if (permits <= 0) {
        throw std::runtime_error("RateLimiter: Must request positive amount of permits");
    }

    auto wait_time = claim_next(permits);
    std::this_thread::sleep_for(wait_time);

    return wait_time.count() / 1000.0;
}

bool RateLimiter::try_aquire(int permits) {
    return try_aquire(permits, 0);
}
bool RateLimiter::try_aquire(int permits, int timeout) {
    using namespace std::chrono;

    unsigned long long now = duration_cast<microseconds>(system_clock::now().time_since_epoch()).count();

    // Check to see if the next free aquire time is within the
    // specified timeout. If it's not, return false and DO NOT BLOCK,
    // otherwise, calculate time needed to claim, and block
    if (next_free_ > now + timeout * 1000)
        return false;
    else {
        aquire(permits);
    }

    return true;
}

void RateLimiter::sync(unsigned long long now) {
    // If we're passed the next_free, then recalculate
    // stored permits, and update next_free_
    if (now > next_free_) {
        /**
         * ȡ��ǰ����Ͱ��������������ܴ�������Ͱ������
         * (now - next_free_) / interval_: ʱ���(now - next_free_)���²�����������
         * stored_permits_ + (now - next_free_) / interval_: ��ǰ����Ͱ��ʣ�༰�²�����������
         */
        stored_permits_ = std::min(max_permits_, stored_permits_ + (now - next_free_) / interval_);
        next_free_ = now; ///< �����������ƵĿ�ʼʱ��
    }
}
std::chrono::microseconds RateLimiter::claim_next(double permits) {
    using namespace std::chrono;

    std::lock_guard<std::mutex> lock(mut_);

    unsigned long long now = duration_cast<microseconds>(system_clock::now().time_since_epoch()).count();

    // Make sure we're synced
    sync(now);

    // Since we synced before hand, this will always be >= 0.
    unsigned long long wait = next_free_ - now; ///< ���������Ƶ���ʼʱ������Ҫ�ȴ���ʱ��

    // Determine how many stored and freh permits to consume
    double stored = std::min(permits, stored_permits_); ///< ��ǰʵ����Ҫ�ҿ��õ�������
    double fresh = permits - stored; ///< ��ǰ����Ͱ�ڲ��㵫ʵ����Ҫ����Ҫ�²�����������

    // In the general RateLimiter, stored permits have no wait time,
    // and thus we only have to wait for however many fresh permits we consume
    long next_free = (long)(fresh * interval_); ///< �������������ʽ������������ת����ʱ���

    next_free_ += next_free;
    stored_permits_ -= stored; ///< ���ĵ�stored������

    return microseconds(wait);
}

double RateLimiter::get_rate() const {
    return 1000000.0 / interval_;
}
void RateLimiter::set_rate(double rate) {
    if (rate <= 0.0) {
        throw std::runtime_error("RateLimiter: Rate must be greater than 0");
    }

    /**
     * �������Ƶ����ʵ�λֵ
     * interval_: ��/��
     * rate: ��/��
     */
    std::lock_guard<std::mutex> lock(mut_);
    interval_ = 1000000.0 / rate;
}
