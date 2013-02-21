#ifndef PERFMON_INL_H
#error "You should no use this file directly. Please include the 'perfmon.h'."
#endif // PERFMON_INL_H

#include <list>

namespace perfmon {
namespace internal {

class Monitor {
public:
    explicit Monitor(::perfmon::Counter& counter_ref)
      : counter_ref_(counter_ref)
      , start_tick_(ReadTickCounter())
    { }

    ~Monitor()
    {
        counter_ref_.ticks += TicksElapsed(start_tick_, ReadTickCounter());
        ++counter_ref_.calls;
    }

    explicit operator bool () const
    {
        return false;
    }

private:
    ::perfmon::Counter& counter_ref_;
    const Tick start_tick_;
};

class Counters {
public:
    typedef std::list<Counter>::const_iterator const_iterator;

    Counters(const_iterator begin, const_iterator end)
      : begin_(begin)
      , end_(end)
    { }

    const_iterator begin() const
    {
        return begin_;
    }

    const_iterator end() const
    {
        return end_;
    }

private:
    const_iterator begin_;
    const_iterator end_;
};

Counters counters();

Counter& counter(const char* counter_name);

} } // namespace perfmon::internal


#undef PERFMON_COUNTERS
#undef PERFMON_COUNTER
#undef PERFMON_SCOPE
#undef PERFMON_FUNCTION_SCOPE
#undef PERFMON_EXPRESSION

#define PERFMON_COUNTERS               (::perfmon::internal::counters())
#define PERFMON_COUNTER(counter_name)  ([](const char* name)->::perfmon::Counter& { static auto& counter_ref = ::perfmon::internal::counter(name); return counter_ref; }(counter_name))

#ifdef NDEBUG
#define PERFMON_SCOPE(counter_name)       if (false) { throw; } else
#define PERFMON_FUNCTION_SCOPE            do { } while (false)
#define PERFMON_EXPRESSION(counter_name, EXPR)  (EXPR)

#else // NDEBUG
#define PERFMON_INTERNAL__CONCAT(x, y)    x ## y
#define PERFMON_INTERNAL_CONCAT(x, y)     PERFMON_INTERNAL__CONCAT(x, y)
#define PERFMON_INTERNAL_SCOPE_VAR        PERFMON_INTERNAL_CONCAT(perfmon_scope_monitor_, __COUNTER__)

#define PERFMON_SCOPE(counter_name)       if (const auto PERFMON_INTERNAL_SCOPE_VAR = ::perfmon::internal::Monitor(PERFMON_COUNTER(counter_name))) { /* Suppress the 'control may reach end of non-void function' warning */ throw; } else
#define PERFMON_FUNCTION_SCOPE            ::perfmon::internal::Monitor PERFMON_INTERNAL_SCOPE_VAR (PERFMON_COUNTER(__FUNCTION__))
#define PERFMON_EXPRESSION(counter_name, EXPR)  ([&]() -> decltype((EXPR)) { PERFMON_SCOPE(counter_name) { return (EXPR); } }())

#endif // NDEBUG
