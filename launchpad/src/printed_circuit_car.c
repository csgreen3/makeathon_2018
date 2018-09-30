#include <stdio.h>
#include <stdlib.h>
#include "printed_circuit_car.h"

void turn_left(void)
{
    printf("%s called\r\n", __func__);
}

void turn_right(void)
{
    printf("%s called\r\n", __func__);
}

void stop(void)
{
    printf("%s called\r\n", __func__);
}

void move_forward(int speed)
{
    printf("%s called (%d)\r\n", __func__, speed);
}

void move_backward(int speed)
{
    printf("%s called (%d)\r\n", __func__, speed);
}

void help(void)
{
    printf("Options:\r\n");
    printf("[s]top                   - stop moving\r\n");
    printf("[l]eft                   - turn left\r\n");
    printf("[r]ight                  - turn right\r\n");
    printf("[f]orward  <percent_max> - move forward with throttle\r\n");
    printf("[b]ackward <percent_max> - move backward with throttle\r\n");
}

void entry(int argc, char **argv)
{
    if (argc == 1)
    {
        if (argv[0][0] == 's') stop();
        else if (argv[0][0] == 'l') turn_left();
        else if (argv[0][0] == 'r') turn_right();
        else help();
    }
    else if (argc == 2)
    {
        int throttle = strtol(argv[1], NULL, 10);
        if (throttle > 100) throttle = 100;
        if (argv[0][0] == 'f') move_forward(throttle);
        else if (argv[0][0] == 'b') move_backward(throttle);
        else help();
    }
    else help();
}
