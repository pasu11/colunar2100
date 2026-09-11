/* ***********************************************************************
 * This is a modified version of the Colunar program (2010)
 * Colunar was a very small lunar calendar under command line.            
 * It was a lite version of Lunar 2.2.   
 * Colunar was distributed under either version 2 of the GNU GPL,   
 * source: https://code.google.com/archive/p/colunar/
 
 * What's news in this modified version:
 * Extends the calendar range from 2050 to 2100, 
 * Some minor modifications to the Chinese date display.
 * Shows JieQi's name.
 * Codes/structure update 
 * */

#include <stdio.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>

typedef struct {
    int year, month, day, hour, weekday;
    int leap;   /* the lunar month is a leap month */
} Date;

typedef char byte;

char version[] = "CoLunar Version 0.1 (2010-01-30)";

#include "tables.h"

static int daysInSolarMonth[13] = {
    0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31
};

static int moon[2] = {29, 30}; /* a short (long) lunar month has 29 (30) days */

static char *Gan[] = {
    "Jia3", "Yi3", "Bing3", "Ding1", "Wu4",
    "Ji3",  "Geng1", "Xin1", "Ren2",  "Gui3"
};

static char *Zhi[] = {
    "Zi3", "Chou3", "Yin2", "Mao3", "Chen2", "Si4",
    "Wu3", "Wei4",  "Shen1", "You3", "Xu1",  "Hai4"
};

static char *ShengXiao[] = {
    "Mouse", "Ox", "Tiger", "Rabbit", "Dragon", "Snake",
    "Horse", "Goat", "Monkey", "Rooster", "Dog", "Pig"
};

static char *weekday[] = {
    "Sunday", "Monday", "Tuesday", "Wednesday",
    "Thursday", "Friday", "Saturday"
};

static char *GanHanzi[] = {
    "甲", "乙", "丙", "丁", "戊",
    "己", "庚", "辛", "壬", "癸"
};

static char *ZhiHanzi[] = {
    "子", "丑", "寅", "卯", "辰", "巳",
    "午", "未", "申", "酉", "戌", "亥"
};

static char *ShengXiaoHanzi[] = {
    "鼠", "牛", "虎", "兔", "龙", "蛇",
    "马", "羊", "猴", "鸡", "狗", "猪"
};

static char *weekdayHanzi[] = {
    "日", "一", "二", "三", "四", "五", "六"
};

static const char *JieNameHanzi[12] = {
    "立春", "惊蛰", "清明", "立夏", "芒种", "小暑",
    "立秋", "白露", "寒露", "立冬", "大雪", "小寒"
};
static const char *JieNameEn[12] = {
    "Lichun", "Jingzhe", "Qingming", "Lixia", "Mangzhong", "Xiaoshu",
    "Liqiu", "Bailu", "Hanlu", "Lidong", "Daxue", "Xiaohan"
};

static char *numHanzi[] = {
    "初一", "初二", "初三", "初四", "初五", "初六", "初七", "初八", "初九", "初十",
    "十一日", "十二日", "十三日", "十四日", "十五日", "十六日", "十七日", "十八日", "十九日", "二十",
    "廿一日", "廿二日", "廿三日", "廿四日", "廿五日", "廿六日", "廿七日", "廿八日", "廿九日", "三十"
};

static char *monthHanzi[] = {
    "正", "二", "三", "四", "五", "六",
    "七", "八", "九", "十", "十一", "十二"
};

static Date solar, lunar, gan, zhi, gan2, zhi2, lunar2;

static int ymonth[Nyear];      /* number of lunar months in the years */
static int yday[Nyear];        /* number of lunar days in the years */
static int mday[Nmonth + 1];   /* number of days in the months of the lunar year */
static int jieAlert;           /* if there is uncertainty in JieQi calculation */
static int jieIndex = -1;   /* 0..11, -1 means not on jie day */

static int showHZ = 1;         /* output in Chinese character */
static const char *progname = "colunar";

static void Solar2Lunar(void);
static long Solar2Day(const Date *d);
static long Solar2Day1(const Date *d);
static void Day2Lunar(long offset, Date *d);
static void __attribute__((unused)) Day2Solar(long offset, Date *d);
static int make_yday(void);
static int make_mday(int year);
static int GZcycle(int g, int z);
static void CalGZ(long offset, const Date *d, Date *g, Date *z);
static int JieDate(const Date *ds, Date *dl, int *jieIdx);
static void usage(void);
static void Error(const char *s);
static int CmpDate(int month1, int day1, int month2, int day2);
static void Report(void);
static void ReportHanzi(void);
static void ReportE(void);

#define LeapYear(y) ((((y) % 4 == 0) && ((y) % 100 != 0)) || ((y) % 400 == 0))
#define BYEAR 1201
/* BYEAR % 4 == 1 and BYEAR % 400 == 1 for easy calculation of leap years */
/* assert(BYEAR <= SolarFirstDate.year) */

int main(int argc, char *argv[])
{
    int year, month, day, hour, i, k, option;
    int dateInfo[4];
    struct tm *tm_area;
    time_t clock_now;

    for (k = 1; k < argc && argv[k][0] == '-'; k++) {
        option = argv[k][1];
        switch (option) {
            case 'e':
                showHZ = 0;
                break;
            case 'v':
                printf("%s\n\n", version);
                return 0;
            default:
                usage();
                break;
        }
    }

    if (argc - k == 0) {
        time(&clock_now);
        tm_area = localtime(&clock_now);
        if (tm_area == NULL) {
            Error("Failed to get local time.");
        }
        tm_area->tm_year += (tm_area->tm_year >= 91) ? 1900 : 2000;
        tm_area->tm_mon += 1;
        year = tm_area->tm_year;
        month = tm_area->tm_mon;
        day = tm_area->tm_mday;
        hour = tm_area->tm_hour;
    } else {
        if (!((argc - k >= 3) && (argc - k <= 4))) usage();

        dateInfo[3] = 0;
        for (i = 0; k < argc && i < 4; k++, i++) {
            if (sscanf(argv[k], "%d", &dateInfo[i]) != 1) {
                usage();
            }
        }

        year = dateInfo[0];
        month = dateInfo[1];
        day = dateInfo[2];
        hour = dateInfo[3];

        if (!(year >= Cyear && year < Cyear + Nyear)) Error("Year out of range.");
        if (!(month >= 1 && month <= 12)) Error("Month out of range.");
        if (!(day >= 1 && day <= 31)) Error("Day out of range.");
        if (!(hour >= 0 && hour <= 23)) Error("Hour out of range.");

        if (year == SolarFirstDate.year &&
            CmpDate(month, day, SolarFirstDate.month, SolarFirstDate.day) < 0) {
            Error("Date out of range.");
        }
    }

    solar.year = year;
    solar.month = month;
    solar.day = day;
    solar.hour = hour;

    Solar2Lunar();
    Report();
    return 0;
}

static void usage(void)
{
    printf("Usage:\n");
    printf("\tJUST '%s' for NOW\n\n", progname);
    printf("\tOR 'colunar [-e] year month day [hour]'\n");
    printf("\t(in Solar Calendar, 24 hour clock)\n\n");
    printf("\t-e means output in English (Pinyin)\n");
    printf(" Date range: 1900.1.31.00 - 2100.12.31.23\n");
    printf(" Colunar 2100, August 2026\n");
    exit(1);
}

static void Solar2Lunar(void)
{
    long offset;

    offset = Solar2Day(&solar);
    solar.weekday = (int)((offset + SolarFirstDate.weekday) % 7);

    /* A lunar day begins at 11 p.m. */
    if (solar.hour == 23) {
        offset++;
    }

    Day2Lunar(offset, &lunar);
    lunar.hour = solar.hour;
    CalGZ(offset, &lunar, &gan, &zhi);

	jieAlert = JieDate(&solar, &lunar2, &jieIndex);
    lunar2.day = lunar.day;
    lunar2.hour = lunar.hour;
    CalGZ(offset, &lunar2, &gan2, &zhi2);
}

static long Solar2Day(const Date *d)
{
    return Solar2Day1(d) - Solar2Day1(&SolarFirstDate);
}

/* Compute the number of days from the Solar date BYEAR.1.1 */
static long Solar2Day1(const Date *d)
{
    long offset, delta;
    int i;

    delta = d->year - BYEAR;
    if (delta < 0) Error("Internal error: pick a larger constant for BYEAR.");

    offset = delta * 365 + delta / 4 - delta / 100 + delta / 400;

    for (i = 1; i < d->month; i++) {
        offset += daysInSolarMonth[i];
    }

    if ((d->month > 2) && LeapYear(d->year)) {
        offset++;
    }

    offset += d->day - 1;

    if ((d->month == 2) && LeapYear(d->year)) {
        if (d->day > 29) Error("Day out of range.");
    } else if (d->day > daysInSolarMonth[d->month]) {
        Error("Day out of range.");
    }

    return offset;
}

static void Day2Lunar(long offset, Date *d)
{
    int i, m, nYear, leapMonth;

    nYear = make_yday();

    for (i = 0; i < nYear && offset > 0; i++) {
        offset -= yday[i];
    }
    if (offset < 0) {
        offset += yday[--i];
    }

    if (i == Nyear) Error("Year out of range.");
    d->year = i + LunarFirstDate.year;

    leapMonth = make_mday(i);

    for (m = 1; m <= Nmonth && offset > 0; m++) {
        offset -= mday[m];
    }
    if (offset < 0) {
        offset += mday[--m];
    }

    d->leap = 0; /* don't know leap or not yet */

    if (leapMonth > 0) { /* has leap month */

        /* if preceeding month number is the leap month,
         * this month is the actual extra leap month */
        d->leap = (leapMonth == (m - 1));

        /* month > leapMonth is off by 1, so adjust it */
        if (m > leapMonth) --m;
    }

    d->month = m;
    d->day = (int)offset + 1;
}

static void __attribute__((unused)) Day2Solar(long offset, Date *d)
{
    int i, m, days;

    /* offset is the number of days from SolarFirstDate */
    offset -= Solar2Day(&LunarFirstDate); /* the argument is negative */
    /* offset is now the number of days from SolarFirstDate.year.1.1 */

    for (i = SolarFirstDate.year;
         (i < SolarFirstDate.year + Nyear) && (offset > 0); i++) {
        offset -= 365 + LeapYear(i);
    }

    if (offset < 0) {
        --i;
        offset += 365 + LeapYear(i);
    }

    if (i == (SolarFirstDate.year + Nyear)) Error("Year out of range.");
    d->year = i;

    for (m = 1; m <= 12; m++) {
        days = daysInSolarMonth[m];
        if ((m == 2) && LeapYear(i)) days++;
        if (offset < days) {
            d->month = m;
            d->day = (int)offset + 1;
            return;
        }
        offset -= days;
    }
}

static int GZcycle(int g, int z)
{
    int gz;

    for (gz = z; gz % 10 != g && gz < 60; gz += 12) {
        /* loop */
    }
    if (gz >= 60) {
        printf("internal error\n");
    }
    return gz + 1;
}

static void CalGZ(long offset, const Date *d, Date *g, Date *z)
{
    int year, month;

    year = d->year - LunarFirstDate.year;
    month = year * 12 + d->month - 1; /* leap months do not count */

    g->year = (GanFirstDate.year + year) % 10;
    z->year = (ZhiFirstDate.year + year) % 12;
    g->month = (GanFirstDate.month + month) % 10;
    z->month = (ZhiFirstDate.month + month) % 12;
    g->day = (GanFirstDate.day + (int)offset) % 10;
    z->day = (ZhiFirstDate.day + (int)offset) % 12;
    z->hour = ((d->hour + 1) / 2) % 12;
    g->hour = (g->day * 12 + z->hour) % 10;
}

static void Error(const char *s)
{
    printf("%s\n", s);
    exit(1);
}

/* Compare two dates and return <,=,> 0 if the 1st is <,=,> the 2nd */
static int CmpDate(int month1, int day1, int month2, int day2)
{
    if (month1 != month2) return (month1 - month2);
    if (day1 != day2) return (day1 - day2);
    return 0;
}

/*
 * Given a solar date, find the "lunar" date for the purpose of
 * calculating the "4-columns" by taking jie into consideration.
 */
static int JieDate(const Date *ds, Date *dl, int *jieIdx)
{
    int m, flag;
    if (jieIdx) *jieIdx = -1;
    if (ds->month == 1) {
        /* January compares with previous lunar year's 12th jie = Xiaohan */
        flag = CmpDate(ds->month, ds->day,
                       1, fest[ds->year - SolarFirstDate.year - 1][11]);
        if (flag < 0) dl->month = 11;
        else if (flag > 0) dl->month = 12;
        dl->year = ds->year - 1;
        if (flag == 0 && jieIdx) *jieIdx = 11; /* 小寒 */
        return (flag == 0);
    }
    for (m = 2; m <= 12; m++) {
        flag = CmpDate(ds->month, ds->day,
                       m, fest[ds->year - SolarFirstDate.year][m - 2]);
        if (flag == 0) {
            if (jieIdx) *jieIdx = m - 2; /* 命中当前节 */
            m++; /* 保持你原有月柱推进逻辑 */
        }
        if (flag <= 0) break;
    }
    dl->month = (m - 2) % 12;
    dl->year = ds->year;
    if (dl->month == 0) {
        dl->year = ds->year - 1;
        dl->month = 12;
    }
    return (flag == 0);
}

/* Compute the number of days in each lunar year in the table */
static int make_yday(void)
{
    int year, i, leap;
    long code;

    for (year = 0; year < Nyear; year++) {
        code = yearInfo[year];
        leap = code & 0xf;
        yday[year] = 0;

        if (leap != 0) {
            i = (int)((code >> 16) & 0x1);
            yday[year] += moon[i];
        }

        code >>= 4;
        for (i = 0; i < Nmonth - 1; i++) {
            yday[year] += moon[code & 0x1];
            code >>= 1;
        }

        ymonth[year] = 12;
        if (leap != 0) ymonth[year]++;
    }
    return Nyear;
}

/* Compute the days of each month in the given lunar year */
static int make_mday(int year)
{
    int i, leapMonth;
    long code;

    code = yearInfo[year];
    leapMonth = (int)(code & 0xf);
    /* leapMonth == 0 means no leap month */
    code >>= 4;

    if (leapMonth == 0) {
        mday[Nmonth] = 0;
        for (i = Nmonth - 1; i >= 1; i--) {
            mday[i] = moon[code & 0x1];
            code >>= 1;
        }
    } else {
        /*
         * There is a leap month L in this year.
         * mday[L+1] contains the number of days in the L-th leap month.
         */
        i = (int)((yearInfo[year] >> 16) & 0x1);
        mday[leapMonth + 1] = moon[i];

        for (i = Nmonth; i >= 1; i--) {
            if (i == leapMonth + 1) i--;
            mday[i] = moon[code & 0x1];
            code >>= 1;
        }
    }

    return leapMonth;
}

static void Report(void)
{
    if (showHZ) ReportHanzi();
    else ReportE();
}

static void ReportHanzi(void)
{
    printf("%s%d%s%2d%s%2d%s%2d%s%s%s\n", "阳历: ",
           solar.year, "年", solar.month, "月", solar.day,
           "日", solar.hour, "时　",
           "星期", weekdayHanzi[solar.weekday]);

    printf("%s%d%s%s%s%s%s%s%s%s%s%s\n", "阴历: ",
           lunar.year, "年 ", (lunar.leap ? " 闰" : ""),
           monthHanzi[lunar.month - 1], "月 ",
           numHanzi[lunar.day - 1], " ",
           ZhiHanzi[zhi.hour], "时 ",
           "生肖属", ShengXiaoHanzi[zhi.year]);

    printf("%s%s%s%s%s%s%s%s%s%s%s%s%s\n", "干支: ",
           GanHanzi[gan2.year], ZhiHanzi[zhi2.year], "年 ",
           GanHanzi[gan2.month], ZhiHanzi[zhi2.month], "月 ",
           GanHanzi[gan2.day], ZhiHanzi[zhi2.day], "日 ",
           GanHanzi[gan2.hour], ZhiHanzi[zhi2.hour], "时 ");

    printf("%s%s%s%s%s%s%s%s%s%s%s%s%s\n",
           "用四柱神算推算之时辰八字: \n",
           GanHanzi[gan2.year], ZhiHanzi[zhi2.year], "年 ",
           GanHanzi[gan2.month], ZhiHanzi[zhi2.month], "月 ",
           GanHanzi[gan2.day], ZhiHanzi[zhi2.day], "日 ",
           GanHanzi[gan2.hour], ZhiHanzi[zhi2.hour], "时 ");

    if (jieAlert) {
		if (jieIndex >= 0 && jieIndex < 12)
        printf("* 今日%s，月柱可能要修改\n", JieNameHanzi[jieIndex]);
/* 防范修改节气导致节气号码超出范围的错误。这里不用
 *     else
        printf("* 是日为节，月柱可能要修改\n");
*/
		if (lunar2.month == 1)
        printf("* 年柱亦可能要修改\n");
        printf("* %s\n", "请查有节气时间之万年历");
}
}

static void ReportE(void)
{
    printf("Solar : %d.%d.%d.%d\t%s\n",
           solar.year, solar.month, solar.day, solar.hour, weekday[solar.weekday]);

    /* fixed: month/day index must be -1 */
    printf("Lunar : %d.%s%s.%s.%d\tShengXiao: %s\n",
           lunar.year,
           monthHanzi[lunar.month - 1],
           (lunar.leap ? "Leap" : ""),
           numHanzi[lunar.day - 1],
           lunar.hour,
           ShengXiao[zhi.year]);

    printf("GanZhi: %s-%s.%s-%s.%s-%s.%s-%s\n",
           Gan[gan.year], Zhi[zhi.year], Gan[gan2.month], Zhi[zhi2.month],
           Gan[gan.day], Zhi[zhi.day], Gan[gan.hour], Zhi[zhi.hour]);

    printf("        (GanZhi Order)\t%d-%d.%d-%d.%d-%d.%d-%d\n",
           gan.year + 1, zhi.year + 1, gan2.month + 1, zhi2.month + 1,
           gan.day + 1, zhi.day + 1, gan.hour + 1, zhi.hour + 1);

    printf("        (JiaZi Cycle)\t%d.%d.%d.%d\n\n",
           GZcycle(gan.year, zhi.year), GZcycle(gan2.month, zhi2.month),
           GZcycle(gan.day, zhi.day), GZcycle(gan.hour, zhi.hour));

    printf("BaZi (8-characters) according to 'Four Column Calculation':\n");
    printf("        %s-%s.%s-%s.%s-%s.%s-%s\n",
           Gan[gan2.year], Zhi[zhi2.year], Gan[gan2.month], Zhi[zhi2.month],
           Gan[gan2.day], Zhi[zhi2.day], Gan[gan2.hour], Zhi[zhi2.hour]);

    printf("        (GanZhi Order)\t%d-%d.%d-%d.%d-%d.%d-%d\n",
           gan2.year + 1, zhi2.year + 1, gan2.month + 1, zhi2.month + 1,
           gan2.day + 1, zhi2.day + 1, gan2.hour + 1, zhi2.hour + 1);

    printf("        (JiaZi Cycle)\t%d.%d.%d.%d\n\n",
           GZcycle(gan2.year, zhi2.year), GZcycle(gan2.month, zhi2.month),
           GZcycle(gan2.day, zhi2.day), GZcycle(gan2.hour, zhi2.hour));

    if (jieAlert) {
		if (jieIndex >= 0 && jieIndex < 12)
        printf("* Today is %s. The month column may need adjustment.\n",
               JieNameEn[jieIndex]);
		if (lunar2.month == 1)
        printf("* The year column may need adjustment, too.\n");
        printf("* Please consult a detailed conversion table.\n");
}
}
