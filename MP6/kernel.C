/* 
    File: kernel.C

    Author: R. Bettati
            Department of Computer Science
            Texas A&M University
    Date  : 2017/05/02


    This file has the main entry point to the operating system.

    MAIN FILE FOR MACHINE PROBLEM "KERNEL-LEVEL DEVICE MANAGEMENT"

*/

/*--------------------------------------------------------------------------*/
/* DEFINES */
/*--------------------------------------------------------------------------*/

/* -- (none) -- */

#define MB * (0x1 << 20)
#define KB * (0x1 << 10)

/*--------------------------------------------------------------------------*/
/* INCLUDES */
/*--------------------------------------------------------------------------*/

#include "machine.H"         /* LOW-LEVEL STUFF   */
#include "console.H"
#include "gdt.H"
#include "idt.H"             /* EXCEPTION MGMT.   */
#include "irq.H"
#include "exceptions.H"
#include "interrupts.H"

#include "simple_timer.H"    /* TIMER MANAGEMENT  */

#include "frame_pool.H"      /* MEMORY MANAGEMENT */
#include "mem_pool.H"

#include "thread.H"         /* THREAD MANAGEMENT */

#ifdef _USES_SCHEDULER_

#include "scheduler.H"      /* WE WILL NEED A SCHEDULER WITH BlockingDisk */

#endif

#include "scheduler.H"
#include "simple_disk.H"
#include "blocking_disk.H"    /* DISK DEVICE */
#include "mirroring_disk.H"
/*--------------------------------------------------------------------------*/
/* MEMORY MANAGEMENT */
/*--------------------------------------------------------------------------*/

/* -- A POOL OF FRAMES FOR THE SYSTEM TO USE */
FramePool *SYSTEM_FRAME_POOL;

/* -- A POOL OF CONTIGUOUS MEMORY FOR THE SYSTEM TO USE */
MemPool *MEMORY_POOL;

typedef unsigned int size_t;

//replace the operator "new"
void *operator new(size_t size) {
    unsigned long a = MEMORY_POOL->allocate((unsigned long) size);
    return (void *) a;
}

//replace the operator "new[]"
void *operator new[](size_t size) {
    unsigned long a = MEMORY_POOL->allocate((unsigned long) size);
    return (void *) a;
}

//replace the operator "delete"
void operator delete(void *p) {
    MEMORY_POOL->release((unsigned long) p);
}

//replace the operator "delete[]"
void operator delete[](void *p) {
    MEMORY_POOL->release((unsigned long) p);
}

/*--------------------------------------------------------------------------*/
/* SCHEDULER */
/*--------------------------------------------------------------------------*/

#ifdef _USES_SCHEDULER_

/* -- A POINTER TO THE SYSTEM SCHEDULER */
Scheduler *SYSTEM_SCHEDULER;

#endif

/*--------------------------------------------------------------------------*/
/* DISK */
/*--------------------------------------------------------------------------*/

/* -- A POINTER TO THE SYSTEM DISK */
#ifdef _BLOCKING_DISK_
BlockingDisk *SYSTEM_DISK;
#else
#ifdef _MIRRORING_DISK_
MirroringDisk *SYSTEM_DISK;
#else
SimpleDisk *SYSTEM_DISK;
#endif
#endif

#define SYSTEM_DISK_SIZE (10 MB)

#define DISK_BLOCK_SIZE ((1 KB) / 2)

/*--------------------------------------------------------------------------*/
/* JUST AN AUXILIARY FUNCTION */
/*--------------------------------------------------------------------------*/

void pass_on_CPU(Thread *_to_thread) {

#ifndef _USES_SCHEDULER_

    /* We don't use a scheduler. Explicitely pass control to the next
       thread in a co-routine fashion. */
    Thread::dispatch_to(_to_thread);

#else

    /* We use a scheduler. Instead of dispatching to the next thread,
       we pre-empt the current thread by putting it onto the ready
       queue and yielding the CPU. */

    SYSTEM_SCHEDULER->resume(Thread::CurrentThread());
    SYSTEM_SCHEDULER->yield();
#endif
}

/*--------------------------------------------------------------------------*/
/* A FEW THREADS (pointer to TCB's and thread functions) */
/*--------------------------------------------------------------------------*/

Thread *thread1;
Thread *thread2;
Thread *thread3;
Thread *thread4;

void fun1() {
    Console::puts("THREAD: ");
    Console::puti(Thread::CurrentThread()->ThreadId());
    Console::puts("\n");

    Console::puts("FUN 1 INVOKED!\n");

    for (int j = 0;; j++) {
        if (j % 10 == 0 && j != 0) {
            Console::puts("I'm fun1 and I'm still running..\n");
        }

//        Console::puts("FUN 1 IN ITERATION[");
//        Console::puti(j);
//        Console::puts("]\n");

        for (int i = 0; i < 10; i++) {
//            Console::puts("FUN 1: TICK [");
//            Console::puti(i);
//            Console::puts("]\n");
        }

        pass_on_CPU(thread2);
    }
}

void fun2() {
    Console::puts("THREAD: ");
    Console::puti(Thread::CurrentThread()->ThreadId());
    Console::puts("\n");

    Console::puts("FUN 2 INVOKED!\n");

    unsigned char buf[DISK_BLOCK_SIZE];
    int read_block = 1;
    int write_block = 0;

    for (int j = 0;; j++) {

//        Console::puts("FUN 2 IN ITERATION[");
//        Console::puti(j);
//        Console::puts("]\n");

        /* -- Read */
        Console::puts("FUN2: Reading ");
        Console::puti(read_block);
        Console::puts(" block from disk...\n");
        SYSTEM_DISK->read(read_block, buf);

        /* -- Display */
        for (int i = 0; i < DISK_BLOCK_SIZE; i++) {
            Console::putch(buf[i]);
        }
        Console::puts("\n");

        Console::puts("FUN2: Writing ");
        Console::puti(write_block);
        Console::puts(" block to disk...\n");
        SYSTEM_DISK->write(write_block, buf);

        /* -- Move to next block */
        write_block = read_block;
        read_block = (read_block + 1) % 10;

        /* -- Give up the CPU */
        pass_on_CPU(thread3);
    }
}

void fun3() {
    Console::puts("THREAD: ");
    Console::puti(Thread::CurrentThread()->ThreadId());
    Console::puts("\n");

    Console::puts("FUN 3 INVOKED!\n");

    for (int j = 0;; j++) {
        if (j % 10 == 0 && j != 0) {
            Console::puts("I'm fun3 and I'm still running..\n");
        }

//        Console::puts("FUN 3 IN BURST[");
//        Console::puti(j);
//        Console::puts("]\n");

        for (int i = 0; i < 10; i++) {
//            Console::puts("FUN 3: TICK [");
//            Console::puti(i);
//            Console::puts("]\n");
        }

        pass_on_CPU(thread4);
    }
}

void fun4() {
    Console::puts("THREAD: ");
    Console::puti(Thread::CurrentThread()->ThreadId());
    Console::puts("\n");

    Console::puts("FUN 4 INVOKED!\n");

    unsigned char buf[DISK_BLOCK_SIZE];
    int read_block = 1;
    int write_block = 0;

    for (int j = 0;; j++) {

//        Console::puts("FUN 4 IN ITERATION[");
//        Console::puti(j);
//        Console::puts("]\n");

        /* -- Read */
        Console::puts("FUN4: Reading ");
        Console::puti(read_block);
        Console::puts(" block from disk...\n");
        SYSTEM_DISK->read(read_block, buf);

        /* -- Display */
        for (int i = 0; i < DISK_BLOCK_SIZE; i++) {
            Console::putch(buf[i]);
        }
        Console::puts("\n");

        Console::puts("FUN4: Writing ");
        Console::puti(write_block);
        Console::puts(" block to disk...\n");
        SYSTEM_DISK->write(write_block, buf);

        /* -- Move to next block */
        write_block = read_block;
        read_block = (read_block + 1) % 10;

        /* -- Give up the CPU */
        pass_on_CPU(thread1);
    }
}

void disk_test() {
    unsigned char message[DISK_BLOCK_SIZE] = "Far far away, behind the word mountains, far from the countries Vokalia"
                                             " and Consonantia, there live the blind texts. Separated they live in"
                                             " Bookmarksgrove right at the coast of the Semantics, a large language"
                                             " ocean. A small river named Duden flows by their place and supplies it"
                                             " with the necessary regelialia. It is a paradisematic country, in which"
                                             " roasted parts of sentences fly into your mouth. Even the all-powerful"
                                             " Pointing has no control about the blind texts it is an almost"
                                             " unorthographic life One day.\n";
    int read_block = 0;
    int write_block = 0;
    unsigned char buf[DISK_BLOCK_SIZE];

    for (int i = 0; i < 10; ++i) {
        SYSTEM_DISK->write(i, message);
    }

    for (int i = 0; i < 10; ++i) {
        for (int j = DISK_BLOCK_SIZE; j >= 0; j--) {
            /* -- Read */
            SYSTEM_DISK->read(read_block, buf);

            for (int k = j; k < DISK_BLOCK_SIZE; ++k) {
                if (buf[k] == NULL)
                    break;
                buf[k] = NULL;
            }

            /* -- Write */
            SYSTEM_DISK->write(write_block, buf);
        }
        read_block++;
        write_block++;
    }

    for (int i = 0; i < 10; ++i) {
        SYSTEM_DISK->read(i, buf);
        if (buf[i] != NULL) {
            Console::puts("Test fail at block ");
            Console::puti(i);
            Console::puts("\n");
            assert(false)
        }
    }
}

/*--------------------------------------------------------------------------*/
/* MAIN ENTRY INTO THE OS */
/*--------------------------------------------------------------------------*/

int main() {

    GDT::init();
    Console::init();
    IDT::init();
    ExceptionHandler::init_dispatcher();
    IRQ::init();
    InterruptHandler::init_dispatcher();

    /* -- EXAMPLE OF AN EXCEPTION HANDLER -- */

    class DBZ_Handler : public ExceptionHandler {
    public:
        virtual void handle_exception(REGS *_regs) {
            Console::puts("DIVISION BY ZERO!\n");
            for (;;);
        }
    } dbz_handler;

    ExceptionHandler::register_handler(0, &dbz_handler);

    class _15_Handler : public InterruptHandler {
    public:
        virtual void handle_interrupt(REGS *_regs) {
            // Skip interrupt
        }
    } _15_handler;

    InterruptHandler::register_handler(15, &_15_handler);

    /* -- INITIALIZE MEMORY -- */
    /*    NOTE: We don't have paging enabled in this MP. */
    /*    NOTE2: This is not an exercise in memory management. The implementation
                of the memory management is accordingly *very* primitive! */

    /* ---- Initialize a frame pool; details are in its implementation */
    FramePool system_frame_pool;
    SYSTEM_FRAME_POOL = &system_frame_pool;

    /* ---- Create a memory pool of 256 frames. */
    MemPool memory_pool(SYSTEM_FRAME_POOL, 256);
    MEMORY_POOL = &memory_pool;

    /* -- MEMORY ALLOCATOR SET UP. WE CAN NOW USE NEW/DELETE! -- */

    /* -- INITIALIZE THE TIMER (we use a very simple timer).-- */

    /* Question: Why do we want a timer? We have it to make sure that 
                 we enable interrupts correctly. If we forget to do it,
                 the timer "dies". */

    SimpleTimer timer(100); /* timer ticks every 10ms. */
    InterruptHandler::register_handler(0, &timer);
    /* The Timer is implemented as an interrupt handler. */

#ifdef _USES_SCHEDULER_

    /* -- SCHEDULER -- IF YOU HAVE ONE -- */

    SYSTEM_SCHEDULER = new Scheduler();

#endif

    /* -- DISK DEVICE -- */

#ifdef _BLOCKING_DISK_
    SYSTEM_DISK = new BlockingDisk(MASTER, SYSTEM_DISK_SIZE);
    SYSTEM_SCHEDULER->add_disk(SYSTEM_DISK);
#else
#ifdef _MIRRORING_DISK_
    SYSTEM_DISK = new MirroringDisk();
    SYSTEM_SCHEDULER->add_disk(SYSTEM_DISK);
#else
    SYSTEM_DISK = new SimpleDisk(MASTER, SYSTEM_DISK_SIZE);
#endif
#endif

    //Let's test the disk
//    disk_test();

    /* NOTE: The timer chip starts periodically firing as 
             soon as we enable interrupts.
             It is important to install a timer handler, as we 
             would get a lot of uncaptured interrupts otherwise. */

    /* -- ENABLE INTERRUPTS -- */

    Machine::enable_interrupts();

    /* -- MOST OF WHAT WE NEED IS SETUP. THE KERNEL CAN START. */

    Console::puts("Hello World!\n");

    /* -- LET'S CREATE SOME THREADS... */

    Console::puts("CREATING THREAD 1...\n");
    char *stack1 = new char[1024];
    thread1 = new Thread(fun1, stack1, 1024);
    Console::puts("DONE\n");

    Console::puts("CREATING THREAD 2...");
    char *stack2 = new char[2048];
    thread2 = new Thread(fun2, stack2, 2048);
    Console::puts("DONE\n");

    Console::puts("CREATING THREAD 3...");
    char *stack3 = new char[1024];
    thread3 = new Thread(fun3, stack3, 1024);
    Console::puts("DONE\n");

    Console::puts("CREATING THREAD 4...");
    char *stack4 = new char[2048];
    thread4 = new Thread(fun4, stack4, 2048);
    Console::puts("DONE\n");

#ifdef _USES_SCHEDULER_

    /* WE ADD thread2 - thread4 TO THE READY QUEUE OF THE SCHEDULER. */

    SYSTEM_SCHEDULER->add(thread2);
    SYSTEM_SCHEDULER->add(thread3);
    SYSTEM_SCHEDULER->add(thread4);

#endif

    /* -- KICK-OFF THREAD1 ... */

    Console::puts("STARTING THREAD 1 ...\n");
    Thread::dispatch_to(thread1);

    /* -- AND ALL THE REST SHOULD FOLLOW ... */

    assert(false); /* WE SHOULD NEVER REACH THIS POINT. */

    /* -- WE DO THE FOLLOWING TO KEEP THE COMPILER HAPPY. */
    return 1;
}
