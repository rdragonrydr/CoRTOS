# CoRTOS
A simple task scheduling library for the Arduino ecosystem.
Caveats: 
1. It's not preemptive, so if a task takes too long, other tasks will wait. Any tasks that should have run during the waiting period will then run all at once in priority ordering. This *includes* important tasks that happen to be stuck behind a rare, slow task.
2. If there are a lot of tasks, or one of them doesn't have a `sleep` in it, some lower-priority tasks will never run. This is called starvation.

### Installation:
Assuming that you have downloaded this as a zip file, unzip the file and open the resulting folder.
It should contain this readme and a folder labeled "CoRTOS."
Copy the folder to your [Arduino library directory](https://www.arduino.cc/en/hacking/libraries) (usually a folder called "libraries" in the same place your sketches are saved).

### Usage of this library:
Setup:
Add `#include <CoRTOS.h>` to the top of your sketch.
Put `cortos.init();` within the setup() function of your sketch.
Put `cortos.scheduler();` within the loop() function of your sketch.

### Creating a task:
For your project to run correctly, you need to ensure at least one of the following:
* Every task has a sleep() call somewhere within it.
* The task was created with the `PERIODIC` flag (and optionally `SLEEPFOR` or it and `DEADLINE`).
* You used `setSleepTime()` on it with the third optional parameter set to true.
* All your tasks have equal priority.

Assuming that the task you want to handle is as follows (must have signature void <name>(void) ):
```c
void Tick(){//declared as returning Void, no arguments.
  static counter=0;
  digitalWrite(LedBuiltin,!digitalRead(LedBuiltin));
  Serial.print("Tick: ");
  Serial.println(counter++);
  cortos.sleep();//If you omit this, the task will run continually and things with lower priority will _never_ run.
}
```
Add a task by issuing the following commands once (aka in setup() or your initialization code):
```c
cortos.registerTask(Tick, <task priority>, [optional flags]);//replace Tick with the name of your own function, and set a priority.
cortos.setSleepTime(Tick, <time>, [optional PERIODIC]);//replace Tick with your function, and set the time between runs in milliseconds. Do NOT set <time> to be zero.
```
Notes: 
* Do not include parentheses after the function name when registering it. 
* `<priority>` is a value between 0 and 255; tasks with lower numbers run first if given the option. The number is not unique.
* If you want something more **reliable**, use the `PERIODIC` (etc) flags rather than calling `sleep()`.
 * `PERIODIC` is like sleep, except that it's automatic and you don't need to change your code.
 * `PERIODIC | SLEEPFOR` is best for timekeeping, since it will always run on the dot and it'll catch up if it falls behind.
 * `PERIODIC | SLEEPFOR | DEADLINE` is best for code that should always run on a fixed schedule, but can't run more than once in that time. (SLEEPFOR alone will cause the task to run two or more times in quick succession if it misses its deadline. This is problematic for things like sensors.)
 * There's a function set for enabling (setting) or disabling (clearing) flags after setup, if you want to change their state later.
 * `DELAFTER` is the opposite of `PERIODIC`. Use of this flag will remove the task after running, and to use it again will require re-registering it.
 * There exist other flags, but they are for internal use only and using them in your code may break something.

### Modifying tasks:
Tasks can be deleted, have their priorities updated, paused, or set to sleep for different times.
Refer to the listing under "task interface functions" in Cortos.h for more information.
Using these functions is exactly as shown in the `setSleepTime` in the "Creating a task" entry: list the task's function name, and then the
updated parameter (if it takes one).

As a further helpful option, editing the currently-running task (usually from code inside the task itself) can be easily done by leaving out the task name as the first parameter. Similarly, during setup, this will refer to the last task you called registerTask() on.

Example: `cortos.sleep(1000);` (without the task name) will set the current task's sleep time to be one second in-between
iterations. It will also put the task to sleep, so note that the task won't run for one second after this function call.
This last is only really relevant in initialization code, where you (may or may not) want the task to run instantly the first time
when the Arduino starts up.

### Wishlist:
 - [ ] Add a method to allow lambda functions to be used. As it stands now, tasks are referenced by the function pointer, which can't be readily accessed for lambda functions. I'm thinking of using an index into that task's TCB index combined with some kind of UID. This will be stored into a single integer (16-bit?) for ease of access and storage.
 - [ ] Add multithreading support. Not something that will be seen soon... You can always just make another instance of the scheduler so long as they don't share tasks, though.

A code skeleton exists (it's worse than barebones, pardon the pun) for using a dynamically-allocated task list. However, doing so will lead to heap fragmentation and you aren't likely to have more than a handful of tasks. I've reverted it back to a static list for now.
