/***************************************************************************
* FILE NAME:    ISO8583Engine.C                                            *
* MODULE NAME:  ISO8583Engine                                              *
* PROGRAMMER:                                                              *
* DESCRIPTION:                                                             *
* REVISION:                                                                *
****************************************************************************/

#include <conio.h>
#include <string.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#include "ISO8583Engine.h"

/*-----------------------------------------------------------------------------
 * Internal variables / constants
 *-----------------------------------------------------------------------------*/
static unsigned char BitMapMode = ISO8583_BITMAP64;
static unsigned char FldFormatSetFlag = FALSE;

//ISO8583 field format definitions, should initiated by ISO8583Engine_InitFieldFormat()
static ISO8583_FieldFormat ISO8583FldFormat[ ISO8583_MAXFIELD ];

/* -----------------------------------------------------------------------------
 * FUNCTION NAME:   ISO8583Engine_InitFieldFormat
 * DESCRIPTION:     Set ISO8583 field type and format, should be called
 *                  before the using of ISO8583 engine module
 * PARAMETERS:      bBitMode: Iso8583 bitmap mode, see enum ISO8583_BitMode in ISO8583Engine.h
 *                  pFieldFormat: poFieldFormat definitions
 * RETURN:          None
 ---------------------------------------------------------------------------- */
int ISO8583Engine_InitFieldFormat( ISO8583_BitMode bBitMode, ISO8583_FieldFormat *pIso8583FieldFormat )
{
    FldFormatSetFlag = TRUE;
    BitMapMode = bBitMode;

    if( ISO8583_MAXFIELD == 64 )
        BitMapMode = ISO8583_BITMAP64;
    else
        BitMapMode = ISO8583_BITMAP128;

    memcpy(( unsigned char * ) &ISO8583FldFormat, ( unsigned char * )pIso8583FieldFormat, sizeof( ISO8583FldFormat ) );
    return ISOENGINE_OK;
}


/* -----------------------------------------------------------------------------
 * FUNCTION NAME:   ISO8583Engine_ClearAllFields
 * DESCRIPTION:     Clear all field data in ISO8583_Rec structure
 * PARAMETERS:      ptIso8583Data: ISO8583 data structure
 * RETURN:          None.
 ---------------------------------------------------------------------------- */
int ISO8583Engine_ClearAllFields( ISO8583_Rec * ptIso8583Data )
{
    int i;
    memset(( char * ) ptIso8583Data, 0, sizeof( ISO8583_Rec ) );
    ptIso8583Data->iOffset = 0;

    for( i = 0; i < ISO8583_MAXFIELD; i ++ )
        ptIso8583Data->Field[ i ].bitf = 0;

    return ISOENGINE_OK;
}


/* -----------------------------------------------------------------------------
 * FUNCTION NAME:   ISO8583Engine_ClearOneField
 * DESCRIPTION:     Clear one field data
 * PARAMETERS:      ptIso8583Data: pointer to ISO8583 data structure
 *                  iFieldNo: Field No
 * RETURN:          None
 ---------------------------------------------------------------------------- */
int ISO8583Engine_ClearOneField( ISO8583_Rec * ptIso8583Data, int iFieldNo )
{
    if( iFieldNo >= 1 && iFieldNo < ISO8583_MAXFIELD )
        ptIso8583Data->Field[ iFieldNo - 1 ].bitf = 0;
    else
        return ISOENGINE_INVALID_FIELD_NO;

    return ISOENGINE_OK;
}


/* -----------------------------------------------------------------------------
 * FUNCTION NAME:   ISO8583Engine_SetField
 * DESCRIPTION:     Set ISO8583 field data
 * PARAMETERS:      pIso8583Data: ISO8583 data struct
 *                  iFieldNo: Field No
 *                  pFieldData: Field data
 *                  iDataLength: Length of field data
 * RETURN:          ISOENGINE_OK: Suceess
 *                  ISOENGINE_NOT_SET_FIELD_FMT: not set iso8583 field format
 *                  ISOENGINE_INVALID_FIELD_NO: iFieldNo > ISO8583_MAXFIELD or iFieldNo <= 1
 *                  ISOENGINE_TOO_LONG_FILED_LENGTH: iDataLength > 999
 *                  -4: iso8583 string total length already > ISO8583_MAXLENTH
 ---------------------------------------------------------------------------- */
int ISO8583Engine_SetField( ISO8583_Rec * pIso8583Data, int iFieldNo, unsigned char * pFieldData, int iDataLength )
{
    int i, len;
    int iFieldNum, iLength;
    byte cTemp[ 1000 ];
    byte * pRpt;

    if( FldFormatSetFlag != TRUE )
        return ISOENGINE_NOT_SET_FIELD_FMT;

    iFieldNum = iFieldNo;
    iLength = iDataLength;

    if( iLength <= 0 )
        return ISOENGINE_INVALID_FIELD_LENGTH;

    if( iFieldNum == 0 )
    {
        memcpy( pIso8583Data->cMsgID, pFieldData, 4 );
        pIso8583Data->cMsgID[ 4 ] = 0;
        return ISOENGINE_OK;
    }

    if( iFieldNum <= 1 || iFieldNum > ISO8583_MAXFIELD )
    {
        return ISOENGINE_INVALID_FIELD_NO;
    }

    iFieldNum --;

    if( iLength > ISO8583FldFormat[ iFieldNum ].iMaxLength )
        iLength = ISO8583FldFormat[ iFieldNum ].iMaxLength;

    pIso8583Data->Field[ iFieldNum ].bitf = 1;
    len = iLength;

    if( ISO8583FldFormat[ iFieldNum ].bType & ISO8583TYPE_FIX )
        iLength = ISO8583FldFormat[ iFieldNum ].iMaxLength;
    else if( ISO8583FldFormat[ iFieldNum ].bType & ISO8583TYPE_BIN )
        iLength = ISO8583FldFormat[ iFieldNum ].iMaxLength / 8;

    if( iLength > 999 )
        return ISOENGINE_TOO_LONG_FILED_LENGTH;

    pIso8583Data->Field[ iFieldNum ].len = iLength;
    pRpt = & ( pIso8583Data->cData[ pIso8583Data->iOffset ] );
    pIso8583Data->Field[ iFieldNum ].addr = pIso8583Data->iOffset;

    if( iLength + pIso8583Data->iOffset >= ISO8583_MAXLENTH )
        return ISOENGINE_OVER_MAXLENGTH;

    pIso8583Data->iOffset += iLength;
    i = 0;
    memset( cTemp, 0, sizeof( cTemp ) );

    if( ISO8583FldFormat[ iFieldNum ].bType & ISO8583TYPE_DIGIT )
    {
        for( ; i < iLength - len; i ++ )
            cTemp[ i ] = '0';
    }

    memcpy( cTemp + i, pFieldData, len ) ;
    i += len;

    if( ISO8583FldFormat[ iFieldNum ].bType & ISO8583TYPE_BIN )
    {
        for( ; i < iLength; i ++ )
            cTemp[ i ] = 0;
    }
    else if( iFieldNum == 1 )
    {
        cTemp[ i ] = 'F';
        iLength ++;
    }
    else
    {
        for( ; i < iLength; i ++ )
            cTemp[ i ] = ' ';
    }

    if( ISO8583FldFormat[ iFieldNum ].bType & ISO8583TYPE_BCD )
        ISO8583Utils_ASC2BCD( cTemp, pRpt, iLength );
    else
        memcpy( pRpt, pFieldData, iLength );

    return ISOENGINE_OK;
}


/* -----------------------------------------------------------------------------
 * FUNCTION NAME:   ISO8583Engine_GetField
 * DESCRIPTION:     Get ISO8583 field data, pRetFieldData must be ASC format
 * PARAMETERS:      pIso8583Data: ISO8583 data struct
 *                  iFieldNo: Field No
 *                  pRetFieldData: Return field data buffer
 *                  iSizeofRetFieldData: Length of pRetFieldData field data buffer
 * RETURN:          >=0: suceess, return data length
 *                  ISOENGINE_NOT_SET_FIELD_FMT: not set iso8583 field format
 *                  -2: iFieldNo > ISO8583_MAXFIELD or iFieldNo <= 1
 *                  -3: iDataLength > 999
 *                  -4: iso8583 string total length already > ISO8583_MAXLENTH
 ---------------------------------------------------------------------------- */
int ISO8583Engine_GetField( ISO8583_Rec * pIso8583Data, int iFieldNo, unsigned char * pRetFieldData, int iSizeofRetFieldData )
{
    int iLength;
    int iFieldNum;
    byte * pRpt;

    if( FldFormatSetFlag != TRUE )
        return ISOENGINE_NOT_SET_FIELD_FMT;

    iFieldNum = iFieldNo;

    if( iFieldNum == 0 )
    {
        if(iSizeofRetFieldData < 5)
        {
            return ISOENGINE_TOO_SMALL_FIELD_BUF_SIZE;
        }

        memcpy( pRetFieldData, pIso8583Data->cMsgID, 4 );
        pRetFieldData[ 4 ] = 0;
        return (4);
    }

    if( iFieldNum <= 1 || iFieldNum > ISO8583_MAXFIELD )
        return (-2);

    iFieldNum --;

    if( pIso8583Data->Field[ iFieldNum ].bitf == 0 )
    {
        pRetFieldData[ 0 ] = 0;
        return ISOENGINE_OK;
    }

    if( pIso8583Data->Field[ iFieldNum ].addr < 0 || pIso8583Data->Field[ iFieldNum ].addr >= ISO8583_MAXLENTH )
        return (-4);

    pRpt = &pIso8583Data->cData[ pIso8583Data->Field[ iFieldNum ].addr ];
    iLength = pIso8583Data->Field[ iFieldNum ].len;

    if( iLength < 0 || iLength > 999 )
        return (-3);

    if( iLength > iSizeofRetFieldData )
        iLength = iSizeofRetFieldData;

    if( ISO8583FldFormat[ iFieldNum ].bType & ( ISO8583TYPE_BCD | ISO8583TYPE_DIGIT ) )
        ISO8583Utils_BCD2ASC( pRpt, pRetFieldData, iLength );
    else
        memcpy( pRetFieldData, pRpt, iLength );

    return (iLength);
}


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
int ISO8583Engine_HexbufToIso8583( ISO8583_Rec * pIso8583Data, byte * pBuf )
{
    int iOffSize, iLength, iBitnum;
    int i, j, k, iFieldNum;
    byte cBitmask, cVarLen[ 3 ];
    byte * pRpt;

    for( i = 0; i < ISO8583_MAXFIELD; i ++ )
        pIso8583Data->Field[ i ].bitf = 0;

    iOffSize = 0;
    ISO8583Utils_BCD2ASC( pBuf, pIso8583Data->cMsgID, 4 );
    pIso8583Data->cMsgID[ 4 ] = 0;
    BitMapMode = ISO8583_BITMAP64;

    if(( pBuf[ 2 ] & 0x80 ) && ( ISO8583_MAXFIELD == 128 ) )
    {
        iBitnum = 16;
        BitMapMode = ISO8583_BITMAP64;
    }
    else
        iBitnum = 8;

    pRpt = pBuf + 2 + iBitnum;

    for( i = 0; i < iBitnum; i ++ )
    {
        cBitmask = 0x80;

        for( j = 0; j < 8; j ++, cBitmask >>= 1 )
        {
            if( i == 0 && cBitmask == 0x80 )
                continue;

            if(( pBuf[ i + 2 ] & cBitmask ) == 0 )
                continue;

            iFieldNum = ( i << 3 ) + j;

            if( iFieldNum < 1 || iFieldNum >= ISO8583_MAXFIELD )
                return( -2 );

            if( ISO8583FldFormat[ iFieldNum ].bType & ISO8583TYPE_VAR )
            {
                memset( cVarLen, 0, sizeof( cVarLen ) );
                cVarLen[ 0 ] = *pRpt;
                pRpt ++;
                ISO8583Utils_BCD2LEN( cVarLen, &iLength, 1 );

                if( ISO8583FldFormat[ iFieldNum ].iMaxLength > 99 )
                {
                    cVarLen[ 1 ] = *pRpt;
                    pRpt ++;
                    ISO8583Utils_BCD2LEN( cVarLen, &iLength, 2 );
                }

                if( iLength > ISO8583FldFormat[ iFieldNum ].iMaxLength )
                    return( -1 );
            }
            else if( ISO8583FldFormat[ iFieldNum ].bType & ISO8583TYPE_BIN )
                iLength = ISO8583FldFormat[ iFieldNum ].iMaxLength / 8;
            else
                iLength = ISO8583FldFormat[ iFieldNum ].iMaxLength;

            pIso8583Data->Field[ iFieldNum ].len = iLength;
            pIso8583Data->Field[ iFieldNum ].addr = iOffSize;

            if( ISO8583FldFormat[ iFieldNum ].bType & ( ISO8583TYPE_BCD | ISO8583TYPE_DIGIT ) )
            {
                iLength ++;
                iLength >>= 1;
            }

            if( iLength + iOffSize >= ISO8583_MAXLENTH )
                return( -3 );

            for( k = 0; k < iLength; k ++ )
                pIso8583Data->cData[ iOffSize ++ ] = *pRpt ++;

            pIso8583Data->Field[ iFieldNum ].bitf = 1;
        }
    }

    pIso8583Data->iOffset = iOffSize;
    return( 0 );
}

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
int ISO8583Engine_Iso8583ToHexbuf( ISO8583_Rec * pIso8583Data, byte * pRetBuf, int iSizeRetBuf )
{
    byte * cpWpt, cBitmask, cBitmap;
    int iFieldNum, iBitnum;
    int i, j, k, iLength;
    ISO8583Utils_ASC2BCD( pIso8583Data->cMsgID, pRetBuf, 4 );

    if(( BitMapMode == ISO8583_BITMAP128 ) && ( ISO8583_MAXFIELD == 128 ) )
        iBitnum = 16;
    else
        iBitnum = 8;

    cpWpt = pRetBuf + 2 + iBitnum;

    for( i = 0; i < iBitnum; i ++ )
    {
        cBitmap = 0;
        cBitmask = 0x80;

        for( j = 0; j < 8; j ++, cBitmask >>= 1 )
        {
            iFieldNum = ( i << 3 ) + j ;

            if( pIso8583Data->Field[ iFieldNum ].bitf == 0 )
                continue;

            if( iFieldNum < 1 || iFieldNum >= ISO8583_MAXFIELD )
                return( -2 );

            if(( cpWpt - pRetBuf ) > iSizeRetBuf )
                return ( -3 );

            cBitmap |= cBitmask;
            iLength = pIso8583Data->Field[ iFieldNum ].len;

            if( ISO8583FldFormat[ iFieldNum ].bType & ISO8583TYPE_VAR )
            {
                if( ISO8583FldFormat[ iFieldNum ].iMaxLength <= 99 )
                {
                    ISO8583Utils_LEN2BCD( iLength, cpWpt, 1 );
                    cpWpt ++;
                }
                else
                {
                    ISO8583Utils_LEN2BCD( iLength, cpWpt, 2 );
                    cpWpt += 2;
                }
            }

            k = 0 ;

            if( ISO8583FldFormat[ iFieldNum ].bType & ( ISO8583TYPE_BCD | ISO8583TYPE_DIGIT ) )
            {
                iLength ++ ;
                iLength >>= 1;
            }

            for( ; k < iLength; k ++ )
            {
                if(( pIso8583Data->Field[ iFieldNum ].addr + k < 0 ) || ( pIso8583Data->Field[ iFieldNum ].addr + k >= ISO8583_MAXLENTH ) )
                    return( -4 );

                ( *cpWpt ++ ) = pIso8583Data->cData[ pIso8583Data->Field[ iFieldNum ].addr + k ];
            }
        }

        pRetBuf[ i + 2 ] = cBitmap;
    }

    if( iBitnum == 16 )
        pRetBuf[ 2 ] |= 0x80;

    return( cpWpt - pRetBuf );
}



/* --------------------------------------------------------------------------
* FUNCTION NAME: ISO8583Utils_BCD2ASC
* DESCRIPTION:   Convert BCD code to ASCII code.
* PARAMETERS:    BcdBuf - BCD input buffer, Len - double length of Bcdbuf bytes
*                AscBuf - converted result
* RETURN:        0
* NOTES:
* ------------------------------------------------------------------------ */
int ISO8583Utils_BCD2ASC(unsigned char * BcdBuf, unsigned char * AscBuf, int Len)
{
    int i;

    for (i = 0; i < Len; i++)
    {
        AscBuf[i] = (i % 2) ? (BcdBuf[i / 2] & 0x0f) : ((BcdBuf[i / 2] >> 4) & 0x0f);
        AscBuf[i] += ((AscBuf[i] > 9) ? ('A' - 10) : '0');
    }

    return 0;
}


/* --------------------------------------------------------------------------
* FUNCTION NAME: ISO8583Utils_ASC2BCD
* DESCRIPTION:   Convert ASCII code to BCD code.
* PARAMETERS:    AscBuf - Ascii input buffer, must ended by '\0'
*                Len - double length of BCD code, should be even.
*                BcdBuf - converted BCD code result
* RETURN:        0
* NOTES:         support 'A'-'F' convertion(extend BCD code)
* ------------------------------------------------------------------------ */
int ISO8583Utils_ASC2BCD(unsigned char * AscBuf, unsigned char * BcdBuf, int Len)
{
    int  i;
    unsigned char str[2] = {0};

    for (i = 0; i < Len; i += 2)
    {
        if ((AscBuf[i] >= 'a') && (AscBuf[i] <= 'f'))
        {
            str[0] = AscBuf[i] - 'a' + 0x0A;
        }
        else if ((AscBuf[i] >= 'A') && (AscBuf[i] <= 'F'))
        {
            str[0] = AscBuf[i] - 'A' + 0x0A;
        }
        else if (AscBuf[i] >= '0')
        {
            str[0] = AscBuf[i] - '0';
        }
        else
        {
            str[0] = 0;
        }

        if ((AscBuf[i + 1] >= 'a') && (AscBuf[i + 1] <= 'f'))
        {
            str[1] = AscBuf[i + 1] - 'a' + 0x0A;
        }
        else if ((AscBuf[i + 1] >= 'A') && (AscBuf[i + 1] <= 'F'))
        {
            str[1] = AscBuf[i + 1] - 'A' + 0x0A;
        }
        else if (AscBuf[1] >= '0')
        {
            str[1] = AscBuf[i + 1] - '0';
        }
        else
        {
            str[1] = 0;
        }

        BcdBuf[i / 2] = (str[0] << 4) | (str[1] & 0x0F);
    }

    return 0;
}

/* -----------------------------------------------------------------------------
 * FUNCTION NAME:   ISO8583Utils_BCD2LEN
 * DESCRIPTION:     Convert BcdLen bytes BCD length to int
 * RETURN:          None
 ---------------------------------------------------------------------------- */
int ISO8583Utils_BCD2LEN( unsigned char * BcdBuf, int * Len, int BcdLen )
{
    int i;
    unsigned short usLen = 0;

    if(BcdLen <= 0 || BcdLen > 10)
        return -1;

    for (i = 0; i < BcdLen; i++)
    {
        usLen = usLen * 100 + (((*(BcdBuf + i)) & 0xF0) >> 4) * 10 + ((*(BcdBuf + i)) & 0x0F);
    }

    *Len = (int)usLen;
    return 0;
}

/* -----------------------------------------------------------------------------
 * FUNCTION NAME:   ISO8583Utils_LEN2BCD
 * DESCRIPTION:     Convert BcdLen bytes BCD length to int
 * PARAMETERS:
 * RETURN:          None
 ---------------------------------------------------------------------------- */
int ISO8583Utils_LEN2BCD( int Len, byte * BcdBuf, int BcdLen)
{
    char  format[10], str[20];

    if (BcdLen > 10)
    {
        return -1;
    }

    memset(format, 0, sizeof(format));
    sprintf(format, "%c0%lulu", '%', (unsigned long)(BcdLen * 2));
    sprintf((char *)str, (char *)format, Len);
    ISO8583Utils_ASC2BCD((byte *)str, BcdBuf, BcdLen * 2);
    return 0;
}
