
#include "PIC.h"
#include "AssemblyUtility.h"


void kInitializePIC()
{
	// ICW1: Edge trigger, Cascade, IC4
	kOutPortByte(PIC_MASTER_PORT1, 0x11);

	// ICW2: IRQ vector starting number = 0x20
	kOutPortByte(PIC_MASTER_PORT2, PIC_IRQSTARTVECTOR);

	// ICW3: Slave PIC is connected on the second pin (2^2 = 4)
	kOutPortByte(PIC_MASTER_PORT2, 0x04);

	// ICW4: uPM(8086/88)
	kOutPortByte(PIC_MASTER_PORT2, 0x01);



	// ICW1: Edge trigger, Cascade, IC4
	kOutPortByte(PIC_SLAVE_PORT1, 0x11);

	// ICW2: IRQ vector starting number = 0x28
	kOutPortByte(PIC_SLAVE_PORT2, PIC_IRQSTARTVECTOR + 8);

	// ICW3: Slave PIC is connected on the second(2) pin
	kOutPortByte(PIC_SLAVE_PORT2, 0x02);

	// ICW4: uPM(8086/88)
	kOutPortByte(PIC_SLAVE_PORT2, 0x01);
}


// IMR setting
void kMaskPICInterrupt(WORD wIRQBitmask)
{
	// OCW1
	kOutPortByte(PIC_MASTER_PORT2, (BYTE)wIRQBitmask);

	// OCW1
	kOutPortByte(PIC_SLAVE_PORT2, (BYTE)(wIRQBitmask >> 8));
}


void kSendEOIToPIC(int iIRQNumber)
{
	// Send EOI to the master
	// Slave is connected to the master pin,
    //  so the master should receive the EOI for every case
	kOutPortByte(PIC_MASTER_PORT1, 0x20);

	// Send EOI to the slave
	if (iIRQNumber >= 8)
		kOutPortByte(PIC_SLAVE_PORT1, 0x20);
}
