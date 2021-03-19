#include <CoRTOS.h>

//This sketch demonstrates the ease of multitasking with CoRTOS.

//Please note: It is recommended to modify certain items in CoRTOS.h for best performance.

//Item One: TBL_SIZE: This is the maximum number of tasks that can be run simultaneously.

//Item Two: CORTOS_DEBUG: This prints task information for debugging/demonstration purposes.
//It is suggested to comment out that line (starting with #define) to save resources in a
//production system. The printing takes a lot of overhead and monopolizes the UART.

int reading;

void task0() {
  Serial.println("...eeeeEEEEEeeee...!");
}

void task1() {
  Serial.println("Woohoo!");
}

void task2() {
Serial.println("Aha!");
}

void task3() {
Serial.println("Whee!");
}

void setup() {
  Serial.begin(115200);
  cortos.init();
  cortos.enableDebug(true);//Alternatively, omit this line to disable most of the debug print overhead.

  //By setting no sleep times/flags for these tasks and making them all the same priority, they will run forever
  //in round-robin order (that is, in the order they were added).


  cortos.registerTask(task0, "Task A", 255);
  cortos.registerTask(task1, "Task B", 255);
  cortos.registerTask(task2, "Task C", 255);
  cortos.registerTask(task3, "Task D", 255);
}

void loop() { cortos.scheduler(); }
