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
#define	XR_jj	RJ(P2,0,0)	/* �J */
#define	XR_pp	RJ(P5,3,0)	/* �K */
#define	XR_oo	RJ(P7,6,1)	/* �E */
#define	XR_rr	RJ(P5,4,0)	/* �[ */
#define	XR_aa	RJ(P5,3,1)	/* �u */
#define	XR_ee	RJ(P5,5,1)	/* �v */
#define	XR_xx	RJ(P7,5,1)	/* �B */
#define	XR_yy	RJ(P7,4,1)	/* �A */


static const romaji_list list_NN = { "", { XR__N, } };
static const romaji_list list_tu = { "", { XR_tu, } };

static const romaji_list list_mark[] =
{
		     
/*  	*/ { " ",    { XR_sp, } },
/* �J 	*/ { "@",    { XR_jj, } },
/* �K 	*/ { "[",    { XR_pp, } },
/* �E 	*/ { "/",    { XR_oo, } },
/* �[ 	*/ { "-",    { XR_rr, } },
/* �u 	*/ { "{",    { XR_aa, } },
/* �v 	*/ { "}",    { XR_ee, } },
/* �B 	*/ { ".",    { XR_xx, } },
/* �A 	*/ { ",",    { XR_yy, } },

};

static const romaji_list list_msime[] =
{

/* ��   */ { "A",    { XR__A, } },

/* ��   */ { "BA",   { XR_HA, XR_jj, } },
/* ��   */ { "BI",   { XR_HI, XR_jj, } },
/* ��   */ { "BU",   { XR_HU, XR_jj, } },
/* ��   */ { "BE",   { XR_HE, XR_jj, } },
/* ��   */ { "BO",   { XR_HO, XR_jj, } },

/* ��   */ { "CA",   { XR_KA, } },
/* ��   */ { "CI",   { XR_SI, } },
/* ��   */ { "CU",   { XR_KU, } },
/* ��   */ { "CE",   { XR_SE, } },
/* ��   */ { "CO",   { XR_KO, } },

/* ��   */ { "DA",   { XR_TA, XR_jj, } },
/* ��   */ { "DI",   { XR_TI, XR_jj, } },
/* ��   */ { "DU",   { XR_TU, XR_jj, } },
/* ��   */ { "DE",   { XR_TE, XR_jj, } },
/* ��   */ { "DO",   { XR_TO, XR_jj, } },

/* ��   */ { "E",    { XR__E, } },

/* �ӂ� */ { "FA",   { XR_HU, XR__a, } },
/* �ӂ� */ { "FI",   { XR_HU, XR__i, } },
/* ��   */ { "FU",   { XR_HU,        } },
/* �ӂ� */ { "FE",   { XR_HU, XR__e, } },
/* �ӂ� */ { "FO",   { XR_HU, XR__o, } },

/* ��   */ { "GA",   { XR_KA, XR_jj, } },
/* ��   */ { "GI",   { XR_KI, XR_jj, } },
/* ��   */ { "GU",   { XR_KU, XR_jj, } },
/* ��   */ { "GE",   { XR_KE, XR_jj, } },
/* ��   */ { "GO",   { XR_KO, XR_jj, } },

/* ��   */ { "HA",   { XR_HA, } },
/* ��   */ { "HI",   { XR_HI, } },
/* ��   */ { "HU",   { XR_HU, } },
/* ��   */ { "HE",   { XR_HE, } },
/* ��   */ { "HO",   { XR_HO, } },

/* ��   */ { "I",    { XR__I, } },

/* ���� */ { "JA",   { XR_SI, XR_jj, XR_ya, } },
/* ��   */ { "JI",   { XR_SI, XR_jj,        } },
/* ���� */ { "JU",   { XR_SI, XR_jj, XR_yu, } },
/* ���� */ { "JE",   { XR_SI, XR_jj, XR__e, } },
/* ���� */ { "JO",   { XR_SI, XR_jj, XR_yo, } },

/* ��   */ { "KA",   { XR_KA, } },
/* ��   */ { "KI",   { XR_KI, } },
/* ��   */ { "KU",   { XR_KU, } },
/* ��   */ { "KE",   { XR_KE, } },
/* ��   */ { "KO",   { XR_KO, } },

/* ��   */ { "LA",   { XR__a, } },
/* ��   */ { "LI",   { XR__i, } },
/* ��   */ { "LU",   { XR__u, } },
/* ��   */ { "LE",   { XR__e, } },
/* ��   */ { "LO",   { XR__o, } },

/* ��   */ { "MA",   { XR_MA, } },
/* ��   */ { "MI",   { XR_MI, } },
/* ��   */ { "MU",   { XR_MU, } },
/* ��   */ { "ME",   { XR_ME, } },
/* ��   */ { "MO",   { XR_MO, } },

/* ��   */ { "NA",   { XR_NA, } },
/* ��   */ { "NI",   { XR_NI, } },
/* ��   */ { "NU",   { XR_NU, } },
/* ��   */ { "NE",   { XR_NE, } },
/* ��   */ { "NO",   { XR_NO, } },

/* ��   */ { "O",    { XR__O, } },

/* ��   */ { "PA",   { XR_HA, XR_pp, } },
/* ��   */ { "PI",   { XR_HI, XR_pp, } },
/* ��   */ { "PU",   { XR_HU, XR_pp, } },
/* ��   */ { "PE",   { XR_HE, XR_pp, } },
/* ��   */ { "PO",   { XR_HO, XR_pp, } },

/* ���� */ { "QA",   { XR_KU, XR__a, } },
/* ���� */ { "QI",   { XR_KU, XR__i, } },
/* ��   */ { "QU",   { XR_KU,        } },
/* ���� */ { "QE",   { XR_KU, XR__e, } },
/* ���� */ { "QO",   { XR_KU, XR__o, } },

/* ��   */ { "RA",   { XR_RA, } },
/* ��   */ { "RI",   { XR_RI, } },
/* ��   */ { "RU",   { XR_RU, } },
/* ��   */ { "RE",   { XR_RE, } },
/* ��   */ { "RO",   { XR_RO, } },

/* ��   */ { "SA",   { XR_SA, } },
/* ��   */ { "SI",   { XR_SI, } },
/* ��   */ { "SU",   { XR_SU, } },
/* ��   */ { "SE",   { XR_SE, } },
/* ��   */ { "SO",   { XR_SO, } },

/* ��   */ { "TA",   { XR_TA, } },
/* ��   */ { "TI",   { XR_TI, } },
/* ��   */ { "TU",   { XR_TU, } },
/* ��   */ { "TE",   { XR_TE, } },
/* ��   */ { "TO",   { XR_TO, } },

/* ��   */ { "U",    { XR__U, } },

/* ���� */ { "VA",   { XR__U, XR_jj, XR__a, } },
/* ���� */ { "VI",   { XR__U, XR_jj, XR__i, } },
/* ��   */ { "VU",   { XR__U, XR_jj,        } },
/* ���� */ { "VE",   { XR__U, XR_jj, XR__e, } },
/* ���� */ { "VO",   { XR__U, XR_jj, XR__o, } },

/* ��   */ { "WA",   { XR_WA,        } },
/* ���� */ { "WI",   { XR__U, XR__i, } },
/* ��   */ { "WU",   { XR__U,        } },
/* ���� */ { "WE",   { XR__U, XR__e, } },
/* ��   */ { "WO",   { XR_WO,        } },

/* ��   */ { "XA",   { XR__a, } },
/* ��   */ { "XI",   { XR__i, } },
/* ��   */ { "XU",   { XR__u, } },
/* ��   */ { "XE",   { XR__e, } },
/* ��   */ { "XO",   { XR__o, } },

/* ��   */ { "YA",   { XR_YA,        } },
/* ��   */ { "YI",   { XR__I,        } },
/* ��   */ { "YU",   { XR_YU,        } },
/* ���� */ { "YE",   { XR__I, XR__e, } },
/* ��   */ { "YO",   { XR_YO,        } },

/* ��   */ { "ZA",   { XR_SA, XR_jj, } },
/* ��   */ { "ZI",   { XR_SI, XR_jj, } },
/* ��   */ { "ZU",   { XR_SU, XR_jj, } },
/* ��   */ { "ZE",   { XR_SE, XR_jj, } },
/* ��   */ { "ZO",   { XR_SO, XR_jj, } },

/* �т� */ { "BYA",  { XR_HI, XR_jj, XR_ya, } },
/* �т� */ { "BYI",  { XR_HI, XR_jj, XR__i, } },
/* �т� */ { "BYU",  { XR_HI, XR_jj, XR_yu, } },
/* �т� */ { "BYE",  { XR_HI, XR_jj, XR__e, } },
/* �т� */ { "BYO",  { XR_HI, XR_jj, XR_yo, } },
		     
/* ���� */ { "CYA",  { XR_TI, XR_ya, } },
/* ���� */ { "CYI",  { XR_TI, XR__i, } },
/* ���� */ { "CYU",  { XR_TI, XR_yu, } },
/* ���� */ { "CYE",  { XR_TI, XR__e, } },
/* ���� */ { "CYO",  { XR_TI, XR_yo, } },
		     
/* ���� */ { "DYA",  { XR_TI, XR_jj, XR_ya, } },
/* ���� */ { "DYI",  { XR_TI, XR_jj, XR__i, } },
/* ���� */ { "DYU",  { XR_TI, XR_jj, XR_yu, } },
/* ���� */ { "DYE",  { XR_TI, XR_jj, XR__e, } },
/* ���� */ { "DYO",  { XR_TI, XR_jj, XR_yo, } },
		     
/* �ӂ� */ { "FYA",  { XR_HU, XR_ya, } },
/* �ӂ� */ { "FYI",  { XR_HU, XR__i, } },
/* �ӂ� */ { "FYU",  { XR_HU, XR_yu, } },
/* �ӂ� */ { "FYE",  { XR_HU, XR__e, } },
/* �ӂ� */ { "FYO",  { XR_HU, XR_yo, } },
		     
/* ���� */ { "GYA",  { XR_KI, XR_jj, XR_ya, } },
/* ���� */ { "GYI",  { XR_KI, XR_jj, XR__i, } },
/* ���� */ { "GYU",  { XR_KI, XR_jj, XR_yu, } },
/* ���� */ { "GYE",  { XR_KI, XR_jj, XR__e, } },
/* ���� */ { "GYO",  { XR_KI, XR_jj, XR_yo, } },
		     
/* �Ђ� */ { "HYA",  { XR_HI, XR_ya, } },
/* �Ђ� */ { "HYI",  { XR_HI, XR__i, } },
/* �Ђ� */ { "HYU",  { XR_HI, XR_yu, } },
/* �Ђ� */ { "HYE",  { XR_HI, XR__e, } },
/* �Ђ� */ { "HYO",  { XR_HI, XR_yo, } },
		     
/* ���� */ { "JYA",  { XR_SI, XR_jj, XR_ya, } },
/* ���� */ { "JYI",  { XR_SI, XR_jj, XR__i, } },
/* ���� */ { "JYU",  { XR_SI, XR_jj, XR_yu, } },
/* ���� */ { "JYE",  { XR_SI, XR_jj, XR__e, } },
/* ���� */ { "JYO",  { XR_SI, XR_jj, XR_yo, } },
		     
/* ���� */ { "KYA",  { XR_KI, XR_ya, } },
/* ���� */ { "KYI",  { XR_KI, XR__i, } },
/* ���� */ { "KYU",  { XR_KI, XR_yu, } },
/* ���� */ { "KYE",  { XR_KI, XR__e, } },
/* ���� */ { "KYO",  { XR_KI, XR_yo, } },
		     
/* ��   */ { "LYA",  { XR_ya, } },
/* ��   */ { "LYI",  { XR__i, } },
/* ��   */ { "LYU",  { XR_yu, } },
/* ��   */ { "LYE",  { XR__e, } },
/* ��   */ { "LYO",  { XR_yo, } },
		     
/* �݂� */ { "MYA",  { XR_MI, XR_ya, } },
/* �݂� */ { "MYI",  { XR_MI, XR__i, } },
/* �݂� */ { "MYU",  { XR_MI, XR_yu, } },
/* �݂� */ { "MYE",  { XR_MI, XR__e, } },
/* �݂� */ { "MYO",  { XR_MI, XR_yo, } },

/* �ɂ� */ { "NYA",  { XR_NI, XR_ya, } },
/* �ɂ� */ { "NYI",  { XR_NI, XR__i, } },
/* �ɂ� */ { "NYU",  { XR_NI, XR_yu, } },
/* �ɂ� */ { "NYE",  { XR_NI, XR__e, } },
/* �ɂ� */ { "NYO",  { XR_NI, XR_yo, } },
		     
/* �҂� */ { "PYA",  { XR_HI, XR_pp, XR_ya, } },
/* �҂� */ { "PYI",  { XR_HI, XR_pp, XR__i, } },
/* �҂� */ { "PYU",  { XR_HI, XR_pp, XR_yu, } },
/* �҂� */ { "PYE",  { XR_HI, XR_pp, XR__e, } },
/* �҂� */ { "PYO",  { XR_HI, XR_pp, XR_yo, } },
		     
/* ���� */ { "QYA",  { XR_KU, XR_ya, } },
/* ���� */ { "QYI",  { XR_KU, XR__i, } },
/* ���� */ { "QYU",  { XR_KU, XR_yu, } },
/* ���� */ { "QYE",  { XR_KU, XR__e, } },
/* ���� */ { "QYO",  { XR_KU, XR_yo, } },
		     
/* ��� */ { "RYA",  { XR_RI, XR_ya, } },
/* �股 */ { "RYI",  { XR_RI, XR__i, } },
/* ��� */ { "RYU",  { XR_RI, XR_yu, } },
/* �肥 */ { "RYE",  { XR_RI, XR__e, } },
/* ��� */ { "RYO",  { XR_RI, XR_yo, } },

/* ���� */ { "SYA",  { XR_SI, XR_ya, } },
/* ���� */ { "SYI",  { XR_SI, XR__i, } },
/* ���� */ { "SYU",  { XR_SI, XR_yu, } },
/* ���� */ { "SYE",  { XR_SI, XR__e, } },
/* ���� */ { "SYO",  { XR_SI, XR_yo, } },
		     
/* ���� */ { "TYA",  { XR_TI, XR_ya, } },
/* ���� */ { "TYI",  { XR_TI, XR__i, } },
/* ���� */ { "TYU",  { XR_TI, XR_yu, } },
/* ���� */ { "TYE",  { XR_TI, XR__e, } },
/* ���� */ { "TYO",  { XR_TI, XR_yo, } },
		     
/* ���� */ { "VYA",  { XR__U, XR_jj, XR_ya, } },
/* ���� */ { "VYI",  { XR__U, XR_jj, XR__i, } },
/* ���� */ { "VYU",  { XR__U, XR_jj, XR_yu, } },
/* ���� */ { "VYE",  { XR__U, XR_jj, XR__e, } },
/* ���� */ { "VYO",  { XR__U, XR_jj, XR_yo, } },
		     
/* ��   */ { "XYA",  { XR_ya, } },
/* ��   */ { "XYI",  { XR__i, } },
/* ��   */ { "XYU",  { XR_yu, } },
/* ��   */ { "XYE",  { XR__e, } },
/* ��   */ { "XYO",  { XR_yo, } },

/* ���� */ { "ZYA",  { XR_SI, XR_jj, XR_ya, } },
/* ���� */ { "ZYI",  { XR_SI, XR_jj, XR__i, } },
/* ���� */ { "ZYU",  { XR_SI, XR_jj, XR_yu, } },
/* ���� */ { "ZYE",  { XR_SI, XR_jj, XR__e, } },
/* ���� */ { "ZYO",  { XR_SI, XR_jj, XR_yo, } },
		     
/* ���� */ { "CHA",  { XR_TI, XR_ya, } },
/* ��   */ { "CHI",  { XR_TI,        } },
/* ���� */ { "CHU",  { XR_TI, XR_yu, } },
/* ���� */ { "CHE",  { XR_TI, XR__e, } },
/* ���� */ { "CHO",  { XR_TI, XR_yo, } },
		     
/* �ł� */ { "DHA",  { XR_TE, XR_jj, XR_ya, } },
/* �ł� */ { "DHI",  { XR_TE, XR_jj, XR__i, } },
/* �ł� */ { "DHU",  { XR_TE, XR_jj, XR_yu, } },
/* �ł� */ { "DHE",  { XR_TE, XR_jj, XR__e, } },
/* �ł� */ { "DHO",  { XR_TE, XR_jj, XR_yo, } },
		     
/* ���� */ { "SHA",  { XR_SI, XR_ya, } },
/* ��   */ { "SHI",  { XR_SI,        } },
/* ���� */ { "SHU",  { XR_SI, XR_yu, } },
/* ���� */ { "SHE",  { XR_SI, XR__e, } },
/* ���� */ { "SHO",  { XR_SI, XR_yo, } },
		     
/* �Ă� */ { "THA",  { XR_TE, XR_ya, } },
/* �Ă� */ { "THI",  { XR_TE, XR__i, } },
/* �Ă� */ { "THU",  { XR_TE, XR_yu, } },
/* �Ă� */ { "THE",  { XR_TE, XR__e, } },
/* �Ă� */ { "THO",  { XR_TE, XR_yo, } },
		     
/* ���� */ { "WHA",  { XR__U, XR_ya, } },
/* ���� */ { "WHI",  { XR__U, XR__i, } },
/* ��   */ { "WHU",  { XR__U,        } },
/* ���� */ { "WHE",  { XR__U, XR__e, } },
/* ���� */ { "WHO",  { XR__U, XR_yo, } },
		     
/* �� */ { "TSA",  { XR_TU, XR_ya, } },
/* �� */ { "TSI",  { XR_TU, XR__i, } },
/* ��   */ { "TSU",  { XR_TU,        } },
/* �� */ { "TSE",  { XR_TU, XR__e, } },
/* �� */ { "TSO",  { XR_TU, XR_yo, } },
		     
/* ��   */ { "XTU",  { XR_tu, } },
		     
/* ���� */ { "QWA",  { XR_KU, XR__a, } },
/* ���� */ { "QWI",  { XR_KU, XR__i, } },
/* ���� */ { "QWU",  { XR_KU, XR__u, } },
/* ���� */ { "QWE",  { XR_KU, XR__e, } },
/* ���� */ { "QWO",  { XR_KU, XR__o, } },
		     
/* ��   */ { "NN",   { XR__N, } },
/* ��   */ { "N'",   { XR__N, } },
		     
};

static const romaji_list list_atok[] =
{

/* ��   */ { "A",    { XR__A, } },

/* ��   */ { "BA",   { XR_HA, XR_jj, } },
/* ��   */ { "BI",   { XR_HI, XR_jj, } },
/* ��   */ { "BU",   { XR_HU, XR_jj, } },
/* ��   */ { "BE",   { XR_HE, XR_jj, } },
/* ��   */ { "BO",   { XR_HO, XR_jj, } },

/* ��   */ { "DA",   { XR_TA, XR_jj, } },
/* ��   */ { "DI",   { XR_TI, XR_jj, } },
/* ��   */ { "DU",   { XR_TU, XR_jj, } },
/* ��   */ { "DE",   { XR_TE, XR_jj, } },
/* ��   */ { "DO",   { XR_TO, XR_jj, } },

/* ��   */ { "E",    { XR__E, } },

/* �ӂ� */ { "FA",   { XR_HU, XR__a, } },
/* �ӂ� */ { "FI",   { XR_HU, XR__i, } },
/* ��   */ { "FU",   { XR_HU,        } },
/* �ӂ� */ { "FE",   { XR_HU, XR__e, } },
/* �ӂ� */ { "FO",   { XR_HU, XR__o, } },

/* ��   */ { "GA",   { XR_KA, XR_jj, } },
/* ��   */ { "GI",   { XR_KI, XR_jj, } },
/* ��   */ { "GU",   { XR_KU, XR_jj, } },
/* ��   */ { "GE",   { XR_KE, XR_jj, } },
/* ��   */ { "GO",   { XR_KO, XR_jj, } },

/* ��   */ { "HA",   { XR_HA, } },
/* ��   */ { "HI",   { XR_HI, } },
/* ��   */ { "HU",   { XR_HU, } },
/* ��   */ { "HE",   { XR_HE, } },
/* ��   */ { "HO",   { XR_HO, } },

/* ��   */ { "I",    { XR__I, } },

/* ���� */ { "JA",   { XR_SI, XR_jj, XR_ya, } },
/* ��   */ { "JI",   { XR_SI, XR_jj,        } },
/* ���� */ { "JU",   { XR_SI, XR_jj, XR_yu, } },
/* ���� */ { "JE",   { XR_SI, XR_jj, XR__e, } },
/* ���� */ { "JO",   { XR_SI, XR_jj, XR_yo, } },

/* ��   */ { "KA",   { XR_KA, } },
/* ��   */ { "KI",   { XR_KI, } },
/* ��   */ { "KU",   { XR_KU, } },
/* ��   */ { "KE",   { XR_KE, } },
/* ��   */ { "KO",   { XR_KO, } },

/* ��   */ { "LA",   { XR__a, } },
/* ��   */ { "LI",   { XR__i, } },
/* ��   */ { "LU",   { XR__u, } },
/* ��   */ { "LE",   { XR__e, } },
/* ��   */ { "LO",   { XR__o, } },

/* ��   */ { "MA",   { XR_MA, } },
/* ��   */ { "MI",   { XR_MI, } },
/* ��   */ { "MU",   { XR_MU, } },
/* ��   */ { "ME",   { XR_ME, } },
/* ��   */ { "MO",   { XR_MO, } },

/* ��   */ { "NA",   { XR_NA, } },
/* ��   */ { "NI",   { XR_NI, } },
/* ��   */ { "NU",   { XR_NU, } },
/* ��   */ { "NE",   { XR_NE, } },
/* ��   */ { "NO",   { XR_NO, } },

/* ��   */ { "O",    { XR__O, } },

/* ��   */ { "PA",   { XR_HA, XR_pp, } },
/* ��   */ { "PI",   { XR_HI, XR_pp, } },
/* ��   */ { "PU",   { XR_HU, XR_pp, } },
/* ��   */ { "PE",   { XR_HE, XR_pp, } },
/* ��   */ { "PO",   { XR_HO, XR_pp, } },

/* ��   */ { "RA",   { XR_RA, } },
/* ��   */ { "RI",   { XR_RI, } },
/* ��   */ { "RU",   { XR_RU, } },
/* ��   */ { "RE",   { XR_RE, } },
/* ��   */ { "RO",   { XR_RO, } },

/* ��   */ { "SA",   { XR_SA, } },
/* ��   */ { "SI",   { XR_SI, } },
/* ��   */ { "SU",   { XR_SU, } },
/* ��   */ { "SE",   { XR_SE, } },
/* ��   */ { "SO",   { XR_SO, } },

/* ��   */ { "TA",   { XR_TA, } },
/* ��   */ { "TI",   { XR_TI, } },
/* ��   */ { "TU",   { XR_TU, } },
/* ��   */ { "TE",   { XR_TE, } },
/* ��   */ { "TO",   { XR_TO, } },

/* ��   */ { "U",    { XR__U, } },

/* ���� */ { "VA",   { XR__U, XR_jj, XR__a, } },
/* ���� */ { "VI",   { XR__U, XR_jj, XR__i, } },
/* ��   */ { "VU",   { XR__U, XR_jj,        } },
/* ���� */ { "VE",   { XR__U, XR_jj, XR__e, } },
/* ���� */ { "VO",   { XR__U, XR_jj, XR__o, } },

/* ��   */ { "WA",   { XR_WA,        } },
/* ���� */ { "WI",   { XR__U, XR__i, } },
/* ��   */ { "WU",   { XR__U,        } },
/* ���� */ { "WE",   { XR__U, XR__e, } },
/* ��   */ { "WO",   { XR_WO,        } },

/* ��   */ { "XA",   { XR__a, } },
/* ��   */ { "XI",   { XR__i, } },
/* ��   */ { "XU",   { XR__u, } },
/* ��   */ { "XE",   { XR__e, } },
/* ��   */ { "XO",   { XR__o, } },

/* ��   */ { "YA",   { XR_YA,        } },
/* ��   */ { "YI",   { XR__I,        } },
/* ��   */ { "YU",   { XR_YU,        } },
/* ���� */ { "YE",   { XR__I, XR__e, } },
/* ��   */ { "YO",   { XR_YO,        } },

/* ��   */ { "ZA",   { XR_SA, XR_jj, } },
/* ��   */ { "ZI",   { XR_SI, XR_jj, } },
/* ��   */ { "ZU",   { XR_SU, XR_jj, } },
/* ��   */ { "ZE",   { XR_SE, XR_jj, } },
/* ��   */ { "ZO",   { XR_SO, XR_jj, } },

/* �т� */ { "BYA",  { XR_HI, XR_jj, XR_ya, } },
/* �т� */ { "BYI",  { XR_HI, XR_jj, XR__i, } },
/* �т� */ { "BYU",  { XR_HI, XR_jj, XR_yu, } },
/* �т� */ { "BYE",  { XR_HI, XR_jj, XR__e, } },
/* �т� */ { "BYO",  { XR_HI, XR_jj, XR_yo, } },
		     
/* ���� */ { "CYA",  { XR_TI, XR_ya, } },
/* ���� */ { "CYI",  { XR_TI, XR__i, } },
/* ���� */ { "CYU",  { XR_TI, XR_yu, } },
/* ���� */ { "CYE",  { XR_TI, XR__e, } },
/* ���� */ { "CYO",  { XR_TI, XR_yo, } },
		     
/* ���� */ { "DYA",  { XR_TI, XR_jj, XR_ya, } },
/* ���� */ { "DYI",  { XR_TI, XR_jj, XR__i, } },
/* ���� */ { "DYU",  { XR_TI, XR_jj, XR_yu, } },
/* ���� */ { "DYE",  { XR_TI, XR_jj, XR__e, } },
/* ���� */ { "DYO",  { XR_TI, XR_jj, XR_yo, } },
		     
/* �ӂ� */ { "FYA",  { XR_HU, XR_ya, } },
/* �ӂ� */ { "FYI",  { XR_HU, XR__i, } },
/* �ӂ� */ { "FYU",  { XR_HU, XR_yu, } },
/* �ӂ� */ { "FYE",  { XR_HU, XR__e, } },
/* �ӂ� */ { "FYO",  { XR_HU, XR_ya, } },
		     
/* ���� */ { "GYA",  { XR_KI, XR_jj, XR_ya, } },
/* ���� */ { "GYI",  { XR_KI, XR_jj, XR__i, } },
/* ���� */ { "GYU",  { XR_KI, XR_jj, XR_yu, } },
/* ���� */ { "GYE",  { XR_KI, XR_jj, XR__e, } },
/* ���� */ { "GYO",  { XR_KI, XR_jj, XR_yo, } },
		     
/* �Ђ� */ { "HYA",  { XR_HI, XR_ya, } },
/* �Ђ� */ { "HYI",  { XR_HI, XR__i, } },
/* �Ђ� */ { "HYU",  { XR_HI, XR_yu, } },
/* �Ђ� */ { "HYE",  { XR_HI, XR__e, } },
/* �Ђ� */ { "HYO",  { XR_HI, XR_yo, } },
		     
/* ���� */ { "JYA",  { XR_SI, XR_jj, XR_ya, } },
/* ���� */ { "JYI",  { XR_SI, XR_jj, XR__i, } },
/* ���� */ { "JYU",  { XR_SI, XR_jj, XR_yu, } },
/* ���� */ { "JYE",  { XR_SI, XR_jj, XR__e, } },
/* ���� */ { "JYO",  { XR_SI, XR_jj, XR_yo, } },

/* ���� */ { "KYA",  { XR_KI, XR_ya, } },
/* ���� */ { "KYI",  { XR_KI, XR__i, } },
/* ���� */ { "KYU",  { XR_KI, XR_yu, } },
/* ���� */ { "KYE",  { XR_KI, XR__e, } },
/* ���� */ { "KYO",  { XR_KI, XR_yo, } },
		     
/* ��   */ { "LYA",  { XR_ya, } },
/* ��   */ { "LYI",  { XR__i, } },
/* ��   */ { "LYU",  { XR_yu, } },
/* ��   */ { "LYE",  { XR__e, } },
/* ��   */ { "LYO",  { XR_yo, } },
		     
/* �݂� */ { "MYA",  { XR_MI, XR_ya, } },
/* �݂� */ { "MYI",  { XR_MI, XR__i, } },
/* �݂� */ { "MYU",  { XR_MI, XR_yu, } },
/* �݂� */ { "MYE",  { XR_MI, XR__e, } },
/* �݂� */ { "MYO",  { XR_MI, XR_yo, } },
		     
/* �ɂ� */ { "NYA",  { XR_NI, XR_ya, } },
/* �ɂ� */ { "NYI",  { XR_NI, XR__i, } },
/* �ɂ� */ { "NYU",  { XR_NI, XR_yu, } },
/* �ɂ� */ { "NYE",  { XR_NI, XR__e, } },
/* �ɂ� */ { "NYO",  { XR_NI, XR_yo, } },
		     
/* �҂� */ { "PYA",  { XR_HI, XR_pp, XR_ya, } },
/* �҂� */ { "PYI",  { XR_HI, XR_pp, XR__i, } },
/* �҂� */ { "PYU",  { XR_HI, XR_pp, XR_yu, } },
/* �҂� */ { "PYE",  { XR_HI, XR_pp, XR__e, } },
/* �҂� */ { "PYO",  { XR_HI, XR_pp, XR_yo, } },
		     
/* ��� */ { "RYA",  { XR_RI, XR_ya, } },
/* �股 */ { "RYI",  { XR_RI, XR__i, } },
/* ��� */ { "RYU",  { XR_RI, XR_yu, } },
/* �肥 */ { "RYE",  { XR_RI, XR__e, } },
/* ��� */ { "RYO",  { XR_RI, XR_yo, } },
		     
/* ���� */ { "SYA",  { XR_SI, XR_ya, } },
/* ���� */ { "SYI",  { XR_SI, XR__i, } },
/* ���� */ { "SYU",  { XR_SI, XR_yu, } },
/* ���� */ { "SYE",  { XR_SI, XR__e, } },
/* ���� */ { "SYO",  { XR_SI, XR_yo, } },

/* ���� */ { "TYA",  { XR_TI, XR_ya, } },
/* ���� */ { "TYI",  { XR_TI, XR__i, } },
/* ���� */ { "TYU",  { XR_TI, XR_yu, } },
/* ���� */ { "TYE",  { XR_TI, XR__e, } },
/* ���� */ { "TYO",  { XR_TI, XR_yo, } },
		     
/* ��   */ { "XYA",  { XR_ya, } },
/* ��   */ { "XYI",  { XR__i, } },
/* ��   */ { "XYU",  { XR_yu, } },
/* ��   */ { "XYE",  { XR__e, } },
/* ��   */ { "XYO",  { XR_yo, } },
		     
/* ���� */ { "ZYA",  { XR_SI, XR_jj, XR_ya, } },
/* ���� */ { "ZYI",  { XR_SI, XR_jj, XR__i, } },
/* ���� */ { "ZYU",  { XR_SI, XR_jj, XR_yu, } },
/* ���� */ { "ZYE",  { XR_SI, XR_jj, XR__e, } },
/* ���� */ { "ZYO",  { XR_SI, XR_jj, XR_yo, } },
		     
/* ���� */ { "CHA",  { XR_TI, XR_ya, } },
/* ��   */ { "CHI",  { XR_TI,        } },
/* ���� */ { "CHU",  { XR_TI, XR_yu, } },
/* ���� */ { "CHE",  { XR_TI, XR__e, } },
/* ���� */ { "CHO",  { XR_TI, XR_yo, } },

/* �ł� */ { "DHA",  { XR_TE, XR_jj, XR_ya, } },
/* �ł� */ { "DHI",  { XR_TE, XR_jj, XR__i, } },
/* �ł� */ { "DHU",  { XR_TE, XR_jj, XR_yu, } },
/* �ł� */ { "DHE",  { XR_TE, XR_jj, XR__e, } },
/* �ł� */ { "DHO",  { XR_TE, XR_jj, XR_yo, } },
		     
/* ���� */ { "SHA",  { XR_SI, XR_ya, } },
/* ��   */ { "SHI",  { XR_SI,        } },
/* ���� */ { "SHU",  { XR_SI, XR_yu, } },
/* ���� */ { "SHE",  { XR_SI, XR__e, } },
/* ���� */ { "SHO",  { XR_SI, XR_yo, } },
		     
/* �Ă� */ { "THA",  { XR_TE, XR_ya, } },
/* �Ă� */ { "THI",  { XR_TE, XR__i, } },
/* �Ă� */ { "THU",  { XR_TE, XR_yu, } },
/* �Ă� */ { "THE",  { XR_TE, XR__e, } },
/* �Ă� */ { "THO",  { XR_TE, XR_yo, } },
		     
/* �� */ { "TSA",  { XR_TU, XR__a, } },
/* �� */ { "TSI",  { XR_TU, XR__i, } },
/* ��   */ { "TSU",  { XR_TU,        } },
/* �� */ { "TSE",  { XR_TU, XR__e, } },
/* �� */ { "TSO",  { XR_TU, XR__o, } },
		     
/* ��   */ { "XTU",  { XR_tu, } },
/* ��   */ { "XTSU", { XR_tu, } },
		     
/* ��   */ { "NN",   { XR__N, } },
/* ��   */ { "N'",   { XR__N, } },

};

static const romaji_list list_egg[] =
{

/* ��   */ { "A",    { XR__A, } },

/* ��   */ { "BA",   { XR_HA, XR_jj, } },
/* ��   */ { "BI",   { XR_HI, XR_jj, } },
/* ��   */ { "BU",   { XR_HU, XR_jj, } },
/* ��   */ { "BE",   { XR_HE, XR_jj, } },
/* ��   */ { "BO",   { XR_HO, XR_jj, } },

/* ��   */ { "DA",   { XR_TA, XR_jj, } },
/* ��   */ { "DI",   { XR_TI, XR_jj, } },
/* ��   */ { "DU",   { XR_TU, XR_jj, } },
/* ��   */ { "DE",   { XR_TE, XR_jj, } },
/* ��   */ { "DO",   { XR_TO, XR_jj, } },

/* ��   */ { "E",    { XR__E, } },

/* �ӂ� */ { "FA",   { XR_HU, XR__a, } },
/* �ӂ� */ { "FI",   { XR_HU, XR__i, } },
/* ��   */ { "FU",   { XR_HU,        } },
/* �ӂ� */ { "FE",   { XR_HU, XR__e, } },
/* �ӂ� */ { "FO",   { XR_HU, XR__o, } },

/* ��   */ { "GA",   { XR_KA, XR_jj, } },
/* ��   */ { "GI",   { XR_KI, XR_jj, } },
/* ��   */ { "GU",   { XR_KU, XR_jj, } },
/* ��   */ { "GE",   { XR_KE, XR_jj, } },
/* ��   */ { "GO",   { XR_KO, XR_jj, } },

/* ��   */ { "HA",   { XR_HA, } },
/* ��   */ { "HI",   { XR_HI, } },
/* ��   */ { "HU",   { XR_HU, } },
/* ��   */ { "HE",   { XR_HE, } },
/* ��   */ { "HO",   { XR_HO, } },

/* ��   */ { "I",    { XR__I, } },

/* ���� */ { "JA",   { XR_SI, XR_jj, XR_ya, } },
/* ��   */ { "JI",   { XR_SI, XR_jj,        } },
/* ���� */ { "JU",   { XR_SI, XR_jj, XR_yu, } },
/* ���� */ { "JE",   { XR_SI, XR_jj, XR__e, } },
/* ���� */ { "JO",   { XR_SI, XR_jj, XR_yo, } },

/* ��   */ { "KA",   { XR_KA, } },
/* ��   */ { "KI",   { XR_KI, } },
/* ��   */ { "KU",   { XR_KU, } },
/* ��   */ { "KE",   { XR_KE, } },
/* ��   */ { "KO",   { XR_KO, } },

/* ��   */ { "LA",   { XR_RA, } },
/* ��   */ { "LI",   { XR_RI, } },
/* ��   */ { "LU",   { XR_RU, } },
/* ��   */ { "LE",   { XR_RE, } },
/* ��   */ { "LO",   { XR_RO, } },

/* ��   */ { "MA",   { XR_MA, } },
/* ��   */ { "MI",   { XR_MI, } },
/* ��   */ { "MU",   { XR_MU, } },
/* ��   */ { "ME",   { XR_ME, } },
/* ��   */ { "MO",   { XR_MO, } },

/* ��   */ { "NA",   { XR_NA, } },
/* ��   */ { "NI",   { XR_NI, } },
/* ��   */ { "NU",   { XR_NU, } },
/* ��   */ { "NE",   { XR_NE, } },
/* ��   */ { "NO",   { XR_NO, } },

/* ��   */ { "O",    { XR__O, } },

/* ��   */ { "PA",   { XR_HA, XR_pp, } },
/* ��   */ { "PI",   { XR_HI, XR_pp, } },
/* ��   */ { "PU",   { XR_HU, XR_pp, } },
/* ��   */ { "PE",   { XR_HE, XR_pp, } },
/* ��   */ { "PO",   { XR_HO, XR_pp, } },

/* ��   */ { "RA",   { XR_RA, } },
/* ��   */ { "RI",   { XR_RI, } },
/* ��   */ { "RU",   { XR_RU, } },
/* ��   */ { "RE",   { XR_RE, } },
/* ��   */ { "RO",   { XR_RO, } },

/* ��   */ { "SA",   { XR_SA, } },
/* ��   */ { "SI",   { XR_SI, } },
/* ��   */ { "SU",   { XR_SU, } },
/* ��   */ { "SE",   { XR_SE, } },
/* ��   */ { "SO",   { XR_SO, } },

/* ��   */ { "TA",   { XR_TA, } },
/* ��   */ { "TI",   { XR_TI, } },
/* ��   */ { "TU",   { XR_TU, } },
/* ��   */ { "TE",   { XR_TE, } },
/* ��   */ { "TO",   { XR_TO, } },

/* ��   */ { "U",    { XR__U, } },

/* ���� */ { "VA",   { XR__U, XR_jj, XR__a, } },
/* ���� */ { "VI",   { XR__U, XR_jj, XR__i, } },
/* ��   */ { "VU",   { XR__U, XR_jj,        } },
/* ���� */ { "VE",   { XR__U, XR_jj, XR__e, } },
/* ���� */ { "VO",   { XR__U, XR_jj, XR__o, } },

/* ��   */ { "WA",   { XR_WA,        } },
/* ��   */ { "WI",   { XR__I,        } },
/* ��   */ { "WU",   { XR__U,        } },
/* ��   */ { "WE",   { XR__E,        } },
/* ��   */ { "WO",   { XR_WO,        } },

/* ��   */ { "XA",   { XR__a, } },
/* ��   */ { "XI",   { XR__i, } },
/* ��   */ { "XU",   { XR__u, } },
/* ��   */ { "XE",   { XR__e, } },
/* ��   */ { "XO",   { XR__o, } },

/* ��   */ { "YA",   { XR_YA,        } },
/* ��   */ { "YI",   { XR__I,        } },
/* ��   */ { "YU",   { XR_YU,        } },
/* ���� */ { "YE",   { XR__I, XR__e, } },
/* ��   */ { "YO",   { XR_YO,        } },

/* ��   */ { "ZA",   { XR_SA, XR_jj, } },
/* ��   */ { "ZI",   { XR_SI, XR_jj, } },
/* ��   */ { "ZU",   { XR_SU, XR_jj, } },
/* ��   */ { "ZE",   { XR_SE, XR_jj, } },
/* ��   */ { "ZO",   { XR_SO, XR_jj, } },

/* �т� */ { "BYA",  { XR_HI, XR_jj, XR_ya, } },
/* �т� */ { "BYU",  { XR_HI, XR_jj, XR_yu, } },
/* �т� */ { "BYE",  { XR_HI, XR_jj, XR__e, } },
/* �т� */ { "BYO",  { XR_HI, XR_jj, XR_yo, } },
		     
/* ���� */ { "DYA",  { XR_TI, XR_jj, XR_ya, } },
/* �ł� */ { "DYI",  { XR_TE, XR_jj, XR__i, } },
/* ���� */ { "DYU",  { XR_TI, XR_jj, XR_yu, } },
/* ���� */ { "DYE",  { XR_TI, XR_jj, XR__e, } },
/* ���� */ { "DYO",  { XR_TI, XR_jj, XR_yo, } },
		     
/* ���� */ { "GYA",  { XR_KI, XR_jj, XR_ya, } },
/* ���� */ { "GYU",  { XR_KI, XR_jj, XR_yu, } },
/* ���� */ { "GYE",  { XR_KI, XR_jj, XR__e, } },
/* ���� */ { "GYO",  { XR_KI, XR_jj, XR_yo, } },
		     
/* �Ђ� */ { "HYA",  { XR_HI, XR_ya, } },
/* �Ђ� */ { "HYU",  { XR_HI, XR_yu, } },
/* �Ђ� */ { "HYE",  { XR_HI, XR__e, } },
/* �Ђ� */ { "HYO",  { XR_HI, XR_yo, } },
		     
/* ���� */ { "JYA",  { XR_SI, XR_jj, XR_ya, } },
/* ���� */ { "JYU",  { XR_SI, XR_jj, XR_yu, } },
/* ���� */ { "JYE",  { XR_SI, XR_jj, XR__e, } },
/* ���� */ { "JYO",  { XR_SI, XR_jj, XR_yo, } },
		     
/* ���� */ { "KYA",  { XR_KI, XR_ya, } },
/* ���� */ { "KYU",  { XR_KI, XR_yu, } },
/* ���� */ { "KYE",  { XR_KI, XR__e, } },
/* ���� */ { "KYO",  { XR_KI, XR_yo, } },

/* ��� */ { "LYA",  { XR_RI, XR_ya, } },
/* ��� */ { "LYU",  { XR_RI, XR_yu, } },
/* �肥 */ { "LYE",  { XR_RI, XR__e, } },
/* ��� */ { "LYO",  { XR_RI, XR_yo, } },
		     
/* �݂� */ { "MYA",  { XR_MI, XR_ya, } },
/* �݂� */ { "MYU",  { XR_MI, XR_yu, } },
/* �݂� */ { "MYE",  { XR_MI, XR__e, } },
/* �݂� */ { "MYO",  { XR_MI, XR_yo, } },
		     
/* �ɂ� */ { "NYA",  { XR_NI, XR_ya, } },
/* �ɂ� */ { "NYU",  { XR_NI, XR_yu, } },
/* �ɂ� */ { "NYE",  { XR_NI, XR__e, } },
/* �ɂ� */ { "NYO",  { XR_NI, XR_yo, } },
		     
/* �҂� */ { "PYA",  { XR_HI, XR_pp, XR_ya, } },
/* �҂� */ { "PYU",  { XR_HI, XR_pp, XR_yu, } },
/* �҂� */ { "PYE",  { XR_HI, XR_pp, XR__e, } },
/* �҂� */ { "PYO",  { XR_HI, XR_pp, XR_yo, } },
		     
/* ��� */ { "RYA",  { XR_RI, XR_ya, } },
/* ��� */ { "RYU",  { XR_RI, XR_yu, } },
/* �肥 */ { "RYE",  { XR_RI, XR__e, } },
/* ��� */ { "RYO",  { XR_RI, XR_yo, } },

/* ���� */ { "SYA",  { XR_SI, XR_ya, } },
/* ���� */ { "SYU",  { XR_SI, XR_yu, } },
/* ���� */ { "SYE",  { XR_SI, XR__e, } },
/* ���� */ { "SYO",  { XR_SI, XR_yo, } },
		     
/* ���� */ { "TYA",  { XR_TI, XR_ya, } },
/* �Ă� */ { "TYI",  { XR_TE, XR__i, } },
/* ���� */ { "TYU",  { XR_TI, XR_yu, } },
/* ���� */ { "TYE",  { XR_TI, XR__e, } },
/* ���� */ { "TYO",  { XR_TI, XR_yo, } },
		     
/* ��   */ { "XYA",  { XR_ya, } },
/* ��   */ { "XYU",  { XR_yu, } },
/* ��   */ { "XYO",  { XR_yo, } },
		     
/* ���� */ { "ZYA",  { XR_SI, XR_jj, XR_ya, } },
/* ���� */ { "ZYU",  { XR_SI, XR_jj, XR_yu, } },
/* ���� */ { "ZYE",  { XR_SI, XR_jj, XR__e, } },
/* ���� */ { "ZYO",  { XR_SI, XR_jj, XR_yo, } },
		     
/* ���� */ { "CHA",  { XR_TI, XR_ya, } },
/* ��   */ { "CHI",  { XR_TI,        } },
/* ���� */ { "CHU",  { XR_TI, XR_yu, } },
/* ���� */ { "CHE",  { XR_TI, XR__e, } },
/* ���� */ { "CHO",  { XR_TI, XR_yo, } },

/* ���� */ { "SHA",  { XR_SI, XR_ya, } },
/* ��   */ { "SHI",  { XR_SI,        } },
/* ���� */ { "SHU",  { XR_SI, XR_yu, } },
/* ���� */ { "SHE",  { XR_SI, XR__e, } },
/* ���� */ { "SHO",  { XR_SI, XR_yo, } },
		     
/* �� */ { "TSA",  { XR_TU, XR__a, } },
/* �� */ { "TSI",  { XR_TU, XR__i, } },
/* ��   */ { "TSU",  { XR_TU,        } },
/* �� */ { "TSE",  { XR_TU, XR__e, } },
/* �� */ { "TSO",  { XR_TU, XR__o, } },
		     
/* �Ă� */ { "XTI",  { XR_TE, XR__i, } },
/* ��   */ { "XTU",  { XR_tu, } },
/* ��   */ { "XTSU", { XR_tu, } },
		     
/* ��   */ { "N'",   { XR__N, } },
		     
};


/*
MS-IME �� ���[�}���ϊ�

	��   ��   ��   ��   ��
B	��   ��   ��   ��   ��
C	��   ��   ��   ��   ��
D	��   ��   ��   ��   ��
F	�ӂ� �ӂ� ��   �ӂ� �ӂ�
G	��   ��   ��   ��   ��
H	��   ��   ��   ��   ��
J	���� ��   ���� ���� ����
K	��   ��   ��   ��   ��
L	��   ��   ��   ��   ��
M	��   ��   ��   ��   ��
N	��   ��   ��   ��   ��
P	��   ��   ��   ��   ��
Q	���� ���� ��   ���� ����
R	��   ��   ��   ��   ��
S	��   ��   ��   ��   ��
T	��   ��   ��   ��   ��
V	���� ���� ��   ���� ����
W	��   ���� ��   ���� ��
X	��   ��   ��   ��   ��
Y	��   ��   ��   ���� ��
Z	��   ��   ��   ��   ��

BY	�т� �т� �т� �т� �т�
CY	���� ���� ���� ���� ����
DY	���� ���� ���� ���� ����
FY	�ӂ� �ӂ� �ӂ� �ӂ� �ӂ�
GY	���� ���� ���� ���� ����
HY	�Ђ� �Ђ� �Ђ� �Ђ� �Ђ�
JY	���� ���� ���� ���� ����
KY	���� ���� ���� ���� ����
LY	��   ��   ��   ��   ��
MY	�݂� �݂� �݂� �݂� �݂�
NY	�ɂ� �ɂ� �ɂ� �ɂ� �ɂ�
PY	�҂� �҂� �҂� �҂� �҂�
QY	���� ���� ���� ���� ����
RY	��� �股 ��� �肥 ���
SY	���� ���� ���� ���� ����
TY	���� ���� ���� ���� ����
VY	���� ���� ���� ���� ����
WY
XY	��   ��   ��   ��   ��
ZY	���� ���� ���� ���� ����

CH	���� ��   ���� ���� ����
DH	�ł� �ł� �ł� �ł� �ł�
SH	���� ��   ���� ���� ����
TH	�Ă� �Ă� �Ă� �Ă� �Ă�
WH	���� ���� ��   ���� ����

TS	�� �� ��   �� ��

XK	��             ��
XT	          ��
XTS
LW	��
QW	���� ���� ���� ���� ����
LK	��             ��

NN	��
N'	��


ATOK�̃��[�}���ϊ�

	��   ��   ��  ��   ��
B	��   ��   ��  ��   ��
C	
D	��   ��   ��   ��   ��
F	�ӂ� �ӂ� ��   �ӂ� �ӂ�
G	��   ��   ��   ��   ��
H	��   ��   ��   ��   ��
J	���� ��   ���� ���� ����
K	��   ��   ��   ��   ��
L	��   ��   ��   ��   ��
M	��   ��   ��   ��   ��
N	��   ��   ��   ��   ��
P	��   ��   ��   ��   ��
Q	
R	��   ��   ��   ��   ��
S	��   ��   ��   ��   ��
T	��   ��   ��   ��   ��
V	���J�����J�����J���J�����J��
W	��   ���� ��   ���� ��
X	��   ��   ��   ��   ��
Y	��   ��   ��   ���� ��
Z	��   ��   ��   ��   ��

BY	�т� �т� �т� �т� �т�
CY	���� ���� ���� ���� ����
DY	���� ���� ���� ���� ����
FY	�ӂ� �ӂ� �ӂ� �ӂ� �ӂ�
GY	���� ���� ���� ���� ����
HY	�Ђ� �Ђ� �Ђ� �Ђ� �Ђ�
JY	���� ���� ���� ���� ����
KY	���� ���� ���� ���� ����
LY	��   ��   ��   ��   ��
MY	�݂� �݂� �݂� �݂� �݂�
NY	�ɂ� �ɂ� �ɂ� �ɂ� �ɂ�
PY	�҂� �҂� �҂� �҂� �҂�
QY	
RY	��� �股 ��� �肥 ���
SY	���� ���� ���� ���� ����
TY	���� ���� ���� ���� ����
VY	
WY	
XY	��   ��   ��   ��   ��
ZY	���� ���� ���� ���� ����

CH	���� ��   ���� ���� ����
DH	�ł� �ł� �ł� �ł� �ł�
SH	���� ��   ���� ���� ����
TH	�Ă� �Ă� �Ă� �Ă� �Ă�
WH	

TS	�� �� ��   �� ��

XK	��             ��
XT	          ��
XTS	          ��
LW	��
QW	
LK	��             ��

NN	��
N'	��


egg �� ���[�}���ϊ�

	��   ��   ��   ��   ��
B	��   ��   ��   ��   ��
C	
D	��   ��   ��   ��   ��
F	�ӂ� �ӂ� ��   �ӂ� �ӂ�
G	��   ��   ��   ��   ��
H	��   ��   ��   ��   ��
J	���� ��   ���� ���� ����
K	��   ��   ��   ��   ��
L	��   ��   ��   ��   ��
M	��   ��   ��   ��   ��
N	��   ��   ��   ��   ��
P	��   ��   ��   ��   ��
Q	
R	��   ��   ��   ��   ��
S	��   ��   ��   ��   ��
T	��   ��   ��   ��   ��
V	���� ���� ��   ���� ����
W	��   ��   ��   ��   ��
X	��   ��   ��   ��   ��
Y	��   ��   ��   ���� ��
Z	��   ��   ��   ��   ��

BY	�т�      �т� �т� �т�
CY	
DY	���� �ł� ���� ���� ����
FY	
GY	����      ���� ���� ����
HY	�Ђ�      �Ђ� �Ђ� �Ђ�
JY	����      ���� ���� ����
KY	����      ���� ���� ����
LY	���      ��� �肥 ���
MY	�݂�      �݂� �݂� �݂�
NY	�ɂ�      �ɂ� �ɂ� �ɂ�
PY	�҂�      �҂� �҂� �҂�
QY	
RY	���      ��� �肥 ���
SY	����      ���� ���� ����
TY	���� �Ă� ���� ���� ����
VY	
WY	
XY	��        ��        ��
ZY	����      ���� ���� ����

CH	���� ��   ���� ���� ����
DH	
SH	���� ��   ���� ���� ����
TH	
WH	

TS	�� �� ��   �� ��

XK	��             ��
XT	     �Ă� ��
XTS	          ��
LW	
QW	
LK	

N	��
N'	��
*/
