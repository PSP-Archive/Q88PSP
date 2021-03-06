#define XR__A	RJ(P6,3,0)
#define XR__I	RJ(P2,5,0)
#define XR__U	RJ(P6,4,0)
#define XR__E	RJ(P6,5,0)
#define XR__O	RJ(P6,6,0)
#define XR_KA	RJ(P4,4,0)
#define XR_KI	RJ(P2,7,0)
#define XR_KU	RJ(P3,0,0)
#define XR_KE	RJ(P7,2,0)
#define XR_KO	RJ(P2,2,0)
#define XR_SA	RJ(P5,0,0)
#define XR_SI	RJ(P2,4,0)
#define XR_SU	RJ(P4,2,0)
#define XR_SE	RJ(P4,0,0)
#define XR_SO	RJ(P2,3,0)
#define XR_TA	RJ(P4,1,0)
#define XR_TI	RJ(P2,1,0)
#define XR_TU	RJ(P5,2,0)
#define XR_TE	RJ(P4,7,0)
#define XR_TO	RJ(P4,3,0)
#define XR_NA	RJ(P4,5,0)
#define XR_NI	RJ(P3,1,0)
#define XR_NU	RJ(P6,1,0)
#define XR_NE	RJ(P7,4,0)
#define XR_NO	RJ(P3,3,0)
#define XR_HA	RJ(P2,6,0)
#define XR_HI	RJ(P4,6,0)
#define XR_HU	RJ(P6,2,0)
#define XR_HE	RJ(P5,6,0)
#define XR_HO	RJ(P5,7,0)
#define XR_MA	RJ(P3,2,0)
#define XR_MI	RJ(P3,6,0)
#define XR_MU	RJ(P5,5,0)
#define XR_ME	RJ(P7,6,0)
#define XR_MO	RJ(P3,5,0)
#define XR_YA	RJ(P6,7,0)
#define XR_YU	RJ(P7,0,0)
#define XR_YO	RJ(P7,1,0)
#define XR_RA	RJ(P3,7,0)
#define XR_RI	RJ(P3,4,0)
#define XR_RU	RJ(P7,5,0)
#define XR_RE	RJ(P7,3,0)
#define XR_RO	RJ(P7,7,0)
#define XR_WA	RJ(P6,0,0)
#define XR_WO	RJ(P6,0,1)
#define XR__a	RJ(P6,3,1)
#define XR__i	RJ(P2,5,1)
#define XR__u	RJ(P6,4,1)
#define XR__e	RJ(P6,5,1)
#define XR__o	RJ(P6,6,1)
#define XR_ya	RJ(P6,7,1)
#define XR_yu	RJ(P7,0,1)
#define XR_yo	RJ(P7,1,1)
#define XR_tu	RJ(P5,2,1)
#define	XR__N	RJ(P5,1,0)
#define	XR_sp	RJ(P9,6,0)	/*    */
#define	XR_jj	RJ(P2,0,0)	/* J */
#define	XR_pp	RJ(P5,3,0)	/* K */
#define	XR_oo	RJ(P7,6,1)	/* E */
#define	XR_rr	RJ(P5,4,0)	/* [ */
#define	XR_aa	RJ(P5,3,1)	/* u */
#define	XR_ee	RJ(P5,5,1)	/* v */
#define	XR_xx	RJ(P7,5,1)	/* B */
#define	XR_yy	RJ(P7,4,1)	/* A */


static const romaji_list list_NN = { "", { XR__N, } };
static const romaji_list list_tu = { "", { XR_tu, } };

static const romaji_list list_mark[] =
{
		     
/*  	*/ { " ",    { XR_sp, } },
/* J 	*/ { "@",    { XR_jj, } },
/* K 	*/ { "[",    { XR_pp, } },
/* E 	*/ { "/",    { XR_oo, } },
/* [ 	*/ { "-",    { XR_rr, } },
/* u 	*/ { "{",    { XR_aa, } },
/* v 	*/ { "}",    { XR_ee, } },
/* B 	*/ { ".",    { XR_xx, } },
/* A 	*/ { ",",    { XR_yy, } },

};

static const romaji_list list_msime[] =
{

/*     */ { "A",    { XR__A, } },

/* Î   */ { "BA",   { XR_HA, XR_jj, } },
/* Ñ   */ { "BI",   { XR_HI, XR_jj, } },
/* Ô   */ { "BU",   { XR_HU, XR_jj, } },
/* ×   */ { "BE",   { XR_HE, XR_jj, } },
/* Ú   */ { "BO",   { XR_HO, XR_jj, } },

/* ©   */ { "CA",   { XR_KA, } },
/* µ   */ { "CI",   { XR_SI, } },
/* ­   */ { "CU",   { XR_KU, } },
/* ¹   */ { "CE",   { XR_SE, } },
/* ±   */ { "CO",   { XR_KO, } },

/* ¾   */ { "DA",   { XR_TA, XR_jj, } },
/* À   */ { "DI",   { XR_TI, XR_jj, } },
/* Ã   */ { "DU",   { XR_TU, XR_jj, } },
/* Å   */ { "DE",   { XR_TE, XR_jj, } },
/* Ç   */ { "DO",   { XR_TO, XR_jj, } },

/* ¦   */ { "E",    { XR__E, } },

/* Ó */ { "FA",   { XR_HU, XR__a, } },
/* Ó¡ */ { "FI",   { XR_HU, XR__i, } },
/* Ó   */ { "FU",   { XR_HU,        } },
/* Ó¥ */ { "FE",   { XR_HU, XR__e, } },
/* Ó§ */ { "FO",   { XR_HU, XR__o, } },

/* ª   */ { "GA",   { XR_KA, XR_jj, } },
/* ¬   */ { "GI",   { XR_KI, XR_jj, } },
/* ®   */ { "GU",   { XR_KU, XR_jj, } },
/* °   */ { "GE",   { XR_KE, XR_jj, } },
/* ²   */ { "GO",   { XR_KO, XR_jj, } },

/* Í   */ { "HA",   { XR_HA, } },
/* Ð   */ { "HI",   { XR_HI, } },
/* Ó   */ { "HU",   { XR_HU, } },
/* Ö   */ { "HE",   { XR_HE, } },
/* Ù   */ { "HO",   { XR_HO, } },

/* ¢   */ { "I",    { XR__I, } },

/* ¶á */ { "JA",   { XR_SI, XR_jj, XR_ya, } },
/* ¶   */ { "JI",   { XR_SI, XR_jj,        } },
/* ¶ã */ { "JU",   { XR_SI, XR_jj, XR_yu, } },
/* ¶¥ */ { "JE",   { XR_SI, XR_jj, XR__e, } },
/* ¶å */ { "JO",   { XR_SI, XR_jj, XR_yo, } },

/* ©   */ { "KA",   { XR_KA, } },
/* «   */ { "KI",   { XR_KI, } },
/* ­   */ { "KU",   { XR_KU, } },
/* ¯   */ { "KE",   { XR_KE, } },
/* ±   */ { "KO",   { XR_KO, } },

/*    */ { "LA",   { XR__a, } },
/* ¡   */ { "LI",   { XR__i, } },
/* £   */ { "LU",   { XR__u, } },
/* ¥   */ { "LE",   { XR__e, } },
/* §   */ { "LO",   { XR__o, } },

/* Ü   */ { "MA",   { XR_MA, } },
/* Ý   */ { "MI",   { XR_MI, } },
/* Þ   */ { "MU",   { XR_MU, } },
/* ß   */ { "ME",   { XR_ME, } },
/* à   */ { "MO",   { XR_MO, } },

/* È   */ { "NA",   { XR_NA, } },
/* É   */ { "NI",   { XR_NI, } },
/* Ê   */ { "NU",   { XR_NU, } },
/* Ë   */ { "NE",   { XR_NE, } },
/* Ì   */ { "NO",   { XR_NO, } },

/* ¨   */ { "O",    { XR__O, } },

/* Ï   */ { "PA",   { XR_HA, XR_pp, } },
/* Ò   */ { "PI",   { XR_HI, XR_pp, } },
/* Õ   */ { "PU",   { XR_HU, XR_pp, } },
/* Ø   */ { "PE",   { XR_HE, XR_pp, } },
/* Û   */ { "PO",   { XR_HO, XR_pp, } },

/* ­ */ { "QA",   { XR_KU, XR__a, } },
/* ­¡ */ { "QI",   { XR_KU, XR__i, } },
/* ­   */ { "QU",   { XR_KU,        } },
/* ­¥ */ { "QE",   { XR_KU, XR__e, } },
/* ­§ */ { "QO",   { XR_KU, XR__o, } },

/* ç   */ { "RA",   { XR_RA, } },
/* è   */ { "RI",   { XR_RI, } },
/* é   */ { "RU",   { XR_RU, } },
/* ê   */ { "RE",   { XR_RE, } },
/* ë   */ { "RO",   { XR_RO, } },

/* ³   */ { "SA",   { XR_SA, } },
/* µ   */ { "SI",   { XR_SI, } },
/* ·   */ { "SU",   { XR_SU, } },
/* ¹   */ { "SE",   { XR_SE, } },
/* »   */ { "SO",   { XR_SO, } },

/* ½   */ { "TA",   { XR_TA, } },
/* ¿   */ { "TI",   { XR_TI, } },
/* Â   */ { "TU",   { XR_TU, } },
/* Ä   */ { "TE",   { XR_TE, } },
/* Æ   */ { "TO",   { XR_TO, } },

/* ¤   */ { "U",    { XR__U, } },

/*  */ { "VA",   { XR__U, XR_jj, XR__a, } },
/* ¡ */ { "VI",   { XR__U, XR_jj, XR__i, } },
/*    */ { "VU",   { XR__U, XR_jj,        } },
/* ¥ */ { "VE",   { XR__U, XR_jj, XR__e, } },
/* § */ { "VO",   { XR__U, XR_jj, XR__o, } },

/* í   */ { "WA",   { XR_WA,        } },
/* ¤¡ */ { "WI",   { XR__U, XR__i, } },
/* ¤   */ { "WU",   { XR__U,        } },
/* ¤¥ */ { "WE",   { XR__U, XR__e, } },
/* ð   */ { "WO",   { XR_WO,        } },

/*    */ { "XA",   { XR__a, } },
/* ¡   */ { "XI",   { XR__i, } },
/* £   */ { "XU",   { XR__u, } },
/* ¥   */ { "XE",   { XR__e, } },
/* §   */ { "XO",   { XR__o, } },

/* â   */ { "YA",   { XR_YA,        } },
/* ¢   */ { "YI",   { XR__I,        } },
/* ä   */ { "YU",   { XR_YU,        } },
/* ¢¥ */ { "YE",   { XR__I, XR__e, } },
/* æ   */ { "YO",   { XR_YO,        } },

/* ´   */ { "ZA",   { XR_SA, XR_jj, } },
/* ¶   */ { "ZI",   { XR_SI, XR_jj, } },
/* ¸   */ { "ZU",   { XR_SU, XR_jj, } },
/* º   */ { "ZE",   { XR_SE, XR_jj, } },
/* ¼   */ { "ZO",   { XR_SO, XR_jj, } },

/* Ñá */ { "BYA",  { XR_HI, XR_jj, XR_ya, } },
/* Ñ¡ */ { "BYI",  { XR_HI, XR_jj, XR__i, } },
/* Ñã */ { "BYU",  { XR_HI, XR_jj, XR_yu, } },
/* Ñ¥ */ { "BYE",  { XR_HI, XR_jj, XR__e, } },
/* Ñå */ { "BYO",  { XR_HI, XR_jj, XR_yo, } },
		     
/* ¿á */ { "CYA",  { XR_TI, XR_ya, } },
/* ¿¡ */ { "CYI",  { XR_TI, XR__i, } },
/* ¿ã */ { "CYU",  { XR_TI, XR_yu, } },
/* ¿¥ */ { "CYE",  { XR_TI, XR__e, } },
/* ¿å */ { "CYO",  { XR_TI, XR_yo, } },
		     
/* Àá */ { "DYA",  { XR_TI, XR_jj, XR_ya, } },
/* À¡ */ { "DYI",  { XR_TI, XR_jj, XR__i, } },
/* Àã */ { "DYU",  { XR_TI, XR_jj, XR_yu, } },
/* À¥ */ { "DYE",  { XR_TI, XR_jj, XR__e, } },
/* Àå */ { "DYO",  { XR_TI, XR_jj, XR_yo, } },
		     
/* Óá */ { "FYA",  { XR_HU, XR_ya, } },
/* Ó¡ */ { "FYI",  { XR_HU, XR__i, } },
/* Óã */ { "FYU",  { XR_HU, XR_yu, } },
/* Ó¥ */ { "FYE",  { XR_HU, XR__e, } },
/* Óå */ { "FYO",  { XR_HU, XR_yo, } },
		     
/* ¬á */ { "GYA",  { XR_KI, XR_jj, XR_ya, } },
/* ¬¡ */ { "GYI",  { XR_KI, XR_jj, XR__i, } },
/* ¬ã */ { "GYU",  { XR_KI, XR_jj, XR_yu, } },
/* ¬¥ */ { "GYE",  { XR_KI, XR_jj, XR__e, } },
/* ¬å */ { "GYO",  { XR_KI, XR_jj, XR_yo, } },
		     
/* Ðá */ { "HYA",  { XR_HI, XR_ya, } },
/* Ð¡ */ { "HYI",  { XR_HI, XR__i, } },
/* Ðã */ { "HYU",  { XR_HI, XR_yu, } },
/* Ð¥ */ { "HYE",  { XR_HI, XR__e, } },
/* Ðå */ { "HYO",  { XR_HI, XR_yo, } },
		     
/* ¶á */ { "JYA",  { XR_SI, XR_jj, XR_ya, } },
/* ¶¡ */ { "JYI",  { XR_SI, XR_jj, XR__i, } },
/* ¶ã */ { "JYU",  { XR_SI, XR_jj, XR_yu, } },
/* ¶¥ */ { "JYE",  { XR_SI, XR_jj, XR__e, } },
/* ¶å */ { "JYO",  { XR_SI, XR_jj, XR_yo, } },
		     
/* «á */ { "KYA",  { XR_KI, XR_ya, } },
/* «¡ */ { "KYI",  { XR_KI, XR__i, } },
/* «ã */ { "KYU",  { XR_KI, XR_yu, } },
/* «¥ */ { "KYE",  { XR_KI, XR__e, } },
/* «å */ { "KYO",  { XR_KI, XR_yo, } },
		     
/* á   */ { "LYA",  { XR_ya, } },
/* ¡   */ { "LYI",  { XR__i, } },
/* ã   */ { "LYU",  { XR_yu, } },
/* ¥   */ { "LYE",  { XR__e, } },
/* å   */ { "LYO",  { XR_yo, } },
		     
/* Ýá */ { "MYA",  { XR_MI, XR_ya, } },
/* Ý¡ */ { "MYI",  { XR_MI, XR__i, } },
/* Ýã */ { "MYU",  { XR_MI, XR_yu, } },
/* Ý¥ */ { "MYE",  { XR_MI, XR__e, } },
/* Ýå */ { "MYO",  { XR_MI, XR_yo, } },

/* Éá */ { "NYA",  { XR_NI, XR_ya, } },
/* É¡ */ { "NYI",  { XR_NI, XR__i, } },
/* Éã */ { "NYU",  { XR_NI, XR_yu, } },
/* É¥ */ { "NYE",  { XR_NI, XR__e, } },
/* Éå */ { "NYO",  { XR_NI, XR_yo, } },
		     
/* Òá */ { "PYA",  { XR_HI, XR_pp, XR_ya, } },
/* Ò¡ */ { "PYI",  { XR_HI, XR_pp, XR__i, } },
/* Òã */ { "PYU",  { XR_HI, XR_pp, XR_yu, } },
/* Ò¥ */ { "PYE",  { XR_HI, XR_pp, XR__e, } },
/* Òå */ { "PYO",  { XR_HI, XR_pp, XR_yo, } },
		     
/* ­á */ { "QYA",  { XR_KU, XR_ya, } },
/* ­¡ */ { "QYI",  { XR_KU, XR__i, } },
/* ­ã */ { "QYU",  { XR_KU, XR_yu, } },
/* ­¥ */ { "QYE",  { XR_KU, XR__e, } },
/* ­å */ { "QYO",  { XR_KU, XR_yo, } },
		     
/* èá */ { "RYA",  { XR_RI, XR_ya, } },
/* è¡ */ { "RYI",  { XR_RI, XR__i, } },
/* èã */ { "RYU",  { XR_RI, XR_yu, } },
/* è¥ */ { "RYE",  { XR_RI, XR__e, } },
/* èå */ { "RYO",  { XR_RI, XR_yo, } },

/* µá */ { "SYA",  { XR_SI, XR_ya, } },
/* µ¡ */ { "SYI",  { XR_SI, XR__i, } },
/* µã */ { "SYU",  { XR_SI, XR_yu, } },
/* µ¥ */ { "SYE",  { XR_SI, XR__e, } },
/* µå */ { "SYO",  { XR_SI, XR_yo, } },
		     
/* ¿á */ { "TYA",  { XR_TI, XR_ya, } },
/* ¿¡ */ { "TYI",  { XR_TI, XR__i, } },
/* ¿ã */ { "TYU",  { XR_TI, XR_yu, } },
/* ¿¥ */ { "TYE",  { XR_TI, XR__e, } },
/* ¿å */ { "TYO",  { XR_TI, XR_yo, } },
		     
/* á */ { "VYA",  { XR__U, XR_jj, XR_ya, } },
/* ¡ */ { "VYI",  { XR__U, XR_jj, XR__i, } },
/* ã */ { "VYU",  { XR__U, XR_jj, XR_yu, } },
/* ¥ */ { "VYE",  { XR__U, XR_jj, XR__e, } },
/* å */ { "VYO",  { XR__U, XR_jj, XR_yo, } },
		     
/* á   */ { "XYA",  { XR_ya, } },
/* ¡   */ { "XYI",  { XR__i, } },
/* ã   */ { "XYU",  { XR_yu, } },
/* ¥   */ { "XYE",  { XR__e, } },
/* å   */ { "XYO",  { XR_yo, } },

/* ¶á */ { "ZYA",  { XR_SI, XR_jj, XR_ya, } },
/* ¶¡ */ { "ZYI",  { XR_SI, XR_jj, XR__i, } },
/* ¶ã */ { "ZYU",  { XR_SI, XR_jj, XR_yu, } },
/* ¶¥ */ { "ZYE",  { XR_SI, XR_jj, XR__e, } },
/* ¶å */ { "ZYO",  { XR_SI, XR_jj, XR_yo, } },
		     
/* ¿á */ { "CHA",  { XR_TI, XR_ya, } },
/* ¿   */ { "CHI",  { XR_TI,        } },
/* ¿ã */ { "CHU",  { XR_TI, XR_yu, } },
/* ¿¥ */ { "CHE",  { XR_TI, XR__e, } },
/* ¿å */ { "CHO",  { XR_TI, XR_yo, } },
		     
/* Åá */ { "DHA",  { XR_TE, XR_jj, XR_ya, } },
/* Å¡ */ { "DHI",  { XR_TE, XR_jj, XR__i, } },
/* Åã */ { "DHU",  { XR_TE, XR_jj, XR_yu, } },
/* Å¥ */ { "DHE",  { XR_TE, XR_jj, XR__e, } },
/* Åå */ { "DHO",  { XR_TE, XR_jj, XR_yo, } },
		     
/* µá */ { "SHA",  { XR_SI, XR_ya, } },
/* µ   */ { "SHI",  { XR_SI,        } },
/* µã */ { "SHU",  { XR_SI, XR_yu, } },
/* µ¥ */ { "SHE",  { XR_SI, XR__e, } },
/* µå */ { "SHO",  { XR_SI, XR_yo, } },
		     
/* Äá */ { "THA",  { XR_TE, XR_ya, } },
/* Ä¡ */ { "THI",  { XR_TE, XR__i, } },
/* Äã */ { "THU",  { XR_TE, XR_yu, } },
/* Ä¥ */ { "THE",  { XR_TE, XR__e, } },
/* Äå */ { "THO",  { XR_TE, XR_yo, } },
		     
/* ¤ */ { "WHA",  { XR__U, XR_ya, } },
/* ¤¡ */ { "WHI",  { XR__U, XR__i, } },
/* ¤   */ { "WHU",  { XR__U,        } },
/* ¤¥ */ { "WHE",  { XR__U, XR__e, } },
/* ¤§ */ { "WHO",  { XR__U, XR_yo, } },
		     
/* Â */ { "TSA",  { XR_TU, XR_ya, } },
/* Â¡ */ { "TSI",  { XR_TU, XR__i, } },
/* Â   */ { "TSU",  { XR_TU,        } },
/* Â¥ */ { "TSE",  { XR_TU, XR__e, } },
/* Â§ */ { "TSO",  { XR_TU, XR_yo, } },
		     
/* Á   */ { "XTU",  { XR_tu, } },
		     
/* ­ */ { "QWA",  { XR_KU, XR__a, } },
/* ­¡ */ { "QWI",  { XR_KU, XR__i, } },
/* ­£ */ { "QWU",  { XR_KU, XR__u, } },
/* ­¥ */ { "QWE",  { XR_KU, XR__e, } },
/* ­§ */ { "QWO",  { XR_KU, XR__o, } },
		     
/* ñ   */ { "NN",   { XR__N, } },
/* ñ   */ { "N'",   { XR__N, } },
		     
};

static const romaji_list list_atok[] =
{

/*     */ { "A",    { XR__A, } },

/* Î   */ { "BA",   { XR_HA, XR_jj, } },
/* Ñ   */ { "BI",   { XR_HI, XR_jj, } },
/* Ô   */ { "BU",   { XR_HU, XR_jj, } },
/* ×   */ { "BE",   { XR_HE, XR_jj, } },
/* Ú   */ { "BO",   { XR_HO, XR_jj, } },

/* ¾   */ { "DA",   { XR_TA, XR_jj, } },
/* À   */ { "DI",   { XR_TI, XR_jj, } },
/* Ã   */ { "DU",   { XR_TU, XR_jj, } },
/* Å   */ { "DE",   { XR_TE, XR_jj, } },
/* Ç   */ { "DO",   { XR_TO, XR_jj, } },

/* ¦   */ { "E",    { XR__E, } },

/* Ó */ { "FA",   { XR_HU, XR__a, } },
/* Ó¡ */ { "FI",   { XR_HU, XR__i, } },
/* Ó   */ { "FU",   { XR_HU,        } },
/* Ó¥ */ { "FE",   { XR_HU, XR__e, } },
/* Ó§ */ { "FO",   { XR_HU, XR__o, } },

/* ª   */ { "GA",   { XR_KA, XR_jj, } },
/* ¬   */ { "GI",   { XR_KI, XR_jj, } },
/* ®   */ { "GU",   { XR_KU, XR_jj, } },
/* °   */ { "GE",   { XR_KE, XR_jj, } },
/* ²   */ { "GO",   { XR_KO, XR_jj, } },

/* Í   */ { "HA",   { XR_HA, } },
/* Ð   */ { "HI",   { XR_HI, } },
/* Ó   */ { "HU",   { XR_HU, } },
/* Ö   */ { "HE",   { XR_HE, } },
/* Ù   */ { "HO",   { XR_HO, } },

/* ¢   */ { "I",    { XR__I, } },

/* ¶á */ { "JA",   { XR_SI, XR_jj, XR_ya, } },
/* ¶   */ { "JI",   { XR_SI, XR_jj,        } },
/* ¶ã */ { "JU",   { XR_SI, XR_jj, XR_yu, } },
/* ¶¥ */ { "JE",   { XR_SI, XR_jj, XR__e, } },
/* ¶å */ { "JO",   { XR_SI, XR_jj, XR_yo, } },

/* ©   */ { "KA",   { XR_KA, } },
/* «   */ { "KI",   { XR_KI, } },
/* ­   */ { "KU",   { XR_KU, } },
/* ¯   */ { "KE",   { XR_KE, } },
/* ±   */ { "KO",   { XR_KO, } },

/*    */ { "LA",   { XR__a, } },
/* ¡   */ { "LI",   { XR__i, } },
/* £   */ { "LU",   { XR__u, } },
/* ¥   */ { "LE",   { XR__e, } },
/* §   */ { "LO",   { XR__o, } },

/* Ü   */ { "MA",   { XR_MA, } },
/* Ý   */ { "MI",   { XR_MI, } },
/* Þ   */ { "MU",   { XR_MU, } },
/* ß   */ { "ME",   { XR_ME, } },
/* à   */ { "MO",   { XR_MO, } },

/* È   */ { "NA",   { XR_NA, } },
/* É   */ { "NI",   { XR_NI, } },
/* Ê   */ { "NU",   { XR_NU, } },
/* Ë   */ { "NE",   { XR_NE, } },
/* Ì   */ { "NO",   { XR_NO, } },

/* ¨   */ { "O",    { XR__O, } },

/* Ï   */ { "PA",   { XR_HA, XR_pp, } },
/* Ò   */ { "PI",   { XR_HI, XR_pp, } },
/* Õ   */ { "PU",   { XR_HU, XR_pp, } },
/* Ø   */ { "PE",   { XR_HE, XR_pp, } },
/* Û   */ { "PO",   { XR_HO, XR_pp, } },

/* ç   */ { "RA",   { XR_RA, } },
/* è   */ { "RI",   { XR_RI, } },
/* é   */ { "RU",   { XR_RU, } },
/* ê   */ { "RE",   { XR_RE, } },
/* ë   */ { "RO",   { XR_RO, } },

/* ³   */ { "SA",   { XR_SA, } },
/* µ   */ { "SI",   { XR_SI, } },
/* ·   */ { "SU",   { XR_SU, } },
/* ¹   */ { "SE",   { XR_SE, } },
/* »   */ { "SO",   { XR_SO, } },

/* ½   */ { "TA",   { XR_TA, } },
/* ¿   */ { "TI",   { XR_TI, } },
/* Â   */ { "TU",   { XR_TU, } },
/* Ä   */ { "TE",   { XR_TE, } },
/* Æ   */ { "TO",   { XR_TO, } },

/* ¤   */ { "U",    { XR__U, } },

/*  */ { "VA",   { XR__U, XR_jj, XR__a, } },
/* ¡ */ { "VI",   { XR__U, XR_jj, XR__i, } },
/*    */ { "VU",   { XR__U, XR_jj,        } },
/* ¥ */ { "VE",   { XR__U, XR_jj, XR__e, } },
/* § */ { "VO",   { XR__U, XR_jj, XR__o, } },

/* í   */ { "WA",   { XR_WA,        } },
/* ¤¡ */ { "WI",   { XR__U, XR__i, } },
/* ¤   */ { "WU",   { XR__U,        } },
/* ¤¥ */ { "WE",   { XR__U, XR__e, } },
/* ð   */ { "WO",   { XR_WO,        } },

/*    */ { "XA",   { XR__a, } },
/* ¡   */ { "XI",   { XR__i, } },
/* £   */ { "XU",   { XR__u, } },
/* ¥   */ { "XE",   { XR__e, } },
/* §   */ { "XO",   { XR__o, } },

/* â   */ { "YA",   { XR_YA,        } },
/* ¢   */ { "YI",   { XR__I,        } },
/* ä   */ { "YU",   { XR_YU,        } },
/* ¢¥ */ { "YE",   { XR__I, XR__e, } },
/* æ   */ { "YO",   { XR_YO,        } },

/* ´   */ { "ZA",   { XR_SA, XR_jj, } },
/* ¶   */ { "ZI",   { XR_SI, XR_jj, } },
/* ¸   */ { "ZU",   { XR_SU, XR_jj, } },
/* º   */ { "ZE",   { XR_SE, XR_jj, } },
/* ¼   */ { "ZO",   { XR_SO, XR_jj, } },

/* Ñá */ { "BYA",  { XR_HI, XR_jj, XR_ya, } },
/* Ñ¡ */ { "BYI",  { XR_HI, XR_jj, XR__i, } },
/* Ñã */ { "BYU",  { XR_HI, XR_jj, XR_yu, } },
/* Ñ¥ */ { "BYE",  { XR_HI, XR_jj, XR__e, } },
/* Ñå */ { "BYO",  { XR_HI, XR_jj, XR_yo, } },
		     
/* ¿á */ { "CYA",  { XR_TI, XR_ya, } },
/* ¿¡ */ { "CYI",  { XR_TI, XR__i, } },
/* ¿ã */ { "CYU",  { XR_TI, XR_yu, } },
/* ¿¥ */ { "CYE",  { XR_TI, XR__e, } },
/* ¿å */ { "CYO",  { XR_TI, XR_yo, } },
		     
/* Àá */ { "DYA",  { XR_TI, XR_jj, XR_ya, } },
/* À¡ */ { "DYI",  { XR_TI, XR_jj, XR__i, } },
/* Àã */ { "DYU",  { XR_TI, XR_jj, XR_yu, } },
/* À¥ */ { "DYE",  { XR_TI, XR_jj, XR__e, } },
/* Àå */ { "DYO",  { XR_TI, XR_jj, XR_yo, } },
		     
/* Óá */ { "FYA",  { XR_HU, XR_ya, } },
/* Ó¡ */ { "FYI",  { XR_HU, XR__i, } },
/* Óã */ { "FYU",  { XR_HU, XR_yu, } },
/* Ó¥ */ { "FYE",  { XR_HU, XR__e, } },
/* Óå */ { "FYO",  { XR_HU, XR_ya, } },
		     
/* ¬á */ { "GYA",  { XR_KI, XR_jj, XR_ya, } },
/* ¬¡ */ { "GYI",  { XR_KI, XR_jj, XR__i, } },
/* ¬ã */ { "GYU",  { XR_KI, XR_jj, XR_yu, } },
/* ¬¥ */ { "GYE",  { XR_KI, XR_jj, XR__e, } },
/* ¬å */ { "GYO",  { XR_KI, XR_jj, XR_yo, } },
		     
/* Ðá */ { "HYA",  { XR_HI, XR_ya, } },
/* Ð¡ */ { "HYI",  { XR_HI, XR__i, } },
/* Ðã */ { "HYU",  { XR_HI, XR_yu, } },
/* Ð¥ */ { "HYE",  { XR_HI, XR__e, } },
/* Ðå */ { "HYO",  { XR_HI, XR_yo, } },
		     
/* ¶á */ { "JYA",  { XR_SI, XR_jj, XR_ya, } },
/* ¶¡ */ { "JYI",  { XR_SI, XR_jj, XR__i, } },
/* ¶ã */ { "JYU",  { XR_SI, XR_jj, XR_yu, } },
/* ¶¥ */ { "JYE",  { XR_SI, XR_jj, XR__e, } },
/* ¶å */ { "JYO",  { XR_SI, XR_jj, XR_yo, } },

/* «á */ { "KYA",  { XR_KI, XR_ya, } },
/* «¡ */ { "KYI",  { XR_KI, XR__i, } },
/* «ã */ { "KYU",  { XR_KI, XR_yu, } },
/* «¥ */ { "KYE",  { XR_KI, XR__e, } },
/* «å */ { "KYO",  { XR_KI, XR_yo, } },
		     
/* á   */ { "LYA",  { XR_ya, } },
/* ¡   */ { "LYI",  { XR__i, } },
/* ã   */ { "LYU",  { XR_yu, } },
/* ¥   */ { "LYE",  { XR__e, } },
/* å   */ { "LYO",  { XR_yo, } },
		     
/* Ýá */ { "MYA",  { XR_MI, XR_ya, } },
/* Ý¡ */ { "MYI",  { XR_MI, XR__i, } },
/* Ýã */ { "MYU",  { XR_MI, XR_yu, } },
/* Ý¥ */ { "MYE",  { XR_MI, XR__e, } },
/* Ýå */ { "MYO",  { XR_MI, XR_yo, } },
		     
/* Éá */ { "NYA",  { XR_NI, XR_ya, } },
/* É¡ */ { "NYI",  { XR_NI, XR__i, } },
/* Éã */ { "NYU",  { XR_NI, XR_yu, } },
/* É¥ */ { "NYE",  { XR_NI, XR__e, } },
/* Éå */ { "NYO",  { XR_NI, XR_yo, } },
		     
/* Òá */ { "PYA",  { XR_HI, XR_pp, XR_ya, } },
/* Ò¡ */ { "PYI",  { XR_HI, XR_pp, XR__i, } },
/* Òã */ { "PYU",  { XR_HI, XR_pp, XR_yu, } },
/* Ò¥ */ { "PYE",  { XR_HI, XR_pp, XR__e, } },
/* Òå */ { "PYO",  { XR_HI, XR_pp, XR_yo, } },
		     
/* èá */ { "RYA",  { XR_RI, XR_ya, } },
/* è¡ */ { "RYI",  { XR_RI, XR__i, } },
/* èã */ { "RYU",  { XR_RI, XR_yu, } },
/* è¥ */ { "RYE",  { XR_RI, XR__e, } },
/* èå */ { "RYO",  { XR_RI, XR_yo, } },
		     
/* µá */ { "SYA",  { XR_SI, XR_ya, } },
/* µ¡ */ { "SYI",  { XR_SI, XR__i, } },
/* µã */ { "SYU",  { XR_SI, XR_yu, } },
/* µ¥ */ { "SYE",  { XR_SI, XR__e, } },
/* µå */ { "SYO",  { XR_SI, XR_yo, } },

/* ¿á */ { "TYA",  { XR_TI, XR_ya, } },
/* ¿¡ */ { "TYI",  { XR_TI, XR__i, } },
/* ¿ã */ { "TYU",  { XR_TI, XR_yu, } },
/* ¿¥ */ { "TYE",  { XR_TI, XR__e, } },
/* ¿å */ { "TYO",  { XR_TI, XR_yo, } },
		     
/* á   */ { "XYA",  { XR_ya, } },
/* ¡   */ { "XYI",  { XR__i, } },
/* ã   */ { "XYU",  { XR_yu, } },
/* ¥   */ { "XYE",  { XR__e, } },
/* å   */ { "XYO",  { XR_yo, } },
		     
/* ¶á */ { "ZYA",  { XR_SI, XR_jj, XR_ya, } },
/* ¶¡ */ { "ZYI",  { XR_SI, XR_jj, XR__i, } },
/* ¶ã */ { "ZYU",  { XR_SI, XR_jj, XR_yu, } },
/* ¶¥ */ { "ZYE",  { XR_SI, XR_jj, XR__e, } },
/* ¶å */ { "ZYO",  { XR_SI, XR_jj, XR_yo, } },
		     
/* ¿á */ { "CHA",  { XR_TI, XR_ya, } },
/* ¿   */ { "CHI",  { XR_TI,        } },
/* ¿ã */ { "CHU",  { XR_TI, XR_yu, } },
/* ¿¥ */ { "CHE",  { XR_TI, XR__e, } },
/* ¿å */ { "CHO",  { XR_TI, XR_yo, } },

/* Åá */ { "DHA",  { XR_TE, XR_jj, XR_ya, } },
/* Å¡ */ { "DHI",  { XR_TE, XR_jj, XR__i, } },
/* Åã */ { "DHU",  { XR_TE, XR_jj, XR_yu, } },
/* Å¥ */ { "DHE",  { XR_TE, XR_jj, XR__e, } },
/* Åå */ { "DHO",  { XR_TE, XR_jj, XR_yo, } },
		     
/* µá */ { "SHA",  { XR_SI, XR_ya, } },
/* µ   */ { "SHI",  { XR_SI,        } },
/* µã */ { "SHU",  { XR_SI, XR_yu, } },
/* µ¥ */ { "SHE",  { XR_SI, XR__e, } },
/* µå */ { "SHO",  { XR_SI, XR_yo, } },
		     
/* Äá */ { "THA",  { XR_TE, XR_ya, } },
/* Ä¡ */ { "THI",  { XR_TE, XR__i, } },
/* Äã */ { "THU",  { XR_TE, XR_yu, } },
/* Ä¥ */ { "THE",  { XR_TE, XR__e, } },
/* Äå */ { "THO",  { XR_TE, XR_yo, } },
		     
/* Â */ { "TSA",  { XR_TU, XR__a, } },
/* Â¡ */ { "TSI",  { XR_TU, XR__i, } },
/* Â   */ { "TSU",  { XR_TU,        } },
/* Â¥ */ { "TSE",  { XR_TU, XR__e, } },
/* Â§ */ { "TSO",  { XR_TU, XR__o, } },
		     
/* Á   */ { "XTU",  { XR_tu, } },
/* Á   */ { "XTSU", { XR_tu, } },
		     
/* ñ   */ { "NN",   { XR__N, } },
/* ñ   */ { "N'",   { XR__N, } },

};

static const romaji_list list_egg[] =
{

/*     */ { "A",    { XR__A, } },

/* Î   */ { "BA",   { XR_HA, XR_jj, } },
/* Ñ   */ { "BI",   { XR_HI, XR_jj, } },
/* Ô   */ { "BU",   { XR_HU, XR_jj, } },
/* ×   */ { "BE",   { XR_HE, XR_jj, } },
/* Ú   */ { "BO",   { XR_HO, XR_jj, } },

/* ¾   */ { "DA",   { XR_TA, XR_jj, } },
/* À   */ { "DI",   { XR_TI, XR_jj, } },
/* Ã   */ { "DU",   { XR_TU, XR_jj, } },
/* Å   */ { "DE",   { XR_TE, XR_jj, } },
/* Ç   */ { "DO",   { XR_TO, XR_jj, } },

/* ¦   */ { "E",    { XR__E, } },

/* Ó */ { "FA",   { XR_HU, XR__a, } },
/* Ó¡ */ { "FI",   { XR_HU, XR__i, } },
/* Ó   */ { "FU",   { XR_HU,        } },
/* Ó¥ */ { "FE",   { XR_HU, XR__e, } },
/* Ó§ */ { "FO",   { XR_HU, XR__o, } },

/* ª   */ { "GA",   { XR_KA, XR_jj, } },
/* ¬   */ { "GI",   { XR_KI, XR_jj, } },
/* ®   */ { "GU",   { XR_KU, XR_jj, } },
/* °   */ { "GE",   { XR_KE, XR_jj, } },
/* ²   */ { "GO",   { XR_KO, XR_jj, } },

/* Í   */ { "HA",   { XR_HA, } },
/* Ð   */ { "HI",   { XR_HI, } },
/* Ó   */ { "HU",   { XR_HU, } },
/* Ö   */ { "HE",   { XR_HE, } },
/* Ù   */ { "HO",   { XR_HO, } },

/* ¢   */ { "I",    { XR__I, } },

/* ¶á */ { "JA",   { XR_SI, XR_jj, XR_ya, } },
/* ¶   */ { "JI",   { XR_SI, XR_jj,        } },
/* ¶ã */ { "JU",   { XR_SI, XR_jj, XR_yu, } },
/* ¶¥ */ { "JE",   { XR_SI, XR_jj, XR__e, } },
/* ¶å */ { "JO",   { XR_SI, XR_jj, XR_yo, } },

/* ©   */ { "KA",   { XR_KA, } },
/* «   */ { "KI",   { XR_KI, } },
/* ­   */ { "KU",   { XR_KU, } },
/* ¯   */ { "KE",   { XR_KE, } },
/* ±   */ { "KO",   { XR_KO, } },

/* ç   */ { "LA",   { XR_RA, } },
/* è   */ { "LI",   { XR_RI, } },
/* é   */ { "LU",   { XR_RU, } },
/* ê   */ { "LE",   { XR_RE, } },
/* ë   */ { "LO",   { XR_RO, } },

/* Ü   */ { "MA",   { XR_MA, } },
/* Ý   */ { "MI",   { XR_MI, } },
/* Þ   */ { "MU",   { XR_MU, } },
/* ß   */ { "ME",   { XR_ME, } },
/* à   */ { "MO",   { XR_MO, } },

/* È   */ { "NA",   { XR_NA, } },
/* É   */ { "NI",   { XR_NI, } },
/* Ê   */ { "NU",   { XR_NU, } },
/* Ë   */ { "NE",   { XR_NE, } },
/* Ì   */ { "NO",   { XR_NO, } },

/* ¨   */ { "O",    { XR__O, } },

/* Ï   */ { "PA",   { XR_HA, XR_pp, } },
/* Ò   */ { "PI",   { XR_HI, XR_pp, } },
/* Õ   */ { "PU",   { XR_HU, XR_pp, } },
/* Ø   */ { "PE",   { XR_HE, XR_pp, } },
/* Û   */ { "PO",   { XR_HO, XR_pp, } },

/* ç   */ { "RA",   { XR_RA, } },
/* è   */ { "RI",   { XR_RI, } },
/* é   */ { "RU",   { XR_RU, } },
/* ê   */ { "RE",   { XR_RE, } },
/* ë   */ { "RO",   { XR_RO, } },

/* ³   */ { "SA",   { XR_SA, } },
/* µ   */ { "SI",   { XR_SI, } },
/* ·   */ { "SU",   { XR_SU, } },
/* ¹   */ { "SE",   { XR_SE, } },
/* »   */ { "SO",   { XR_SO, } },

/* ½   */ { "TA",   { XR_TA, } },
/* ¿   */ { "TI",   { XR_TI, } },
/* Â   */ { "TU",   { XR_TU, } },
/* Ä   */ { "TE",   { XR_TE, } },
/* Æ   */ { "TO",   { XR_TO, } },

/* ¤   */ { "U",    { XR__U, } },

/*  */ { "VA",   { XR__U, XR_jj, XR__a, } },
/* ¡ */ { "VI",   { XR__U, XR_jj, XR__i, } },
/*    */ { "VU",   { XR__U, XR_jj,        } },
/* ¥ */ { "VE",   { XR__U, XR_jj, XR__e, } },
/* § */ { "VO",   { XR__U, XR_jj, XR__o, } },

/* í   */ { "WA",   { XR_WA,        } },
/* î   */ { "WI",   { XR__I,        } },
/* ¤   */ { "WU",   { XR__U,        } },
/* ï   */ { "WE",   { XR__E,        } },
/* ð   */ { "WO",   { XR_WO,        } },

/*    */ { "XA",   { XR__a, } },
/* ¡   */ { "XI",   { XR__i, } },
/* £   */ { "XU",   { XR__u, } },
/* ¥   */ { "XE",   { XR__e, } },
/* §   */ { "XO",   { XR__o, } },

/* â   */ { "YA",   { XR_YA,        } },
/* ¢   */ { "YI",   { XR__I,        } },
/* ä   */ { "YU",   { XR_YU,        } },
/* ¢¥ */ { "YE",   { XR__I, XR__e, } },
/* æ   */ { "YO",   { XR_YO,        } },

/* ´   */ { "ZA",   { XR_SA, XR_jj, } },
/* ¶   */ { "ZI",   { XR_SI, XR_jj, } },
/* ¸   */ { "ZU",   { XR_SU, XR_jj, } },
/* º   */ { "ZE",   { XR_SE, XR_jj, } },
/* ¼   */ { "ZO",   { XR_SO, XR_jj, } },

/* Ñá */ { "BYA",  { XR_HI, XR_jj, XR_ya, } },
/* Ñã */ { "BYU",  { XR_HI, XR_jj, XR_yu, } },
/* Ñ¥ */ { "BYE",  { XR_HI, XR_jj, XR__e, } },
/* Ñå */ { "BYO",  { XR_HI, XR_jj, XR_yo, } },
		     
/* Àá */ { "DYA",  { XR_TI, XR_jj, XR_ya, } },
/* Å¡ */ { "DYI",  { XR_TE, XR_jj, XR__i, } },
/* Àã */ { "DYU",  { XR_TI, XR_jj, XR_yu, } },
/* À¥ */ { "DYE",  { XR_TI, XR_jj, XR__e, } },
/* Àå */ { "DYO",  { XR_TI, XR_jj, XR_yo, } },
		     
/* ¬á */ { "GYA",  { XR_KI, XR_jj, XR_ya, } },
/* ¬ã */ { "GYU",  { XR_KI, XR_jj, XR_yu, } },
/* ¬¥ */ { "GYE",  { XR_KI, XR_jj, XR__e, } },
/* ¬å */ { "GYO",  { XR_KI, XR_jj, XR_yo, } },
		     
/* Ðá */ { "HYA",  { XR_HI, XR_ya, } },
/* Ðã */ { "HYU",  { XR_HI, XR_yu, } },
/* Ð¥ */ { "HYE",  { XR_HI, XR__e, } },
/* Ðå */ { "HYO",  { XR_HI, XR_yo, } },
		     
/* ¶á */ { "JYA",  { XR_SI, XR_jj, XR_ya, } },
/* ¶ã */ { "JYU",  { XR_SI, XR_jj, XR_yu, } },
/* ¶¥ */ { "JYE",  { XR_SI, XR_jj, XR__e, } },
/* ¶å */ { "JYO",  { XR_SI, XR_jj, XR_yo, } },
		     
/* «á */ { "KYA",  { XR_KI, XR_ya, } },
/* «ã */ { "KYU",  { XR_KI, XR_yu, } },
/* «¥ */ { "KYE",  { XR_KI, XR__e, } },
/* «å */ { "KYO",  { XR_KI, XR_yo, } },

/* èá */ { "LYA",  { XR_RI, XR_ya, } },
/* èã */ { "LYU",  { XR_RI, XR_yu, } },
/* è¥ */ { "LYE",  { XR_RI, XR__e, } },
/* èå */ { "LYO",  { XR_RI, XR_yo, } },
		     
/* Ýá */ { "MYA",  { XR_MI, XR_ya, } },
/* Ýã */ { "MYU",  { XR_MI, XR_yu, } },
/* Ý¥ */ { "MYE",  { XR_MI, XR__e, } },
/* Ýå */ { "MYO",  { XR_MI, XR_yo, } },
		     
/* Éá */ { "NYA",  { XR_NI, XR_ya, } },
/* Éã */ { "NYU",  { XR_NI, XR_yu, } },
/* É¥ */ { "NYE",  { XR_NI, XR__e, } },
/* Éå */ { "NYO",  { XR_NI, XR_yo, } },
		     
/* Òá */ { "PYA",  { XR_HI, XR_pp, XR_ya, } },
/* Òã */ { "PYU",  { XR_HI, XR_pp, XR_yu, } },
/* Ò¥ */ { "PYE",  { XR_HI, XR_pp, XR__e, } },
/* Òå */ { "PYO",  { XR_HI, XR_pp, XR_yo, } },
		     
/* èá */ { "RYA",  { XR_RI, XR_ya, } },
/* èã */ { "RYU",  { XR_RI, XR_yu, } },
/* è¥ */ { "RYE",  { XR_RI, XR__e, } },
/* èå */ { "RYO",  { XR_RI, XR_yo, } },

/* µá */ { "SYA",  { XR_SI, XR_ya, } },
/* µã */ { "SYU",  { XR_SI, XR_yu, } },
/* µ¥ */ { "SYE",  { XR_SI, XR__e, } },
/* µå */ { "SYO",  { XR_SI, XR_yo, } },
		     
/* ¿á */ { "TYA",  { XR_TI, XR_ya, } },
/* Ä¡ */ { "TYI",  { XR_TE, XR__i, } },
/* ¿ã */ { "TYU",  { XR_TI, XR_yu, } },
/* ¿¥ */ { "TYE",  { XR_TI, XR__e, } },
/* ¿å */ { "TYO",  { XR_TI, XR_yo, } },
		     
/* á   */ { "XYA",  { XR_ya, } },
/* ã   */ { "XYU",  { XR_yu, } },
/* å   */ { "XYO",  { XR_yo, } },
		     
/* ¶á */ { "ZYA",  { XR_SI, XR_jj, XR_ya, } },
/* ¶ã */ { "ZYU",  { XR_SI, XR_jj, XR_yu, } },
/* ¶¥ */ { "ZYE",  { XR_SI, XR_jj, XR__e, } },
/* ¶å */ { "ZYO",  { XR_SI, XR_jj, XR_yo, } },
		     
/* ¿á */ { "CHA",  { XR_TI, XR_ya, } },
/* ¿   */ { "CHI",  { XR_TI,        } },
/* ¿ã */ { "CHU",  { XR_TI, XR_yu, } },
/* ¿¥ */ { "CHE",  { XR_TI, XR__e, } },
/* ¿å */ { "CHO",  { XR_TI, XR_yo, } },

/* µá */ { "SHA",  { XR_SI, XR_ya, } },
/* µ   */ { "SHI",  { XR_SI,        } },
/* µã */ { "SHU",  { XR_SI, XR_yu, } },
/* µ¥ */ { "SHE",  { XR_SI, XR__e, } },
/* µå */ { "SHO",  { XR_SI, XR_yo, } },
		     
/* Â */ { "TSA",  { XR_TU, XR__a, } },
/* Â¡ */ { "TSI",  { XR_TU, XR__i, } },
/* Â   */ { "TSU",  { XR_TU,        } },
/* Â¥ */ { "TSE",  { XR_TU, XR__e, } },
/* Â§ */ { "TSO",  { XR_TU, XR__o, } },
		     
/* Ä¡ */ { "XTI",  { XR_TE, XR__i, } },
/* Á   */ { "XTU",  { XR_tu, } },
/* Á   */ { "XTSU", { XR_tu, } },
		     
/* ñ   */ { "N'",   { XR__N, } },
		     
};


/*
MS-IME Ì [}Ï·

	    ¢   ¤   ¦   ¨
B	Î   Ñ   Ô   ×   Ú
C	©   µ   ­   ¹   ±
D	¾   À   Ã   Å   Ç
F	Ó Ó¡ Ó   Ó¥ Ó§
G	ª   ¬   ®   °   ²
H	Í   Ð   Ó   Ö   Ù
J	¶á ¶   ¶ã ¶¥ ¶å
K	©   «   ­   ¯   ±
L	   ¡   £   ¥   §
M	Ü   Ý   Þ   ß   à
N	È   É   Ê   Ë   Ì
P	Ï   Ò   Õ   Ø   Û
Q	­ ­¡ ­   ­¥ ­§
R	ç   è   é   ê   ë
S	³   µ   ·   ¹   »
T	½   ¿   Â   Ä   Æ
V	 ¡    ¥ §
W	í   ¤¡ ¤   ¤¥ ð
X	   ¡   £   ¥   §
Y	â   ¢   ä   ¢¥ æ
Z	´   ¶   ¸   º   ¼

BY	Ñá Ñ¡ Ñã Ñ¥ Ñå
CY	¿á ¿¡ ¿ã ¿¥ ¿å
DY	Àá À¡ Àã À¥ Àå
FY	Óá Ó¡ Óã Ó¥ Óå
GY	¬á ¬¡ ¬ã ¬¥ ¬å
HY	Ðá Ð¡ Ðã Ð¥ Ðå
JY	¶á ¶¡ ¶ã ¶¥ ¶å
KY	«á «¡ «ã «¥ «å
LY	á   ¡   ã   ¥   å
MY	Ýá Ý¡ Ýã Ý¥ Ýå
NY	Éá É¡ Éã É¥ Éå
PY	Òá Ò¡ Òã Ò¥ Òå
QY	­á ­¡ ­ã ­¥ ­å
RY	èá è¡ èã è¥ èå
SY	µá µ¡ µã µ¥ µå
TY	¿á ¿¡ ¿ã ¿¥ ¿å
VY	á ¡ ã ¥ å
WY
XY	á   ¡   ã   ¥   å
ZY	¶á ¶¡ ¶ã ¶¥ ¶å

CH	¿á ¿   ¿ã ¿¥ ¿å
DH	Åá Å¡ Åã Å¥ Åå
SH	µá µ   µã µ¥ µå
TH	Äá Ä¡ Äã Ä¥ Äå
WH	¤ ¤¡ ¤   ¤¥ ¤§

TS	Â Â¡ Â   Â¥ Â§

XK	             
XT	          Á
XTS
LW	ì
QW	­ ­¡ ­£ ­¥ ­§
LK	             

NN	ñ
N'	ñ


ATOKÌ[}Ï·

	    ¢   ¤  ¦   ¨
B	Î   Ñ   Ô  ×   Ú
C	
D	¾   À   Ã   Å   Ç
F	Ó Ó¡ Ó   Ó¥ Ó§
G	ª   ¬   ®   °   ²
H	Í   Ð   Ó   Ö   Ù
J	¶á ¶   ¶ã ¶¥ ¶å
K	©   «   ­   ¯   ±
L	   ¡   £   ¥   §
M	Ü   Ý   Þ   ß   à
N	È   É   Ê   Ë   Ì
P	Ï   Ò   Õ   Ø   Û
Q	
R	ç   è   é   ê   ë
S	³   µ   ·   ¹   »
T	½   ¿   Â   Ä   Æ
V	¤J¤J¡¤J¤J¥¤J§
W	í   ¤¡ ¤   ¤¥ ð
X	   ¡   £   ¥   §
Y	â   ¢   ä   ¢¥ æ
Z	´   ¶   ¸   º   ¼

BY	Ñá Ñ¡ Ñã Ñ¥ Ñå
CY	¿á ¿¡ ¿ã ¿¥ ¿å
DY	Àá À¡ Àã À¥ Àå
FY	Óá Ó¡ Óã Ó¥ Óå
GY	¬á ¬¡ ¬ã ¬¥ ¬å
HY	Ðá Ð¡ Ðã Ð¥ Ðå
JY	¶á ¶¡ ¶ã ¶¥ ¶å
KY	«á «¡ «ã «¥ «å
LY	á   ¡   ã   ¥   å
MY	Ýá Ý¡ Ýã Ý¥ Ýå
NY	Éá É¡ Éã É¥ Éå
PY	Òá Ò¡ Òã Ò¥ Òå
QY	
RY	èá è¡ èã è¥ èå
SY	µá µ¡ µã µ¥ µå
TY	¿á ¿¡ ¿ã ¿¥ ¿å
VY	
WY	
XY	á   ¡   ã   ¥   å
ZY	¶á ¶¡ ¶ã ¶¥ ¶å

CH	¿á ¿   ¿ã ¿¥ ¿å
DH	Åá Å¡ Åã Å¥ Åå
SH	µá µ   µã µ¥ µå
TH	Äá Ä¡ Äã Ä¥ Äå
WH	

TS	Â Â¡ Â   Â¥ Â§

XK	             
XT	          Á
XTS	          Á
LW	ì
QW	
LK	             

NN	ñ
N'	ñ


egg Ì [}Ï·

	    ¢   ¤   ¦   ¨
B	Î   Ñ   Ô   ×   Ú
C	
D	¾   À   Ã   Å   Ç
F	Ó Ó¡ Ó   Ó¥ Ó§
G	ª   ¬   ®   °   ²
H	Í   Ð   Ó   Ö   Ù
J	¶á ¶   ¶ã ¶¥ ¶å
K	©   «   ­   ¯   ±
L	ç   è   é   ê   ë
M	Ü   Ý   Þ   ß   à
N	È   É   Ê   Ë   Ì
P	Ï   Ò   Õ   Ø   Û
Q	
R	ç   è   é   ê   ë
S	³   µ   ·   ¹   »
T	½   ¿   Â   Ä   Æ
V	 ¡    ¥ §
W	í   î   ¤   ï   ð
X	   ¡   £   ¥   §
Y	â   ¢   ä   ¢¥ æ
Z	´   ¶   ¸   º   ¼

BY	Ñá      Ñã Ñ¥ Ñå
CY	
DY	Àá Å¡ Àã À¥ Àå
FY	
GY	¬á      ¬ã ¬¥ ¬å
HY	Ðá      Ðã Ð¥ Ðå
JY	¶á      ¶ã ¶¥ ¶å
KY	«á      «ã «¥ «å
LY	èá      èã è¥ èå
MY	Ýá      Ýã Ý¥ Ýå
NY	Éá      Éã É¥ Éå
PY	Òá      Òã Ò¥ Òå
QY	
RY	èá      èã è¥ èå
SY	µá      µã µ¥ µå
TY	¿á Ä¡ ¿ã ¿¥ ¿å
VY	
WY	
XY	á        ã        å
ZY	¶á      ¶ã ¶¥ ¶å

CH	¿á ¿   ¿ã ¿¥ ¿å
DH	
SH	µá µ   µã µ¥ µå
TH	
WH	

TS	Â Â¡ Â   Â¥ Â§

XK	             
XT	     Ä¡ Á
XTS	          Á
LW	
QW	
LK	

N	ñ
N'	ñ
*/
