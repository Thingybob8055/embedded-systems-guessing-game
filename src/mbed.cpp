/*
Name: Akshay Gopinath
Registration Number: 2005614
Module: CE243 - C Programming with Embedded Systems
Assignment Number: 2
Code for the ARM Mbed
*/

/*Only first 3 switches are usable for difficulty
The latest switch input is taken as the difficulty
Difficulty 1(only left most switch on)
    - Bad Guess -> -5
    - Good Guess -> -2
Difficulty 2(2nd switch from left or first two switches on)
    - Bad Guess -> -10
    - Good Guess -> -5
Difficulty 3(3rd switch from left or all of first three switches on etc)
    - Bad Guess -> -20
    - Good Guess -> -10
*/

#include "mbed.h"
#include "TextLCD.h"
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#define SERIAL_MAX 255

TextLCD lcd(p15, p16, p17, p18, p19, p20);
Serial pc(USBTX, USBRX);

BusOut cols_out(p26, p25, p24); //Output scan the columns of the mbed keypad

BusOut switch_cs(p26, p25, p24);
BusIn switch_reading(p14,p13,p12,p11); //These two are for switches

SPI sw(p5, p6, p7); //For the SPI controlled LEDs

// lat (latch) pin of TLC59281 as shown in extension board schematic.
//Used for the switches
DigitalOut lat(p8);

//This is for the rows of the mbed keypad
BusIn rows_in(p14, p13, p12, p11);

char serial_input[SERIAL_MAX + 1] = {0};

// If function is blocking, it will wait till
// some data is received through serial.
int read_serial(char *buf, bool blocking=true) {
    if (blocking) {
        // Don't allow function to proceed until some data arrives through serial
        while (!pc.readable()) {}   
    }
    
    int count = 0;
    //Enters loop as long as something is receivable through the serial port
    while (pc.readable() && count < SERIAL_MAX) {    
        
        // Fast reading loop. Gets character by character and stores in buff
        while(pc.readable() && count < SERIAL_MAX)
            buf[count++] = pc.getc();
            
        // Allow little additional time for reliability.
        // Without it, the 1st received character would be 
        // treated as a separate message from the remaining ones.
        wait_ms(50);
    }
    buf[count] = 0; 
    return count; //returns the final count, total characters received
}

char Keytable[][4] = {
      {'1', '2', '3', 'F'},
      {'4', '5', '6', 'E'},
      {'7', '8', '9', 'D'},
      {'A', '0', 'B', 'C'}
    };
//The above variable stores all the inputs from the mbeds keypad

//This function is used to get a single character from the keypad
char getKey() {
    int i,j;
    for (i = 0; i <= 3; i++) {
        cols_out = i; //scans the columns
        
        // for each bit in rows
        for (j = 0; j <= 3; j++) {
            
            // if j'th bit of "rows_in" is LOW
            if (~rows_in & (1 << j)) {
                return Keytable[j][3-i]; //returns a character from the array
            }
        }
    }
    return ' '; //returns space if nothing is pressed
}

//This function is used to check if the guess from the host contains non-digits
int is_digit(const char *string)
{
    while (*string) 
    {
        if (isdigit(*string++) == 0) //isdigit() returns 0 if a non-string is found
            return 0;  //returns 0 if the string has non-digits
    }

    return 1; //else returns 1
}

//This function was made to get a multi-digit input from the mbed keypad
int numberEnter()
{
    int lcd_pos = 0; //tracks the LCD position
    char ch = ' '; //stores the keypad input
    char secret[256] = {0}; //keypad inputs are stored in this string (character array)
    int secret_num = 0; //stores the final number from atoi() conversion
    int i = 0; //for the index of the string
    wait(0.1); //a small delay for reliability
    while(1)
    {
        ch = getKey(); //Gets a character from keypad using getKey()
        if((ch != ' ') && (ch != 'C')) //If ch is not C or a space
        {
            if((ch == 'A') || (ch == 'B') || (ch == 'D') || (ch == 'E') || (ch == 'F')) //Any of these indicate invalid keypad input
            {
                lcd.locate(0 , 0);
                lcd.printf("Invalid Input!");
                wait(0.5); //prints invalid input for 0.5 seconds
                lcd.locate(0 , 0);
                lcd.printf("Secret Number:");
                ch = ' ';
                continue; //goes back to beginning of the while loop
            }
            secret[i] = ch; //stores in the character array if valid input
            lcd.locate(lcd_pos++,1); //sets LCD position
            lcd.printf("%c", ch); //prints the character on screen
            ch = ' '; //resets the character
            wait(0.2); //A short delay
            i = i+1; //increment the index by 1
        }
        else if(ch == 'C') //if c is pressed, exit the loop (number is entered)
        {
            wait(0.2); //small delay
            break;
        }
    }
    ch = ' '; //reset it
    secret[i+1] = '\0'; //append a null character to the string to make it valid
    secret_num = atoi(secret); //convert it to an integer
    lcd.cls(); //clear LCD
    return secret_num; //returns the secret number
}

//Calculates the  guess proximity and returns a value
int within(int goal, int value, int n) //int n is 5. Used to check if the guess is within 5 or not
{
    int difference; //stores the difference between the secret number (int goal) and the input (int value)
    if(goal > value) //if secret number is greater than the guess
    {
        difference = goal - value;
        if(difference > n)
        {
            return 0; //0 returned if guess is not within 5 and too small
        }
        else if(difference <= n && difference != 0)
        {
            return 1; //returns 1 if the guess is within 5
        }
    }
    else if(value > goal)
    {
        difference = value - goal;
        if(difference > n)
        {
            return -1; //-1 returned if guess is too large
        }
        else if(difference <= n && difference != 0)
        {
            return 1;
        }
    }
    else if(value == goal)
    {
        difference = goal - value;
        if(difference == 0)
        {
            return 2; //2 is returned if the guess and secret number is the same (user wins)
        }
    }
    return 0;
}

//A simple  function that checks if the score dips below 0.
//This was made to avoid repetitive code in the main function
int scoreCheck(int score_number)
{
    if(score_number > 0)
    {
        return score_number; //returns the score itself it doesn't
    }
    else
    {
        return 0; //otherwise returns 0
    }
    
}

//This function is used to set the difficulty based on the reading from the switch
int difficulty_set(int switches)
{
    if((switches == 128) || (switches == 144))
    {
        return 1; //difficulty 1 returned (easy)
    }
    else if((switches == 64) || (switches == 80) || (switches == 192) || (switches == 208))
    {
        return 2; //difficulty 2 returned (medium)
    }
    else if((switches == 32) || (switches == 48) || (switches == 96) || (switches == 112) || (switches == 176) || (switches == 224) || (switches == 240))
    {
        return 3; //difficulty 3 returned (hard)
    }
    return 0;
}

//This function decides how much decrement the score by
//Depending on the difficulty and good or bad guess
int decrementer(int difficulty_value, char guess)
{
    if(guess == 'b') //'b' indicates bad guess (too big or small)
    {
        if(difficulty_value == 1)
        {
            return 5; //The amount to decrement the score is returned 
        }
        else if(difficulty_value == 2)
        {
            return 10;
        }
        else if(difficulty_value == 3)
        {
            return 20;
        }
    }
    if(guess == 'g') //Indicated good guess (within 5)
    {
        if(difficulty_value == 1)
        {
            return 2;
        }
        else if(difficulty_value == 2)
        {
            return 5;
        }
        else if(difficulty_value == 3)
        {
            return 10;
        }
    }
    return 0;
}

//Simple function to avoid repetitive code. Just used to turn on the LEDs
//Using the value the function receives
void turn_on_led(unsigned int led_value)
{
    sw.write(led_value); 
    lat = 1;
    lat = 0;
}

//This turns on one LED at a time in alternate colours (green, red, green etc)
//To represent Christmas lights if the user wins the game!
void christmasLights()
{
    turn_on_led(0x4000);
    wait(0.5);
    turn_on_led(0x6000);
    wait(0.5);
    turn_on_led(0x6400);
    wait(0.5);
    turn_on_led(0x6500);
    wait(0.5);
    turn_on_led(0x6580);
    wait(0.5);
    turn_on_led(0x6590);
    wait(0.5);
    turn_on_led(0x6598);
    wait(0.5);
    turn_on_led(0x6599);
    wait(0.5);
    turn_on_led(0x0000);
    wait(0.5);
}

//Start of the main code.
int main() 

{
    int score = 100; //score starts at 100
    int printed_score = 1; //stores the score printed on the screen
    int sec_number = 0; //stores the secret number
    char key = ' '; //stores the keypad input
    int diff = 5; //For calculating guess proximity
    int value = 0; //stores the input from the PC when guessing
    int returned = 0; //stores value returned from the within() function
    int difficulty; //stores the difficulty value
    lat = 0; //sets lat/chip select to 0
    
    sw.format(16,0); //16 bit SPI communication and frequency is set below in Hz
    sw.frequency(1000000);
    turn_on_led(0x5400); //Makes the first 3 LEDs green and the rest 0.
    
    lcd.locate(1, 0);
    while(key == ' ')
    {
        lcd.cls();
        lcd.printf("Press C to Enter"); //prints on LCD to press C to Enter
        key = getKey(); //gets a character from keypad
        wait(0.1);
    }
    
    while(key != 'C') //If user enters anything but C, loop is entered
    {
        lcd.cls();
        lcd.locate(0, 0);
        lcd.printf("ONLY enter C"); //Asks to only enter C from keypad
        key = getKey(); //gets a character from keypad
        wait(0.1);
    }
    
    key = ' '; //resets key variable
    
    lcd.cls();
    while(key != 'C') //enters a loop to set the difficulty
    {
        lcd.locate(0, 0);
        lcd.printf("Enter Difficulty");
        lcd.locate(0, 1);
        lcd.printf("using switches"); //Asks to enter difficulty using the switches
        
        //This enabales only the first 4 switches (the last four are not used ('hardware disabled')
        //Only first 3 switches are used for difficulty
        switch_cs = 4;
        int switches = switch_reading; //stores the decimal value from the switches
        switches = (switches << 4); //shifts 4 bits
        
        //If no switches are pressed (0), or if the 4th switch (16 in decimal)
        //The code loops back to beginning. This also 'software disables' the 4th switch
        if((switches == 0) || (switches == 16))
        {
            turn_on_led(0x5400); //Sets LEDs to first 3 greens
            continue;
        }
        else
        {
            difficulty = difficulty_set(switches); //sets difficulty based on switch reading
            if(difficulty == 1)
            {
                turn_on_led(0x9400); //update LED based on difficulty value
            }
            else if(difficulty == 2)
            {
                turn_on_led(0xA400);
            }
            else if(difficulty == 3)
            {
                turn_on_led(0xA800);
            }
        }
        key = getKey(); //get a key, loop exits once difficulty is set and C is pressed
        wait(0.1);
    }
    
    key = ' '; 
    lcd.cls();
    lcd.locate(0, 0);
    lcd.printf("Secret Number:"); //asks to enter secret number on LCD
    sec_number = numberEnter(); //Uses numberEnter() to get secret number
    while(sec_number > 30) //Loops and re-asks for secret number if it is >30
    {
        lcd.cls();
        lcd.locate(0, 0);
        lcd.printf("Number too big!!!");
        lcd.locate(0, 1);
        lcd.printf("Enter <= 30"); //informs that it is too big
        wait(1);
        lcd.cls();
        lcd.printf("Secret Number:");
        sec_number = numberEnter();  
    }
    lcd.cls();
    lcd.locate(0, 0); //After everything is set, the number and difficulty level is printed on the LCD
    lcd.printf("Secret Number:%d", sec_number);
    lcd.locate(0, 1);
    lcd.printf("Difficulty:%d", difficulty);
    wait(0.2);

    pc.printf("Entered"); //sends this message to host as confirmation to start asking for guess
    
    while(1)
    {
        //gets the input from the PC
        //read_serial() waits till something is received.
        int n = read_serial(serial_input);

        //If there is an invalid character in the guess
        if(!is_digit(serial_input))
        {
            pc.printf("Renter"); //sends a re-enter message and PC will reask for guess
            continue; //loops back to beginning where the mbed waits for the new guess
        }

        value = atoi(serial_input); //converts to integer if valid number
        
        if(value > 30) //if the guess is >30, same happens
        {
            pc.printf("Renter");
            continue; //loops back to the start to wait for new guess
        }

        pc.printf("%s", serial_input); //prints guess to PC for confirmation

        returned = within(sec_number, value, diff); //now guess proximity is checked

        wait_ms(70); //delay for reliability, to ensure PC has received all of the beforehand messages
        
        if(returned == -1)
        {
            score = score  - decrementer(difficulty, 'b'); //score decremented based on guess and difficulty
            printed_score = scoreCheck(score); //checks if score dips below 0
            if(printed_score == 0) //If the score is 0, sends a lose message to host and exits the loop
            {
                pc.printf("Way too big!\nScore: %d\n[!]You Lose!!!\n", printed_score);
                break;
            }
            pc.printf("Way too big!\nScore: %d\n", printed_score); //if not, sends guess status and score to host
            returned = 3; //resets the returned value to 3
            value = 0; //resets this variable
            wait(0.1); //wait for reliability
        }
        else if(returned == 0)
        {
            score = score  - decrementer(difficulty, 'b');
            printed_score = scoreCheck(score);
            if(printed_score == 0)
            {
                pc.printf("Way too small!\nScore: %d\n[!]You Lose!!!\n", printed_score);
                break;
            }
            pc.printf("Way too small!\nScore: %d\n", printed_score);
            returned = 3;
            value = 0;
            wait(0.1);
        }
        else if(returned == 1)
        {
            score = score  - decrementer(difficulty, 'g');
            printed_score = scoreCheck(score);
            if(printed_score == 0)
            {
                pc.printf("Within 5\nScore: %d\n[!]You Lose!!!\n", printed_score);
                break;
            }
            pc.printf("Within 5\nScore: %d\n", printed_score);
            returned = 3;
            value = 0;
            wait(0.1);   
        }
        else if(returned == 2)
        {
            pc.printf("You Win!\n"); //If user wins, a win message to host is sent to host
            returned = 3;
            value = 0;
            wait(0.1);
            break; //loop breaks as game ends when user wins
        }  
    }

    if(printed_score != 0) //if the score is not 0, user wins
    {
        lcd.cls();
        lcd.locate(0, 0);
        lcd.printf("Game Over"); //prints game over and score to LCD
        lcd.locate(0, 1);
        lcd.printf("Final Score: %d", score);
        while(1)
        {
            christmasLights(); //turns on Christmas lights
        }
    }
    else
    {
        lcd.cls();
        lcd.locate(0, 0);
        lcd.printf("You Lose!!!"); //prints lose to LCD
        lcd.locate(0, 1);
        lcd.printf("Final Score: %d", printed_score);
        turn_on_led(0xA955); //turns all LEDs red to indicate loss
    }
    return 0;
}