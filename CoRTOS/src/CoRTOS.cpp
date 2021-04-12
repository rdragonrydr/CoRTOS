#include "CoRTOS.h"

byte cortos_class::findByPtr(void (*fcn)()) {
  byte index = 0;
  while (index < TBL_SIZE) {
    if (processes[index].exec == fcn) {
      break;
    }
    index++;
  }
  return index;
}

/*
void cortos_class::resizeProcessTable(byte new_size) {
  struct task *new_table = (struct task *)malloc(new_size * sizeof(struct task));
  if (new_table != NULL) {
    memset(new_table, 0, new_size * sizeof(struct task));
    byte index = 0;
    for (byte i = 0; i < table_size; i++) {
      if (processes[i].exec != NULL) {
        new_table[index] = processes[i];
        index++;
      }
    }
    free(processes);
    processes = new_table;
    table_size = new_size;
  }
}
*/

/*
if (task_count < (table_size / 2) && table_size > TBL_SIZE) {
  resizeProcessTable(table_size / 2);
}
*/

/*
if (task_count==(table_size-1) && table_size < 128){ //may want to use int16 for that size instead
  resizeProcessTable(table_size * 2);
}

*/

#ifdef CORTOS_DEBUG
bool cortos_class::registerTask(void (*toAdd)(), String name, byte pri, byte flags) {
  struct task p;
  p.exec = toAdd;
  p.priority = pri;
  p.flags = flags;
  p.delay = 0;
  p.last_ran = 0;
  strncpy(p.name, name.c_str(), 32);
  byte index = findByPtr(NULL);
  task_count++; // This will remain outside the condition so the user can tell if they overbooked tasks
  // That is, it will increase and potentially be higher than the maximum task count if they did.
  if (processes[index].exec == NULL) {
    processes[index] = p;
    cur_task_ptr = index;
    return true;
  }
  return false;
}

void cortos_class::setTaskName(void (*fcn)(), String name) {
  byte index = findByPtr(fcn);
  if (processes[index].exec == fcn) {
    strncpy(processes[index].name, name.c_str(), 32);
  }
}

#endif

bool cortos_class::registerTask(void (*toAdd)(), byte pri, byte flags) {
  struct task p;
  p.exec = toAdd;
  p.priority = pri;
  p.flags = flags;
  p.delay = 0;
  p.last_ran = 0;
#ifdef CORTOS_DEBUG
  snprintf_P(p.name, 32, PSTR("tID %03d"), task_count);
#endif
  byte index = findByPtr(NULL);
  task_count++; // This will remain outside the condition so the user can tell if they overbooked tasks
  // That is, it will increase and potentially be higher than the maximum task count if they did.
  if (processes[index].exec == NULL) {
    processes[index] = p;
    cur_task_ptr = index;
    return true;
  }
  return false;
}

void cortos_class::setTaskFlags(byte flags) { processes[cur_task_ptr].flags |= flags; }

void cortos_class::setTaskFlags(void (*fcn)(), byte flags) {
  byte index = findByPtr(fcn);
  if (processes[index].exec == fcn) {
    processes[index].flags |= flags;
  }
}

void cortos_class::clearTaskFlags(byte flags) { processes[cur_task_ptr].flags &= ~flags; }

void cortos_class::clearTaskFlags(void (*fcn)(), byte flags) {
  byte index = findByPtr(fcn);
  if (processes[index].exec == fcn) {
    processes[index].flags &= ~flags;
  }
}

void cortos_class::setPriority(void (*fcn)(), byte pri) {
  byte index = findByPtr(fcn);
  if (processes[index].exec == fcn) {
    processes[index].priority = pri;
  }
}

void cortos_class::setPriority(byte pri) { processes[cur_task_ptr].priority = pri; }

void cortos_class::sleep(void (*fcn)(), uint32_t ms) {
  byte index = findByPtr(fcn);
  if (processes[index].exec == fcn) {
    processes[index].flags |= SLEEPING;
    processes[index].last_ran = millis();
    processes[index].delay = ms;
  }
}

void cortos_class::sleep(uint32_t ms) {
  processes[cur_task_ptr].flags |= SLEEPING;
  processes[cur_task_ptr].last_ran = millis();
  processes[cur_task_ptr].delay = ms;
}

void cortos_class::sleep(void (*fcn)()) {
  byte index = findByPtr(fcn);
  if (processes[index].exec == fcn) {
    processes[index].flags |= SLEEPING;
    processes[index].last_ran = millis();
  }
}

void cortos_class::sleep() {
  processes[cur_task_ptr].flags |= SLEEPING;
  processes[cur_task_ptr].last_ran = millis();
}

void cortos_class::setSleepTime(void (*fcn)(), uint32_t ms, bool cyclic) {
  byte index = findByPtr(fcn);
  if (processes[index].exec == fcn) {
    processes[index].delay = ms;
    processes[cur_task_ptr].flags |= SLEEPING;
  }
  if (cyclic) {
    setTaskFlags(fcn, PERIODIC);
  } else {
    clearTaskFlags(fcn, PERIODIC);
  }
}

void cortos_class::setSleepTime(uint32_t ms, bool cyclic) {
  processes[cur_task_ptr].delay = ms;
  processes[cur_task_ptr].flags |= SLEEPING;
  if (cyclic) {
    setTaskFlags(PERIODIC);
  } else {
    clearTaskFlags(PERIODIC);
  }
}

void cortos_class::unlockTask(void (*fcn)()) { clearTaskFlags(fcn, LOCKED); }

void cortos_class::lockTask(void (*fcn)()) { setTaskFlags(fcn, LOCKED); }

void cortos_class::lockTask() { setTaskFlags(LOCKED); }

void cortos_class::killTask(void (*fcn)()) {
  byte index = findByPtr(fcn);
  if (processes[index].exec == fcn) {
    processes[index].exec = NULL;
    task_count--;
  }
}

void cortos_class::killTask() {
  processes[cur_task_ptr].exec = NULL;
  task_count--;
}

void cortos_class::init() {
  cur_task_ptr = 0;
  task_count = 0;
  for (byte i = 0; i < TBL_SIZE; i++) {
    processes[i].exec = NULL;
  }
  last_checked_time = millis();
  accumulated_run_time = 0;
  CPU_use = 0;
  debug = false;
}

void cortos_class::scheduler() {
  byte found = 0;
  uint32_t start_time = millis();
  {
    uint16_t pri = 256; // Since equal priorities are ignored, use one larger to initialize.
    byte index = 0;     // current logical task we are checking
    byte candidate = 0; // task index that has current best priority
    byte startaddr = cur_task_ptr + 1;
    while (index < TBL_SIZE) {
      byte adjusted = (index + startaddr) % TBL_SIZE; // physical index starting on the last task run.
      if (processes[adjusted].exec != NULL) {
        if (processes[adjusted].flags & SLEEPING) {
          if ((start_time - processes[adjusted].last_ran) >= processes[adjusted].delay) {
            processes[adjusted].flags &= ~SLEEPING;
          }
        }
        if ((processes[adjusted].flags & (SLEEPING | LOCKED)) == 0) {
          if (processes[adjusted].priority < pri) {
            found++;
            pri = processes[adjusted].priority;
            candidate = adjusted;
          }
        }
      }
      index++;
    }
    cur_task_ptr = candidate;
  }
  if (found > 0) {
#ifdef CORTOS_DEBUG
    if (debug) {
      char str[132];
      uint32_t del = processes[cur_task_ptr].delay;
      uint32_t lr = processes[cur_task_ptr].last_ran + del;
      snprintf_P(str, 132, PSTR("Task %s (of %u) started %08lu with priority %03u and deadline %08lu"), processes[cur_task_ptr].name, found, start_time,
                 processes[cur_task_ptr].priority, lr + del);
      Serial.println(str);
      snprintf_P(str, 132, PSTR("(Task became runnable at %08lu with period (if applicable) %04lu)"), lr, del);
      Serial.println(str);
    }
#endif
    processes[cur_task_ptr].exec();
    if (processes[cur_task_ptr].flags & DELAFTER) {
      processes[cur_task_ptr].exec = NULL;
      task_count--;
    } else if (processes[cur_task_ptr].flags & PERIODIC) {
      processes[cur_task_ptr].flags |= SLEEPING;
      if (processes[cur_task_ptr].flags & SLEEPFOR) {
        if (processes[cur_task_ptr].flags & DEADLINE) {
          /*
          EXPERIMENTAL!
          In this mode, the time offset due to whenever the task was found or how long it took
          is still maintained, but the offset is computed differently. The task can still run early,
          but it can't run more than once.
          */
          uint32_t td = start_time - processes[cur_task_ptr].last_ran;
          uint32_t remainder = td % processes[cur_task_ptr].delay;
          processes[cur_task_ptr].last_ran += (td - remainder);
        } else {
          /*
          This will make the task run exactly each period, rather than period+runtime.
          Has the (beneficial?) side effect of causing a task to be scheduled >once if
          it has missed its deadline. It will run continually, priority permitting,
          until it has caught up with its proper number of iterations.
          */
          processes[cur_task_ptr].last_ran += processes[cur_task_ptr].delay;
        }
      } else {
        // The task will sleep for this time starting when it concludes.
        // Useful for waiting out sensor update times and such.
        processes[cur_task_ptr].last_ran = millis();
      }
    }
    // Calculations for CPU use analytics
    uint32_t end_time = millis();
#ifdef CORTOS_DEBUG
    if (debug) {
      char str[64];
      snprintf_P(str, 64, PSTR("Task %s ended %08lu with runtime of %04lu\n"), processes[cur_task_ptr].name, end_time, end_time - start_time);
      Serial.println(str);
    }
#endif
    accumulated_run_time += end_time - start_time;
    if (((uint16_t)end_time) - last_checked_time > 500) {
      last_checked_time = (uint16_t)end_time;
      CPU_use = (byte)(accumulated_run_time * 0.2);
      accumulated_run_time = 0;
    }
  }
}

cortos_class cortos;
