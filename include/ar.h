#include <obj.h>

#define ARMAG 'A'
struct	ar_header {
	char	name[14];
	long	date;
	char	uid;
	char	gid;
	int	    mode;
	long	size;
} PACKED;
