#include "mbed.h"
#include "uLCD_4DGL.h"


uLCD_4DGL uLCD(D1, D0, D2);
InterruptIn user_btn(USER_BUTTON);

EventQueue menu_queue(32 * EVENTS_EVENT_SIZE);
Thread menu_t;

int angle = 15; // initial value of angle is 15

void menu(){
    uLCD.color(BLACK);
    uLCD.locate(1, 2);
    uLCD.printf("\n15\n\n");
    uLCD.locate(1, 4);
    uLCD.printf("\n30\n\n");
    uLCD.locate(1, 6);
    uLCD.printf("\n45\n\n");

    if(angle == 15){
        uLCD.color(BLUE);
        uLCD.locate(1, 2);
        // uLCD.printf("\n10Hz\n\n");
        uLCD.printf("\n15\n\n");
    }
    else if(angle == 30){
        uLCD.color(BLUE);
        uLCD.locate(1, 4);
        // uLCD.printf("\n25Hz\n\n");
        uLCD.printf("\n30\n\n");
    }
    else{
        uLCD.color(BLUE);
        uLCD.locate(1, 6);
        // uLCD.printf("\n100Hz\n\n");
        uLCD.printf("\n45\n\n");
    }
}

int main(){
    // init uLCD
    menu_t.start(callback(&menu_queue, &EventQueue::dispatch_forever));
    uLCD.background_color(WHITE);
    uLCD.cls();
    uLCD.textbackground_color(WHITE);
    menu();
}