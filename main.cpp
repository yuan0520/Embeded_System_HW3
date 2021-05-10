#include "mbed.h"
#include "uLCD_4DGL.h"
#include "stm32l475e_iot01_accelero.h"

#include "accelerometer_handler.h"
#include "config.h"
#include "magic_wand_model_data.h"

#include "tensorflow/lite/c/common.h"
#include "tensorflow/lite/micro/kernels/micro_ops.h"
#include "tensorflow/lite/micro/micro_error_reporter.h"
#include "tensorflow/lite/micro/micro_interpreter.h"
#include "tensorflow/lite/micro/micro_mutable_op_resolver.h"
#include "tensorflow/lite/schema/schema_generated.h"
#include "tensorflow/lite/version.h"


uLCD_4DGL uLCD(D1, D0, D2);
InterruptIn user_btn(USER_BUTTON);

EventQueue menu_queue(32 * EVENTS_EVENT_SIZE);
Thread menu_t;
EventQueue acc_queue(32 * EVENTS_EVENT_SIZE); // queue for accelero
Thread acc_t;

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

constexpr int kTensorArenaSize = 60 * 1024;
uint8_t tensor_arena[kTensorArenaSize];

// Return the result of the last prediction
int PredictGesture(float* output) {
  // How many times the most recent gesture has been matched in a row
  static int continuous_count = 0;
  // The result of the last prediction
  static int last_predict = -1;

  // Find whichever output has a probability > 0.8 (they sum to 1)
  int this_predict = -1;
  for (int i = 0; i < label_num; i++) {
    if (output[i] > 0.8) this_predict = i;
  }

  // No gesture was detected above the threshold
  if (this_predict == -1) {
    continuous_count = 0;
    last_predict = label_num;
    return label_num;
  }

  if (last_predict == this_predict) {
    continuous_count += 1;
  } else {
    continuous_count = 0;
  }
  last_predict = this_predict;

  // If we haven't yet had enough consecutive matches for this gesture,
  // report a negative result
  if (continuous_count < config.consecutiveInferenceThresholds[this_predict]) {
    return label_num;
  }
  // Otherwise, we've seen a positive result, so clear all our variables
  // and report it
  continuous_count = 0;
  last_predict = -1;

  return this_predict;
}

int main(){
    // init uLCD
    menu_t.start(callback(&menu_queue, &EventQueue::dispatch_forever));
    uLCD.background_color(WHITE);
    uLCD.cls();
    uLCD.textbackground_color(WHITE);
    menu();

    // init accelero
    printf("Start accelerometer init\n");
    BSP_ACCELERO_Init();
    acc_t.start(callback(&menu_queue, &EventQueue::dispatch_forever));
}