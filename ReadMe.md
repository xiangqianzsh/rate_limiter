g++ test.cpp rate_limiter.cc -lpthread -std=c++14

# 参考git

https://github.com/mfycheng/ratelimiter

https://github.com/jason860306/RateLimiter

# 参考文档
Guava官方文档-RateLimiter类
https://blog.csdn.net/jiesa/article/details/52461703

https://dzone.com/articles/ratelimiter-discovering-google
接口限流算法总结
http://www.kissyu.org/2016/08/13/%E9%99%90%E6%B5%81%E7%AE%97%E6%B3%95%E6%80%BB%E7%BB%93/



google java代码片段参考
```

#if 0
///< Google开源工具包Guava提供的限流工具类RateLimiter的主要实现代码:

public double acquire() {
    return acquire(1);
}

public double acquire(int permits) {
    checkPermits(permits);  //检查参数是否合法（是否大于0）
    long microsToWait;
    synchronized (mutex) { //应对并发情况需要同步
        microsToWait = reserveNextTicket(permits, readSafeMicros()); //获得需要等待的时间
    }
    ticker.sleepMicrosUninterruptibly(microsToWait); //等待，当未达到限制时，microsToWait为0
    return 1.0 * microsToWait / TimeUnit.SECONDS.toMicros(1L);
}

private long reserveNextTicket(double requiredPermits, long nowMicros) {
    resync(nowMicros); //补充令牌
    long microsToNextFreeTicket = nextFreeTicketMicros - nowMicros;
    double storedPermitsToSpend = Math.min(requiredPermits, this.storedPermits); //获取这次请求消耗的令牌数目
    double freshPermits = requiredPermits - storedPermitsToSpend;

    long waitMicros = storedPermitsToWaitTime(this.storedPermits, storedPermitsToSpend)
            + (long) (freshPermits * stableIntervalMicros);

    this.nextFreeTicketMicros = nextFreeTicketMicros + waitMicros;
    this.storedPermits -= storedPermitsToSpend; // 减去消耗的令牌
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


```
