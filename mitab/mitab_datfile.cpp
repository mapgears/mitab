/**********************************************************************
 * $Id: mitab_datfile.cpp,v 1.2 1999-09-20 18:43:20 daniel Exp $
 *
 * Name:     mitab_datfile.cpp
 * Project:  MapInfo TAB Read/Write library
 * Language: C++
 * Purpose:  Implementation of the TABIDFile class used to handle
 *           reading/writing of the .DAT file
 * Author:   Daniel Morissette, danmo@videotron.ca
 *
 **********************************************************************
 * Copyright (c) 1999, Daniel Morissette
 *
 * All rights reserved.  This software may be copied or reproduced, in
 * all or in part, without the prior written consent of its author,
 * Daniel Morissette (danmo@videotron.ca).  However, any material copied
 * or reproduced must bear the original copyright notice (above), this 
 * original paragraph, and the original disclaimer (below).
 * 
 * The entire risk as to the results and performance of the software,
 * supporting text and other information contained in this file
 * (collectively called the "Software") is with the user.  Although 
 * considerable efforts have been used in preparing the Software, the 
 * author does not warrant the accuracy or completeness of the Software.
 * In no event will the author be liable for damages, including loss of
 * profits or consequential damages, arising out of the use of the 
 * Software.
 * 
 **********************************************************************
 *
 * $Log: mitab_datfile.cpp,v $
 * Revision 1.2  1999-09-20 18:43:20  daniel
 * Use binary access to open file.
 *
 * Revision 1.1  1999/07/12 04:18:23  daniel
 * Initial checkin
 *
 **********************************************************************/

#include "mitab.h"

/*=====================================================================
 *                      class TABDATFile
 *
 * Note that the .DAT files are .DBF files with some exceptions:
 *
 * All fields in the DBF header are defined as 'C' type (strings),
 * even for binary integers.  So we have to look in the associated .TAB
 * file to find the real field definition.
 *
 * Even though binary integers are defined as 'C' type, they are stored
 * in binary form inside a 4 bytes string field.
 *====================================================================*/


/**********************************************************************
 *                   TABDATFile::TABDATFile()
 *
 * Constructor.
 **********************************************************************/
TABDATFile::TABDATFile()
{
    m_fp = NULL;
    m_pszFname = NULL;

    m_poHeaderBlock = NULL;
    m_poRecordBlock = NULL;
    m_pasFieldDef = NULL;

    m_numFields = -1;
    m_numRecords = -1;
    m_nFirstRecordPtr = 0;
    m_nBlockSize = 0;
    m_nRecordSize = -1;
    m_nCurRecordId = -1;

}

/**********************************************************************
 *                   TABDATFile::~TABDATFile()
 *
 * Destructor.
 **********************************************************************/
TABDATFile::~TABDATFile()
{
    Close();
}

/**********************************************************************
 *                   TABDATFile::Open()
 *
 * Open a .DAT file, and initialize the structures to be ready to read
 * records from it.
 *
 * Returns 0 on success, -1 on error.
 **********************************************************************/
int TABDATFile::Open(const char *pszFname, const char *pszAccess)
{
    int i;

    if (m_fp)
    {
        CPLError(CE_Failure, CPLE_FileIO,
                 "Open() failed: object already contains an open file");
        return -1;
    }

    /*-----------------------------------------------------------------
     * Validate access mode and make sure we use binary access.
     *----------------------------------------------------------------*/
    if (EQUALN(pszAccess, "r", 1))
    {
        m_eAccessMode = TABRead;
        pszAccess = "rb";
    }
    else if (EQUALN(pszAccess, "w", 1))
    {
        CPLError(CE_Failure, CPLE_NotSupported,
                 "Open() failed: write access not implemented yet!");
        return -1;

        m_eAccessMode = TABWrite;
        pszAccess = "wb";
    }
    else
    {
        CPLError(CE_Failure, CPLE_FileIO,
                 "Open() failed: access mode \"%s\" not supported", pszAccess);
        return -1;
    }

    /*-----------------------------------------------------------------
     * Open file for reading
     *----------------------------------------------------------------*/
    m_pszFname = CPLStrdup(pszFname);
    m_fp = VSIFOpen(m_pszFname, pszAccess);

    if (m_fp == NULL)
    {
        CPLError(CE_Failure, CPLE_FileIO,
                 "Open() failed for %s", m_pszFname);
        CPLFree(m_pszFname);
        m_pszFname = NULL;
        return -1;
    }

    /*---------------------------------------------------------------------
     * Read .DAT file header (record size, num records, etc...)
     * m_poHeaderBlock will be reused later to read field definition
     *--------------------------------------------------------------------*/
    m_poHeaderBlock = new TABRawBinBlock(TRUE);
    m_poHeaderBlock->ReadFromFile(m_fp, 0, 32);

    m_poHeaderBlock->ReadByte();       // Table type ??? 0x03
    m_poHeaderBlock->ReadByte();       // Last update year
    m_poHeaderBlock->ReadByte();       // Last update month
    m_poHeaderBlock->ReadByte();       // Last update day

    m_numRecords      = m_poHeaderBlock->ReadInt32();
    m_nFirstRecordPtr = m_poHeaderBlock->ReadInt16();
    m_nRecordSize     = m_poHeaderBlock->ReadInt16();

    m_numFields = m_nFirstRecordPtr/32 - 1;

    /*---------------------------------------------------------------------
     * Read the field definitions
     * First 32 bytes field definition starts at byte 32 in file
     *--------------------------------------------------------------------*/
    m_pasFieldDef = (TABDATFieldDef*)CPLCalloc(m_numFields, 
                                               sizeof(TABDATFieldDef));

    for(i=0; i<m_numFields; i++)
    {
        m_poHeaderBlock->GotoByteInFile((i+1)*32);
        m_poHeaderBlock->ReadBytes(11, (GByte*)m_pasFieldDef[i].szName);
        m_pasFieldDef[i].szName[10] = '\0';
        m_pasFieldDef[i].cType = (char)m_poHeaderBlock->ReadByte();

        m_poHeaderBlock->ReadInt32();       // Skip Bytes 12-15
        m_pasFieldDef[i].byLength = m_poHeaderBlock->ReadByte();
        m_pasFieldDef[i].byDecimals = m_poHeaderBlock->ReadByte();

        m_pasFieldDef[i].eTABType = TABFUnknown;
    }

    /*---------------------------------------------------------------------
     * Establish a good record block size to use based on record size, and 
     * then create m_poRecordBlock
     * Record block size has to be a multiple of record size.
     *--------------------------------------------------------------------*/
    m_nBlockSize = ((1024/m_nRecordSize)+1)*m_nRecordSize;
    m_nBlockSize = MIN(m_nBlockSize, (m_numRecords*m_nRecordSize));

    m_poRecordBlock = new TABRawBinBlock(FALSE);
    m_poRecordBlock->InitBlock(m_fp, m_nBlockSize);
    m_poRecordBlock->SetFirstBlockPtr(m_nFirstRecordPtr);

    return 0;
}

/**********************************************************************
 *                   TABDATFile::Close()
 *
 * Close current file, and release all memory used.
 *
 * Returns 0 on success, -1 on error.
 **********************************************************************/
int TABDATFile::Close()
{
    if (m_fp == NULL)
        return 0;

    //__TODO__ Commit the latest changes to the file...
    
    // Delete all structures 
    if (m_poHeaderBlock)
    {
        delete m_poHeaderBlock;
        m_poHeaderBlock = NULL;
    }

    if (m_poRecordBlock)
    {
        delete m_poRecordBlock;
        m_poRecordBlock = NULL;
    }

    // Close file
    VSIFClose(m_fp);
    m_fp = NULL;

    CPLFree(m_pszFname);
    m_pszFname = NULL;

    CPLFree(m_pasFieldDef);
    m_pasFieldDef = NULL;

    m_numFields = -1;
    m_numRecords = -1;
    m_nFirstRecordPtr = 0;
    m_nBlockSize = 0;
    m_nRecordSize = -1;
    m_nCurRecordId = -1;

    return 0;
}



/**********************************************************************
 *                   TABDATFile::GetNumFields()
 *
 * Return the number of fields in this table.
 *
 * Returns a value >= 0 on success, -1 on error.
 **********************************************************************/
int  TABDATFile::GetNumFields()
{
    return m_numFields;
}

#ifdef __TODO__DELETE__
/**********************************************************************
 *                   TABDATFile::GetFieldDef()
 *
 * Returns a reference to the internal TABDATFieldDef struct with 
 * the specified field definition information from the header of 
 * the .DAT file.
 * 
 * Note that field ids are positive and start at 0.
 *
 * Since this function returns a reference to an internal structure, 
 * it should not be modified or freed by the caller.  Its contents
 * will be valid for the whole life of this TABDATFile object.
 *
 * Returns NULL if the Field definitions have not been initialized yet
 * or if the specified field does not exist.
 **********************************************************************/
TABDATFieldDef *TABDATFile::GetFieldDef(int nFieldId)
{
    if (m_pasFieldDef == NULL || nFieldId < 0 || nFieldId >= m_numFields)
        return NULL;

    return (m_pasFieldDef + nFieldId);
}
#endif

/**********************************************************************
 *                   TABDATFile::GetNumRecords()
 *
 * Return the number of records in this table.
 *
 * Returns a value >= 0 on success, -1 on error.
 **********************************************************************/
int  TABDATFile::GetNumRecords()
{
    return m_numRecords;
}

/**********************************************************************
 *                   TABDATFile::GetRecordBlock()
 *
 * Return a TABRawBinBlock reference positioned at the beginning of the
 * specified record and ready to read field values from it.  The returned
 * block is guaranteed to contain at least one full record of data.
 * 
 * Note that record ids are positive and start at 1.
 *
 * Returns a reference to the TABRawBinBlock on success or NULL on error.
 * The returned pointer is a reference to a block object owned by this 
 * TABDATFile object and should not be freed by the caller.
 **********************************************************************/
TABRawBinBlock *TABDATFile::GetRecordBlock(int nRecordId)
{
    int nFileOffset;

    nFileOffset = m_nFirstRecordPtr+(nRecordId-1)*m_nRecordSize;

    /*-----------------------------------------------------------------
     * Move record block pointer to the right location
     *----------------------------------------------------------------*/
    if (m_poRecordBlock == NULL || 
        nRecordId < 1 || nRecordId > m_numRecords ||
        m_poRecordBlock->GotoByteInFile(nFileOffset) != 0)
    {
        CPLError(CE_Failure, CPLE_FileIO,
                 "Failed reading .DAT record block for record #%d in %s",
                 nRecordId, m_pszFname);
        return NULL;
    }

    /*-----------------------------------------------------------------
     * The first char of the record is a ' ' for an active record, or
     * '*' for a deleted one.
     * __TODO__ What should we do about deleted records?????
     *----------------------------------------------------------------*/
    if (m_poRecordBlock->ReadByte() != ' ')
    {
        CPLError(CE_Warning, CPLE_NotSupported,
                 "GetRecordBlock(): record %d in file %s appears to have "
                 "been deleted.", nRecordId, m_pszFname);
    }

    return m_poRecordBlock;
}


/**********************************************************************
 *                   TABDATFile::ValidateFieldInfoFromTAB()
 *
 * Check that the value read from the .TAB file by the caller are 
 * consistent with what is found in the .DAT header.
 *
 * Note that field ids are positive and start at 0.
 *
 * We have to use this function when opening a file for reading since 
 * the .DAT file does not contain the full field types information...
 * a .DAT file is actually a .DBF file in which the .DBF types are
 * handled in a special way... type 'C' fields are used to store binary 
 * values for most MapInfo types.
 *
 * Returns a value >= 0 if OK, -1 on error.
 **********************************************************************/
int  TABDATFile::ValidateFieldInfoFromTAB(int iField, const char *pszName,
                                          TABFieldType eType,
                                          int nWidth, int nPrecision)
{
    int i = iField;  // Just to make things shorter

    CPLAssert(m_pasFieldDef);
    CPLAssert(iField >= 0 && iField < m_numFields);

    if (m_pasFieldDef == NULL ||
        !EQUALN(pszName, 
                m_pasFieldDef[i].szName, strlen(m_pasFieldDef[i].szName)) ||
        (eType == TABFChar && (m_pasFieldDef[i].cType != 'C' ||
                               m_pasFieldDef[i].byLength != nWidth )) ||
        (eType == TABFDecimal && (m_pasFieldDef[i].cType != 'N' ||
                                  m_pasFieldDef[i].byLength != nWidth||
                                 m_pasFieldDef[i].byDecimals != nPrecision)) ||
        (eType == TABFInteger && (m_pasFieldDef[i].cType != 'C' ||
                                  m_pasFieldDef[i].byLength != 4  )) ||
        (eType == TABFSmallInt && (m_pasFieldDef[i].cType != 'C' ||
                                   m_pasFieldDef[i].byLength != 2 )) ||
        (eType == TABFFloat && (m_pasFieldDef[i].cType != 'C' ||
                                m_pasFieldDef[i].byLength != 8    )) ||
        (eType == TABFDate && (m_pasFieldDef[i].cType != 'C' ||
                               m_pasFieldDef[i].byLength != 4     )) ||
        (eType == TABFLogical && (m_pasFieldDef[i].cType != 'L' ||
                                  m_pasFieldDef[i].byLength != 1  ))   )
    {
        CPLError(CE_Failure, CPLE_FileIO,
                 "Definition of field %d (%s) from .TAB file does not match "
                 "what is found in %s (name=%s, type=%c, width=%d, prec=%d)",
                 iField+1, pszName, m_pszFname,
                 m_pasFieldDef[i].szName, m_pasFieldDef[i].cType, 
                 m_pasFieldDef[i].byLength, m_pasFieldDef[i].byDecimals);
        return -1;
    }

    m_pasFieldDef[i].eTABType = eType;

    return 0;
}

/**********************************************************************
 *                   TABDATFile::GetFieldType()
 *
 * Returns the native field type for field # nFieldId as previously set
 * by ValidateFieldInfoFromTAB().
 *
 * Note that field ids are positive and start at 0.
 **********************************************************************/
TABFieldType TABDATFile::GetFieldType(int nFieldId)
{
    if (m_pasFieldDef == NULL || nFieldId < 0 || nFieldId >= m_numFields)
        return TABFUnknown;

    return m_pasFieldDef[nFieldId].eTABType;
}

/**********************************************************************
 *                   TABDATFile::GetFieldWidth()
 *
 * Returns the width for field # nFieldId as previously read from the
 * .DAT header.
 *
 * Note that field ids are positive and start at 0.
 **********************************************************************/
int   TABDATFile::GetFieldWidth(int nFieldId)
{
    if (m_pasFieldDef == NULL || nFieldId < 0 || nFieldId >= m_numFields)
        return 0;

    return m_pasFieldDef[nFieldId].byLength;
}

/**********************************************************************
 *                   TABDATFile::ReadCharField()
 *
 * Read the character field value at the current position in the data 
 * block.
 * 
 * Use GetRecordBlock() to position the data block to the beginning of
 * a record before attempting to read values.
 *
 * nWidth is the field length, as defined in the .DAT header.
 *
 * Returns a reference to an internal buffer that will be valid only until
 * the next field is read, or "" if the operation failed, in which case
 * CPLError() will have been called.
 **********************************************************************/
const char *TABDATFile::ReadCharField(int nWidth)
{
    // We know that character strings are limited to 254 chars in MapInfo
    static char szBuf[256];

    if (m_poRecordBlock == NULL)
    {
        CPLError(CE_Failure, CPLE_AssertionFailed,
                 "Can't read field value: file is not opened.");
        return "";
    }

    if (nWidth < 1 || nWidth > 255)
    {
        CPLError(CE_Failure, CPLE_AssertionFailed,
                 "Illegal width for a char field: %d", nWidth);
        return "";
    }

    if (m_poRecordBlock->ReadBytes(nWidth, (GByte*)szBuf) != 0)
        return "";

    szBuf[nWidth] = '\0';

    return szBuf;
}

/**********************************************************************
 *                   TABDATFile::ReadIntegerField()
 *
 * Read the integer field value at the current position in the data 
 * block.
 * 
 * CPLError() will have been called if something fails.
 **********************************************************************/
GInt32 TABDATFile::ReadIntegerField()
{
    if (m_poRecordBlock == NULL)
    {
        CPLError(CE_Failure, CPLE_AssertionFailed,
                 "Can't read field value: file is not opened.");
        return 0;
    }

    return m_poRecordBlock->ReadInt32();
}

/**********************************************************************
 *                   TABDATFile::ReadSmallIntField()
 *
 * Read the smallint field value at the current position in the data 
 * block.
 * 
 * CPLError() will have been called if something fails.
 **********************************************************************/
GInt16 TABDATFile::ReadSmallIntField()
{
    if (m_poRecordBlock == NULL)
    {
        CPLError(CE_Failure, CPLE_AssertionFailed,
                 "Can't read field value: file is not opened.");
        return 0;
    }

    return m_poRecordBlock->ReadInt16();
}

/**********************************************************************
 *                   TABDATFile::ReadFloatField()
 *
 * Read the float field value at the current position in the data 
 * block.
 * 
 * CPLError() will have been called if something fails.
 **********************************************************************/
double TABDATFile::ReadFloatField()
{
    if (m_poRecordBlock == NULL)
    {
        CPLError(CE_Failure, CPLE_AssertionFailed,
                 "Can't read field value: file is not opened.");
        return 0.0;
    }

    return m_poRecordBlock->ReadDouble();
}

/**********************************************************************
 *                   TABDATFile::ReadLogicalField()
 *
 * Read the logical field value at the current position in the data 
 * block.
 *
 * The file contains either 0 or 1, and we return a string with 
 * "F" (false) or "T" (true)
 * 
 * CPLError() will have been called if something fails.
 **********************************************************************/
const char *TABDATFile::ReadLogicalField()
{
    GByte bValue;

    if (m_poRecordBlock == NULL)
    {
        CPLError(CE_Failure, CPLE_AssertionFailed,
                 "Can't read field value: file is not opened.");
        return "";
    }

    bValue =  m_poRecordBlock->ReadByte();

    return bValue? "T":"F";
}

/**********************************************************************
 *                   TABDATFile::ReadDateField()
 *
 * Read the logical field value at the current position in the data 
 * block.
 *
 * A date field is a 4 bytes binary value in which the first byte is
 * the day, followed by 1 byte for the month, and 2 bytes for the year.
 *
 * We return a 10 chars string in the format "DD/MM/YYYY"
 * 
 * Returns a reference to an internal buffer that will be valid only until
 * the next field is read, or "" if the operation failed, in which case
 * CPLError() will have been called.
 **********************************************************************/
const char *TABDATFile::ReadDateField()
{
    int nDay, nMonth, nYear;
    static char szBuf[20];


    if (m_poRecordBlock == NULL)
    {
        CPLError(CE_Failure, CPLE_AssertionFailed,
                 "Can't read field value: file is not opened.");
        return "";
    }

    nDay   = m_poRecordBlock->ReadByte();
    nMonth = m_poRecordBlock->ReadByte();
    nYear  = m_poRecordBlock->ReadInt16();

    if (CPLGetLastErrorNo() != 0)
        return "";

    sprintf(szBuf, "%2.2d/%2.2d/%4.4d", nDay, nMonth, nYear);

    return szBuf;
}

/**********************************************************************
 *                   TABDATFile::ReadDecimalField()
 *
 * Read the decimal field value at the current position in the data 
 * block.
 *
 * A decimal field is a floating point value with a fixed number of digits
 * stored as a character string.
 *
 * nWidth is the field length, as defined in the .DAT header.
 *
 * We return the value as a binary double.
 * 
 * CPLError() will have been called if something fails.
 **********************************************************************/
double TABDATFile::ReadDecimalField(int nWidth)
{
    const char *pszVal;

    pszVal = ReadCharField(nWidth);

    return atof(pszVal);
}

/**********************************************************************
 *                   TABDATFile::Dump()
 *
 * Dump block contents... available only in DEBUG mode.
 **********************************************************************/
#ifdef DEBUG

void TABDATFile::Dump(FILE *fpOut /*=NULL*/)
{
    if (fpOut == NULL)
        fpOut = stdout;

    fprintf(fpOut, "----- TABDATFile::Dump() -----\n");

    if (m_fp == NULL)
    {
        fprintf(fpOut, "File is not opened.\n");
    }
    else
    {
        fprintf(fpOut, "File is opened: %s\n", m_pszFname);
        fprintf(fpOut, "m_numFields  = %d\n", m_numFields);
        fprintf(fpOut, "m_numRecords = %d\n", m_numRecords);
    }

    fflush(fpOut);
}

#endif // DEBUG





