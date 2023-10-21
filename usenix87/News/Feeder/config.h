/* Written by Stephen J. Muir, Lancaster University, England, UK.
 *
 * EMAIL: stephen@comp.lancs.ac.uk
 * UUCP:  ...!mcvax!ukc!dcl-cs!stephen
 *
 * Version 1.0
 */

// Any of the lines below may be commented out

//# define DEBUG

// News uid and gid.
# define NEWSUID		2
# define NEWSGID		4

// Priority to run at.
//# define NICENESS		1

// Working directory.
# define BATCH_OUT_DIR		"/usr/spool/news/batchout/"

// Default maximum size of batch.
# define DEFAULT_MAX_SIZE	200000

// Stamp log even if there is nothing to batch.
//# define ALWAYS_STAMP_LOG

// Number of seconds after which to delete log entries.
# define LOG_EXPIRE_SECONDS	(14*24*60*60)

// Error log.
# define ERROR_LOG		"/usr/lib/news/errlog"
