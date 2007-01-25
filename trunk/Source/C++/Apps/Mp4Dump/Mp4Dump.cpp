/*****************************************************************
|
|    AP4 - MP4 File Dumper
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

/*----------------------------------------------------------------------
|   includes
+---------------------------------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>

#include "Ap4.h"

/*----------------------------------------------------------------------
|   constants
+---------------------------------------------------------------------*/
#define BANNER "MP4 File Dumper - Version 1.0\n"\
               "(Bento4 Version " AP4_VERSION_STRING ")\n"\
               "(c) 2002-2006 Gilles Boccon-Gibod & Julien Boeuf"

/*----------------------------------------------------------------------
|   PrintUsageAndExit
+---------------------------------------------------------------------*/
static void
PrintUsageAndExit()
{
    fprintf(stderr, 
            BANNER 
            "\n\nusage: mp4dump [options] <input>\n"
            "options are:\n"
            "  --track <track_id>[:<key>] writes the track data into a file\n"
            "                             (<mp4filename>.<track_id>) and optionally\n"
            "                             tries to decrypt it with the key (128-bit in hex)\n"
            "           (several --track options can be used, one for each track)\n");
    exit(1);
}

/*----------------------------------------------------------------------
|   CreateTrackDumpByteStream
+---------------------------------------------------------------------*/
static AP4_ByteStream*
CreateTrackDumpByteStream(const char* mp4_filename,
                          AP4_Ordinal track_id)
{
    AP4_FileByteStream* output = NULL;

    // create the output file name
    AP4_Size mp4_filename_len = strlen(mp4_filename);
    char* dump_filename = new char[mp4_filename_len+16]; // <filename>.<trackid>
    strcpy(dump_filename, mp4_filename);
    dump_filename[mp4_filename_len] = '.';
    sprintf(dump_filename+mp4_filename_len+1, "%d", track_id);

    // create a FileByteStream
    try {
        output = new AP4_FileByteStream(dump_filename,
                                        AP4_FileByteStream::STREAM_MODE_WRITE);
    } catch (AP4_Exception e) {
        fprintf(stderr, "ERROR: %d cannot open file for dumping track %d", 
                e.m_Error, track_id);
        goto end;
    }
    
end:
    delete [] dump_filename;
    return output;
}

/*----------------------------------------------------------------------
|   DumpSamples
+---------------------------------------------------------------------*/
void
DumpSamples(AP4_Track* track, AP4_ByteStream* dump)
{
    // write the data
    AP4_Sample sample;
    AP4_DataBuffer sample_data;
    AP4_Ordinal index = 0;

    while (AP4_SUCCEEDED(track->ReadSample(index, sample, sample_data))) {
        dump->Write(sample_data.GetData(), sample_data.GetDataSize());
        index++;
        if (index%10 == 0) printf(".");
    }
    printf("\n");
}

/*----------------------------------------------------------------------
|   DecryptAndDumpSamples
+---------------------------------------------------------------------*/
void
DecryptAndDumpSamples(AP4_Track*             track, 
                      AP4_SampleDescription* sample_desc,
                      const AP4_UI08*        key,
                      AP4_ByteStream*        dump)
{
    AP4_ProtectedSampleDescription* pdesc = 
        dynamic_cast<AP4_ProtectedSampleDescription*>(sample_desc);
    if (pdesc == NULL) {
        fprintf(stderr, "ERROR: unable to obtain cipher info\n");
        return;
    }

    // create the decrypter
    AP4_SampleDecrypter* decrypter = AP4_SampleDecrypter::Create(pdesc, key, 16);
    if (decrypter == NULL) {
        fprintf(stderr, "ERROR: unable to create decrypter\n");
        return;
    }

    AP4_Sample     sample;
    AP4_DataBuffer encrypted_data;
    AP4_DataBuffer decrypted_data;
    AP4_Ordinal    index = 0;
    while (AP4_SUCCEEDED(track->ReadSample(index, sample, encrypted_data))) {
        if (AP4_FAILED(decrypter->DecryptSampleData(encrypted_data, decrypted_data))) {
            fprintf(stderr, "ERROR: failed to decrypt sample\n");
            return;
        }
        dump->Write(decrypted_data.GetData(), decrypted_data.GetDataSize());
        index++;
        if (index%10 == 0) printf(".");
    }
    printf("\n");
}

/*----------------------------------------------------------------------
|   DumpTrackData
+---------------------------------------------------------------------*/
static void
DumpTrackData(const char*                   mp4_filename, 
              AP4_File*                     mp4_file, 
              const AP4_Array<AP4_Ordinal>& tracks_to_dump,
              const AP4_ProtectionKeyMap&   key_map)
{
    // dump all the tracks that need to be dumped
    AP4_Cardinal tracks_to_dump_count = tracks_to_dump.ItemCount();
    for (AP4_Ordinal i=0; i<tracks_to_dump_count; i++) {
        // get the track
        AP4_Ordinal track_id = tracks_to_dump[i];
        AP4_Track* track = mp4_file->GetMovie()->GetTrack(track_id);
        if (track == NULL) {
            fprintf(stderr, "track not found (id = %d)", track_id);
            return;
        }

        // get the sample description
        AP4_SampleDescription* sample_description = track->GetSampleDescription(0);
        if (sample_description == NULL) {
            fprintf(stderr, "ERROR: unable to parse sample description\n");
            return;
        }

        // get the dump data byte stream
        AP4_ByteStream* dump = CreateTrackDumpByteStream(mp4_filename, track_id);
        if (dump == NULL) return;

        printf("\nDumping data for track %d:\n", track_id);
        switch(sample_description->GetType()) {
            case AP4_SampleDescription::TYPE_PROTECTED:
                {
                    const AP4_UI08* key = key_map.GetKey(track_id);
                    if (key == NULL) {
                        fprintf(stderr, 
                                "WARNING: No key found for encrypted track %d... "
                                "dumping encrypted samples\n",
                                track_id);
                        DumpSamples(track, dump);
                    } else {
                        DecryptAndDumpSamples(track, sample_description, key, dump);
                    }
                }
                break;
            default:
                DumpSamples(track, dump);
        
        }
        dump->Release();
    }
}

/*----------------------------------------------------------------------
|   main
+---------------------------------------------------------------------*/
int
main(int argc, char** argv)
{
    if (argc < 2) {
        PrintUsageAndExit();
    }

    // init the variables
    AP4_ByteStream*         input = NULL;
    AP4_ProtectionKeyMap    key_map;
    AP4_Array<AP4_Ordinal>  tracks_to_dump;
    const char*             filename;
    

    // parse the command line
    argv++;
    char* arg;
    while ((arg = *argv++)) {
        if (!strcmp(arg, "--track")) {
            arg = *argv++;
            if (arg == NULL) {
                fprintf(stderr, "ERROR: missing argument after --track option\n");
                return 1;
            }
            char* track_ascii = arg;
            char* key_ascii = NULL;
            char* delimiter = strchr(arg, ':');
            if (delimiter != NULL) {
                *delimiter = '\0';
                key_ascii = delimiter+1;  
            }

            // this track will be dumped
            AP4_Ordinal track_id = (AP4_Ordinal) strtoul(track_ascii, NULL, 10); 
            tracks_to_dump.Append(track_id);

            // see if we have a key for this track
            if (key_ascii != NULL) {
                unsigned char key[16];
                if (AP4_ParseHex(key_ascii, key, 16)) {
                    fprintf(stderr, "ERROR: invalid hex format for key\n");
                    return 1;
                }
                // set the key in the map
                key_map.SetKey(track_id, key);
            }
        } else {
            try {
                filename = arg;
                input = new AP4_FileByteStream(filename,
                                               AP4_FileByteStream::STREAM_MODE_READ);
            } catch(AP4_Exception e) {
                AP4_Debug("ERROR: cannot open input (%d)\n", e.m_Error);
                return 1;
            }
        }
    }

    // open the output
    AP4_ByteStream* output =
        new AP4_FileByteStream("-stdout",
                               AP4_FileByteStream::STREAM_MODE_WRITE);
    
    // create an inspector
    AP4_PrintInspector inspector(*output);

    // inspect the atoms one by one
    AP4_Atom* atom;
    AP4_AtomFactory& atom_factory = AP4_DefaultAtomFactory::Instance;
    while (atom_factory.CreateAtomFromStream(*input, atom) == AP4_SUCCESS) {
        // remember the current stream position because the Inspect method
        // may read from the stream (there may be stream references in some
        // of the atoms
        AP4_Position position;
        input->Tell(position);

        // inspect the atom
        atom->Inspect(inspector);

        // restore the previous stream position
        input->Seek(position);

        // destroy the atom
        delete atom;
    }  

    // inspect the track data if needed
    if (tracks_to_dump.ItemCount() != 0) {
    	// rewind
    	input->Seek(0);
    	
    	// dump the track data
    	AP4_File* file = new AP4_File(*input);
        DumpTrackData(filename, file, tracks_to_dump, key_map);
        delete file;
    }

    if (input) input->Release();
    if (output) output->Release();

    return 0;
}
