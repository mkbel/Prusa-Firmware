/*
 * @file
 * @author Marek Bel
 */

#ifndef TIMER_H
#define TIMER_H

#include "Arduino.h"

/**
 * @brief simple timer
 *
 * Simple and memory saving implementation. Should handle timer register wrap around well.
 * Resolution is one millisecond. To save memory, doesn't store timer period.
 * If you wish timer which is storing period, derive from this.
 */
template <class T>
class Timer
{
public:
    Timer();
    void start();
    void stop(){m_isRunning = false;}
    bool running(){return m_isRunning;}
    bool expired(T msPeriod);
private:
    bool m_isRunning;
    T m_started;
};

/**
 * @brief construct Timer
 *
 * It is guaranteed, that construction is equivalent with zeroing all members.
 * This property can be exploited in MenuData union.
 */
template<typename T>
Timer<T>::Timer() : m_isRunning(false), m_started()
{
}

/**
 * @brief Start timer
 */
template<typename T>
void Timer<T>::start()
{
    m_started = millis();
    m_isRunning = true;
}

/**
 * @brief Timer has expired
 *
 * Timer is considered expired after msPeriod has passed from time the timer was started.
 * Timer is stopped after expiration.
 * This function must be called at least each (T maximum value - msPeriod) milliseconds to be sure to
 * catch first expiration.
 * This function is expected to handle wrap around of time register well.
 *
 * @param msPeriod Time interval in milliseconds. Do not omit "ul" when using constant literal with LongTimer.
 * @retval true Timer has expired
 * @retval false Timer not expired yet, or is not running, or time window in which is timer considered expired passed.
 */
template<typename T>
bool Timer<T>::expired(T msPeriod)
{
    if (!m_isRunning) return false;
    bool expired = false;
    const T now = millis();
    if (m_started <=  m_started + msPeriod)
    {
        if ((now >= m_started + msPeriod) || (now < m_started))
        {
            expired = true;
        }
    }
    else
    {
        if ((now >= m_started + msPeriod) && (now < m_started))
        {
            expired = true;
        }
    }
    if (expired) m_isRunning = false;
    return expired;
}

/**
 * @brief Timer unsigned long specialization
 *
 * Maximum period is at least 49 days.
 */
using LongTimer = Timer<unsigned long>;
/**
 * @brief Timer unsigned short specialization
 *
 * Maximum period is at least 65 seconds.
 */
using ShortTimer = Timer<unsigned short>;

#endif /* TIMER_H */
