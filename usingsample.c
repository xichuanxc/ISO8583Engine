#include<stdio.h>
#include<string.h>

#include "ISO8583Engine.h"

//This is an ISO8583 field type sample, you should follow standard of your specific project
const ISO8583_FieldFormat SampleFldFmt[ISO8583_MAXFIELD] =
{
	{ISO8583TYPE_BIN,                        64},    //  1
	{ISO8583TYPE_BCD | ISO8583TYPE_VAR,      19},    //  2 PAN
	{ISO8583TYPE_BCD,                        6},     //  3 Processing Code
	{ISO8583TYPE_BCD,                        12},    //  4 Amount
	{ISO8583TYPE_BCD,                        12},    //  5
	{ISO8583TYPE_BCD,                        12},    //  6
	{ISO8583TYPE_BCD,                        10},    //  7
	{ISO8583TYPE_ASC,                        1},     //  8
	{ISO8583TYPE_BCD,                        8},     //  9
	{ISO8583TYPE_BCD,                        8},     // 10
	{ISO8583TYPE_BCD,                        6},     // 11 System trace
	{ISO8583TYPE_BCD,                        6},     // 12 Time
	{ISO8583TYPE_BCD,                        4},     // 13 Date
	{ISO8583TYPE_BCD,                        4},     // 14 ExpDate
	{ISO8583TYPE_BCD,                        4},     // 15 Settlement date
	{ISO8583TYPE_ASC,                        1},     // 16
	{ISO8583TYPE_BCD,                        4},     // 17
	{ISO8583TYPE_BCD,                        5},     // 18
	{ISO8583TYPE_BCD,                        3},     // 19
	{ISO8583TYPE_BCD,                        3},     // 20
	{ISO8583TYPE_ASC,                        7},     // 21
	{ISO8583TYPE_BCD,                        3},     // 22 POS entry mode
	{ISO8583TYPE_BCD,                        3},     // 23 IC Application PAN
	{ISO8583TYPE_ASC,                        2},     // 24 NII
	{ISO8583TYPE_BCD,                        2},     // 25
	{ISO8583TYPE_BCD,                        2},     // 26
	{ISO8583TYPE_BCD,                        1},     // 27
	{ISO8583TYPE_BCD,                        8},     // 28
	{ISO8583TYPE_BCD,                        8},     // 29
	{ISO8583TYPE_BCD,                        8},     // 30
	{ISO8583TYPE_BCD,                        8},     // 31
	{ISO8583TYPE_BCD | ISO8583TYPE_VAR,      11},    // 32
	{ISO8583TYPE_BCD | ISO8583TYPE_VAR,      11},    // 33
	{ISO8583TYPE_BCD | ISO8583TYPE_VAR,      28},    // 34
	{ISO8583TYPE_BCD | ISO8583TYPE_VAR,      37},    // 35 Track2
	{ISO8583TYPE_BCD | ISO8583TYPE_VAR,      104},   // 36 Track3
	{ISO8583TYPE_ASC,                        12},    // 37 System Reference No
	{ISO8583TYPE_ASC,                        6},     // 38 System AuthID
	{ISO8583TYPE_ASC,                        2},     // 39 Response Code
	{ISO8583TYPE_ASC,                        3},     // 40
	{ISO8583TYPE_ASC,                        8},     // 41 TID
	{ISO8583TYPE_ASC,                        15},    // 42 CustomID
	{ISO8583TYPE_ASC,                        40},    // 43 Custom Name
	{ISO8583TYPE_ASC | ISO8583TYPE_VAR,      25},    // 44
	{ISO8583TYPE_ASC | ISO8583TYPE_VAR,      76},    // 45 Track1
	{ISO8583TYPE_ASC | ISO8583TYPE_VAR,      999},   // 46
	{ISO8583TYPE_ASC | ISO8583TYPE_VAR,      999},   // 47
	{ISO8583TYPE_BCD | ISO8583TYPE_VAR,      999},   // 48
	{ISO8583TYPE_ASC,                        3},     // 49 Currency Code  Transaction
	{ISO8583TYPE_ASC,                        3},     // 50
	{ISO8583TYPE_ASC,                        3},     // 51
	{ISO8583TYPE_BIN,                        64},    // 52 PIN block Data
	{ISO8583TYPE_BCD,                        16},    // 53 Security Data
	{ISO8583TYPE_ASC | ISO8583TYPE_VAR,      320},   // 54
	{ISO8583TYPE_ASC | ISO8583TYPE_VAR,      999},   // 55 ICC information
	{ISO8583TYPE_ASC | ISO8583TYPE_VAR,      999},   // 56
	{ISO8583TYPE_ASC | ISO8583TYPE_VAR,      999},   // 57
	{ISO8583TYPE_ASC | ISO8583TYPE_VAR,      999},   // 58
	{ISO8583TYPE_ASC | ISO8583TYPE_VAR,      999},   // 59
	{ISO8583TYPE_BCD | ISO8583TYPE_VAR,      999},   // 60 Additional Data
	{ISO8583TYPE_BCD | ISO8583TYPE_VAR,      999},   // 61 Additional Data
	{ISO8583TYPE_ASC | ISO8583TYPE_VAR,      999},   // 62 Additional Data
	{ISO8583TYPE_ASC | ISO8583TYPE_VAR,      999},   // 63 Additional Data
	{ISO8583TYPE_BIN,                        64},    // 64 MAC data
};

int main(int argc, char **argv)
{
    ISO8583_Rec RequestIso8583;
    int iReqLen = 0;
    unsigned char ReqHexBuf[1024];
    char OutputBuf[2048];

	//Initiate structure RequestIso8583
	ISO8583Engine_ClearAllFields( &RequestIso8583 );

	//Initiate field format
	ISO8583Engine_InitFieldFormat( ISO8583_BITMAP64, &SampleFldFmt[0] );


	// Field 0 - Message ID
	ISO8583Engine_SetField( &RequestIso8583, 0, "0800", 4 );

	// Field 4 - Amount
	ISO8583Engine_SetField( &RequestIso8583, 4, "000000000293", 12 );

	// Field 11
	ISO8583Engine_SetField( &RequestIso8583, 11, "000137", 6 );

	// Field 41 - Terminal ID
	ISO8583Engine_SetField( &RequestIso8583, 41, "12345678", 8 );

	// Field 42 - Merchant ID
	ISO8583Engine_SetField( &RequestIso8583, 42, "998877665508642", 15 );

	// Field 60 - Transaction type code + BatchNum + EncryptType
	ISO8583Engine_SetField( &RequestIso8583, 60, "00190812003", 11);

	// field 63 - Operator ID
	ISO8583Engine_SetField( &RequestIso8583, 63, "001", 3 );

	memset(ReqHexBuf, 0, sizeof(ReqHexBuf));
    iReqLen = ISO8583Engine_Iso8583ToHexbuf( &RequestIso8583, ReqHexBuf, sizeof(ReqHexBuf) );
	if( iReqLen <= 0 )
		return -1;

    memset(OutputBuf, 0, sizeof(OutputBuf));
    ISO8583Utils_BCD2ASC(ReqHexBuf, OutputBuf, iReqLen*2);

    printf("ISO8583 Hex Buf:%s\n", OutputBuf);
}

