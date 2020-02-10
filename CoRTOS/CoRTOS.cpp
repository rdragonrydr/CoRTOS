#include "CoRTOS.h"

byte cortos_class::findByPtr(void (*fcn)()) {
  byte index = 0;
  while (index < processes.size()) {
    if (processes[index].exec == fcn) {
      break;
    }
    index++;
  }
  return index;
}

void cortos_class::registerTask(void (*toAdd)(), byte pri) {
  /*
  byte index = findByPtr(NULL);
  if (processes[index].exec == NULL) {
    processes[index].exec = toAdd;
    processes[index].priority = pri;
    processes[index].flags = 0;
  }*/
  struct task p;
  p.exec = toAdd;
  p.priority = pri;
  p.flags = 0;
  processes.push_back(p);
}

void cortos_class::setPriority(void (*fcn)(), byte pri) {
  byte index = findByPtr(fcn);
  if (processes[index].exec == fcn) {
    processes[index].priority = pri;
  }
}

void cortos_class::setPriority(byte pri) { processes[cur_task_ptr].priority = pri; }

void cortos_class::sleep(void (*fcn)(), uint32_t milliseconds) {
  byte index = findByPtr(fcn);
  if (processes[index].exec == fcn) {
    processes[index].flags |= _BV(SLEEPING);
    processes[index].last_ran = millis();
    processes[index].delay = milliseconds;
  }
}

void cortos_class::sleep(uint32_t milliseconds) {
  processes[cur_task_ptr].flags |= _BV(SLEEPING);
  processes[cur_task_ptr].last_ran = millis();
  processes[cur_task_ptr].delay = milliseconds;
}

void cortos_class::sleep(void (*fcn)()) {
  byte index = findByPtr(fcn);
  if (processes[index].exec == fcn) {
    processes[index].flags |= _BV(SLEEPING);
    processes[index].last_ran = millis();
  }
}

void cortos_class::sleep() {
  processes[cur_task_ptr].flags |= _BV(SLEEPING);
  processes[cur_task_ptr].last_ran = millis();
}

void cortos_class::setSleepTime(void (*fcn)(), uint32_t milliseconds) {
  byte index = findByPtr(fcn);
  if (processes[index].exec == fcn) {
    processes[index].last_ran = millis();
    processes[index].delay = milliseconds;
  }
}

void cortos_class::setSleepTime(uint32_t milliseconds) {
  processes[cur_task_ptr].last_ran = millis();
  processes[cur_task_ptr].delay = milliseconds;
}

void cortos_class::unlockTask(void (*toWake)()) {
  byte index = findByPtr(toWake);
  if (processes[index].exec == toWake) {
    processes[index].flags &= ~_BV(LOCKED);
  }
}

void cortos_class::lockTask(void (*fcn)()) {
  byte index = findByPtr(fcn);
  if (processes[index].exec == fcn) {
    processes[index].flags |= _BV(LOCKED);
  }
}

void cortos_class::lockTask() { processes[cur_task_ptr].flags |= _BV(LOCKED); }

void cortos_class::killTask(void (*fcn)()) {
  byte index = findByPtr(fcn);
  if (processes[index].exec == fcn) {
    // processes[index].exec = NULL;
    // task_count--;
    processes.erase(processes.begin() + index);
  }
}

void cortos_class::killTask() {
  processes.erase(processes.begin() + cur_task_ptr);
  // processes[cur_task_ptr].exec = NULL;
  // task_count--;
}

void cortos_class::queueToDelete(void (*fcn)()) {
  byte index = findByPtr(fcn);
  if (processes[index].exec == fcn) {
    processes[index].flags |= _BV(DEL_ON_FIN);
  }
}

void cortos_class::queueToDelete() { processes[cur_task_ptr].flags |= _BV(DEL_ON_FIN); }

void cortos_class::init() {
  cur_task_ptr = 0;
  processes.clear();
  processes.reserve(TBL_SIZE);
  // task_count = 0;
  // table_size = TBL_SIZE;
  /*
  if (table_size > 0) {
    free(processes);
  }
  processes = (task *)malloc(sizeof(struct task) * TBL_SIZE);
  memset(processes, 0, sizeof(struct task) * TBL_SIZE);
  for (byte i = 0; i < (sizeof(struct task) * TBL_SIZE); i++) {
    Serial.println(((char *)processes)[i]);
  }
  */
}

void cortos_class::scheduler() {
  uint32_t sched_start_run = millis();
  uint16_t pri = 256; // Since equal priorities are ignored, use one larger than possible to initialize.
  bool found = false;
  byte index = 0;
  byte highest_pri = 0;
  byte size_temp = processes.size();
  total_time += sched_start_run - sched_act_last;
  sched_act_last = sched_start_run;
  while (index < size_temp) {
    byte adjusted = (index + cur_task_ptr) % size_temp;
    if (processes[adjusted].exec != NULL) {
      if (processes[adjusted].flags & _BV(SLEEPING)) {
        if ((millis() - processes[adjusted].last_ran) >= processes[adjusted].delay) {
          processes[adjusted].flags &= ~_BV(SLEEPING);
        }
      }
      if ((processes[adjusted].flags & (_BV(SLEEPING) | _BV(LOCKED))) == 0) {
        if (processes[adjusted].priority < pri) {
          found = true;
          pri = processes[adjusted].priority;
          highest_pri = adjusted;
        }
      }
    }
    index++;
  }
  if (found) {
    cur_task_ptr = highest_pri;
    if (processes[highest_pri].exec) {
      processes[highest_pri].exec();
      if (processes[cur_task_ptr].flags & _BV(DEL_ON_FIN)) {
        processes.erase(processes.begin() + cur_task_ptr);
      }
    }
  }
  used_time += millis() - sched_start_run;
  if (total_time > 1000) {
    proc_use = used_time / (total_time / 100.0);
    total_time = 0;
    used_time = 0;
  }
}

cortos_class cortos;
