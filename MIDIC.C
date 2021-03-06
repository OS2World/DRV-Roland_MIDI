/****************************************************/
/*                                                  */
/*  Musical Instrument Digital Interface OS/2       */
/*  Device Driver, primarily written in Microsoft   */
/*  C (tm).                                         */
/*                                                  */
/*  Copyright IBM 1991.                             */
/*                                                  */
/*                                                  */
/****************************************************/



#include "midic.h"

/***************************************************/
/*                                                 */
/* Device Header must be the FIRST data area       */
/*                                                 */
/***************************************************/

DeviceHeader devhdr = {
  (void far *) -1,
  (DAW_CHR | DAW_OPN | DAW_LEVEL),
  (void near *) STRAT,
  (void near *) 0,
  {'M','I','D','I','$',' ',' ',' '},
  {0}
};

/****************************************************/
/*                                                  */
/* These next variables must be initialized all to  */
/* zero.  Also, the MsgData is used not only to     */
/* display the Device Driver message during the     */
/* boot process, but is also used to write the end  */
/* of data offset into the Request Header.          */
/*                                                  */
/****************************************************/

unsigned far *DevHlp=0L;/* Storage area for DevHlps */
unsigned char rx_queue[10]={0}; /* receiver queue   */
unsigned char tx_queue[10]={0}; /* transmitter queue*/

InitExit far  *InitExt=0L;   /* pointer to InitExit */
DeviceRead far *DevRead=0L;  /* points to read strc */
DeviceWrite far *DevWrite=0L;/* points to write str */

unsigned char MsgData[] = "Midi Device Driver  \r\n";



/*********************************/
/*                               */
/* Device Initialization Routine */
/* is called only once during    */
/* the OS/2 BOOT process.  The   */
/* address of the DevHlp         */
/*                               */
/*********************************/

void Init(InitEntry far *InitPtr, int dev)
{

    DevHlp = (unsigned far *)InitPtr->DevHlp;
                                /* callable DevHlp  */
                                /* entry point..    */



    /*********************************/
    /*                               */
    /* During Initialization we can  */
    /* install the interrupt handler */
    /* which will allow OS/2 to know */
    /* where the code is that will   */
    /* run, each time the interrupt  */
    /* 9 happens.                    */
    /*                               */
    /*********************************/

    DOSPUTMESSAGE(1, 8, devhdr.name);
    DOSPUTMESSAGE(1,strlen(MsgData),MsgData);


    SetIRQ(9,(void near *)INT_HNDLR,0);


    /* output initialization message */

    /* send back our cs and ds end values to os/2 */

    InitExt           = (InitExit far *)InitPtr;
    InitExt->code_off = (unsigned short)last_code;
    InitExt->data_off = (unsigned short)MsgData+
                        strlen(MsgData);

    return;
}

/* common entry point for calls to strat routines */

void main(ReqHeader far *rp, int dev )
{

    switch(rp->req_cmd)
    {
      case INIT:

        Init((InitEntry far *)rp,dev);

        rp->req_stat = DONE;
        return;

      case OPEN:

        /****************************/
        /*                          */
        /*  During OPEN, place MPU  */
        /*  into "DUMB" mode.       */
        /*                          */
        /****************************/

        outp(MIDI_CMD,DUMB_MODE);

        rp->req_stat = DONE;
        return;

      case CLOSE:

        /****************************/
        /*                          */
        /*  During CLOSE, reset MPU */
        /*  mode.                   */
        /*                          */
        /****************************/

        outp(MIDI_CMD,MPU_RESET);
        rp->req_stat = DONE;
        return;

      case READ:


        rx_queue[0] = inp(MIDI_DATA);
        DevRead     = (DeviceRead far *)rp;

        ReadBytes((unsigned long)DevRead->buff_addr,
                  (unsigned long)rx_queue,
                  1);

        rp->req_stat = DONE;
        return;


      case WRITE:

        DevWrite = (DeviceWrite far *)rp;

        WriteBytes((unsigned long)DevWrite->buff_addr,
                   (unsigned long)tx_queue,
                   1);

        rp->req_stat = DONE;
        return;

      case IOCTL:
        rp->req_stat = DONE;
        return;

      default:
        rp->req_stat = DONE;
        return;

    }

}

/***************************/
/*                         */
/* This interrupt handler  */
/* is called by _INT_HNDLR */
/* from the assembler pro- */
/* cedure.  After return-  */
/* ing, the DevHlp End of  */
/* Interrupt function.     */
/*                         */
/***************************/

void near interrupt_handler ()
{
  rx_queue[0] = inp(0x0330);
}

/**************************/
/*                        */
/* Last code offset....   */
/*                        */
/**************************/

void last_code() {}

