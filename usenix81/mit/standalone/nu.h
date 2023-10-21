/* Machine dependent information for 68000 system */

#define NPAGES 1024		/* number of pages in pagetbl */

/* Status bits:							*/

#define	S_TRACE	0x8000		/* Trace enabled		*/
#define	S_PMASK	0x0700		/* Priority mask		*/
#define	S_PRONE	0x0100		/* Priority level one.		*/
#define	S_SUPER	0x2000		/* Super-mode enabled		*/

#define	MMAP	0x800000	/* Memory map location		*/
#define	MCTRL	0x801000	/* Control word location	*/
#define	MEMON	0x000F		/* mmgt on; bus err, parity, timeout enabled */

#define PID	0x802000	/* Processor number */
#define PNUMREG	0x803000	/* process number for mmap	*/
#define ERROR	0x804000	/* Error register */
#define SEGMENT_ERROR	1	/* segmentation error */
#define TIMEOUT_ERROR	2	/* bus timeout error */
#define PARITY_ERROR	4	/* Memory parity error */

#define	INTON	0xFF		/* mmgt and interrupts on	*/
#define	PAGES	1024		/* Number of pages		*/
#define	PSIZE	10		/* Number of bits in page offset*/

#define	IOB	0x10000L	/* address of IOB		*/
#define	INT0	64		/* first I/O interrupt number	*/
#define	IOEVRAM	IOB		/* adr of I/O event rams	*/
#define	EVNUM	16		/* Number of event rams		*/
#define	IOCTRL	(IOB+0x60)	/* I/O control reg		*/
#define	IOENAB	0x2000L		/* bit for event enables	*/
#define	PCI_DATA (IOB+0xF0)	/* PCI register adrs		*/
#define	PCI_SR	(IOB+0xF4)
#define	PCI_MR	(IOB+0xF8)
#define	PCI_CR	(IOB+0xFC)
#define	PCI2_DATA (IOB+0xE0)	/* PCI2 register adrs		*/
#define	PCI2_SR	(IOB+0xE4)
#define	PCI2_MR	(IOB+0xE8)
#define	PCI2_CR	(IOB+0xEC)

#define DMA	0x20000L			/* address of DMA	*/
#define	D_ADD	((unsigned short *)DMA)		/* DMA address register	*/
#define D_EVT	((unsigned short *)(DMA+0x04))	/* DMA event register	*/
#define	D_CMD	((unsigned char  *)(DMA+0x08))	/* DMA command register	*/
#define	D_CTL	((unsigned char  *)(DMA+0x0C))	/* DMA control register	*/
#define	D_STATUS ((unsigned short *)(DMA+0x10))	/* DMA status register	*/

#define SYNC	0x30000L		/* address of SYNC card		*/
#define SYNC_PGM (long  *)SYNC		/* start of sync program	*/
#define SYNC_CTL (short *)(SYNC+0x1000)	/* control reg for sync program */

#define	STARTLINE	0xB0		/* constants for VCON registers	*/
#define	HBOFF		0x78
#define	HBON		0x3B8
#define	VBOFF		0x02
#define	VBON		0x402

#define VCON	0x40000L		/* address of VCON		*/
#define V1_START (short  *)VCON		/* start video */
#define V1_HBOFF (short  *)(VCON+0x04)	/* H blanking off */
#define V1_HBON  (short  *)(VCON+0x08)	/* H blanking on */
#define V1_VBOFF (short  *)(VCON+0x0C)	/* V blanking off */
#define V1_VBON  (short  *)(VCON+0x10)	/* V blanking on */
#define V1_SCROLL (short *)(VCON+0x14)	/* scroll register */
#define V1_CNTL	 (short  *)(VCON+0x1C)	/* vidctrl register. bit 0 = vidxor-,
							     bit 1 = envid-  */
#define V0_START (short  *)VCON		/* start video */
#define V0_HBOFF (short  *)(VCON+0x400)	/* H blanking off */
#define V0_HBON  (short  *)(VCON+0x800)	/* H blanking on */
#define V0_VBOFF (short  *)(VCON+0xC00)	/* V blanking off */
#define V0_VBON  (short  *)(VCON+0x1000)/* V blanking on */
#define V0_SCROLL (short *)(VCON+0x1400)/* scroll register */
#define V0_CNTL	 (short  *)(VCON+0x1C00)/* vidctrl register. bit 0 = vidxor-,
							     bit 1 = envid-  */
#define V0_LTBL	(short *)(VCON+0x2000)	/* line table */
#define V1_LTBL	(short *)(VCON+0x2000)	/* line table */

#define VMEM	0xC0000		/* address of VMEM		*/

#define RAM	0xE0000		/* start of system RAM		*/
