// Copyright 2014 by Bill Torpey. All Rights Reserved.
// This work is licensed under a Creative Commons Attribution-NonCommercial-NoDerivs 3.0 United States License.
// http://creativecommons.org/licenses/by-nc-nd/3.0/us/deed.en

#include <time.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/utsname.h>
#include <cstring>
#include <chrono>
#include <vector>
#include <cmath>
#include <cpuid.h>

// using namespace std;

#define ONE_BILLION 1000000000L

// NOTE: if you change this value you prob also need to adjust later code that does "& 0xff" to use modulo arithmetic instead
const int BUCKETS = 16384; // how many samples to collect per iteration
const int ITERS = 100;     // how many iterations to run
double CPU_FREQ = 1;

inline unsigned long long cpuid_rdtsc() {
    unsigned int lo, hi;
    asm volatile(
        "cpuid \n"
        "rdtsc"
        : "=a"(lo), "=d"(hi) /* outputs */
        : "a"(0)             /* inputs */
        : "%ebx", "%ecx");   /* clobbers*/
    return ((unsigned long long)lo) | (((unsigned long long)hi) << 32);
}

inline unsigned long long rdtsc()
{
    unsigned int lo, hi;
    asm volatile(
        "rdtsc"
        : "=a"(lo), "=d"(hi) /* outputs */
        : "a"(0)             /* inputs */
        : "%ebx", "%ecx");   /* clobbers*/
    return ((unsigned long long)lo) | (((unsigned long long)hi) << 32);
}

inline unsigned long long rdtscp()
{
    unsigned int lo, hi;
    asm volatile(
        "rdtscp"
        : "=a"(lo), "=d"(hi) /* outputs */
        : "a"(0)             /* inputs */
        : "%ebx", "%ecx");   /* clobbers*/
    return ((unsigned long long)lo) | (((unsigned long long)hi) << 32);
}

// macro to call clock_gettime w/different values for CLOCK
#define do_clock(CLOCK)                                             \
    do                                                              \
    {                                                               \
        for (int i = 0; i < ITERS * BUCKETS; ++i)                   \
        {                                                           \
            struct timespec x;                                      \
            clock_gettime(CLOCK, &x);                               \
            int n = i & (BUCKETS - 1);                              \
            timestamp[n] = (x.tv_sec * ONE_BILLION) + x.tv_nsec;    \
        }                                                           \
        deltaT x(#CLOCK, timestamp);                                \
        x.print();                                                  \
    } while (0)

// macro to test C++ chrono clocks
#define do_chrono_clock(CLOCK_TYPE, CLOCK_NAME)                                         \
    do                                                                                  \
    {                                                                                   \
        for (int i = 0; i < ITERS * BUCKETS; ++i)                                       \
        {                                                                               \
            auto now = CLOCK_TYPE::now();                                               \
            auto nanos = std::chrono::time_point_cast<std::chrono::nanoseconds>(now);   \
            auto value = nanos.time_since_epoch().count();                              \
            int n = i & (BUCKETS - 1);                                                  \
            timestamp[n] = value;                                                       \
        }                                                                               \
        deltaT x(CLOCK_NAME, timestamp);                                                \
        x.print();                                                                      \
    } while (0)

struct deltaT
{
    deltaT(const char *name, std::vector<long> v) : sum(0), sum2(0), min(-1), max(0), avg(0), median(0), stdev(0), name(name)
    {
        // create vector with deltas between adjacent entries
        x = v;
        count = x.size() - 1;
        for (int i = 0; i < count; ++i)
        {
            x[i] = x[i + 1] - x[i];
        }

        for (int i = 0; i < count; ++i)
        {
            sum += x[i];
            sum2 += (x[i] * x[i]);
            if (x[i] > max)
                max = x[i];
            if ((min == -1) || (x[i] < min))
                min = x[i];
        }

        avg = sum / count;
        median = min + ((max - min) / 2);
        stdev = sqrt((count * sum2 - (sum * sum)) / (count * count));
    }

    void print()
    {
        printf("%-25s %10ld %8.2f %8.2f %8.2f %8.2f %8.2f\n",
               name, count, min, max, avg, median, stdev);
    }

    void dump(FILE *file)
    {
        if (file == NULL)
            return;

        fprintf(file, "%s", name);
        for (int i = 0; i < count; ++i)
        {
            fprintf(file, "\t%ld", x[i]);
        }
        fprintf(file, "\n");
    }

    std::vector<long> x;
    long count;
    double sum;
    double sum2;
    double min;
    double max;
    double avg;
    double median;
    double stdev;
    const char *name;
};

double get_cpu_frequency()
{
    FILE *fp = fopen("/proc/cpuinfo", "r");
    if (fp == NULL)
        return 2.4; // 默认值

    char line[256];
    double freq = 2.4; // 默认2.4GHz

    while (fgets(line, sizeof(line), fp))
    {
        if (strstr(line, "cpu MHz"))
        {
            char *colon = strchr(line, ':');
            if (colon)
            {
                freq = atof(colon + 1) / 1000.0; // 转换为GHz
                break;
            }
        }
    }
    fclose(fp);
    return freq;
}

bool supports_rdtscp()
{
    unsigned int eax, ebx, ecx, edx;
    if (__get_cpuid(0x80000001, &eax, &ebx, &ecx, &edx))
    {
        return (edx & (1 << 27));
    }
    return false;
}

int main(int argc, char **argv)
{
    struct utsname sysinfo;
    uname(&sysinfo);
    printf("System: %s %s %s\n", sysinfo.sysname, sysinfo.release, sysinfo.machine);
    if (argc > 1)
    {
        CPU_FREQ = strtod(argv[1], NULL);
    }
    else
    {
        CPU_FREQ = get_cpu_frequency();
        printf("Auto-detected CPU frequency: %.3f GHz\n", CPU_FREQ);
    }

    FILE *file = NULL;
    if (argc > 2)
    {
        file = fopen(argv[2], "w");
    }

    std::vector<long> timestamp;
    timestamp.resize(BUCKETS);

    printf("\n%-25s %10s %8s %8s %8s %8s %8s\n",
           "Method", "samples", "min", "max", "avg", "median", "stdev");
    printf("%-25s %10s %8s %8s %8s %8s %8s\n",
           "-------------------------", "----------", "--------", "--------", "--------", "--------", "--------");

#if _POSIX_TIMERS > 0
#ifdef CLOCK_REALTIME
    do_clock(CLOCK_REALTIME);
#endif

#ifdef CLOCK_REALTIME_COARSE
    do_clock(CLOCK_REALTIME_COARSE);
#endif

#ifdef CLOCK_REALTIME_HR
    do_clock(CLOCK_REALTIME_HR);
#endif

#ifdef CLOCK_MONOTONIC
    do_clock(CLOCK_MONOTONIC);
#endif

#ifdef CLOCK_MONOTONIC_RAW
    do_clock(CLOCK_MONOTONIC_RAW);
#endif

#ifdef CLOCK_MONOTONIC_COARSE
    do_clock(CLOCK_MONOTONIC_COARSE);
#endif

#ifdef CLOCK_BOOTTIME
    do_clock(CLOCK_BOOTTIME);
#endif
#endif

    do_chrono_clock(std::chrono::high_resolution_clock, "high_resolution_clock");
    do_chrono_clock(std::chrono::steady_clock, "steady_clock");
    do_chrono_clock(std::chrono::system_clock, "system_clock");

    {
        for (int i = 0; i < ITERS * BUCKETS; ++i)
        {
            int n = i & (BUCKETS - 1);
            timestamp[n] = cpuid_rdtsc();
        }
        for (int i = 0; i < BUCKETS; ++i)
        {
            timestamp[i] = (long)((double)timestamp[i] / CPU_FREQ);
        }
        deltaT x("cpuid+rdtsc", timestamp);
        x.print();
        x.dump(file);
    }

    // will throw SIGILL on machine w/o rdtscp instruction
    if (supports_rdtscp())
    {
        for (int i = 0; i < ITERS * BUCKETS; ++i)
        {
            int n = i & (BUCKETS - 1);
            timestamp[n] = rdtscp();
        }
        for (int i = 0; i < BUCKETS; ++i)
        {
            timestamp[i] = (long)((double)timestamp[i] / CPU_FREQ);
        }
        deltaT x("rdtscp", timestamp);
        x.print();
        x.dump(file);
    }
    else
    {
        printf("rdtscp                  \t(not supported on this CPU)\n");
    }

    {
        for (int i = 0; i < ITERS * BUCKETS; ++i)
        {
            int n = i & (BUCKETS - 1);
            timestamp[n] = rdtsc();
        }
        for (int i = 0; i < BUCKETS; ++i)
        {
            timestamp[i] = (long)((double)timestamp[i] / CPU_FREQ);
        }
        deltaT x("rdtsc", timestamp);
        x.print();
        x.dump(file);
    }

    printf("\nUsing CPU frequency = %.3f GHz\n", CPU_FREQ);

    if (file != NULL)
    {
        fclose(file);
    }

    return 0;
}