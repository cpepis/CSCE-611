/*
     File        : blocking_disk.H

     Author      : Chrysanthos Pepi

     Date        : 08 APR 21
     Description : 

*/

#ifndef _BLOCKING_DISK_H_
#define _BLOCKING_DISK_H_

/*--------------------------------------------------------------------------*/
/* DEFINES */
/*--------------------------------------------------------------------------*/

/* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* INCLUDES */
/*--------------------------------------------------------------------------*/

#include "queue.H"
#include "interrupts.H"
#include "simple_disk.H"

/*--------------------------------------------------------------------------*/
/* DATA STRUCTURES */
/*--------------------------------------------------------------------------*/

/* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* B l o c k i n g D i s k  */
/*--------------------------------------------------------------------------*/

class BlockingDisk : public SimpleDisk, public InterruptHandler {
private:
    Queue blocked_queue;

public:
    BlockingDisk(DISK_ID _disk_id, unsigned int _size);
    /* Creates a BlockingDisk device with the given size connected to the
       MASTER or SLAVE slot of the primary ATA controller.
       NOTE: We are passing the _size argument out of laziness.
       In a real system, we would infer this information from the
       disk controller. */

    /* DISK OPERATIONS */

    virtual void read(unsigned long _block_no, unsigned char *_buf);

    /* Reads 512 Bytes from the given block of the disk and copies them
       to the given buffer. No error check! */

    virtual void write(unsigned long _block_no, unsigned char *_buf);

    /* Writes 512 Bytes from the buffer to the given block on the disk. */

    void issue_operation(DISK_OPERATION _op, unsigned long _block_no);

    void handle_interrupt(REGS *_regs);

    void wait_until_ready();

    Thread *get_thread();

    bool is_empty();

    bool is_ready();
};

#endif
