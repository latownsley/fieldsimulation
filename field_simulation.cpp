/** Farm Simulation
 */

#define _USE_MATH_DEFINES
#include <omp.h>
#include <cstdio>
#include <math.h>
#include <iostream>
#include <random>
#include <string>



/************ GLOBAL VARIABLES ********** */
int	g_curr_year;		    // 2026 - 2031    
int	g_curr_month;		    // 0 - 11
int	g_curr_num_rabbits;		// number of rabbits in the current population

float g_curr_precip;		// inches of rain per month
float g_curr_temp;		// temperature this month
float g_curr_height;		// rye grass height in inches


/************ GLOBAL CONSTANTS ********** */

const float RYEGRASS_GROW_RATE = 20.0;   // ryegrass grows per month
const float RABBIT_EAT_RATE = 1.0;  // 1 rabbit eats per month

const float AVG_PRECIP = 12.0;			// average precip per month
const float AMP_PRECIP = 4.0;	// plus or minus / AMP precip per month
const float RANDOM_PRECIP = 2.0;	// plus or minus noise

const float AVG_TEMP = 60.0;	// average
const float AMP_TEMP = 20.0;	// plus or minus
const float RANDOM_TEMP = 10.0;	// plus or minus noise

const float MIDTEMP = 60.0;
const float MIDPRECIP = 14.0;

/************ FUNCTION PROTOTYPES ********** */
int Rabbits();
float RyeGrass();
void Watcher(std::mt19937& rng);
float Sqr(float);
void CalcWeather(std::mt19937& rng);


/************ MAIN ********** */

int main(int argc, char* argv[]) {
    // starting state 
	g_curr_month = 0;
	g_curr_year = 2026;
	g_curr_num_rabbits = 100;
	g_curr_height = 300.0;
    bool error = false;

    // initialize with seed (random or from commandline)
    unsigned int seed = std:: random_device{}();
    if (argc >= 2 ) {
        seed = static_cast<unsigned int>(std::stoul(argv[1]));
    }
    std::cout << seed << "\n";      // for testing purposes
    std::mt19937 rng(seed);

    // calculate current enviormental parameters
	CalcWeather(rng);

    // set up threads
    omp_set_dynamic(0);             // ask not to shrink threads 
    #pragma omp parallel num_threads(3)  
    {
        // threads check. Make sure enough to run
        bool pass_check = true;

        if (omp_get_num_threads() != 3) {
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

            // holders
            int next_num_rabbits = g_curr_num_rabbits;			// carries next num of rabbits in a local variable
            float next_height = g_curr_height;			// carries next height of Rye in local variable

            // Read State
            if (id == 0) {
                next_num_rabbits = Rabbits();   
            }
            if (id == 1) {
                next_height = RyeGrass();
            }

            #pragma omp barrier                      // all reads done

            // Write State 
            if (id == 0) {
                g_curr_num_rabbits = next_num_rabbits;   
            }
            if (id == 1) {
                g_curr_height = next_height;
            }

            #pragma omp barrier                      // all writes visible

            // Print State

            if (id == 2) {
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
		std::cout << g_curr_height << ", " << g_curr_num_rabbits << ", " << g_curr_precip << ", " << g_curr_temp << ", " << g_curr_month + 1 << "/" << g_curr_year << std::endl;

		//increment time
		g_curr_month += 1;
		if (g_curr_month == 12) {
			g_curr_month = 0;
			g_curr_year += 1;
		}

		//calculate new enviormental parameters
		CalcWeather(rng);

}

int Rabbits() 
{		

	    int next_num_rabbits = g_curr_num_rabbits;			// carries next num of rabbits in a local variable
		int carrying_capacity = (int)(g_curr_height);		// Carrying Capacity of the rabbits is the number of inches of height of the rye grass


		if (next_num_rabbits < carrying_capacity)
			next_num_rabbits++;
		else
			if (next_num_rabbits > carrying_capacity)
				next_num_rabbits--;

		if (next_num_rabbits < 0)
			next_num_rabbits = 0;

		return next_num_rabbits;

}

float RyeGrass() {

		float next_height = g_curr_height;	

		//see if conditions are good (ideal temp and precip)
		float tempFactor = exp(-Sqr((g_curr_temp - MIDTEMP) / 10.));
		float precipFactor = exp(-Sqr((g_curr_precip - MIDPRECIP) / 10.));

		next_height += tempFactor * precipFactor * RYEGRASS_GROW_RATE;
		next_height -= (float)g_curr_num_rabbits * RABBIT_EAT_RATE;

		if (next_height < 0.) {
            next_height = 0.;
        }
		
		return next_height;

}
