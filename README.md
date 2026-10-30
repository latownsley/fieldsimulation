# fieldsimulation
Simulation using OpenMP

# Diagram
#pragma omp parallel num_threads(4)      <- one region, 4 threads, lives for the whole run
for each month:

 T0 Rabbits   T1 RyeGrass   T2 Watcher    T3 ActOfGod
 ---------------------------------------------------------
 read state   read state    (idle)        read state
 -> next_*    -> next_*                   -> next_*
 ---------------------------------------------------------
 ============== #pragma omp barrier  (DoneComputing) =====
 ---------------------------------------------------------
 write own    write own     (idle)        write own
 variable     variable                    (order must be fixed)
 ---------------------------------------------------------
 ============== #pragma omp barrier  (DoneAssigning) =====
 ---------------------------------------------------------
 (idle)       (idle)        print line,   (idle)
                            month++,
                            new weather
 ---------------------------------------------------------
 ============== #pragma omp barrier  (DonePrinting) ======
