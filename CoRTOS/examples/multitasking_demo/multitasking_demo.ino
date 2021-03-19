#include <CoRTOS.h>

//This sketch demonstrates the ease of multitasking with CoRTOS.

//Please note: It is recommended to modify certain items in CoRTOS.h for best performance.

//Item One: TBL_SIZE: This is the maximum number of tasks that can be run simultaneously.

//Item Two: CORTOS_DEBUG: This prints task information for debugging/demonstration purposes.
//It is suggested to comment out that line (starting with #define) to save resources in a
//production system. The printing takes a lot of overhead and monopolizes the UART.

int reading;

void bigdelay() {
  Serial.println("BwwwWWWWUUUUUMMMMMM...");
  delay(1000);
  Serial.println("...MMMmmmmvreeeeee");
}

void task1() { reading = analogRead(A0); }

void task2() {
  digitalWrite(13, !digitalRead(13));
  Serial.print("The Reading: ");
  Serial.println(reading);
}

void task3() {
  while (random(0, 10) < 5)
    delay(1);
  Serial.print("CPU Load: ");
  Serial.println(cortos.getLoad());
}

void setup() {
  Serial.begin(115200);
  cortos.init();
  cortos.enableDebug(true);//Alternatively, omit this line to disable most of the debug print overhead.

  //registerTask takes the function name (no paren), the display name,
  //the priority (smaller number=higher priority), and the flags.

  //DELAFTER means that the task runs once and then frees up that RTOS schedule slot.
  //The task will need to be registered again if you wish to reuse it.

  //PERIODIC makes the task sleep for a certain time after it completes.
  //It is also configured by the third parameter of setSleepTime. (true=>set, false=>cleared)
  //As an alternative to a periodic task, it can be locked and unlocked to enable aperiodic scheduling.
  //Or you may manually call the sleep functions to make it sleep once (and then keep calling them).
  //The latter is useful if the time you wish to sleep is determined programmatically.

  //SLEEPFOR extends PERIODIC by forcing the task to always run on schedule. It will always enable
  //itself exactly at the period, no matter how long it took to run or how late it started.
  //It will also run multiple times in a row to catch up, if for some reason it could not run for a while.

  //DEADLINE further extends SLEEPFOR. With the help of additional processing, it always enables a task
  //at its fixed periodic interval like SLEEPFOR, but it can't run more than once per period.

  //You can test this by changing the flags on the lines below. (Remember, setSleepTime sets PERIODIC)


  cortos.registerTask(bigdelay, "Simulated Long Task", 255, (SLEEPFOR | DEADLINE));
  cortos.setSleepTime(3000, true);
  cortos.registerTask(task1, "SensorTask", 40, (SLEEPFOR | DEADLINE));
  cortos.setSleepTime(200, true);
  cortos.registerTask(task2, "DisplayTask", 30, (SLEEPFOR | DEADLINE));
  cortos.setSleepTime(100, true);
  cortos.registerTask(task3, "VLT+Load", 20, (SLEEPFOR | DEADLINE));
  cortos.setSleepTime(200, true);
}

void loop() { cortos.scheduler(); }
