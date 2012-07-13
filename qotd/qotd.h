#ifndef	_SYS_QOTD_H
#define	_SYS_QOTD_H

#define	QOTDIOC		('q' << 24 | 't' << 16 | 'd' << 8)
#define	QOTDIOCGSZ	(QOTDIOC | 1)	/* Get quote buffer size */
#define	QOTDIOCSSZ	(QOTDIOC | 2)	/* Set new quote buffer size */
#define	QOTDIOCDISCARD	(QOTDIOC | 3)	/* Discard quotes and reset */

#endif /* _SYS_QOTD_H */
