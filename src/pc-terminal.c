/*
Name: Akshay Gopinath
Code for the host computer
*/

/* ####################################################
Compile using:
gcc assignment2_final.c rs232.c -Wall -Wextra -o2 -o assignment2
##################################################### */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#ifdef _WIN32
#include <Windows.h>
#else
#include <unistd.h>
#endif
#include "rs232.h"

#define SERIAL_MAX 4095

//wait_ms() is used for create a small delay where required. 
void wait_ms(int ms) {
#ifdef _WIN32
    Sleep(ms);
#else
    usleep(ms*1000);  
#endif
}

//read_serial() uses RS232_PollComport to read from serial port
int read_serial(int cport_nr, unsigned char *buf) {
    int n = 0;

    // Block further execution until a character is received
    while (n == 0)
        n += RS232_PollComport(cport_nr, buf, 1);

    //here, it keeps on receiving till nothing is left to receive
    int n2 = -1;
    while (n2 != 0 && n < SERIAL_MAX) {
        n2 = RS232_PollComport(cport_nr, buf+n, SERIAL_MAX-n);
        n += n2;
        
        // Prevents a single message being interpretted as 2 separate ones.
        // Which happens in case of mbed following a board reset.
        wait_ms(50); 
    }
    buf[n] = 0;
    return n; //returns the number of bytes received
}

int main() {
   
    // For example:
    //    Linux   - /dev/ttyS0
    //    Windows - COM1
    int cport_nr = RS232_GetPortnr("COM4"); //gets the COM Port number
    if (cport_nr == -1) {
        puts("Didn't find specified port. Exiting...\n");
        return 1; //code exits if port is not found
    }

    // Baud and mode below work well with mbed default setting
    int bdrate = 9600; 
    char mode[] = {'8','N','1',0}; 

    // Guess typed in command prompt
    char cmd_input[256]; 

    // Data received from mbed
    unsigned char serial_input[4095] = {'0'};

    if(RS232_OpenComport(cport_nr, bdrate, mode, 0)) {
        printf("Can not open comport\n");
        return(0); //code exits of com port cannot be opened
    }

    //prints game rules
    printf("\nGAME RULES:\n>All numbers MUST be less than or equal to 30\n>Press C on mbed to enter the game\n>Enter the difficulty using the first three switches first and press C\n>Score starts at 100\n>If more than one switch is on, the latest input is taken as set difficulty\n>Enter the secret number using the keypad and press C\n>Enjoy the game!!!\n>All LEDs will be red if you lose!\n>LEDs will flash alternate red and green (christmas lights) if you win!\n>Final score is displayed on the mbed's LCD\n");
    printf("----------------------------------------------------------------------------------\n");

    //prints the scoring system
    printf("SCORING SYSTEM\n");

    printf("|------------+------------+-------------|\n");
    printf("| Difficulty | Good Guess |  Bad Guess  |\n");
    printf("|------------+------------+-------------|\n");
    printf("|    Low     |     -2     |     -5      |\n");
    printf("|   Medium   |     -5     |     -10     |\n");
    printf("|    Hard    |     -10    |     -20     |\n");
    printf("|------------+------------+-------------|\n\n");

    //informs the user that its waiting for difficulty and secret number
    puts("Waiting for user to enter difficulty and secret number...\n");

    //This part is when it receives the confirmation message from mbed.
    int n = read_serial(cport_nr, serial_input);

    if(n > 0)
    {
        printf("##################################################################################\n\n");
        printf("Welcome to the Guessing Game!!!\n\n"); 
        //Prints welcome to guessing game
    }

    while (1) {
        // read_serial blocks further execution until it receives a message

        //asks for guess, and uses scanf() to enter guess
        printf("What is your guess: ");
        scanf("%s", cmd_input);

        RS232_cputs(cport_nr, cmd_input); //Sends guess to mbed

        //This is for the two way handshake, mbed sends the guess received
        n = read_serial(cport_nr, serial_input);

        //If mbed computes guess as inavlid and PC receives invalid input
        if(!strcmp((const char*)serial_input, "Renter"))
        {
            printf("[!]Invalid input\n\n");
            fflush(stdin); //stdin flushed
            continue; //Goes back to begenning of loop
        }

        printf("Received %d bytes\n", n); //prints number of bytes
        printf("Received guess = %s\n", serial_input); //and received guess

        //Gets another message from mbed
        n = read_serial(cport_nr, serial_input);

        //*p checks if the message is a lose message
        char *p = strstr((const char*)serial_input, "Lose");

        if(p) //if *p is not a null pointer
        {
            printf("%s", serial_input); //prints lose message received
            break; //exits the loop
        }

        //checks if no win message is received
        if(strcmp((const char*)serial_input, "You Win!\n"))
            printf("Your guess is: %s\n", serial_input); //prints mbed message
        else
        {
            printf("%s", serial_input); //else prints the win message
            break; //exits the loop
        }

        fflush(stdin); //stdin is flushed
    }

    RS232_CloseComport(cport_nr); //comport closed
    return(0); //code returns 0 and exits
}