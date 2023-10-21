/*	standard ascii definitions	*/
#define	NUL	0x00	/* null */
#define	SOH	0x01	/* start of header */
#define	STX	0x02	/* start of text */
#define	ETX	0x03	/* end of text */
#define	EOT	0x04	/* end of transmittion */
#define	ENQ	0x05	/* enquire */
#define	ACK	0x06	/* acknowledge */
#define	BEL	0x07	/* bell */
#define	BS	0x08	/* back space */
#define	HT	0x09	/* horizontal tab */
#define	LF	0x0a	/* line feed */
#define	VT	0x0b	/* vertical tab */
#define	FF	0x0c	/* form feed */
#define	CR	0x0d	/* carrage return */
#define	SO	0x0e	/* shift out */
#define	SI	0x0f	/* shift in */
#define	DLE	0x10	/* data link escape */
#define	DC1	0x11	/* device control 1 */
#	define	XON	0x11	/* start transmition */
#define	DC2	0x12	/* device control 2 */
#define	DC3	0x13	/* device control 3 */
#	define	XOFF	0x13	/* stop transmition */
#define	DC4	0x14	/* device control 4 */
#define	NAK	0x15	/* negative acknowledge */
#define	SYN	0x16	/* sync??? */
#define	ETB	0x17	/* */
#define	CAN	0x18	/* cancle */
#define	EM	0x19	/* end medium */
#define	SUB	0x1a	/* substitute */
#define	ESC	0x1b	/* escape */
#define	FS	0x1c	/* file seperator */
#define	GS	0x1d	/* group seperator */
#define	RS	0x1e	/* record seperator */
#define	US	0x1f	/* unit seperator */

#define	DEL	127
#define	CTRL(ch)	('ch'&037)

