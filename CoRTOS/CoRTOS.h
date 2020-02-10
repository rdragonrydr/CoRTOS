#include <Arduino.h>
#include <vector>

#ifndef CORTOS_DEF
#define CORTOS_DEF

// Configure the following to control memory usage. (Default only, increases as needed. Smaller==less memory)
#define TBL_SIZE 16

/*Defined flags for tasks. Do not edit.*/
#define DEL_ON_FIN 0 // Causes the task to be deleted on completion
#define LOCKED 1     // Makes the task unrunnable until the flag is removed
#define SLEEPING 7   // Task is waiting for a certain time event.

#ifndef _BV // Used in several places to set/clear bits. SHOULD be defined already.
#define _BV(x) (1 << x)
#endif

struct task {
  byte flags;
  byte priority;
  uint32_t last_ran;
  uint32_t delay;
  void (*exec)();
};

class cortos_class {
  uint32_t sched_act_last;
  uint16_t used_time;
  uint16_t total_time;
  // uint16_t task_count;
  // uint16_t table_size;
  byte cur_task_ptr;
  byte proc_use;
  byte findByPtr(void (*fcn)());
  // struct task *processes;
  std::vector<task> processes;

public:
  // Task interface functions
  void registerTask(void (*toAdd)(), byte pri);
  void setPriority(void (*fcn)(), byte pri);               // Changes the task priority.
  void setPriority(byte pri);                              // Change the priority of the executing task.
  void sleep(void (*fcn)(), uint32_t milliseconds);        // Sleeps the task for a time
  void sleep(uint32_t milliseconds);                       // Sleeps the current task for a time.
  void sleep(void (*fcn)());                               // sleeps the task for the last delay
  void sleep();                                            // sleeps the current task for the last delay
  void setSleepTime(void (*fcn)(), uint32_t milliseconds); // set sleep time, but do not sleep
  void setSleepTime(uint32_t milliseconds);                // set sleep time of cur. task. Do not sleep.
  void unlockTask(void (*toWake)());                       // Allows a task to run again
  void lockTask(void (*fcn)());                            // Halts a task's further execution
  void lockTask();                                         // Stops the current task from executing again
  void killTask(void (*fcn)());                            // Removes a task from the process list
  void killTask();                                         // Removes the current task
  void queueToDelete(void (*fcn)());                       // Sets the task to delete itself after it runs once
  void queueToDelete();                                    // Sets the current task to delete after it runs once

  int getNumTasks() { return processes.size(); }           // Returns the current number of tasks
  int getRTOSMaxTasks() { return processes.capacity(); }   // Returns the current table size
  byte getCPUUse() { return proc_use; }                    // Returns the CPU use. It's a work in progress and may not be accurate.

  // RTOS operation functions.
  void init();      // Place somewhere in Setup()
  void scheduler(); // Place somewhere in Loop(). NOT RE-ENTRANT!!! (If you don't know what that is, you are safe.)
};
extern cortos_class cortos;

#endif
