/* UNIX Reasearch V7*/

#define	ARMAG	0177545

struct	ar_hdr {
	char	ar_name[14];
	int		ar_date;
	char	ar_uid;
	char	ar_gid;
	int		ar_mode;
	int		ar_size;
};
