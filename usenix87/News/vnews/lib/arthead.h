/*
 * header.h - Article header format
 */

#define NUNREC 50
#define H_SPACE 1024
#define NHDNAME 24

/* article header */
struct	arthead {
	char	*h_relayversion;	/* Relay-Version:	*/
	char	*h_postversion;		/* Posting-Version:	*/
	char	*h_path;		/* Path:		*/
	char	*h_from;		/* From:		*/
	char	*h_nbuf;		/* Newsgroups:		*/
	char	*h_title;		/* Subject:		*/
	char	*h_ident;		/* Message-ID:		*/
	char	*h_subdate;		/* Date: (submission)	*/
	char	*h_oident;		/* Article-I.D.:	*/
	char	*h_postdate;		/* Posted:		*/
	char	*h_recdate;		/* Date-Received:	*/
	char	*h_expdate;		/* Expires:		*/
	char	*h_references;		/* References:		*/
	char	*h_ctlmsg;		/* Control:		*/
	char	*h_sender;		/* Sender:		*/
	char	*h_replyto;		/* Reply-To:		*/
	char	*h_followto;		/* Followup-To:		*/
	char	*h_distribution;	/* Distribution:	*/
	char	*h_organization;	/* Organization:	*/
	char	*h_numlines;		/* Lines:		*/
	char	*h_keywords;		/* Keywords:		*/
	char	*h_approved;		/* Approved:		*/
	char	*h_summary;		/* Summary:		*/
	char	*h_priority;		/* Priority:		*/
	char	*h_unrec[NUNREC];	/* unrecognized lines	*/
	time_t	h_subtime;		/* subdate in secs	*/
	time_t	h_rectime;		/* recdate in secs	*/
	time_t	h_exptime;		/* expdate in secs	*/
	int	h_intnumlines;		/* Integer version	*/
	int	h_intpriority;		/* Integer version	*/
	char	*h_space[8];		/* string space		*/
};

#define hset(hdrline)	((hdrline) != NULL)

FILE *gethead();
