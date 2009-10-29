/*****************************************************************
|
|    AP4 - Fragment Parser Test
|
|    Copyright 2002-2009 Axiomatic Systems, LLC
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
 
/*----------------------------------------------------------------------
|   includes
+---------------------------------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>

#include "Ap4.h"

/*----------------------------------------------------------------------
|   macros
+---------------------------------------------------------------------*/
#define CHECK(x) do { \
    if (!(x)) { fprintf(stderr, "ERROR line %d\n", __LINE__); return -1; }\
} while (0)

/*----------------------------------------------------------------------
|   constants
+---------------------------------------------------------------------*/
#define BANNER "Fragment Parser Test - Version 1.0\n"\
               "(Bento4 Version " AP4_VERSION_STRING ")\n"\
               "(c) 2002-2009 Axiomatic Systems, LLC"

/*----------------------------------------------------------------------
|   PrintUsageAndExit
+---------------------------------------------------------------------*/
static void
PrintUsageAndExit()
{
    fprintf(stderr, 
            BANNER 
            "\n\nusage: fragmentparsertest <test-filename>\n");
    exit(1);
}

/*----------------------------------------------------------------------
|   ShowSample
+---------------------------------------------------------------------*/
static void
ShowSample(AP4_Sample& sample, unsigned int index)
{
    printf("[%06d] size=%6d duration=%6d", 
           index, 
           (int)sample.GetSize(), 
           (int)sample.GetDuration());
    printf(" offset=%10lld dts=%10lld cts=%10lld ", 
           sample.GetOffset(),
           sample.GetDts(), 
           sample.GetCts());
    if (sample.IsSync()) {
        printf(" [S] ");
    } else {
        printf("     ");
    }

    AP4_DataBuffer sample_data;
    sample.ReadData(sample_data);
    unsigned int show = sample_data.GetDataSize();
    if (show > 12) show = 12; // max first 12 chars
    
    for (unsigned int i=0; i<show; i++) {
        printf("%02x", sample_data.GetData()[i]);
    }
    if (show == sample_data.GetDataSize()) {
        printf("\n");
    } else {
        printf("...\n");
    }
}

/*----------------------------------------------------------------------
|   ProcessSamples
+---------------------------------------------------------------------*/
static int
ProcessSamples(AP4_FragmentSampleTable* sample_table)
{
    printf("found %d samples\n", sample_table->GetSampleCount());
    for (unsigned int i=0; i<sample_table->GetSampleCount(); i++) {
        AP4_Sample sample;
        AP4_Result result = sample_table->GetSample(i, sample);
        CHECK(AP4_SUCCEEDED(result));
        ShowSample(sample, i);
    }
    
    return 0;
}

/*----------------------------------------------------------------------
|   ProcessMoof
+---------------------------------------------------------------------*/
static int
ProcessMoof(AP4_Movie* movie, AP4_ContainerAtom* moof, AP4_ByteStream* sample_stream, AP4_Offset mdat_payload_offset)
{
    AP4_Result result;
    
    AP4_MovieFragment* fragment = new AP4_MovieFragment(moof);
    printf("fragment sequence number=%d\n", fragment->GetSequenceNumber());
    
    AP4_FragmentSampleTable* sample_table = NULL;
    
    // get all track IDs in this fragment
    AP4_Array<AP4_UI32> ids;
    fragment->GetTrackIds(ids);
    printf("Found %d tracks in fragment: ", ids.ItemCount());
    for (unsigned int i=0; i<ids.ItemCount(); i++) {
        printf("%d ", ids[i]);
    }
    printf("\n");
    
    for (unsigned int i=0; i<ids.ItemCount(); i++) {
        printf("processing moof for track id %d\n", ids[i]);
        result = fragment->CreateSampleTable(movie, ids[i], sample_stream, mdat_payload_offset, sample_table);
        CHECK(result == AP4_SUCCESS || result == AP4_ERROR_NO_SUCH_ITEM);
        if (AP4_SUCCEEDED(result)) {
            ProcessSamples(sample_table);
            delete sample_table;
        } else {
            printf("no sample table for this track\n");
        }
    }
    
    delete fragment;
    return 0;
}

/*----------------------------------------------------------------------
|   main
+---------------------------------------------------------------------*/
int
main(int argc, char** argv)
{
    if (argc != 2) {
        PrintUsageAndExit();
    }
    const char* input_filename  = argv[1];
    
    // open the input
    AP4_ByteStream* input = NULL;
    AP4_Result result = AP4_FileByteStream::Create(input_filename, AP4_FileByteStream::STREAM_MODE_READ, input);
    if (AP4_FAILED(result)) {
        fprintf(stderr, "ERROR: cannot open input file (%s)\n", input_filename);
        return 1;
    }
        
    // get the movie
    AP4_File* file = new AP4_File(*input, AP4_DefaultAtomFactory::Instance, true);
    AP4_Movie* movie = file->GetMovie();
    
    AP4_Atom* atom = NULL;
    do {
        // process the next atom
        result = AP4_DefaultAtomFactory::Instance.CreateAtomFromStream(*input, atom);
        if (AP4_SUCCEEDED(result)) {
            printf("atom size=%lld\n", atom->GetSize());
            if (atom->GetType() == AP4_ATOM_TYPE_MOOF) {
                AP4_ContainerAtom* moof = AP4_DYNAMIC_CAST(AP4_ContainerAtom, atom);
                if (moof) {
                    // remember where we are in the stream
                    AP4_Position position = 0;
                    input->Tell(position);
        
                    // process the movie fragment
                    ProcessMoof(movie, moof, input, position+8);

                    // go back to where we were before processing the fragment
                    input->Seek(position);
                }
            } else {
                delete atom;
            }            
        }
    } while (AP4_SUCCEEDED(result));
    
    // cleanup
    delete file;
    input->Release();

    return 0;                                            
}
