/** Farm Simulation
 */

#define _USE_MATH_DEFINES
#include <omp.h>
#include <cstdio>
#include <math.h>
#include <iostream>
#include <random>
#include <string>
#include <algorithm>



/************ GLOBAL VARIABLES ********** */
int	g_curr_year;		    // 2026 - 2031    
int	g_curr_month;		    // 0 - 11
double	g_curr_num_rabbits;		// number of rabbits in the current population
double g_curr_wolves;          // number of wolves in the current population
double g_devoured_rabbits;      // number of rabbits eaten. 

float g_curr_precip;		// inches of rain per month
float g_curr_temp;		// temperature this month
float g_curr_height;		// rye grass height in inches


/************ GLOBAL CONSTANTS ********** */

const float RYEGRASS_GROW_RATE = 20.0;   // per month
const float RABBIT_EAT_RATE = 0.057;      // per rabbit

const float RABBIT_GROWTH_RATE = 0.1;   // trying a logistic growth rate instead of just adding 1
const float WOLF_CONVERT_RATE = 0.20;    // rate at which wovles convert food to more wolves
const float WOLF_SKILL_RATE = 0.005;    // how good they are at hunting rabbits
const float WOLF_DEATH_RATE = 0.10;
const float MAX_WOLF_KILLS = 3;

const float AVG_PRECIP = 12.0;			// average precip per month
const float AMP_PRECIP = 4.0;	// plus or minus / AMP precip per month
const float RANDOM_PRECIP = 2.0;	// plus or minus noise

const float AVG_TEMP = 60.0;	// average
const float AMP_TEMP = 20.0;	// plus or minus
const float RANDOM_TEMP = 10.0;	// plus or minus noise

const float MIDTEMP = 60.0;
const float MIDPRECIP = 14.0;

/************ FUNCTION PROTOTYPES ********** */
double Rabbits();
float RyeGrass();
void Watcher(std::mt19937& rng);
float Sqr(float);
void CalcWeather(std::mt19937& rng);
double Wolves();
double DevourRabbits();


/************ MAIN ********** */

int main(int argc, char* argv[]) {
    // starting state 
	g_curr_month = 0;
	g_curr_year = 2026;
	g_curr_num_rabbits = 100;
    g_curr_wolves = 13;
	g_curr_height = 300.0;
    g_devoured_rabbits = 0.0;
    g_devoured_rabbits = DevourRabbits();
    bool error = false;

    // initialize with seed (random or from commandline)
    unsigned int seed = std:: random_device{}();
    if (argc >= 2 ) {
        seed = static_cast<unsigned int>(std::stoul(argv[1]));
    }
    std::cerr << seed << "\n";      // for testing purposes
    std::mt19937 rng(seed);

    // calculate current enviormental parameters
	CalcWeather(rng);

    //print starting state
    std::cout << "Rye Height" << ", " << "Rabbits" << ", "  << "Wolves" << ", " << "Precip" << ", " << "Temp" << ", " << "Month" << "/" << "Year" << std::endl;
        std::cout << "------" << ", " << "-----" << ", "  << "-----" << ", " << "-----" << ", " << "-----" << ", " << "-----" << "/" << "-----" << std::endl;
    std::cout << g_curr_height << ", " << static_cast<int>(std::round(g_curr_num_rabbits)) << ", "  << static_cast<int>(std::round(g_curr_wolves)) << ", " << g_curr_precip << ", " << g_curr_temp << ", " << "Starting States" << std::endl;


    // set up threads
    omp_set_dynamic(0);             // ask not to shrink threads 
    #pragma omp parallel num_threads(4)  
    {
        // threads check. Make sure enough to run
        bool pass_check = true;

        if (omp_get_num_threads() != 4) {
            pass_check = false;

            int id = omp_get_thread_num();

            if (id == 0) {
                std::cerr << "Error: Not enough threads.\n";
                error= true;
            }

        } 

        // simulation state
        while (pass_check && g_curr_year < 2031) {
            int id = omp_get_thread_num();

            // local variable holders
            double next_num_rabbits = g_curr_num_rabbits;			
            float next_height = g_curr_height;			
            double next_num_wolves = g_curr_wolves;        
            double next_num_eaten_rabbits = g_devoured_rabbits;

            // Read State
            if (id == 0) {
                next_num_eaten_rabbits = DevourRabbits();
                next_num_rabbits = Rabbits();
            }
            if (id == 1) {
                next_height = RyeGrass();
            }
            if (id == 2) {
                next_num_wolves = Wolves();
            }

            #pragma omp barrier                      // all reads done

            // Write State 
            if (id == 0) {
                g_curr_num_rabbits = next_num_rabbits;
                g_devoured_rabbits = next_num_eaten_rabbits;   
            }
            if (id == 1) {
                g_curr_height = next_height;
            }
            if (id == 2) {
                g_curr_wolves = next_num_wolves;
            }

            #pragma omp barrier                      // all writes visible

            // Print State

            if (id == 3) {
                Watcher(rng);
            }
            #pragma omp barrier                      // print done

        }
        
    }

    if (error) {
        return 1;
    }
    
    return 0;
}

float Sqr(float x)
{
	return x * x;
}

void CalcWeather(std::mt19937& rng) {

	// calculate current enviormental parameters
	float ang = (30. * (float)g_curr_month + 15.) * (M_PI / 180.);

	float temp = AVG_TEMP - AMP_TEMP * cos(ang);		// holds temp
	std::uniform_real_distribution<double> dist(-RANDOM_TEMP, RANDOM_TEMP);
	g_curr_temp = temp + dist(rng);

	float precip = AVG_PRECIP + AMP_PRECIP * sin(ang);	//hold precipitation
	std::uniform_real_distribution precip_dist(-RANDOM_PRECIP, RANDOM_PRECIP);
	g_curr_precip = precip + precip_dist(rng);

	if (g_curr_precip < 0.)
		g_curr_precip = 0.;

}

void Watcher(std::mt19937& rng) {

    //print results
    std::cout << g_curr_height << ", " << static_cast<int>(std::round(g_curr_num_rabbits)) << ", "  << static_cast<int>(std::round(g_curr_wolves)) << ", " << g_curr_precip << ", " << g_curr_temp << ", " << g_curr_month + 1 << "/" << g_curr_year << std::endl;

    //increment time
    g_curr_month += 1;
    if (g_curr_month == 12) {
        g_curr_month = 0;
        g_curr_year += 1;
    }

    //calculate new enviormental parameters
    CalcWeather(rng);

}

double Wolves() {
    double next_num_wolves = g_curr_wolves;

     // Wolves = current wolves + conversion rate * eaten rabbits - death rate * wolves (gain from what they eat and lose a fraction each month)
    next_num_wolves = next_num_wolves + (WOLF_CONVERT_RATE * g_devoured_rabbits) - (WOLF_DEATH_RATE * next_num_wolves);

    if (next_num_wolves < 0) {
        next_num_wolves = 0;
    }

    return next_num_wolves;
}

double DevourRabbits() {
    double kills_per_wolf = fmin(WOLF_SKILL_RATE * g_curr_num_rabbits, MAX_WOLF_KILLS); // simplified Rosenzweig–MacArthur 

    // use min to prevent them from eating rabbits that don't exist
    return fmin(g_curr_num_rabbits, g_curr_wolves * kills_per_wolf);
}

double Rabbits() {		

    double next_num_rabbits = g_curr_num_rabbits;			// carries next num of rabbits in a local variable
    double carrying_capacity = (double)(g_curr_height);		// Carrying Capacity of the rabbits is the number of inches of height of the rye grass

    // rabbits logistic growth algorithm
    next_num_rabbits = next_num_rabbits + (RABBIT_GROWTH_RATE * next_num_rabbits * (1 - (next_num_rabbits / carrying_capacity)) - g_devoured_rabbits);

    if (next_num_rabbits < 0) {
        next_num_rabbits = 0;
    }

    return next_num_rabbits;

}

float RyeGrass() {

    float next_height = g_curr_height;	

    //see if conditions are good (ideal temp and precip)
    float temp_factor = exp(-Sqr((g_curr_temp - MIDTEMP) / 10.));
    float precip_factor = exp(-Sqr((g_curr_precip - MIDPRECIP) / 10.));

    next_height += temp_factor * precip_factor * RYEGRASS_GROW_RATE;
    next_height -= (float)g_curr_num_rabbits * RABBIT_EAT_RATE;

    if (next_height < 0.) {
        next_height = 0.;
    }
    
    return next_height;

}
