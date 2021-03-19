#include <Arduino.h>

#ifndef CORTOS_DEF
#define CORTOS_DEF

// Configure the following to control memory usage.
#define TBL_SIZE 8

// Enable the following or set it before including this library to enable debug printing
#define CORTOS_DEBUG

/*Defined flags for tasks. Do not edit.*/
#define DELAFTER 0x01 // Causes the task to be deleted on completion
#define LOCKED 0x02   // Makes the task unrunnable until the flag is removed.
#define PERIODIC 0x04 // The task shall stay sleeping; a timed task run on an interval.
#define SLEEPING 0x08 // Task is waiting for a certain time to pass.
#define SLEEPFOR 0x10 // Task will sleep from its start time, not the time sleep was called.
#define DEADLINE 0x20 // Task will run maximum of ONCE even if deadline is missed (cannot "catch up" to expected run count).

struct task {
  byte flags;
  byte priority;
  uint32_t last_ran;
  uint32_t delay;
  void (*exec)();
#ifdef CORTOS_DEBUG
  char name[32];
#endif
};

class cortos_class {
  uint16_t last_checked_time = 0;
  uint16_t accumulated_run_time = 0;
  byte CPU_use = 0;
  byte task_count;
  byte cur_task_ptr;
#ifdef CORTOS_DEBUG
  bool debug;
#endif
  struct task processes[TBL_SIZE];
  byte findByPtr(void (*fcn)());
  // void resizeProcessTable(byte new_size);

public:
// Task interface functions
#ifdef CORTOS_DEBUG
  // Management of named tasks
  bool registerTask(void (*toAdd)(), String name, byte pri, byte flags = 0);
  void setTaskName(void (*toAdd)(), String name);
#endif
  bool registerTask(void (*toAdd)(), byte pri, byte flags = 0);       // registers the task.
  void setTaskFlags(byte flags);                                      // Allows the user to set special flags that control execution
  void setTaskFlags(void (*fcn)(), byte flags);                       // Allows the user to set special flags that control execution
  void clearTaskFlags(byte flags);                                    // Allows the user to clear task flags
  void clearTaskFlags(void (*fcn)(), byte flags);                     // Allows the user to clear task flags
  void setPriority(void (*fcn)(), byte pri);                          // Changes the selected task priority.
  void setPriority(byte pri);                                         // Change the priority of the executing task.
  void sleep(void (*fcn)(), uint32_t ms);                             // Sleeps the task for a time
  void sleep(uint32_t ms);                                            // Sleeps the current task for a time.
  void sleep(void (*fcn)());                                          // sleeps the task for the last delay
  void sleep();                                                       // sleeps the current task for the last delay
  void setSleepTime(void (*fcn)(), uint32_t ms, bool cyclic = false); // set sleep time, but do not sleep
  void setSleepTime(uint32_t ms, bool cyclic = false);                // set sleep time of cur. task. Do not sleep.
  void unlockTask(void (*toWake)());                                  // Allows a task to run again
  void lockTask(void (*fcn)());                                       // Halts a task's further execution
  void lockTask();                                                    // Stops the current task from executing again
  void killTask(void (*fcn)());                                       // Removes a task from the process list
  void killTask();                                                    // Kills the current task

  byte getNumTasks() { return task_count; }                           // Returns the number of tasks. Can be >MAX
                                                                      // if too many tasks were registered
  byte getRTOSMaxTasks() { return TBL_SIZE; }                         // Returns the maximum # tasks that can be added
  byte getLoad() { return CPU_use; }                                  // Returns the CPU load (experimental)
                                                                      // May be >100 if overloaded (intentional).
#ifdef CORTOS_DEBUG
  void enableDebug(bool ed) { debug = ed; }                           // Enables debug printing, if enabled in the header
#endif

  // RTOS operation functions.
  void init();      // Place somewhere in Setup()
  void scheduler(); // Place somewhere in Loop(). NOT RE-ENTRANT!!!
};
extern cortos_class cortos;

#endif
