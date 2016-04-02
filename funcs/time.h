#ifndef __TAB_FUNCS_TIME_H
#define __TAB_FUNCS_TIME_H

void now(const obj::Object* in, obj::Object*& out) {

    Int& v = obj::get<obj::Int>(out).v;

    v = ::time(NULL);
}

int is_leap(int year) {
    return (year % 4) == 0 && ((year % 100) != 0 || (year % 400) == 0);
}

void calc_month_day(time_t& days, time_t& month, int leap) {

    static int const mon_days[13] = { 0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

    month = 1;

    while (1) {
        int md = mon_days[month] + (month == 2 ? leap : 0);

        if (days <= md)
            break;

        month++;
        days -= md;
    }
}

void simple_gmtime(time_t _t, struct tm& xtm) {

    bool neg = (_t < 0);

    time_t t = (neg ? -_t : _t);

    time_t days = t / (24*3600);
    time_t days_part = t % (24*3600);

    time_t hours = days_part / 3600;
    time_t hours_part = days_part % 3600;

    time_t minutes = hours_part / 60;
    time_t seconds = hours_part % 60;

    time_t year = (neg ? 1969 : 1970);
    time_t month;

    while (1) {
        int leap = is_leap(year);
        time_t days_in_year = 365 + leap;

        if (days < days_in_year) {

            if (neg)
                days = days_in_year - days;
            else
                days++;

            calc_month_day(days, month, leap);
            break;
        }

        days -= days_in_year;

        year += (neg ? -1 : 1);
    }

    if (neg) {
        hours = 23 - hours;
        minutes = 59 - minutes;
        seconds = 60 - seconds;
    }

    if (year <= 0)
        --year;

    xtm.tm_year = year;
    xtm.tm_mon = month;
    xtm.tm_mday = days;
    xtm.tm_hour = hours;
    xtm.tm_min = minutes;
    xtm.tm_sec = seconds;
}

void gmtime(const obj::Object* in, obj::Object*& out) {

    Int t = obj::get<obj::Int>(in).v;

    obj::Tuple& args = obj::get<obj::Tuple>(out);

    struct tm xtm;
    simple_gmtime(t, xtm);

    obj::get<obj::Int>(args.v[0]).v = xtm.tm_year;
    obj::get<obj::Int>(args.v[1]).v = xtm.tm_mon;
    obj::get<obj::Int>(args.v[2]).v = xtm.tm_mday;
    obj::get<obj::Int>(args.v[3]).v = xtm.tm_hour;
    obj::get<obj::Int>(args.v[4]).v = xtm.tm_min;
    obj::get<obj::Int>(args.v[5]).v = xtm.tm_sec;
}

void date(const obj::Object* in, obj::Object*& out) {

    Int t = obj::get<obj::Int>(in).v;
    std::string& res = obj::get<obj::String>(out).v;

    struct tm xtm;
    simple_gmtime(t, xtm);

    res.reserve(11);
    res.resize(10);
    ::snprintf((char*)res.data(), 11, "%04d-%02d-%02d", xtm.tm_year, xtm.tm_mon, xtm.tm_mday);
}

void time(const obj::Object* in, obj::Object*& out) {

    Int t = obj::get<obj::Int>(in).v;
    std::string& res = obj::get<obj::String>(out).v;

    struct tm xtm;
    simple_gmtime(t, xtm);

    res.reserve(9);
    res.resize(8);
    ::snprintf((char*)res.data(), 9, "%02d:%02d:%02d", xtm.tm_hour, xtm.tm_min, xtm.tm_sec);
}

void datetime(const obj::Object* in, obj::Object*& out) {

    Int t = obj::get<obj::Int>(in).v;
    std::string& res = obj::get<obj::String>(out).v;

    struct tm xtm;
    simple_gmtime(t, xtm);

    res.reserve(20);
    res.resize(19);
    ::snprintf((char*)res.data(), 20, "%04d-%02d-%02d %02d:%02d:%02d",
               xtm.tm_year, xtm.tm_mon, xtm.tm_mday, xtm.tm_hour, xtm.tm_min, xtm.tm_sec);
}

void register_time(Functions& funcs) {

    funcs.add("now", Type(), Type(Type::INT), now);

    funcs.add("gmtime",
              Type(Type::INT),
              Type(Type::TUP,
                   { Type(Type::INT), Type(Type::INT), Type(Type::INT),
                     Type(Type::INT), Type(Type::INT), Type(Type::INT) }),
              gmtime);

    funcs.add("date", Type(Type::INT), Type(Type::STRING), date);
    funcs.add("time", Type(Type::INT), Type(Type::STRING), time);
    funcs.add("datetime", Type(Type::INT), Type(Type::STRING), datetime);
}

#endif
