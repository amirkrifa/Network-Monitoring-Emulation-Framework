#include <stdio.h>
#include "GlobalEstimation.h"

void remplir_topologie()
{
int x = 0;
int y = 0;

for(x = 0; x < NOMBRE_AS; x++)
{
	for(y = 0; y < NOMBRE_AS; y++)
	{
		chemin[x][y].interface = NULL;
	}
}

}

void remplir_les_moniteurs()
{
char * m0;
char * m1;
char * m2;
char * m3;
char * m4;
char * m5;
char * m6;
char * m7;
char * m8;
char * m9;

char * m10;
char * m11;
char * m12;
char * m13;
char * m14;
char * m15;
char * m16;
char * m17;
char * m18;
char * m19;

char * m20;
char * m21;
char * m22;
char * m23;
char * m24;
char * m25;
char * m26;
char * m27;
char * m28;
char * m29;

char * m30;
char * m31;
char * m32;
char * m33;
char * m34;
char * m35;
char * m36;
char * m37;
char * m38;
char * m39;

char * m40;
char * m41;
char * m42;
char * m43;
char * m44;
char * m45;
char * m46;
char * m47;
char * m48;
char * m49;

char * m50;
char * m51;
char * m52;
char * m53;
char * m54;
char * m55;
char * m56;
char * m57;
char * m58;
char * m59;

char * m60;
char * m61;
char * m62;
char * m63;
char * m64;
char * m65;
char * m66;
char * m67;
char * m68;
char * m69;

char * m70;
char * m71;
char * m72;
char * m73;
char * m74;
char * m75;
char * m76;
char * m77;
char * m78;
char * m79;

char * m80;
char * m81;
char * m82;
char * m83;
char * m84;
char * m85;
char * m86;
char * m87;
char * m88;
char * m89;

char * m90;
char * m91;
char * m92;
char * m93;
char * m94;
char * m95;
char * m96;

les_moniteurs[0].sampling_rate = 0.01;
m0 = (char *)malloc(sizeof("255.255.255.255"));
strcpy(m0, "10.0.0.127");
les_moniteurs[0].adresse = m0;
les_moniteurs[0].les_observations = NULL;

les_moniteurs[1].sampling_rate = 0.01;
m1 = (char *)malloc(sizeof("255.255.255.255"));
strcpy(m1, "10.0.0.169");
les_moniteurs[1].adresse = m1;
les_moniteurs[1].les_observations = NULL;

les_moniteurs[2].sampling_rate = 0.01;
m2 = (char *)malloc(sizeof("255.255.255.255"));
strcpy(m2, "10.0.1.1");
les_moniteurs[2].adresse = m2;
les_moniteurs[2].les_observations = NULL;
////////////////////////////////////////////////////////////////////////////
les_moniteurs[3].sampling_rate = 0.01;
m3 = (char *)malloc(sizeof("255.255.255.255"));
strcpy(m3, "10.0.0.138");
les_moniteurs[3].adresse = m3;
les_moniteurs[3].les_observations = NULL;

les_moniteurs[4].sampling_rate = 0.01;
m4 = (char *)malloc(sizeof("255.255.255.255"));
strcpy(m4, "10.0.0.126");
les_moniteurs[4].adresse = m4;
les_moniteurs[4].les_observations = NULL;

les_moniteurs[5].sampling_rate = 0.01;
m5 = (char *)malloc(sizeof("255.255.255.255"));
strcpy(m5, "10.0.0.182");
les_moniteurs[5].adresse = m5;
les_moniteurs[5].les_observations = NULL;

les_moniteurs[6].sampling_rate = 0.01;
m6 = (char *)malloc(sizeof("255.255.255.255"));
strcpy(m6, "10.0.0.90");
les_moniteurs[6].adresse = m6;
les_moniteurs[6].les_observations = NULL;

les_moniteurs[7].sampling_rate = 0.01;
m7 = (char *)malloc(sizeof("255.255.255.255"));
strcpy(m7, "10.0.0.69");
les_moniteurs[7].adresse = m7;
les_moniteurs[7].les_observations = NULL;

les_moniteurs[8].sampling_rate = 0.01;
m8 = (char *)malloc(sizeof("255.255.255.255"));
strcpy(m8, "10.0.0.170");
les_moniteurs[8].adresse = m8;
les_moniteurs[8].les_observations = NULL;

les_moniteurs[9].sampling_rate = 0.01;
m9 = (char *)malloc(sizeof("255.255.255.255"));
strcpy(m9, "10.0.2.1");
les_moniteurs[9].adresse = m9;
les_moniteurs[9].les_observations = NULL;
//////////////////////////////////////////////////////////////////////////////////////
les_moniteurs[10].sampling_rate = 0.01;
m10 = (char *)malloc(sizeof("255.255.255.255"));
strcpy(m10, "10.0.0.166");
les_moniteurs[10].adresse = m10;
les_moniteurs[10].les_observations = NULL;


les_moniteurs[11].sampling_rate = 0.01;
m11 = (char *)malloc(sizeof("255.255.255.255"));
strcpy(m11, "10.0.0.137");
les_moniteurs[11].adresse = m11;
les_moniteurs[11].les_observations = NULL;

les_moniteurs[12].sampling_rate = 0.01;
m12 = (char *)malloc(sizeof("255.255.255.255"));
strcpy(m12, "10.0.3.1");
les_moniteurs[12].adresse = m12;
les_moniteurs[12].les_observations = NULL;

//////////////////////////////////////////////////////////////////////////////////////////

les_moniteurs[13].sampling_rate = 0.01;
m13 = (char *)malloc(sizeof("255.255.255.255"));
strcpy(m13, "10.0.0.102");
les_moniteurs[13].adresse = m13;
les_moniteurs[13].les_observations = NULL;

les_moniteurs[14].sampling_rate = 0.01;
m14 = (char *)malloc(sizeof("255.255.255.255"));
strcpy(m14, "10.0.0.22");
les_moniteurs[14].adresse = m14;
les_moniteurs[14].les_observations = NULL;

les_moniteurs[15].sampling_rate = 0.01;
m15 = (char *)malloc(sizeof("255.255.255.255"));
strcpy(m15, "10.0.0.117");
les_moniteurs[15].adresse = m15;
les_moniteurs[15].les_observations = NULL;

les_moniteurs[16].sampling_rate = 0.01;
m16 = (char *)malloc(sizeof("255.255.255.255"));
strcpy(m16, "10.0.0.181");
les_moniteurs[16].adresse = m16;
les_moniteurs[16].les_observations = NULL;

les_moniteurs[17].sampling_rate = 0.01;
m17 = (char *)malloc(sizeof("255.255.255.255"));
strcpy(m17, "10.0.4.1");
les_moniteurs[17].adresse = m17;
les_moniteurs[17].les_observations = NULL;

///////////////////////////////////////////////////////////////////////////////////////////////

les_moniteurs[18].sampling_rate = 0.01;
m18 = (char *)malloc(sizeof("255.255.255.255"));
strcpy(m18, "10.0.0.25");
les_moniteurs[18].adresse = m18;
les_moniteurs[18].les_observations = NULL;

les_moniteurs[19].sampling_rate = 0.01;
m19 = (char *)malloc(sizeof("255.255.255.255"));
strcpy(m19, "10.0.0.17");
les_moniteurs[19].adresse = m19;
les_moniteurs[19].les_observations = NULL;
////////
les_moniteurs[20].sampling_rate = 0.01;
m20 = (char *)malloc(sizeof("255.255.255.255"));
strcpy(m20, "10.0.0.21");
les_moniteurs[20].adresse = m20;
les_moniteurs[20].les_observations = NULL;

les_moniteurs[21].sampling_rate = 0.01;
m21 = (char *)malloc(sizeof("255.255.255.255"));
strcpy(m21, "10.0.5.1");
les_moniteurs[21].adresse = m21;
les_moniteurs[21].les_observations = NULL;

///////////////////////////////////////////////////////////////////////////////////////////////


les_moniteurs[22].sampling_rate = 0.01;
m22 = (char *)malloc(sizeof("255.255.255.255"));
strcpy(m22, "10.0.0.89");
les_moniteurs[22].adresse = m22;
les_moniteurs[22].les_observations = NULL;

les_moniteurs[23].sampling_rate = 0.01;
m23 = (char *)malloc(sizeof("255.255.255.255"));
strcpy(m23, "10.0.0.18");
les_moniteurs[23].adresse = m23;
les_moniteurs[23].les_observations = NULL;

les_moniteurs[24].sampling_rate = 0.01;
m24 = (char *)malloc(sizeof("255.255.255.255"));
strcpy(m24, "10.0.0.85");
les_moniteurs[24].adresse = m24;
les_moniteurs[24].les_observations = NULL;

les_moniteurs[25].sampling_rate = 0.01;
m25 = (char *)malloc(sizeof("255.255.255.255"));
strcpy(m25, "10.0.0.50");
les_moniteurs[25].adresse = m25;
les_moniteurs[25].les_observations = NULL;

les_moniteurs[26].sampling_rate = 0.01;
m26 = (char *)malloc(sizeof("255.255.255.255"));
strcpy(m26, "10.0.0.10");
les_moniteurs[26].adresse = m26;
les_moniteurs[26].les_observations = NULL;

les_moniteurs[27].sampling_rate = 0.01;
m27 = (char *)malloc(sizeof("255.255.255.255"));
strcpy(m27, "10.0.0.30");
les_moniteurs[27].adresse = m27;
les_moniteurs[27].les_observations = NULL;

les_moniteurs[28].sampling_rate = 0.01;
m28 = (char *)malloc(sizeof("255.255.255.255"));
strcpy(m28, "10.0.6.1");
les_moniteurs[28].adresse = m28;
les_moniteurs[28].les_observations = NULL;

/////////////////////////////////////////////////////////////////////////////

les_moniteurs[29].sampling_rate = 0.01;
m29 = (char *)malloc(sizeof("255.255.255.255"));
strcpy(m29, "10.0.0.29");
les_moniteurs[29].adresse = m29;
les_moniteurs[29].les_observations = NULL;

les_moniteurs[30].sampling_rate = 0.01;
m30 = (char *)malloc(sizeof("255.255.255.255"));
strcpy(m30, "10.0.0.33");
les_moniteurs[30].adresse = m30;
les_moniteurs[30].les_observations = NULL;

les_moniteurs[31].sampling_rate = 0.01;
m31 = (char *)malloc(sizeof("255.255.255.255"));
strcpy(m31, "10.0.0.2");
les_moniteurs[31].adresse = m31;
les_moniteurs[31].les_observations = NULL;

les_moniteurs[32].sampling_rate = 0.01;
m32 = (char *)malloc(sizeof("255.255.255.255"));
strcpy(m32, "10.0.7.1");
les_moniteurs[32].adresse = m32;
les_moniteurs[32].les_observations = NULL;

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

les_moniteurs[33].sampling_rate = 0.01;
m33 = (char *)malloc(sizeof("255.255.255.255"));
strcpy(m33, "10.0.0.70");
les_moniteurs[33].adresse = m33;
les_moniteurs[33].les_observations = NULL;

les_moniteurs[34].sampling_rate = 0.01;
m34 = (char *)malloc(sizeof("255.255.255.255"));
strcpy(m34, "10.0.0.78");
les_moniteurs[34].adresse = m34;
les_moniteurs[34].les_observations = NULL;

les_moniteurs[35].sampling_rate = 0.01;
m35 = (char *)malloc(sizeof("255.255.255.255"));
strcpy(m35, "10.0.8.1");
les_moniteurs[35].adresse = m35;
les_moniteurs[35].les_observations = NULL;

///////////////////////////////////////////////////////////////
les_moniteurs[36].sampling_rate = 0.01;
m36 = (char *)malloc(sizeof("255.255.255.255"));
strcpy(m36, "10.0.0.26");
les_moniteurs[36].adresse = m36;
les_moniteurs[36].les_observations = NULL;

les_moniteurs[37].sampling_rate = 0.01;
m37 = (char *)malloc(sizeof("255.255.255.255"));
strcpy(m37, "10.0.0.86");
les_moniteurs[37].adresse = m37;
les_moniteurs[37].les_observations = NULL;

les_moniteurs[38].sampling_rate = 0.01;
m38 = (char *)malloc(sizeof("255.255.255.255"));
strcpy(m38, "10.0.9.1");
les_moniteurs[38].adresse = m38;
les_moniteurs[38].les_observations = NULL;

/////////////////////////////////////////////////////////////////////////////////////////
les_moniteurs[39].sampling_rate = 0.01;
m39 = (char *)malloc(sizeof("255.255.255.255"));
strcpy(m39, "10.0.0.185");
les_moniteurs[39].adresse = m39;
les_moniteurs[39].les_observations = NULL;

les_moniteurs[40].sampling_rate = 0.01;
m40 = (char *)malloc(sizeof("255.255.255.255"));
strcpy(m40, "10.0.0.77");
les_moniteurs[40].adresse = m40;
les_moniteurs[40].les_observations = NULL;

les_moniteurs[41].sampling_rate = 0.01;
m41 = (char *)malloc(sizeof("255.255.255.255"));
strcpy(m41, "10.0.0.9");
les_moniteurs[41].adresse = m41;
les_moniteurs[41].les_observations = NULL;

les_moniteurs[42].sampling_rate = 0.01;
m42 = (char *)malloc(sizeof("255.255.255.255"));
strcpy(m42, "10.0.10.1");
les_moniteurs[42].adresse = m42;
les_moniteurs[42].les_observations = NULL;

////////////////////////////////////////////////////////////////////////////////////////////
les_moniteurs[43].sampling_rate = 0.01;
m43 = (char *)malloc(sizeof("255.255.255.255"));
strcpy(m43, "10.0.0.101");
les_moniteurs[43].adresse = m43;
les_moniteurs[43].les_observations = NULL;

les_moniteurs[44].sampling_rate = 0.01;
m44 = (char *)malloc(sizeof("255.255.255.255"));
strcpy(m44, "10.0.0.165");
les_moniteurs[44].adresse = m44;
les_moniteurs[44].les_observations = NULL;

les_moniteurs[45].sampling_rate = 0.01;
m45 = (char *)malloc(sizeof("255.255.255.255"));
strcpy(m45, "10.0.0.38");
les_moniteurs[45].adresse = m45;
les_moniteurs[45].les_observations = NULL;

les_moniteurs[46].sampling_rate = 0.01;
m46 = (char *)malloc(sizeof("255.255.255.255"));
strcpy(m46, "10.0.0.129");
les_moniteurs[46].adresse = m46;
les_moniteurs[46].les_observations = NULL;

les_moniteurs[47].sampling_rate = 0.01;
m47 = (char *)malloc(sizeof("255.255.255.255"));
strcpy(m47, "10.0.0.49");
les_moniteurs[47].adresse = m47;
les_moniteurs[47].les_observations = NULL;

les_moniteurs[48].sampling_rate = 0.01;
m48 = (char *)malloc(sizeof("255.255.255.255"));
strcpy(m48, "10.0.11.1");
les_moniteurs[48].adresse = m48;
les_moniteurs[48].les_observations = NULL;
/////////////////////////////////////////////////////////////////////////////////////////
les_moniteurs[49].sampling_rate = 0.01;
m49 = (char *)malloc(sizeof("255.255.255.255"));
strcpy(m49, "10.0.0.113");
les_moniteurs[49].adresse = m49;
les_moniteurs[49].les_observations = NULL;

les_moniteurs[50].sampling_rate = 0.01;
m50 = (char *)malloc(sizeof("255.255.255.255"));
strcpy(m50, "10.0.0.66");
les_moniteurs[50].adresse = m50;
les_moniteurs[50].les_observations = NULL;

les_moniteurs[51].sampling_rate = 0.01;
m51 = (char *)malloc(sizeof("255.255.255.255"));
strcpy(m51, "10.0.0.125");
les_moniteurs[51].adresse = m51;
les_moniteurs[51].les_observations = NULL;

les_moniteurs[52].sampling_rate = 0.01;
m52 = (char *)malloc(sizeof("255.255.255.255"));
strcpy(m52, "10.0.12.1");
les_moniteurs[52].adresse = m52;
les_moniteurs[52].les_observations = NULL;
////////////////////////////////////////////////////////////////////////////////////////////////

les_moniteurs[53].sampling_rate = 0.01;
m53 = (char *)malloc(sizeof("255.255.255.255"));
strcpy(m53, "10.0.0.65");
les_moniteurs[53].adresse = m53;
les_moniteurs[53].les_observations = NULL;

les_moniteurs[54].sampling_rate = 0.01;
m54 = (char *)malloc(sizeof("255.255.255.255"));
strcpy(m54, "10.0.0.58");
les_moniteurs[54].adresse = m54;
les_moniteurs[54].les_observations = NULL;

les_moniteurs[55].sampling_rate = 0.01;
m55 = (char *)malloc(sizeof("255.255.255.255"));
strcpy(m55, "10.0.0.82");
les_moniteurs[55].adresse = m55;
les_moniteurs[55].les_observations = NULL;

les_moniteurs[56].sampling_rate = 0.01;
m56 = (char *)malloc(sizeof("255.255.255.255"));
strcpy(m56, "10.0.0.61");
les_moniteurs[56].adresse = m56;
les_moniteurs[56].les_observations = NULL;

les_moniteurs[57].sampling_rate = 0.01;
m57 = (char *)malloc(sizeof("255.255.255.255"));
strcpy(m57, "10.0.0.130");
les_moniteurs[57].adresse = m57;
les_moniteurs[57].les_observations = NULL;

les_moniteurs[58].sampling_rate = 0.01;
m58 = (char *)malloc(sizeof("255.255.255.255"));
strcpy(m58, "10.0.13.1");
les_moniteurs[58].adresse = m58;
les_moniteurs[58].les_observations = NULL;

//////////////////////////////////////////////////////////////////////////////////////////

les_moniteurs[59].sampling_rate = 0.01;
m59 = (char *)malloc(sizeof("255.255.255.255"));
strcpy(m59, "10.0.0.186");
les_moniteurs[59].adresse = m59;
les_moniteurs[59].les_observations = NULL;

les_moniteurs[60].sampling_rate = 0.01;
m60 = (char *)malloc(sizeof("255.255.255.255"));
strcpy(m60, "10.0.0.62");
les_moniteurs[60].adresse = m60;
les_moniteurs[60].les_observations = NULL;

les_moniteurs[61].sampling_rate = 0.01;
m61 = (char *)malloc(sizeof("255.255.255.255"));
strcpy(m61, "10.0.0.154");
les_moniteurs[61].adresse = m61;
les_moniteurs[61].les_observations = NULL;

les_moniteurs[62].sampling_rate = 0.01;
m62 = (char *)malloc(sizeof("255.255.255.255"));
strcpy(m62, "10.0.0.142");
les_moniteurs[62].adresse = m62;
les_moniteurs[62].les_observations = NULL;

les_moniteurs[63].sampling_rate = 0.01;
m63 = (char *)malloc(sizeof("255.255.255.255"));
strcpy(m63, "10.0.0.34");
les_moniteurs[63].adresse = m63;
les_moniteurs[63].les_observations = NULL;

les_moniteurs[64].sampling_rate = 0.01;
m64 = (char *)malloc(sizeof("255.255.255.255"));
strcpy(m64, "10.0.14.1");
les_moniteurs[64].adresse = m64;
les_moniteurs[64].les_observations = NULL;
///////////////////////////////////////////////////////////////////////////////////////////////////

les_moniteurs[65].sampling_rate = 0.01;
m65 = (char *)malloc(sizeof("255.255.255.255"));
strcpy(m65, "10.0.0.177");
les_moniteurs[65].adresse = m65;
les_moniteurs[65].les_observations = NULL;

les_moniteurs[66].sampling_rate = 0.01;
m66 = (char *)malloc(sizeof("255.255.255.255"));
strcpy(m66, "10.0.0.13");
les_moniteurs[66].adresse = m66;
les_moniteurs[66].les_observations = NULL;

les_moniteurs[67].sampling_rate = 0.01;
m67 = (char *)malloc(sizeof("255.255.255.255"));
strcpy(m67, "10.0.0.110");
les_moniteurs[67].adresse = m67;
les_moniteurs[67].les_observations = NULL;

les_moniteurs[68].sampling_rate = 0.01;
m68 = (char *)malloc(sizeof("255.255.255.255"));
strcpy(m68, "10.0.0.1");
les_moniteurs[68].adresse = m68;
les_moniteurs[68].les_observations = NULL;

les_moniteurs[69].sampling_rate = 0.01;
m69 = (char *)malloc(sizeof("255.255.255.255"));
strcpy(m69, "10.0.0.57");
les_moniteurs[69].adresse = m69;
les_moniteurs[69].les_observations = NULL;

les_moniteurs[70].sampling_rate = 0.01;
m70 = (char *)malloc(sizeof("255.255.255.255"));
strcpy(m70, "10.0.15.1");
les_moniteurs[70].adresse = m70;
les_moniteurs[70].les_observations = NULL;

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
les_moniteurs[71].sampling_rate = 0.01;
m71 = (char *)malloc(sizeof("255.255.255.255"));
strcpy(m71, "10.0.0.46");
les_moniteurs[71].adresse = m71;
les_moniteurs[71].les_observations = NULL;

les_moniteurs[72].sampling_rate = 0.01;
m72 = (char *)malloc(sizeof("255.255.255.255"));
strcpy(m72, "10.0.0.114");
les_moniteurs[72].adresse = m72;
les_moniteurs[72].les_observations = NULL;

les_moniteurs[73].sampling_rate = 0.01;
m73 = (char *)malloc(sizeof("255.255.255.255"));
strcpy(m73, "10.0.16.1");
les_moniteurs[73].adresse = m73;
les_moniteurs[73].les_observations = NULL;

//////////////////////////////////////////////////////////////////////////////////////
les_moniteurs[74].sampling_rate = 0.01;
m74 = (char *)malloc(sizeof("255.255.255.255"));
strcpy(m74, "10.0.0.45");
les_moniteurs[74].adresse = m74;
les_moniteurs[74].les_observations = NULL;

les_moniteurs[75].sampling_rate = 0.01;
m75 = (char *)malloc(sizeof("255.255.255.255"));
strcpy(m75, "10.0.0.37");
les_moniteurs[75].adresse = m75;
les_moniteurs[75].les_observations = NULL;

les_moniteurs[76].sampling_rate = 0.01;
m76 = (char *)malloc(sizeof("255.255.255.255"));
strcpy(m76, "10.0.0.41");
les_moniteurs[76].adresse = m76;
les_moniteurs[76].les_observations = NULL;

les_moniteurs[77].sampling_rate = 0.01;
m77 = (char *)malloc(sizeof("255.255.255.255"));
strcpy(m77, "10.0.17.1");
les_moniteurs[77].adresse = m77;
les_moniteurs[77].les_observations = NULL;
///////////////////////////////////////////////////////////////////////////////////////////
les_moniteurs[78].sampling_rate = 0.01;
m78 = (char *)malloc(sizeof("255.255.255.255"));
strcpy(m78, "10.0.0.42");
les_moniteurs[78].adresse = m78;
les_moniteurs[78].les_observations = NULL;

les_moniteurs[79].sampling_rate = 0.01;
m79 = (char *)malloc(sizeof("255.255.255.255"));
strcpy(m79, "10.0.0.106");
les_moniteurs[79].adresse = m79;
les_moniteurs[79].les_observations = NULL;

les_moniteurs[80].sampling_rate = 0.01;
m80 = (char *)malloc(sizeof("255.255.255.255"));
strcpy(m80, "10.0.18.1");
les_moniteurs[80].adresse = m80;
les_moniteurs[80].les_observations = NULL;

////////////////////////////////////////////////////////////////////////////////////////////////
les_moniteurs[81].sampling_rate = 0.01;
m81 = (char *)malloc(sizeof("255.255.255.255"));
strcpy(m81, "10.0.0.105");
les_moniteurs[81].adresse = m81;
les_moniteurs[81].les_observations = NULL;

les_moniteurs[82].sampling_rate = 0.01;
m82 = (char *)malloc(sizeof("255.255.255.255"));
strcpy(m82, "10.0.0.146");
les_moniteurs[82].adresse = m82;
les_moniteurs[82].les_observations = NULL;

les_moniteurs[83].sampling_rate = 0.01;
m83 = (char *)malloc(sizeof("255.255.255.255"));
strcpy(m83, "10.0.0.178");
les_moniteurs[83].adresse = m83;
les_moniteurs[83].les_observations = NULL;

les_moniteurs[84].sampling_rate = 0.01;
m84 = (char *)malloc(sizeof("255.255.255.255"));
strcpy(m84, "10.0.19.1");
les_moniteurs[84].adresse = m84;
les_moniteurs[84].les_observations = NULL;

/////////////////////////////////////////////////////////////////////////////////

les_moniteurs[85].sampling_rate = 0.01;
m85 = (char *)malloc(sizeof("255.255.255.255"));
strcpy(m85, "10.0.0.145");
les_moniteurs[85].adresse = m85;
les_moniteurs[85].les_observations = NULL;

les_moniteurs[86].sampling_rate = 0.01;
m86 = (char *)malloc(sizeof("255.255.255.255"));
strcpy(m86, "10.0.0.73");
les_moniteurs[86].adresse = m86;
les_moniteurs[86].les_observations = NULL;

les_moniteurs[87].sampling_rate = 0.01;
m87 = (char *)malloc(sizeof("255.255.255.255"));
strcpy(m87, "10.0.20.1");
les_moniteurs[87].adresse = m87;
les_moniteurs[87].les_observations = NULL;

///////////////////////////////////////////////////////////////////////////

les_moniteurs[88].sampling_rate = 0.01;
m88 = (char *)malloc(sizeof("255.255.255.255"));
strcpy(m88, "10.0.0.74");
les_moniteurs[88].adresse = m88;
les_moniteurs[88].les_observations = NULL;

les_moniteurs[89].sampling_rate = 0.01;
m89 = (char *)malloc(sizeof("255.255.255.255"));
strcpy(m89, "10.0.0.14");
les_moniteurs[89].adresse = m89;
les_moniteurs[89].les_observations = NULL;

les_moniteurs[90].sampling_rate = 0.01;
m90 = (char *)malloc(sizeof("255.255.255.255"));
strcpy(m90, "10.0.21.1");
les_moniteurs[90].adresse = m90;
les_moniteurs[90].les_observations = NULL;

////////////////////////////////////////////////////////////////////////////////////////////

les_moniteurs[91].sampling_rate = 0.01;
m91 = (char *)malloc(sizeof("255.255.255.255"));
strcpy(m91, "10.0.0.141");
les_moniteurs[91].adresse = m91;
les_moniteurs[91].les_observations = NULL;

les_moniteurs[92].sampling_rate = 0.01;
m92 = (char *)malloc(sizeof("255.255.255.255"));
strcpy(m92, "10.0.0.81");
les_moniteurs[92].adresse = m92;
les_moniteurs[92].les_observations = NULL;

les_moniteurs[93].sampling_rate = 0.01;
m93 = (char *)malloc(sizeof("255.255.255.255"));
strcpy(m93, "10.0.22.1");
les_moniteurs[93].adresse = m93;
les_moniteurs[93].les_observations = NULL;

//////////////////////////////////////////////////////////////////////////////////////

les_moniteurs[94].sampling_rate = 0.01;
m94 = (char *)malloc(sizeof("255.255.255.255"));
strcpy(m94, "10.0.0.153");
les_moniteurs[94].adresse = m94;
les_moniteurs[94].les_observations = NULL;

les_moniteurs[95].sampling_rate = 0.01;
m95 = (char *)malloc(sizeof("255.255.255.255"));
strcpy(m95, "10.0.0.118");
les_moniteurs[95].adresse = m95;
les_moniteurs[95].les_observations = NULL;

les_moniteurs[96].sampling_rate = 0.01;
m96 = (char *)malloc(sizeof("255.255.255.255"));
strcpy(m96, "10.0.23.1");
les_moniteurs[96].adresse = m96;
les_moniteurs[96].les_observations = NULL;
}
