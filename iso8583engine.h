/***************************************************************************
* FILE NAME:    ISO8583Engine.H                                            *
* MODULE NAME:  ISO8583Engine                                              *
* PROGRAMMER:                                                              *
* DESCRIPTION:                                                             *
* REVISION:                                                                *
****************************************************************************/

#ifndef _ISO8583ENGINE_H
#define _ISO8583ENGINE_H

//Return values enum
typedef enum
{
    ISOENGINE_OK = 0,
    ISOENGINE_NOT_SET_FIELD_FMT = -100,
    ISOENGINE_INVALID_FIELD_NO,
    ISOENGINE_TOO_LONG_FILED_LENGTH,
    ISOENGINE_INVALID_FIELD_LENGTH,
    ISOENGINE_OVER_MAXLENGTH,
    ISOENGINE_INVALID_FIELD_DATA,
    ISOENGINE_TOO_SMALL_FIELD_BUF_SIZE,
} ISO8583_ENGINE_RetVal;

//Maximum field number: default 64, determined by Application Message format standard
#define ISO8583_MAXFIELD        64

#ifndef byte
typedef unsigned char byte;
#endif

//Maximum length of ISO8583 data
#define ISO8583_MAXLENTH        1024

#define ISO8583TYPE_FIX         0x01    // type fix length
#define ISO8583TYPE_VAR         0x02    // type Variable length - 99/999
#define ISO8583TYPE_BIN         0x04    // type Binary  - 'b','h'
#define ISO8583TYPE_ASC         0x08    // type ASCII   - 'a','an','ans'
#define ISO8583TYPE_BCD         0x10    // type BCD     - 'n','z'
#define ISO8583TYPE_DIGIT       0x20    // type Digit   - '0'~'9'

//BITMAP type 64 / 128
typedef enum
{
    ISO8583_BITMAP64 = 0,
    ISO8583_BITMAP128,
} ISO8583_BitMode;


//ISO8583 field format struct
typedef struct
{
    unsigned char bType;         // (Fix or Var) | (BIN or ASC or BCD)
    int iMaxLength;     // data max length
} ISO8583_FieldFormat;

typedef struct
{
    short bitf;
    short len;
    int addr;
} ISO8583_ElementFlag;

typedef struct
{
    int iOffset;
    unsigned char cData[ ISO8583_MAXLENTH ];
    unsigned char cMsgID[ 5 ];
    ISO8583_ElementFlag Field[ ISO8583_MAXFIELD ];
} ISO8583_Rec;


/* -----------------------------------------------------------------------------
 * FUNCTION NAME:   ISO8583Engine_InitFieldFormat
 * DESCRIPTION:     Set ISO8583 field type and format, should be called
 *                  before the using of ISO8583 engine module
 * PARAMETERS:      bBitMode: Iso8583 bitmap mode, see enum ISO8583_BitMode in ISO8583Engine.h
 *                  pFieldFormat: poFieldFormat definitions
 * RETURN:          None
 ---------------------------------------------------------------------------- */
int ISO8583Engine_InitFieldFormat( ISO8583_BitMode bBitMode, ISO8583_FieldFormat *pIso8583FieldFormat );

/* -----------------------------------------------------------------------------
 * FUNCTION NAME:   ISO8583Engine_ClearAllFields
 * DESCRIPTION:     Clear all field data in ISO8583_Rec structure
 * PARAMETERS:      ptIso8583Data: ISO8583 data structure
 * RETURN:          None.
 ---------------------------------------------------------------------------- */
int ISO8583Engine_ClearAllFields( ISO8583_Rec * ptIso8583Data );

/* -----------------------------------------------------------------------------
 * FUNCTION NAME:   ISO8583Engine_ClearOneField
 * DESCRIPTION:     Clear one field data
 * PARAMETERS:      ptIso8583Data: pointer to ISO8583 data structure
 *                  iFieldNo: Field No
 * RETURN:          None
 ---------------------------------------------------------------------------- */
int ISO8583Engine_ClearOneField( ISO8583_Rec * ptIso8583Data, int iFieldNo );

/* -----------------------------------------------------------------------------
 * FUNCTION NAME:   ISO8583Engine_SetField
 * DESCRIPTION:     Set ISO8583 field data
                    pFieldData must be the ASC format
 * return:          if successful, return 0; else
 *                  -1: not set iso8583 field format
 *                  -2: iFieldNo > APPISO8583_MAXFIELD or iFieldNo <= 1
 *                  -3: iDataLength > 999
 *                  -4: iso8583 string total length already > ISO8583_MAXLENTH
 ---------------------------------------------------------------------------- */
int ISO8583Engine_SetField(ISO8583_Rec * pIsoRec, int iFieldNo, unsigned char * pFieldData, int iDataLength);

/* -----------------------------------------------------------------------------
 * FUNCTION NAME:   ISO8583Engine_GetField
 * DESCRIPTION:     Get ISO8583 field data
                    pRetFieldData must be the ASC format
 * return:          Return FieldData length has gotten
 ---------------------------------------------------------------------------- */
int ISO8583Engine_GetField(ISO8583_Rec * cpIsoRec, int iFieldNo, unsigned char * pRetFieldData, int iSizeofRetFieldData);

/* -----------------------------------------------------------------------------
 * FUNCTION NAME:   ISO8583Engine_HexbufToIso8583
 * DESCRIPTION:     Convert ISO8583 RAW hex buffer data to ISO8583_Rec struct
 * PARAMETERS:      pIso8583Data(out): Converted Iso8583 data structure
 *                  pBuf(in): RAW iso8583 hex buf data
 * RETURN:          =0: success,
 *                  -1: variable field length error
 *                  -2: iFieldNo >= ISO8583_MAXFIELD or iFieldNo < 1
 *                  -3: iso8583 string total length already > ISO8583_MAXLENTH
 *                  -4: iso8583 string total length already > ISO8583_MAXLENTH
 ---------------------------------------------------------------------------- */
int ISO8583Engine_HexbufToIso8583( ISO8583_Rec * pIso8583Data, unsigned char * pBuf );

/* -----------------------------------------------------------------------------
 * FUNCTION NAME:   ISO8583Engine_Iso8583ToHexbuf
 * DESCRIPTION:     Convert ISO8583_Rec struct to Hex buffer - RAW ISO8583 data
 * PARAMETERS:      pIso8583Data: Iso8583 data structure
 *                  pRetBuf: RAW iso8583 hex buf data
 *                  iSizeRetBuf: size of pRetBuf
 * RETURN:          >0: success,
 *                  -1: variable field length error
 *                  -2: iFieldNo >= ISO8583_MAXFIELD or iFieldNo < 1
 *                  -3: iso8583 string total length already > iSizeRetBuf
 *                  -4: iso8583 string total length already > ISO8583_MAXLENTH
 ---------------------------------------------------------------------------- */
int ISO8583Engine_Iso8583ToHexbuf( ISO8583_Rec * pIso8583Data, unsigned char * pRetBuf, int iSizeRetBuf );

/* --------------------------------------------------------------------------
* FUNCTION NAME: ISO8583Utils_BCD2ASC
* DESCRIPTION:   Convert BCD code to ASCII code.
* PARAMETERS:    BcdBuf - BCD input buffer, Len - double length of Bcdbuf bytes
*                AscBuf - converted result
* RETURN:        0
* NOTES:
* ------------------------------------------------------------------------ */
int ISO8583Utils_BCD2ASC(unsigned char * BcdBuf, unsigned char * AscBuf, int Len);

/* --------------------------------------------------------------------------
* FUNCTION NAME: ISO8583Utils_ASC2BCD
* DESCRIPTION:   Convert ASCII code to BCD code.
* PARAMETERS:    AscBuf - Ascii input buffer, must ended by '\0'
*                Len - double length of BCD code, should be even.
*                BcdBuf - converted BCD code result
* RETURN:        0
* NOTES:         support 'A'-'F' convertion(extend BCD code)
* ------------------------------------------------------------------------ */
int ISO8583Utils_ASC2BCD(unsigned char * AscBuf, unsigned char * BcdBuf, int Len);

/* -----------------------------------------------------------------------------
 * FUNCTION NAME:   ISO8583Utils_BCD2LEN
 * DESCRIPTION:     Convert BcdLen bytes BCD length to int
 * RETURN:          None
 ---------------------------------------------------------------------------- */
int ISO8583Utils_BCD2LEN( unsigned char * BcdBuf, int * Len, int BcdLen );

/* -----------------------------------------------------------------------------
 * FUNCTION NAME:   ISO8583Utils_LEN2BCD
 * DESCRIPTION:     Convert BcdLen bytes BCD length to int
 * PARAMETERS:
 * RETURN:          None
 ---------------------------------------------------------------------------- */
int ISO8583Utils_LEN2BCD( int Len, byte * BcdBuf, int BcdLen);

#endif
