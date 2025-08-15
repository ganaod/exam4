# SIGNALS: EMANATION CALCULUS
## From Fundamental Reality to Implementation

---

## LEVEL 0: THE PRIMORDIAL PROBLEM
### Why Signals Must Exist

**The Fundamental Tension**: 
- **Processes execute sequentially** - one instruction after another
- **Events occur asynchronously** - hardware interrupts, timeouts, user actions
- **Reality is not sequential** - keyboard presses, timer expirations, memory faults happen at arbitrary times

**The Necessity**: Some mechanism must bridge **asynchronous reality** with **synchronous process execution**.

**Without Signals**: A process could never be interrupted. A hanging program would be unkillable. Hardware errors would crash the entire system.

---

## LEVEL 1: THE KERNEL AS EVENT MEDIATOR
### The Operating System's Role

**Fundamental Architecture**:
```
Asynchronous Reality → Kernel → Synchronous Process
     (events)       (mediator)    (sequential code)
```

**The Kernel's Job**:
1. **Detect asynchronous events** (timer expiry, memory fault, user input)
2. **Translate events into process-specific notifications**
3. **Deliver notifications at process-safe moments**
4. **Allow processes to define responses**

**Key Insight**: The kernel becomes a **universal translator** between chaotic reality and orderly process execution.

---

## LEVEL 2: THE SIGNAL ABSTRACTION
### What Signals Actually Are

**Conceptual Definition**: A signal is a **software interrupt** - an asynchronous notification delivered by the kernel to a process.

**The Signal as Message**:
- **Sender**: Kernel (representing reality)
- **Receiver**: Process
- **Content**: "Event X has occurred"
- **Delivery**: Asynchronous (can happen anytime)

**Signal Numbers as Event Types**:
```c
SIGALRM  = "Timer expired"
SIGSEGV  = "Memory access violation" 
SIGINT   = "User pressed Ctrl+C"
SIGKILL  = "Terminate immediately"
SIGCHLD  = "Child process died"
```

**The Enumeration**: Each signal number represents a **different category of asynchronous event**.

---

## LEVEL 3: SIGNAL DELIVERY MECHANICS
### How the Kernel Delivers Signals

**The Delivery Process**:
1. **Event occurs** (timer expires, memory fault, etc.)
2. **Kernel marks process** with pending signal
3. **Process reaches safe execution point** (system call boundary, return from kernel)
4. **Kernel checks for pending signals**
5. **Signal handler executes** (or default action taken)
6. **Process resumes normal execution**

**Critical Timing**: Signals are delivered at **kernel transition points**, not during arbitrary instruction execution.

**The Signal Mask**: Process can temporarily **block signal delivery**:
```c
sigset_t mask;
sigemptyset(&mask);
sigaddset(&mask, SIGALRM);
sigprocmask(SIG_BLOCK, &mask, NULL);  // Block SIGALRM delivery
```

---

## LEVEL 4: SIGNAL HANDLERS AS EVENT-DRIVEN PROGRAMMING
### The Handler Paradigm

**Conceptual Model**: Signal handlers implement **event-driven programming** within otherwise sequential processes.

**Handler Execution**:
```c
void signal_handler(int sig) {
    // This code runs asynchronously
    // Interrupts normal program flow
    // Must be carefully designed
}
```

**The Execution Context Switch**:
```
Normal execution → Signal arrives → Handler executes → Resume normal execution
    main()             kernel           handler()           main()
```

**Handler Constraints**:
- **Limited function calls allowed** (async-signal-safe only)
- **No heap allocation** (malloc not safe)
- **Minimal logic** (can be interrupted by other signals)
- **Quick execution** (blocking operations dangerous)

---

## LEVEL 5: SIGNAL TYPES AND PURPOSES
### The Signal Taxonomy

**Process Control Signals**:
```c
SIGTERM  // Polite termination request
SIGKILL  // Immediate termination (uncatchable)
SIGSTOP  // Pause process (uncatchable)
SIGCONT  // Resume paused process
```

**Error Signals** (Hardware Exceptions):
```c
SIGSEGV  // Segmentation fault (memory violation)
SIGFPE   // Floating point exception (divide by zero)
SIGILL   // Illegal instruction
SIGBUS   // Bus error (alignment, hardware)
```

**Timer/Alarm Signals**:
```c
SIGALRM  // alarm() timer expired
SIGVTALRM // Virtual timer (CPU time)
SIGPROF   // Profiling timer
```

**Communication Signals**:
```c
SIGPIPE  // Broken pipe (reader died)
SIGCHLD  // Child process died
SIGUSR1/2 // User-defined signals
```

**Interactive Signals**:
```c
SIGINT   // Ctrl+C (interrupt)
SIGQUIT  // Ctrl+\ (quit with core dump)
SIGTSTP  // Ctrl+Z (terminal stop)
```

---

## LEVEL 6: SIGNAL ACTIONS AND CONTROL
### How Processes Respond to Signals

**Three Possible Actions**:
1. **Default action** (terminate, ignore, stop, core dump)
2. **Ignore signal** (SIG_IGN)
3. **Custom handler** (user-defined function)

**Signal Action Configuration**:
```c
struct sigaction sa;
sa.sa_handler = custom_handler;  // or SIG_DFL, SIG_IGN
sa.sa_flags = 0;                 // Control flags
sigemptyset(&sa.sa_mask);        // Signals blocked during handler
sigaction(SIGALRM, &sa, NULL);   // Install handler
```

**Signal Masking During Handler**:
- **sa.sa_mask**: Additional signals to block during handler execution
- **Automatic masking**: Signal being handled is blocked by default
- **Prevents signal recursion**: Handler won't be interrupted by same signal

---

## LEVEL 7: ADVANCED SIGNAL CONCEPTS
### Sophisticated Signal Control

**Signal Flags** (sa_flags):
```c
SA_RESTART    // Restart interrupted system calls
SA_NODEFER    // Don't block signal during handler
SA_RESETHAND  // Reset to SIG_DFL after one delivery
SA_SIGINFO    // Provide detailed signal information
```

**The SA_RESTART Problem**: 
- **Without SA_RESTART**: System calls return EINTR when interrupted
- **With SA_RESTART**: System calls automatically resume after handler
- **Critical Choice**: Affects program behavior fundamentally

**Signal Queuing**:
- **Standard signals**: Not queued (multiple deliveries = one delivery)
- **Real-time signals**: Queued (every delivery preserved)
- **Implications**: Standard signals can be "lost" if delivered repeatedly

---

## LEVEL 8: SIGNAL-BASED PATTERNS
### Common Signal Usage Patterns

**Pattern 1: Timeout Implementation**
```c
alarm(seconds);           // Schedule SIGALRM
// ... do work ...
// If SIGALRM arrives, handler executes
```

**Pattern 2: Graceful Shutdown**
```c
signal(SIGTERM, cleanup_handler);
// Handler sets flag, main loop checks flag and exits cleanly
```

**Pattern 3: Inter-Process Communication**
```c
kill(other_pid, SIGUSR1);  // Send signal to other process
// Other process has handler for SIGUSR1
```

**Pattern 4: Child Process Monitoring**
```c
signal(SIGCHLD, child_handler);
// Handler calls wait() to collect dead children
```

---

## LEVEL 9: SIGNAL SAFETY AND PITFALLS
### The Async-Signal-Safe Problem

**The Fundamental Issue**: Most library functions are **not async-signal-safe**.

**Why Unsafe**: 
- **Shared data structures** (malloc heap, stdio buffers)
- **Non-atomic operations** (updating complex data)
- **Deadlock potential** (signal interrupts function that holds lock)

**Safe Functions** (async-signal-safe):
```c
write()    // Direct system call
read()     // Direct system call  
exit()     // Direct system call
signal()   // Signal management
kill()     // Signal sending
```

**Unsafe Functions**:
```c
printf()   // Uses buffered I/O
malloc()   // Manages heap data structures
pthread_mutex_lock()  // Threading primitives
```

**Best Practice**: Handlers should only **set flags** or use **async-signal-safe functions**.

---

## LEVEL 10: THE PHILOSOPHICAL FOUNDATION
### Why This Architecture Exists

**The Design Principles**:
1. **Asynchronous events are fundamental to computing**
2. **Processes need protection from each other**
3. **The kernel must mediate all cross-process communication**
4. **Simple, uniform interface better than complex special cases**
5. **Default behaviors should be safe**

**Alternative Approaches** (not chosen):
- **Polling**: Process constantly checks for events (inefficient)
- **Callbacks**: Direct hardware → software callbacks (unsafe)
- **Exceptions**: Language-level exception handling (limited scope)

**Why Signals Won**: 
- **Universal**: Works for all event types
- **Efficient**: No polling overhead
- **Safe**: Kernel-mediated delivery
- **Flexible**: Customizable responses

---

## FUNDAMENTAL PRINCIPLES FOR MASTERY

1. **Signals bridge asynchronous reality with synchronous processes**
2. **The kernel is the universal event translator and mediator**
3. **Signal delivery happens at safe execution boundaries**
4. **Handlers implement event-driven programming within sequential programs**
5. **Signal safety requires careful function selection**
6. **Signal masking prevents dangerous recursion and timing issues**
7. **Different signals represent different categories of asynchronous events**
8. **Signal actions (default/ignore/custom) provide flexible response control**

**The Meta-Insight**: Signals are the operating system's solution to the fundamental problem that **reality is asynchronous but programs are sequential**. Every signal mechanism emerges from this core necessity.