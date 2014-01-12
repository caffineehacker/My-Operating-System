#include <cstdint>
#include "floppy.h"
#include "dma.h"
#include "hal.h"
#include "interrupt.h"
#include "ports.h"
#include "physical_memorymgr.h"
#include "filesystem.h"
#include "..\Fat12\fat12.h"
#include "syscall.h"
#include "..\CRT\systemcall.h"

/* FDC irq */
const int FLOPPY_IRQ = 6;

/* Sectors per track */
const int FLPY_SECTORS_PER_TRACK = 18;

uint32_t _CurrentDrive = 0;

/* DMA tranfer buffer starts here and ends at 0x1000+64k */
/* You can change this as needed. It must be below 16MB and in idenitity mapped memory! */
int DMA_BUFFER = 0x1000;

bool floppyDMABufferInitialized = false;

/* FDC uses DMA channel 2 */
const int FDC_DMA_CHANNEL = 2;

/* Set when IRQ fires */
static volatile uint8_t _FloppyDiskIRQ = 0;
 
void interrupt _cdecl i86_flpy_irq()
{
	INTERRUPT_START();

	_FloppyDiskIRQ = 1;

	HARDWARE_INTERRUPT_DONE(FLOPPY_IRQ);
	INTERRUPT_RETURN();
}

/* Wait for floppy disk data */
inline void flpydsk_wait_irq()
{
	/* Wait for irq to fire */
	while (_FloppyDiskIRQ == 0)
		;
	_FloppyDiskIRQ = 0;
}

/* Initialize DMA */
bool flpydsk_initialize_dma(unsigned length)
{
	//union{
	//	uint8_t byte[4]; /* Lo[0], Mid[1], Hi[2] */
	//	unsigned long l;
	//} a, c;

	//a.l=(unsigned)DMA_BUFFER;
	//c.l=(unsigned)length-1;

	///* Check for buffer issues */
	//if ((a.l >> 24) || (c.l >> 16) || (((a.l & 0xffff) + c.l) >> 16))
	//{
	//	return false;
	//}

	//dma_reset(1);
	//dma_mask_channel(FDC_DMA_CHANNEL); /* Mask channel 2 */
	//dma_reset_flipflop(1); /* Flipflop reset on DMA 1 */

	//dma_set_address(FDC_DMA_CHANNEL, a.byte[0],a.byte[1]); /* Buffer address */
	//dma_reset_flipflop(1); /* Flipflop reset on DMA 1 */

	////dma_set_count(FDC_DMA_CHANNEL, c.byte[0],c.byte[1]); /* Set count */
	//dma_set_count(FDC_DMA_CHANNEL, 0xFF, 0x23);
	//dma_set_external_page_register(FDC_DMA_CHANNEL, 0);
	//dma_set_read(FDC_DMA_CHANNEL);

	//dma_unmask_all(1); /* Unmask dma channel 2 */

	if (!floppyDMABufferInitialized)
	{
		DMA_BUFFER = (uint32_t)physical_memorymgr_allocate_block();
		floppyDMABufferInitialized = true;
	}

	outb (0x0a,0x06);	//mask dma channel 2
	outb (0xd8,0xff);	//reset master flip-flop
	outb (0x04, DMA_BUFFER & 0xFF);     //address=0x1000
	outb (0x04, (DMA_BUFFER & 0xFF00) >> 8);
	outb (0xd8, 0xff);  //reset master flip-flop
	outb (0x05, 0xff);  //count to 0x23ff (number of bytes in a 3.5" floppy disk track)
	outb (0x05, 0x23);
	outb (0x80, 0);     //external page register = 0
	outb (0x0a, 0x02);  //unmask dma channel 2

	return true;
}
 
/* Prepare the DMA for read transfer */
void flpydsk_dma_read()
{
	//dma_mask_channel(FDC_DMA_CHANNEL); /* Mask channel 2 */
	outb(0x0a, 0x06);
	outb(DMA0_MODE_REG, DMA_MODE_TRANSFER_SINGLE | DMA_MODE_MASK_AUTO | DMA_MODE_READ_TRANSFER | 0x02); /* Single transfer, address increment, autoinit, read, channel 2 */
	outb(0x0a, 0x02);
	//dma_unmask_all(1); /* Unmask dma channel 2 */
}
 
/* Prepare the DMA for write transfer */
void flpydsk_dma_write()
{
	dma_mask_channel(FDC_DMA_CHANNEL); /* Mask channel 2 */
	outb(DMA0_MODE_REG, DMA_MODE_TRANSFER_SINGLE | DMA_MODE_MASK_AUTO | DMA_MODE_WRITE_TRANSFER | 0x02); /* Single transfer, address increment, autoinit, write, channel 2 */
	dma_unmask_all(1); /* Unmask dma channel 2 */
}

/* Return FDC status */
uint8_t flpydsk_read_status()
{
	/* Just return main status register */
	return inb (FLPYDSK_MSR);
}

/* Write to the FDC digital output register */
void flpydsk_write_dor(uint8_t val)
{
	/* Write the digital output register */
	outb(FLPYDSK_DOR, val);
}

/* Send command byte to FDC */
void flpydsk_send_command(uint8_t cmd)
{
	/* Wait until data register is ready. We send commands to the data register */
	for (int i = 0; i < 500; i++)
		if (flpydsk_read_status() & FLPYDSK_MSR_MASK_DATAREG)
			return outb(FLPYDSK_FIFO, cmd);
}

/* Get data from FDC */
uint8_t flpydsk_read_data()
{
	/* Same as above function but returns data register for reading */
	for (int i = 0; i < 500; i++)
		if (flpydsk_read_status() & FLPYDSK_MSR_MASK_DATAREG)
			return inb(FLPYDSK_FIFO);

	return 0;
}

/* Write to the configuation control register */
void flpydsk_write_ccr(uint8_t val)
{
	/* Write the configuation control */
	outb(FLPYDSK_CTRL, val);
}

/**
*	Controller Command Routines
*/

/* Check interrupt status command */
void flpydsk_check_int(uint32_t* st0, uint32_t* cyl)
{
	flpydsk_send_command(FDC_CMD_CHECK_INT);

	*st0 = flpydsk_read_data();
	*cyl = flpydsk_read_data();
}

/* Turns the current floppy drive's motor on/off */
void flpydsk_control_motor(bool b)
{
	/* Sanity check: invalid drive */
	if (_CurrentDrive > 3)
		return;

	uint8_t motor = 0;

	/* Select the correct mask based on current drive */
	switch (_CurrentDrive)
	{
		case 0:
			motor = FLPYDSK_DOR_MASK_DRIVE0_MOTOR;
			break;
		case 1:
			motor = FLPYDSK_DOR_MASK_DRIVE1_MOTOR;
			break;
		case 2:
			motor = FLPYDSK_DOR_MASK_DRIVE2_MOTOR;
			break;
		case 3:
			motor = FLPYDSK_DOR_MASK_DRIVE3_MOTOR;
			break;
	}

	/* Turn on or off the motor of that drive */
	if (b)
		flpydsk_write_dor(uint8_t(_CurrentDrive | motor | FLPYDSK_DOR_MASK_RESET | FLPYDSK_DOR_MASK_DMA));
	else
		flpydsk_write_dor(FLPYDSK_DOR_MASK_RESET);

	/* In all cases; wait a little bit for the motor to spin up/turn off */
	for (volatile int i = 0; i < 500; i++)
		;
}

/* Configure drive */
void flpydsk_drive_data(uint8_t stepr, uint8_t loadt, uint8_t unloadt, bool dma)
{
	uint8_t data = 0;

	/* Send command */
	flpydsk_send_command(FDC_CMD_SPECIFY);
	data = ((stepr & 0xf) << 4) | (unloadt & 0xf);
	flpydsk_send_command(data);
	
	data = ((loadt << 1) | (dma ? 0 : 1 ));
	flpydsk_send_command(data);
}

/* Calibrates the drive */
int flpydsk_calibrate(uint8_t drive)
{
	uint32_t st0, cyl;

	if (drive >= 4)
		return -2;

	/* Turn on the motor */
	flpydsk_control_motor(true);

	for (int i = 0; i < 10; i++)
	{
		/* Send command */
		flpydsk_send_command(FDC_CMD_CALIBRATE);
		flpydsk_send_command(drive);
		flpydsk_wait_irq();
		flpydsk_check_int(&st0, &cyl);

		/* Did we find cylinder 0? if so, we are done */
		if (!cyl)
		{
			flpydsk_control_motor(false);
			return 0;
		}
	}

	flpydsk_control_motor(false);
	return -1;
}

/* Disable controller */
void flpydsk_disable_controller()
{
	flpydsk_write_dor(0);
}

/* Enable controller */
void flpydsk_enable_controller()
{
	flpydsk_write_dor(FLPYDSK_DOR_MASK_RESET | FLPYDSK_DOR_MASK_DMA);
}

/* Reset controller */
void flpydsk_reset()
{
	uint32_t st0, cyl;

	/* Reset the controller */
	flpydsk_disable_controller();
	flpydsk_enable_controller();
	flpydsk_wait_irq();

	/* Send CHECK_INT/SENSE INTERRUPT command to all drives */
	for (int i = 0; i < 4; i++)
		flpydsk_check_int(&st0, &cyl);

	/* Transfer speed 500kb/s */
	flpydsk_write_ccr(0);

	/* Pass mechanical drive info. steprate=3ms, unload time=240ms, load time=16ms */
	flpydsk_drive_data(3, 16, 240, true);

	/* Calibrate the disk */
	flpydsk_calibrate(_CurrentDrive);
}

/* Read a sector */
void flpydsk_read_sector_imp(uint8_t head, uint8_t track, uint8_t sector)
{
	uint32_t st0, cyl;

	/* Set the DMA for read transfer */
	//dma_set_read(FDC_DMA_CHANNEL);
	outb (0x0a, 0x06); //mask dma channel 2
	outb (0x0b, 0x56); //single transfer, address increment, autoinit, read, channel 2
	outb (0x0a, 0x02); //unmask dma channel 2

	/* Read in a sector */
	flpydsk_send_command(FDC_CMD_READ_SECT | FDC_CMD_EXT_MULTITRACK | FDC_CMD_EXT_SKIP | FDC_CMD_EXT_DENSITY);
	flpydsk_send_command(head << 2 | _CurrentDrive);
	flpydsk_send_command(track);
	flpydsk_send_command(head);
	flpydsk_send_command(sector);
	flpydsk_send_command(FLPYDSK_SECTOR_DTL_512);
	flpydsk_send_command(((sector + 1) >= FLPY_SECTORS_PER_TRACK) ? FLPY_SECTORS_PER_TRACK : sector + 1);
	flpydsk_send_command(FLPYDSK_GAP3_LENGTH_3_5);
	flpydsk_send_command(0xff);

	/* Wait for irq */
	flpydsk_wait_irq();

	/* Read status info */
	for (int j = 0; j < 7; j++)
		flpydsk_read_data();

	/* Let FDC know we handled interrupt */
	flpydsk_check_int(&st0, &cyl);
}

/* Seek to given track/cylinder */
int flpydsk_seek(uint8_t cyl, uint8_t head)
{
	uint32_t st0, cyl0;

	if (_CurrentDrive >= 4)
		return -1;

	//flpydsk_check_int(&st0, &cyl0); // No need to seek if we're already here
	//if (cyl0 == cyl)
	//	return 0;

	for (int i = 0; i < 10; i++)
	{
		/* Send the command */
		flpydsk_send_command(FDC_CMD_SEEK);
		flpydsk_send_command((head) << 2 | _CurrentDrive);
		flpydsk_send_command(cyl);

		/* Wait for the results phase IRQ */
		flpydsk_wait_irq();
		flpydsk_check_int(&st0, &cyl0);

		/* Found the cylinder? */
		if (cyl0 == cyl)
			return 0;
	}

	return -1;
}

/*============================================================================
 *    INTERFACE FUNCTIONS
 *==========================================================================*/

/* Convert LBA to CHS */
void flpydsk_lba_to_chs(int lba, int *head, int *track, int *sector)
{
   *head = (lba % (FLPY_SECTORS_PER_TRACK * 2)) / (FLPY_SECTORS_PER_TRACK);
   *track = lba / (FLPY_SECTORS_PER_TRACK * 2);
   *sector = lba % FLPY_SECTORS_PER_TRACK + 1;
}

void* flpydsk_handleReadSectorRequest(void* data)
{
	return flpydsk_read_sector((uint32_t)data);
}

/* Install floppy driver */
void flpydsk_install(int irq)
{
	/* Install irq handler */
	InstallInterruptHandler(irq, i86_flpy_irq);
	SetSyscallFunctionHandler(flpydsk_handleReadSectorRequest, SYSCALL_READ_FLOPPY_SECTOR);

	/* Initialize DMA */
	flpydsk_initialize_dma(512);

	/* Reset the fdc */
	flpydsk_reset();

	/* Set drive information */
	flpydsk_drive_data(13, 1, 0xf, true);
}

/* Set current working drive */
void flpydsk_set_working_drive(uint8_t drive)
{
	if (drive < 4)
		_CurrentDrive = drive;
}

/* Get current working drive */
uint8_t flpydsk_get_working_drive()
{
	return _CurrentDrive;
}

/* Read a sector */
uint8_t* flpydsk_read_sector(int sectorLBA)
{
	if (_CurrentDrive >= 4)
		return 0;

	/* Convert LBA sector to CHS */
	int head=0, track=0, sector=1;
	flpydsk_lba_to_chs(sectorLBA, &head, &track, &sector);

	/* Turn motor on and seek to track */
	flpydsk_control_motor(true);
	if (flpydsk_seek((uint8_t)track, (uint8_t)head) != 0)
		return 0;

	/* Read sector and turn motor off */
	flpydsk_read_sector_imp((uint8_t)head, (uint8_t)track, (uint8_t)sector);
	flpydsk_control_motor(false);

	/* Warning: this is a bit hackish */
	return (uint8_t*)DMA_BUFFER;
}

/* Mount a filesystem from the floppy disk.  Assumes the floppy is formatted as Fat12 */
void flpydsk_mount_filesystem()
{
	/* Boot sector info */
	PFAT12_BOOTSECTOR bootsector;

	/* Read boot sector */
	bootsector = (PFAT12_BOOTSECTOR)flpydsk_read_sector(0);

	// TODO: There is a bug here!!!!!
	volRegisterFileSystem(fat12_mount(bootsector), 0);
}
