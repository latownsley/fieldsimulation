# Field Simulation (OpenMP)

A small multi-threaded predator-prey simulation: rye grass grows with the weather, rabbits eat the grass, and wolves eat the rabbits. It runs month by month from January 2026 through December 2030 and prints one line per month

## What it models

- **Weather:** temperature and rainfall follow seasonal sine curves plus random noise, recomputed each month.
- **Rye grass:** grows each month by a factor that is highest near 60 °F and 14 inches of rain, and loses 1 inch per rabbit.
- **Rabbits:** the carrying capacity is the grass height in inches. Each month the population moves one step toward it.
- **Wolves:** gain population from the rabbits they eat and lose a fraction of the population each month. Each wolf can only eat so many rabbits per month (a simplified Rosenzweig-MacArthur cap).

Start state: 100 rabbits, 300 inches of grass, January 2026.

## How it is parallelized

Four threads live for the whole run, one per agent. Each month has three phases, separated by `#pragma omp barrier`:

```
        T0 Rabbits       T1 RyeGrass      T2 Wolves        T3 Watcher
        ----------       -----------      ---------        ----------
COMPUTE read state       read state       read state       (idle)
        -> next rabbits  -> next height   -> next wolves
        -> next kills
   ============================ barrier ============================
ASSIGN  write rabbits    write height     write wolves     (idle)
        and kills
   ============================ barrier ============================
PRINT   (idle)           (idle)           (idle)           print the month,
                                                           advance the calendar,
                                                           draw next month's weather
   ============================ barrier ============================
                                next month
```

Rules:

- In the compute phase, threads only read shared state and write to local variables.
- In the assign phase, each shared variable has exactly one writer.
- Only the Watcher thread prints, moves the calendar, and uses the random number engine.
- The thread-to-agent mapping is fixed (by thread id), so a given seed always gives the same run.


## Build and run

Requires a C++17 compiler with OpenMP (tested with g++).

```
g++ -std=c++17 -fopenmp -O2 -Wall -Wextra -o field_simulation field_simulation.cpp
./field_simulation            # random seed (printed to stderr)
./field_simulation 12345      # fixed seed, repeatable run
```

Output goes to stdout, one line per month: `grass height, rabbits, wolves, precip, temp, month/year`. The first data row is the starting state. The seed and any error message go to stderr, so stdout can be compared between runs.

## Testing

```
python3 field_sim_test.py field_simulation.cpp
```

The test compiles the program, runs it twice with the same seed (outputs must be identical), and once with a different seed (output must differ). It exits with a non-zero code if a run fails, produces no output, or either comparison does not hold.

A pass means the same seed reproduced the same output in those runs. 


## Limitations

- With four threads and tiny work per phase, it demonstrates functional decomposition and barrier phases, not speedup.
- `Rabbits()` and `Wolves()` use the kill count computed from the previous month's state, so there is a one-month delay. A fix would be to pass this month's kill count as a parameter. The difference in output is about one rabbit.
- The state lives in global variables, which is simple but means only one simulation can run per process.

## Updates

- **Barrier:** the hand-built barrier (an `omp_lock_t`, `volatile` counters and busy-waiting) is replaced with `#pragma omp barrier`. 
- **Lock leak:** the old code called `omp_init_lock` every month and never called `omp_destroy_lock`. The lock is gone now, so the leak is too.
- **Randomness:** `rand()` and `srand(time(0))` are replaced with one `std::mt19937` engine, passed by reference and used only by the Watcher thread. A seed can be given on the command line to repeat a run. The first version of this fix passed the engine by value, so every month got the same copy of the same state; passing by reference fixed it.
- **Thread check:** the program asks for exactly 4 threads and stops with an error message and a non-zero exit code if the runtime gives fewer.
- **Removed the "acts of God" agent.** In the original, two agents wrote the rabbit count in the same phase, which was a race. I cut that agent rather than fix it, then added wolves as a new agent with a single writer per variable.
- **Population rules:** the original rabbit rule changed the population by +1 or -1 a month, so rabbits could not recover from anything and the model collapsed within months. The new rules are continuous, and their constants come from solving for an equilibrium instead of tuning by trial.

