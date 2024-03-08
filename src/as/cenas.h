extern struct modexpr m_rc[];
extern struct modexpr m_arac[];
extern struct modexpr m_rbrb[];
extern struct modexpr m_rair[];
extern struct modexpr m_m[];
#define MOD6N 17
extern struct modexpr m_mod6[];

/*
	Addresssing mode groups
		rel		-127:128 PC offset
		c		4 bit index
		iw		immediate word
		ib		immediate byte
		m		memory addr
		reg		register
		rna 	register that is not a
		regb	byte register
		addr	16 bit address
*/
/* op */
#define MNONE 0

/* op rel */
#define MREL 1

/* op reg */
#define MRB 2

/* op regb,c
	[op:8] [reg:3][1][c:4] 
#define MRBC 3 */

/* op r,c 
	[op:8] [reg:3][1][c:4] */
#define MRC 4

/* op ara,c
	op/ m,c
	[op:8] [0:3][1][c:4] [m:16]
	op- rna,iw,c
	[op:8] [rna:3][1][c:4] [iw:16]
*/
#define MARAC 5

/* op rnab, regb 
	[op:8] [rai:3][1][rai:3][1]
	*/
#define MRBRB 6

/* op rai,r
	op reg,reg
	[op:8] [reg:3][0][reg:3][0]
	op- reg, iw, reg
	[op:8] [reg:3][1][reg:3][1] [iw:16]
*/
#define MRAIR 7

/* op ib 
	[op:8] [ib:8]*/
#define MIB 8

/* op/ m 
	[op:8] [m:16] */
#define MM 9

#define MOD6 10
/* 	op= iw
		[op+0:8] [iw:16]
	op/ m
		[op+1:8] [m:16]
	op$ m
		[op+2:8] [m:16]
	op  rel
		[op+3:8] [rel:8]
	op* rel
		[op+4:8] [rel:8]
	op- reg      
		[op+5:8] [reg:3][0][0:4]
	op- reg+     
		[op+5:8] [reg:3][0][1:4]
	op- reg-     
		[op+5:8] [reg:3][0][2:4]
	op- *reg     
		[op+5:8] [reg:3][0][4:4]
	op- *reg+    
		[op+5:8] [reg:3][0][5:4]
	op- *reg-    
		[op+5:8] [reg:3][0][6:4]
	op- reg,ib   
		[op+5:8] [reg:3][0][8:4] [ib:8]
	op- reg+,ib  
		[op+5:8] [reg:3][0][9:4] [ib:8]
	op- reg-,ib  
		[op+5:8] [reg:3][0][a:4] [ib:8]
	op- *reg,ib  
		[op+5:8] [reg:3][0][c:4] [ib:8]
	op- *reg+,ib 
		[op+5:8] [reg:3][0][d:4] [ib:8]
	op- *reg-,ib 
		[op+5:8] [reg:3][0][e:4] [ib:8]
*/


/* Like MOD6 but without op= m */
#define MOD5 11

/* Like MOD6 but includes 8 op+ reg instrucitons with offset*/
#define MOD14 12

/* Opcode argument types */

/* #define MEMIMM 0	 = immediate */
/* #define MEMDIR 1 	 / direct word */
/* #define MEMIND 2 	 $ indirect word */
/* #define MEMPC 3 	   relative */
/* #define MEMPCRIND 4  * indirect relavite */
/* #define MEMINDX 5 	 - indexed */
/* #define MEMINDXALT 6 	 - indexed with alt (op a, op b, etc) */
/* #define MEMIMMB 7  immediate byte */
/* #define RB_C	8	 rb, c */
/* #define RB_RB	9	 rb, rb */
/* #define R_C		10 	 r, c */
/* #define RAI_R   11    rai, r */
/* #define ARA_C   11    ara, c */

/* Indexmodes: "OPC" ("+" | "-") SP ["*"] WordReg ["-" | "+"] ["," Offset] */
/* 0 reg:       JSR+ X      ;7d40    OR  JSR- X      ;7d40 */
/* 1 ^postincr  JSR+ X+     ;7d41    OR  JSR- X+     ;7d41 */
/* 2 ^predecr   JSR+ X-     ;7d42    OR  JSR- X-     ;7d42 */
/* 4 IndReg     JSR+ *X     ;7d44    OR  JSR- *X     ;7d44 */
/* 5 ^postincr  JSR+ *X+    ;7d45    OR  JSR- *X+    ;7d45 */
/* 6 ^predecr   JSR+ *X-    ;7d46    OR  JSR- *X-    ;7d46 */
/* 8 RegOfs     JSR+ X,LB   ;7d485e  OR  JSR- X,LB   ;7d485e */
/* 9 ^postincr  JSR+ X+,LB  ;7d495e  OR  JSR- X+,LB  ;7d495e */
/* A ^predecr   JSR+ X-,LB  ;7d4a5e  OR  JSR- X-,LB  ;7d4a5e */
/* C IndRegOfs  JSR+ *X,LB  ;7d4c5e  OR  JSR- *X,LB  ;7d4c5e */
/* D ^postincr  JSR+ *X+,LB ;7d4d5e  OR  JSR- *X+,LB ;7d4d5e */
/* E ^predecr   JSR+ *X-,LB ;7d4e5e  OR  JSR- *X-,LB ;7d4e5e */
#define AREG			(0 << 4)	/* r register */
#define AREGPOI			(1 << 4)	/* r+ register post incr */
#define AREGPRD			(2 << 4)	/* r- register pre decr */
#define AREGDISP		(4 << 4)	/* r,ib register + disp */
#define AREGDISPPOI		(5 << 4)	/* r+,ib register + disp, post incr */
#define AREGPRDDISP		(6 << 4)	/* r-,ib register + disp, pre decr */
#define AREGIND			(8 << 4)	/* *r indirect register */
#define AREGPOIIND		(9 << 4)	/* *r+ indirect register post incr */
#define AREGPRDIND		(0xA << 4)	/* *r- indirect register, pre decr */
#define AREGDISPIND		(0xC << 4)	/* *r,ib indirect register + disp */
#define AREGDISPPOIIND	(0xD << 4)	/* *r+,ib indirect register + disp, post incr */
#define AREGPRDDISPIND	(0xE << 4)	/* *r-,ib indirect register + disp, pre decr */
