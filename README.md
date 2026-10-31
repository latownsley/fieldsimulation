# Field Simulation (OpenMP)

A small multi-threaded simulation of a field: rye grass grows with the weather, and rabbits eat it. It runs month by month from January 2026 through December 2030 and prints one line per month.

## What it models

- **Weather:** temperature and rainfall follow seasonal sine curves plus random noise, recomputed each month.
- **Rye grass:** grows each month by a factor that is highest near 60 °F and 14 inches of rain, and loses 1 inch per rabbit.
- **Rabbits:** the carrying capacity is the grass height in inches. Each month the population moves one step toward it.

Start state: 100 rabbits, 300 inches of grass, January 2026.

## How it is parallelized

Three threads live for the whole run, one per agent. Each month has three phases, separated by `#pragma omp barrier`:

```
        T0 Rabbits        T1 RyeGrass        T2 Watcher
        ----------        -----------        ----------
COMPUTE read state        read state         (idle)
        -> next count     -> next height
   ======================= barrier =======================
ASSIGN  write rabbits     write height       (idle)
   ======================= barrier =======================
PRINT   (idle)            (idle)             print the month,
                                             advance the calendar,
                                             draw next month's weather
   ======================= barrier =======================
                         next month
```

## Build and run

Requires a C++17 compiler with OpenMP (tested with g++).

```
g++ -std=c++17 -fopenmp -O2 -Wall -Wextra -o field_simulation field_simulation.cpp
./field_simulation            # random seed, printed at the start
./field_simulation 12345      # fixed seed, repeatable run
```

Each output line is: `grass height, rabbits, precip, temp, month/year`.

## Limitations

- With three threads and tiny work per phase, it demonstrates functional decomposition and barrier phases, not speedup.
- The state lives in global variables, which is simple but means only one simulation can run per process.
- The same seed gives the same run on the same compiler and standard library. The C++ standard fixes `mt19937`'s output but not `uniform_real_distribution`'s, so numbers may differ across toolchains.
