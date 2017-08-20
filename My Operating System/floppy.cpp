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
#include "PIT8253.h"

/* FDC irq */
const int FLOPPY_IRQ = 6;

/* Sectors per track */
const int FLPY_SECTORS_PER_TRACK = 18;
const int FLPY_HEADS_PER_CYLINDER = 2;

uint32_t _CurrentDrive = 0;

/* DMA tranfer buffer starts here and ends at 0x8000+64k */
/* You can change this as needed. It must be below 16MB and in idenitity mapped memory! */
int DMA_BUFFER = 0x8000;

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
		sleep_ms(10);

	_FloppyDiskIRQ = 0;
}

/* Initialize DMA */
bool flpydsk_initialize_dma(bool read)
{
	if (!floppyDMABufferInitialized)
	{
		// TODO: For now we're using a fixed memory location...
		//DMA_BUFFER = (uint32_t)physical_memorymgr_allocate_block();
		floppyDMABufferInitialized = true;
	}

	outb(0x0a, 0x06);	//mask dma channel 2
	outb(0x0c, 0xff);	//reset master flip-flop
	outb(0x04, DMA_BUFFER & 0xFF);
	outb(0x04, (DMA_BUFFER & 0xFF00) >> 8);
	outb(0x81, 0); //external page register = 0

	outb(0x0c, 0xff);  //reset master flip-flop
	outb(0x05, 0xff);  //count to 0x47ff (max size)
	outb(0x05, 0x47);

	if (read) {
		outb(0x0b, 0x46); //single transfer, address increment, no-auto, read, channel 2
	}
	else {
		outb(0x0b, 0x4a); //single transfer, address increment, no-auto, write, channel 2
	}

	outb(0x0a, 0x02);  //unmask dma channel 2

	return true;
}

/* Prepare the DMA for read transfer */
void flpydsk_dma_read()
{
	//dma_mask_channel(FDC_DMA_CHANNEL); /* Mask channel 2 */
	outb(0x0a, 0x06); // Mask dma channel 2
	outb(DMA0_MODE_REG, DMA_MODE_TRANSFER_SINGLE | DMA_MODE_READ_TRANSFER | 0x02); /* read channel 2 */
	outb(0x0a, 0x02); // Unmask dma channel 2
	//dma_unmask_all(1); /* Unmask dma channel 2 */
}

/* Prepare the DMA for write transfer */
void flpydsk_dma_write()
{
	dma_mask_channel(FDC_DMA_CHANNEL); /* Mask channel 2 */
	outb(DMA0_MODE_REG, DMA_MODE_TRANSFER_SINGLE | DMA_MODE_WRITE_TRANSFER | 0x02); /* Single transfer, address increment, autoinit, write, channel 2 */
	dma_unmask_all(1); /* Unmask dma channel 2 */
}

/* Return FDC status */
uint8_t flpydsk_read_status()
{
	/* Just return main status register */
	return inb(FLPYDSK_MSR);
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
	for (int i = 0; i < 600; i++)
	{
		if (flpydsk_read_status() & FLPYDSK_MSR_MASK_DATAREG)
		{
			return outb(FLPYDSK_FIFO, cmd);
		}
		sleep_ms(10);
	}
}

/* Get data from FDC */
uint8_t flpydsk_read_data()
{
	/* Same as above function but returns data register for reading */
	for (int i = 0; i < 600; i++)
	{
		if (flpydsk_read_status() & FLPYDSK_MSR_MASK_DATAREG)
		{
			return inb(FLPYDSK_FIFO);
		}

		sleep_ms(10);
	}

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
	// TODO: record the state of the motor and only turn on if not already on
	if (b)
	{
		flpydsk_write_dor(uint8_t(motor | FLPYDSK_DOR_MASK_RESET | FLPYDSK_DOR_MASK_DMA));
		sleep_ms(500);
	}
	else
	{
		// TODO: Don't shut off the motor, but start a count down so it will turn off automatically when not needed
		flpydsk_write_dor(FLPYDSK_DOR_MASK_RESET | FLPYDSK_DOR_MASK_DMA);
	}
}

/* Configure drive */
void flpydsk_drive_data(uint8_t stepr, uint8_t loadt, uint8_t unloadt, bool dma)
{
	uint8_t data = 0;

	/* Send command */
	flpydsk_send_command(FDC_CMD_SPECIFY);
	data = ((stepr & 0xf) << 4) | (unloadt & 0xf);
	flpydsk_send_command(data);

	data = ((loadt << 1) | (dma ? 0 : 1));
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
		_FloppyDiskIRQ = 0;
		flpydsk_send_command(FDC_CMD_CALIBRATE);
		flpydsk_send_command(drive);
		flpydsk_wait_irq();
		flpydsk_check_int(&st0, &cyl);

		if (st0 & 0xC0) {
			//printk("Drive 0 is an invalid drive!");
			// Let's crash until we can print...
			_asm {
				int 0
			}
		}

		/* Did we find cylinder 0? if so, we are done */
		if (!cyl)
		{
			flpydsk_control_motor(false);
			return 0;
		}
	}

	// Let's crash until we can print...
	_asm {
		int 0
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
	// Reset this flag so we can know when the drive is enabled.
	_FloppyDiskIRQ = 0;
	flpydsk_write_dor(FLPYDSK_DOR_MASK_RESET | FLPYDSK_DOR_MASK_DMA);
}

/* Reset controller */
int flpydsk_reset()
{
	/* Reset the controller */
	flpydsk_disable_controller();
	_FloppyDiskIRQ = 0;
	flpydsk_enable_controller();
	flpydsk_wait_irq();

	/* Send CHECK_INT/SENSE INTERRUPT command to all drives */
	for (int i = 0; i < 4; i++)
	{
		uint32_t st0, cyl;
		flpydsk_check_int(&st0, &cyl);
	}

	/* Transfer speed 500kb/s */
	flpydsk_write_ccr(0);

	/* Pass mechanical drive info. steprate=3ms, unload time=240ms, load time=16ms, no-dma */
	flpydsk_drive_data(0xd /* 3ms */, 1 /* 16ms */, 0xf /* 240ms */, true);

	/* Calibrate the disk */
	if (flpydsk_calibrate(_CurrentDrive)) return -1;

	return 0;
}

/* Read a sector */
int flpydsk_read_sector_imp(uint8_t head, uint8_t cyl, uint8_t sector)
{
	for (int i = 0; i < 20; i++)
	{
		flpydsk_control_motor(true);

		flpydsk_initialize_dma(true);

		sleep_ms(100);

		/* Read in a sector */
		_FloppyDiskIRQ = 0;
		flpydsk_send_command(FDC_CMD_READ_SECT | FDC_CMD_EXT_MULTITRACK | FDC_CMD_EXT_DENSITY);
		flpydsk_send_command(head << 2); // Head and drive 0:0:0:0:0:HD:US1:US0
		flpydsk_send_command(cyl);
		flpydsk_send_command(head);
		flpydsk_send_command(sector);
		flpydsk_send_command(FLPYDSK_SECTOR_DTL_512);
		//flpydsk_send_command(((sector + 1) >= FLPY_SECTORS_PER_TRACK) ? FLPY_SECTORS_PER_TRACK : sector + 1);
		flpydsk_send_command(FLPY_SECTORS_PER_TRACK);
		flpydsk_send_command(FLPYDSK_GAP3_LENGTH_3_5);
		flpydsk_send_command(0xff);

		/* Wait for irq */
		flpydsk_wait_irq();

		/* Read status info */
		unsigned char st0 = flpydsk_read_data();
		unsigned char st1 = flpydsk_read_data();
		unsigned char st2 = flpydsk_read_data();

		unsigned char rcy = flpydsk_read_data();
		unsigned char rhe = flpydsk_read_data();
		unsigned char rse = flpydsk_read_data();
		// bytes per sector, should be what we programmed in
		unsigned char bps = flpydsk_read_data();

		int error = 0;

		if (st0 & 0xC0) {
			//static const char * status[] =
			//{ 0, "error", "invalid command", "drive not ready" };
			//printk("floppy_do_sector: status = %s\n", status[st0 >> 6]);
			error = 1;
		}
		if (st1 & 0x80) {
			//printk("floppy_do_sector: end of cylinder\n");
			error = 1;
		}
		if (st0 & 0x08) {
			//printk("floppy_do_sector: drive not ready\n");
			error = 1;
		}
		if (st1 & 0x20) {
			//printk("floppy_do_sector: CRC error\n");
			error = 1;
		}
		if (st1 & 0x10) {
			//printk("floppy_do_sector: controller timeout\n");
			error = 1;
		}
		if (st1 & 0x04) {
			//printk("floppy_do_sector: no data found\n");
			error = 1;
		}
		if ((st1 | st2) & 0x01) {
			//printk("floppy_do_sector: no address mark found\n");
			error = 1;
		}
		if (st2 & 0x40) {
			//printk("floppy_do_sector: deleted address mark\n");
			error = 1;
		}
		if (st2 & 0x20) {
			//printk("floppy_do_sector: CRC error in data\n");
			error = 1;
		}
		if (st2 & 0x10) {
			//printk("floppy_do_sector: wrong cylinder\n");
			error = 1;
		}
		if (st2 & 0x04) {
			//printk("floppy_do_sector: uPD765 sector not found\n");
			error = 1;
		}
		if (st2 & 0x02) {
			//printk("floppy_do_sector: bad cylinder\n");
			error = 1;
		}
		if (bps != 0x2) {
			//printk("floppy_do_sector: wanted 512B/sector, got %d", (1 << (bps + 7)));
			error = 1;
		}
		if (st1 & 0x02) {
			//printk("floppy_do_sector: not writable\n");
			error = 2;
		}

		if (!error) {
			return 0;
		}
		if (error > 1) {
			//printk("floppy_do_sector: not retrying..\n");
			_asm {
				int 0
			}
			return -2;
		}
	}

	_asm {
		int 0
	}

	return -1;
}

/* Seek to given track/cylinder */
int flpydsk_seek(uint8_t cyl, uint8_t head)
{
	uint32_t st0, cyl0;

	if (_CurrentDrive >= 4)
		return -1;


	flpydsk_control_motor(true);

	//flpydsk_check_int(&st0, &cyl0); // No need to seek if we're already here
	//if (cyl0 == cyl)
	//	return 0;

	for (int i = 0; i < 10; i++)
	{
		/* Send the command */
		_FloppyDiskIRQ = 0;
		flpydsk_send_command(FDC_CMD_SEEK);
		flpydsk_send_command(head << 2);
		flpydsk_send_command(cyl);

		/* Wait for the results phase IRQ */
		flpydsk_wait_irq();
		flpydsk_check_int(&st0, &cyl0);

		if (st0 & 0xC0) {
			_asm {
				int 0
			}
		}

		/* Found the cylinder? */
		if (cyl0 == cyl)
			return 0;

		sleep_ms(100);
	}

	_asm {
		int 0
	}

	return -1;
}

/*============================================================================
 *    INTERFACE FUNCTIONS
 *==========================================================================*/

 /* Convert LBA to CHS */
void flpydsk_lba_to_chs(int lba, int *cyl, int *head, int *sector)
{
	*cyl = lba / (FLPY_HEADS_PER_CYLINDER * FLPY_SECTORS_PER_TRACK);
	*head = (lba / FLPY_SECTORS_PER_TRACK) % FLPY_HEADS_PER_CYLINDER;
	*sector = (lba % FLPY_SECTORS_PER_TRACK) + 1;
}

void* flpydsk_handleReadSectorRequest(void* data)
{
	/* Initialize DMA */
	//flpydsk_initialize_dma(true);

	/* Reset the fdc */
	//flpydsk_reset();

	return flpydsk_read_sector((uint32_t)data);
}

/* Install floppy driver */
void flpydsk_install(int irq)
{
	/* Install irq handler */
	InstallInterruptHandler(irq, i86_flpy_irq);
	SetSyscallFunctionHandler(flpydsk_handleReadSectorRequest, SYSCALL_READ_FLOPPY_SECTOR);

	/* Initialize DMA */
	flpydsk_initialize_dma(true);

	/* Reset the fdc */
	flpydsk_reset();
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
	int cyl = 0, head = 0, sector = 1;
	flpydsk_lba_to_chs(sectorLBA, &cyl, &head, &sector);

	/* Turn motor on and seek to track */
	flpydsk_control_motor(true);
	if (flpydsk_seek((uint8_t)cyl, (uint8_t)head) != 0)
		return 0;

	/* Read sector and turn motor off */
	flpydsk_read_sector_imp((uint8_t)head, (uint8_t)cyl, (uint8_t)sector);
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
