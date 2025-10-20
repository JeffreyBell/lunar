// Translation of
// <http://www.cs.brandeis.edu/~storer/LunarLander/LunarLander/LunarLanderListing.jpg>
// by Jim Storer from FOCAL to C.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdbool.h>

// Coming soon: turn outcomes.
typedef enum  {
    NORMAL,
    FUELOUT,
    IMPACT
} TurnResult;

// Global variables
//
// A - Alt - Altitude (miles)
// G - Gravitational constant
// I - NextAlt - Intermediate altitude (miles)
// J - NextV - Intermediate velocity (miles/sec)
// K - Fuel rate (lbs/sec)
// L - Elapsed - Elapsed time (sec)
// M - Mass -  Total weight (lbs)
// N - EmptyWeight - Empty weight (lbs, Note: M - N is remaining fuel weight)
// SubTimestep - Time elapsed in current 10-second turn (sec)
// T - FullTimestep - Time remaining in current 10-second turn (sec)
// V - Downward speed (miles/sec)
// Z - SpecificImpulse Thrust per pound of fuel burned

static double Alt, NextAlt, NextV, K, Elapsed, Mass, EmptyWeight, SubTimestep, FullTimestep, V;

// physical constants
static const double G = .001;
static const double SpecificImpulse = 1.8;

static bool echo_input = false;

static void update_lander_state(void);
static void apply_thrust(void);

static void play_a_game(void);
static void report_landing(void);

static void print_status(void);

// Input routines (substitutes for FOCAL ACCEPT command)
static void prompt_for_k(void);
static int accept_double(double *value);
static int accept_yes_or_no(void);
static void accept_line(char **buffer, size_t *buffer_length);



int main(int argc, const char **argv) {
    if (argc > 1)
    {
        // If --echo is present, then write all input back to standard output.
        // (This is useful for testing with files as redirected input.)
        if (strcmp(argv[1], "--echo") == 0)
            echo_input = true;
    }

    // Print Preamble
    puts("CONTROL CALLING LUNAR MODULE. MANUAL CONTROL IS NECESSARY");
    puts("YOU MAY RESET FUEL RATE K EACH 10 SECS TO 0 OR ANY VALUE");
    puts("BETWEEN 8 & 200 LBS/SEC. YOU'VE 16000 LBS FUEL. ESTIMATED");
    puts("FREE FALL IMPACT TIME-120 SECS. CAPSULE WEIGHT-32500 LBS\n\n");

    do {
        play_a_game();

        puts("CONTROL OUT\n\n");
        puts("\n\n\nTRY AGAIN?");
    } while (accept_yes_or_no());

    exit(0);
} // main


static void play_a_game(void) {
    puts("FIRST RADAR CHECK COMING UP\n\n");
    puts("COMMENCE LANDING PROCEDURE");
    puts("TIME,SECS   ALTITUDE,MILES+FEET   VELOCITY,MPH   FUEL,LBS   FUEL RATE");

    Alt = 120;
    V = 1;
    Mass = 32500;
    EmptyWeight = 16500;
    Elapsed = 0;

start_turn:

    print_status();
    prompt_for_k();

    FullTimestep = 10;

turn_loop:
    for (;;)
    {
        if (Mass - EmptyWeight < .001)
            goto fuel_out;

        if (FullTimestep < .001)
            goto start_turn;

        SubTimestep = FullTimestep;

        // Gonna run out of fuel this step.
        if (EmptyWeight + SubTimestep * K - Mass > 0)
            SubTimestep = (Mass - EmptyWeight) / K;

        apply_thrust();

        if (NextAlt <= 0)
            goto loop_until_on_the_moon; // impact

        // Special case where we swoop.
        // We reversed direction and started going back up.
        // We might hit ground in the midddle so subdivide the subtime.
        if ((V > 0) && (NextV < 0))
        {
            for (;;)
            {
                double W = (1 - Mass * G / (SpecificImpulse * K)) / 2;
                SubTimestep = Mass * V / (SpecificImpulse * K * (W + sqrt(W * W + V / SpecificImpulse))) + 0.5;
                apply_thrust();
                if (NextAlt <= 0)
                    goto loop_until_on_the_moon; // impact
                update_lander_state();
                if (NextV > 0)
                    goto turn_loop;  // Going Down next.  Normal.
                if (V <= 0)
                    goto turn_loop;
            }
        }

        update_lander_state();
    }

loop_until_on_the_moon:
    while (SubTimestep >= .005)
    {
        SubTimestep = 2 * Alt / (V + sqrt(V * V + 2 * Alt * (G - SpecificImpulse * K / Mass)));
        apply_thrust();
        update_lander_state();
    }
    goto on_the_moon;

fuel_out:
    printf("FUEL OUT AT %8.2f SECS\n", Elapsed);
    SubTimestep = (sqrt(V * V + 2 * Alt * G) - V) / G;
    V += G * SubTimestep;
    Elapsed += SubTimestep;

    // Report landing quality.
on_the_moon:
    report_landing();
    }


// Update next states to the game state
void update_lander_state(void) {
    Elapsed += SubTimestep;
    FullTimestep -= SubTimestep;
    Mass -= SubTimestep * K;
    Alt = NextAlt;
    V = NextV;
}

// Calculate next state
void apply_thrust(void) {
    // Taylor series?
    double Q = SubTimestep * K / Mass;
    double Q_2 = pow(Q, 2);
    double Q_3 = pow(Q, 3);
    double Q_4 = pow(Q, 4);
    double Q_5 = pow(Q, 5);

    NextV = V + G * SubTimestep + SpecificImpulse * (-Q - Q_2 / 2 - Q_3 / 3 - Q_4 / 4 - Q_5 / 5);
    NextAlt = Alt - G * SubTimestep * SubTimestep / 2 - V * SubTimestep + SpecificImpulse * SubTimestep * (Q / 2 + Q_2 / 6 + Q_3 / 12 + Q_4 / 20 + Q_5 / 30);
}

void report_landing(void) {
    printf("ON THE MOON AT %8.2f SECS\n", Elapsed);
    double Mph = 3600 * V;
    printf("IMPACT VELOCITY OF %8.2f M.P.H.\n", Mph);
    printf("FUEL LEFT: %8.2f LBS\n", Mass - EmptyWeight);
    if (Mph <= 1)
        puts("PERFECT LANDING !-(LUCKY)");
    else if (Mph <= 10)
        puts("GOOD LANDING-(COULD BE BETTER)");
    else if (Mph <= 22)
        puts("CONGRATULATIONS ON A POOR LANDING");
    else if (Mph <= 40)
        puts("CRAFT DAMAGE. GOOD LUCK");
    else if (Mph <= 60)
        puts("CRASH LANDING-YOU'VE 5 HRS OXYGEN");
    else
    {
        puts("SORRY,BUT THERE WERE NO SURVIVORS-YOU BLEW IT!");
        printf("IN FACT YOU BLASTED A NEW LUNAR CRATER %8.2f FT. DEEP\n", Mph * .277777);
    }
}

void print_status(void) {
    printf("%7.0f%16.0f%7.0f%15.2f%12.1f      ",
            Elapsed,
            trunc(Alt),
            5280 * (Alt - trunc(Alt)),
            3600 * V,
            Mass - EmptyWeight);
}

// Give some hints.
void prompt_for_k(void) {
    bool first_time = true;
    while (true) {
        if (!first_time) {
            // Get all the way over to the right column again
            fputs("NOT POSSIBLE", stdout);
            for (int x = 1; x <= 51; ++x)
                putchar('.');
        }
        first_time = false;
        fputs("K=:", stdout);
        int is_valid_input = accept_double(&K);
        if (!is_valid_input)
            continue;
        if (K < 0) {
            fputs("No Negative Numbers\n", stdout);
            continue;
        }
        if ((0 < K) && (K < 8)) {
            fputs("Minimum nonzero thrust is 8.\n", stdout);
            continue;
        }
        if ( K > 200) {
            fputs("Too Big\n", stdout);
            continue;
        }
        break;
    }
    // Fall through on success.
}

// Read a floating-point value from stdin.
//
// Returns 1 on success, or 0 if input did not contain a number.
//
// Calls exit(-1) on EOF or other failure to read input.
int accept_double(double *value) {
    char *buffer = NULL;
    size_t buffer_length = 0;
    accept_line(&buffer, &buffer_length);
    int is_valid_input = sscanf(buffer, "%lf", value);
    free(buffer);
    if (is_valid_input != 1)
        is_valid_input = 0;
    return is_valid_input;
}

// Reads input and returns 1 if it starts with 'Y' or 'y', or returns 0 if it
// starts with 'N' or 'n'.
//
// If input starts with none of those characters, prompt again.
//
// If unable to read input, calls exit(-1);
int accept_yes_or_no(void) {
    int result = -1;
    do
    {
        fputs("(ANS. YES OR NO):", stdout);
        char *buffer = NULL;
        size_t buffer_length = 0;
        accept_line(&buffer, &buffer_length);

        if (buffer_length > 0)
        {
            switch (buffer[0])
            {
            case 'y':
            case 'Y':
                result = 1;
                break;
            case 'n':
            case 'N':
                result = 0;
                break;
            default:
                break;
            }
        }

        free(buffer);
    } while (result < 0);

    return result;
}

// Reads a line of input.  Caller is responsible for calling free() on the
// returned buffer.
//
// If unable to read input, calls exit(-1).
void accept_line(char **buffer, size_t *buffer_length) {
    if (getline(buffer, buffer_length, stdin) == -1)
    {
        fputs("\nEND OF INPUT\n", stderr);
        exit(-1);
    }

    if (echo_input)
    {
        fputs(*buffer, stdout);
    }
}
