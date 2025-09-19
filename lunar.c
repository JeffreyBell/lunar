// Translation of
// <http://www.cs.brandeis.edu/~storer/LunarLander/LunarLander/LunarLanderListing.jpg>
// by Jim Storer from FOCAL to C.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

// Global variables
//
// A - Alt - Altitude (miles)
// G - Gravitational constant
// I - NextAlt - Intermediate altitude (miles)
// J - NextV - Intermediate velocity (miles/sec)
// K - Fuel rate (lbs/sec)
// L - Elapsed - Elapsed time (sec)
// M - Mass -  Total weight (lbs)
// N - Fuel - Empty weight (lbs, Note: M - N is remaining fuel weight)
// S - Time elapsed in current 10-second turn (sec)
// T - Timestep - Time remaining in current 10-second turn (sec)
// V - Downward speed (miles/sec)
// Z - SpecificImpulse Thrust per pound of fuel burned

static double Alt, NextAlt, NextV, K, Elapsed, Mass, Fuel, S, Timestep, V;

// physical constants
static const double G = .001;
static const double SpecificImpulse = 1.8;

static int echo_input = 0;

static void update_lander_state();
static void apply_thrust();

static void play_a_game();
static void report_landing();

// Input routines (substitutes for FOCAL ACCEPT command)
static void prompt_for_k();
static int accept_double(double *value);
static int accept_yes_or_no();
static void accept_line(char **buffer, size_t *buffer_length);

int main(int argc, const char **argv)
{
    if (argc > 1)
    {
        // If --echo is present, then write all input back to standard output.
        // (This is useful for testing with files as redirected input.)
        if (strcmp(argv[1], "--echo") == 0)
            echo_input = 1;
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
    return 0;
} // main


static void play_a_game(){
    puts("FIRST RADAR CHECK COMING UP\n\n");
    puts("COMMENCE LANDING PROCEDURE");
    puts("TIME,SECS   ALTITUDE,MILES+FEET   VELOCITY,MPH   FUEL,LBS   FUEL RATE");

    Alt = 120;
    V = 1;
    Mass = 32500;
    Fuel = 16500;
    Elapsed = 0;

start_turn: // 02.10 in original FOCAL code
    printf("%7.0f%16.0f%7.0f%15.2f%12.1f      ",
            Elapsed,
            trunc(Alt),
            5280 * (Alt - trunc(Alt)),
            3600 * V,
            Mass - Fuel);

    prompt_for_k();

    Timestep = 10;

turn_loop:
    for (;;) // 03.10 in original FOCAL code
    {
        if (Mass - Fuel < .001)
            goto fuel_out;

        if (Timestep < .001)
            goto start_turn;

        S = Timestep;

        if (Fuel + S * K - Mass > 0)
            S = (Mass - Fuel) / K;

        apply_thrust();

        if (NextAlt <= 0)
            goto loop_until_on_the_moon;

        // Special case where were swoop.
        // We reversed direction and started going back up.
        if ((V > 0) && (NextV < 0))
        {
            for (;;) // 08.10 in original FOCAL code
            {
                // FOCAL-to-C gotcha: In FOCAL, multiplication has a higher
                // precedence than division.  In C, they have the same
                // precedence and are evaluated left-to-right.  So the
                // original FOCAL subexpression `M * G / Z * K` can't be
                // copied as-is into C: `Z * K` has to be parenthesized to
                // get the same result.
                double W = (1 - Mass * G / (SpecificImpulse * K)) / 2;
                S = Mass * V / (SpecificImpulse * K * (W + sqrt(W * W + V / SpecificImpulse))) + 0.5;
                apply_thrust();
                if (NextAlt <= 0)
                    goto loop_until_on_the_moon;
                update_lander_state();
                if (-NextV < 0)
                    goto turn_loop;
                if (V <= 0)
                    goto turn_loop;
            }
        }

        update_lander_state();
    }

loop_until_on_the_moon: // 07.10 in original FOCAL code
    while (S >= .005)
    {
        S = 2 * Alt / (V + sqrt(V * V + 2 * Alt * (G - SpecificImpulse * K / Mass)));
        apply_thrust();
        update_lander_state();
    }
    goto on_the_moon;

fuel_out: // 04.10 in original FOCAL code
    printf("FUEL OUT AT %8.2f SECS\n", Elapsed);
    S = (sqrt(V * V + 2 * Alt * G) - V) / G;
    V += G * S;
    Elapsed += S;

    // Report landing quality.
on_the_moon:
    report_landing();
    }



// Subroutine at line 06.10 in original FOCAL code
void update_lander_state()
{
    Elapsed += S;
    Timestep -= S;
    Mass -= S * K;
    Alt = NextAlt;
    V = NextV;
}

// Subroutine at line 09.10 in original FOCAL code
void apply_thrust()
{
    // Taylor series?
    double Q = S * K / Mass;
    double Q_2 = pow(Q, 2);
    double Q_3 = pow(Q, 3);
    double Q_4 = pow(Q, 4);
    double Q_5 = pow(Q, 5);

    NextV = V + G * S + SpecificImpulse * (-Q - Q_2 / 2 - Q_3 / 3 - Q_4 / 4 - Q_5 / 5);
    NextAlt = Alt - G * S * S / 2 - V * S + SpecificImpulse * S * (Q / 2 + Q_2 / 6 + Q_3 / 12 + Q_4 / 20 + Q_5 / 30);
}

void report_landing()
{
    printf("ON THE MOON AT %8.2f SECS\n", Elapsed);
    double Mph = 3600 * V;
    printf("IMPACT VELOCITY OF %8.2f M.P.H.\n", Mph);
    printf("FUEL LEFT: %8.2f LBS\n", Mass - Fuel);
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

// Give some hints.
void prompt_for_k(){
    int first_time = 1;
    while (1) {
        if (!first_time) {
            // Get all the way over to the right column again
            fputs("NOT POSSIBLE", stdout);
            for (int x = 1; x <= 51; ++x)
                putchar('.');
        }
        first_time=0;
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
int accept_double(double *value)
{
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
int accept_yes_or_no()
{
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
void accept_line(char **buffer, size_t *buffer_length)
{
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
