# CoRTOS
A simple task scheduling library for the Arduino ecosystem.
Caveats: 
1. It's not preemptive, so if a task takes too long, other tasks will wait. Any tasks that should have run during the
waiting period will then run all at once in priority ordering. This *includes* important tasks that happen to be stuck behind a rare, slow task.
2. If there are a lot of tasks, or one of them doesn't have a `sleep` in it, some lower-priority tasks will never run. This is called starvation.

### Installation:
Assuming that you have downloaded this as a zip file, unzip the file and open the resulting folder.
It should contain this readme and a folder labeled "CoRTOS."
Copy this folder to your [Arduino library directory](https://www.arduino.cc/en/hacking/libraries) (usually a folder called "libraries" in the same place your sketches are saved).

### Usage of this library:
Setup:
Add `#include <CoRTOS.h>` to the top of your sketch.
Put `cortos.init();` within the Setup() function of your sketch.
Put `cortos.scheduler();` within the Loop() function of your sketch.

### Creating a task:
MAKE SURE THAT EVERY TASK REGISTERED HAS THE SLEEP FUNCTION CALLED WITHIN IT! (unless you WANT it to run continually AND have no tasks with a larger priority value).

Assuming that the task you want to handle is as follows (must have signature void <name>(void) ):
```c
void Tick(){//declared as returning Void, no arguments.
  static counter=0;
  digitalWrite(LedBuiltin,!digitalRead(LedBuiltin));
  Serial.print("Tick: ");
  Serial.println(counter++);
  cortos.sleep();//If you omit this, the task will run continually and things with lower priority will never run.
}
```
Add a task by issuing the following commands once (aka also in Setup() or your initialization code):
```c
cortos.registerTask(Tick,<task priority>);//replace Tick with the name of your own function, and set a priority.
cortos.setSleepTime(Tick,<time>);//replace Tick with your function, and set the time between runs in milliseconds. Do NOT set <time> to be zero.
```
Notes: Do not include parentheses after the function name. `<priority>` is a value between 0 and 255; tasks with
lower numbers run first if given the option. The number is not unique.

### Modifying tasks:
Tasks can be deleted, have their priorities updated, paused, or set to sleep for different times.
Refer to the listing below under "task interface functions" in Cortos.h for more information.
Using these functions is exactly as shown in the `setSleepTime` in the "Creating a task" entry: list the name, and then the
updated parameter (if it takes one).

As a further helpful option, editing the currently-running task (usually from code inside the task itself) can
be easily done by leaving out the task name as the first parameter.

Example: `cortos.sleep(1000);` (without the task name) will set the current task's sleep time to be one second in-between
iterations. It will also put the task to sleep, so note that the task won't run for one second after this function call.
This is only really relevant in initialization code, where you want the task to run instantly the first time
when the Arduino starts up.

### Wishlist:
 - [ ] Design a custom allocator to handle the TCB. This should improve speed and code efficiency.
 - [ ] Add a method to allow lambda functions to be used. As it stands now, tasks are referenced by the function pointer, which can't be readily accessed for lambda functions. I'm thinking of using an index into that task's TCB index combined with some kind of UID. This will be stored into a single integer (16-bit?) for ease of access and storage.
 - [ ] Add multithreading support. Not something that will be seen soon... You can always just make another instance of the scheduler so long as they don't share tasks, though.
