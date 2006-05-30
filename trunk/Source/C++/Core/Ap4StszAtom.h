/*****************************************************************
|
|    AP4 - stsz Atoms 
|
|    Copyright 2002-2006 Gilles Boccon-Gibod & Julien Boeuf
|
|
|    This file is part of Bento4/AP4 (MP4 Atom Processing Library).
|
|    Unless you have obtained Bento4 under a difference license,
|    this version of Bento4 is Bento4|GPL.
|    Bento4|GPL is free software; you can redistribute it and/or modify
|    it under the terms of the GNU General Public License as published by
|    the Free Software Foundation; either version 2, or (at your option)
|    any later version.
|
|    Bento4|GPL is distributed in the hope that it will be useful,
|    but WITHOUT ANY WARRANTY; without even the implied warranty of
|    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
|    GNU General Public License for more details.
|
|    You should have received a copy of the GNU General Public License
|    along with Bento4|GPL; see the file COPYING.  If not, write to the
|    Free Software Foundation, 59 Temple Place - Suite 330, Boston, MA
|    02111-1307, USA.
|
 ****************************************************************/

#ifndef _AP4_STSZ_ATOM_H_
#define _AP4_STSZ_ATOM_H_

/*----------------------------------------------------------------------
|   includes
+---------------------------------------------------------------------*/
#include "Ap4Array.h"
#include "Ap4Atom.h"

/*----------------------------------------------------------------------
|   AP4_StszAtom
+---------------------------------------------------------------------*/
class AP4_StszAtom : public AP4_Atom
{
public:
    // class methods
    static AP4_StszAtom* Create(AP4_Size size, AP4_ByteStream& stream);

    // methods
    AP4_StszAtom();
    virtual AP4_Result InspectFields(AP4_AtomInspector& inspector);
    virtual AP4_Result WriteFields(AP4_ByteStream& stream);
    virtual AP4_UI32   GetSampleCount();
    virtual AP4_Result GetSampleSize(AP4_Ordinal sample, 
                                     AP4_Size&   sample_size);
    virtual AP4_Result SetSampleSize(AP4_Ordinal sample, 
                                     AP4_Size    sample_size);
    virtual AP4_Result AddEntry(AP4_UI32 size);

private:
    // methods
    AP4_StszAtom(AP4_Size        size, 
                 AP4_UI32        version,
                 AP4_UI32        flags,
                 AP4_ByteStream& stream);

    // members
    AP4_UI32            m_SampleSize;
    AP4_UI32            m_SampleCount;
    AP4_Array<AP4_UI32> m_Entries;
};

#endif // _AP4_STSZ_ATOM_H_
