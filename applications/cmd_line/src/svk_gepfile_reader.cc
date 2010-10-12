/*
 *  Copyright © 2009-2010 The Regents of the University of California.
 *  All Rights Reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions are met:
 *  •   Redistributions of source code must retain the above copyright notice,
 *      this list of conditions and the following disclaimer.
 *  •   Redistributions in binary form must reproduce the above copyright notice,
 *      this list of conditions and the following disclaimer in the documentation
 *      and/or other materials provided with the distribution.
 *  •   None of the names of any campus of the University of California, the name
 *      "The Regents of the University of California," or the names of any of its
 *      contributors may be used to endorse or promote products derived from this
 *      software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 *  ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 *  WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 *  IN NO EVENT SHALL THE COPYRIGHT HOLDERS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
 *  INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 *  NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 *  PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 *  WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) 
 *  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY
 *  OF SUCH DAMAGE. 
 */     


/*
 *  $URL$
 *  $Rev$
 *  $Author$
 *  $Date$
 *
 *  Authors:
 *      Jason C. Crane, Ph.D.
 *      Beck Olson
 *
 *  License: TBD
 *
 *  Utility application for converting between supported file formats. 
 *
 */

#include <vtkSmartPointer.h>

#include <svkImageReaderFactory.h>
#include <svkImageReader2.h>
#include <svkDdfVolumeReader.h>
#include <svkDcmVolumeReader.h>
#include <svkImageWriterFactory.h>
#include <svkImageWriter.h>
#include <svkDICOMMRSWriter.h>
#include <svkDdfVolumeWriter.h>
#include <svkCorrectDCOffset.h>
#include <svkDcmHeader.h>
#include <svkGEPFileReader.h>
#include <svkGEPFileMapper.h>
#include <svkImageAlgorithm.h>

#include <getopt.h>

#define UNDEFINED_TEMP -1111

using namespace svk;



int main (int argc, char** argv)
{

    string usemsg("\n") ; 
    usemsg += "Version " + string(SVK_RELEASE_VERSION) + "\n";   
    usemsg += "svk_gepfile_reader -i input_file_name -o output_file_name [ -t output_data_type ] \n"; 
    usemsg += "                   [ --deid_type type [ --deid_pat_id id ] [ --deid_study_id id ]  ] \n";
    usemsg += "                   [ -u | -s ] [ -anh ] \n";
    usemsg += "                   [ --one_time_pt ] [ --temp tmp ] [ --no_dc_correction ] \n";
    usemsg += "\n";  
    usemsg += "   -i                name   Name of file to convert. \n"; 
    usemsg += "   -o                name   Name of outputfile. \n";
    usemsg += "   -t                type   Target data type: \n";
    usemsg += "                                 2 = UCSF DDF      \n";
    usemsg += "                                 4 = DICOM_MRS (default)    \n";
    usemsg += "   --deid_type       type   Type of deidentification: \n";  
    usemsg += "                                 1 = limited (default) \n";  
    usemsg += "                                 2 = deidentified \n";  
    usemsg += "   --deid_pat_id     id     Use the specified patient id to deidentify patient level PHI fields. \n";          
    usemsg += "   --deid_study_id   id     Use the specified study id to deidentify study level PHI fields. \n";  
    usemsg += "   -u                       If single voxel, write only unsuppressed data (individual acqs. preserved) \n"; 
    usemsg += "   -s                       If single voxel, write only suppressed data (individual acqs. preserved) \n"; 
    usemsg += "   -a                       If single voxel, write average of the specified data  \n"; 
    usemsg += "                            (e.g. all, suppressesd, unsuppressed) \n"; 
    usemsg += "   --one_time_pt            If there are multiple time points, separate each into its own file.  \n";
    usemsg += "                            (supported only for ddf output) \n";
    usemsg += "   --temp             temp  Set the temp (celcius) of the acquisition.  Default is body temperature. Used to \n"; 
    usemsg += "                            set the chemical shift of water. \n"; 
    usemsg += "   --no_dc_correction       Turns DC Offset correction off. \n"; 
    usemsg += "   -h                       Print this help mesage. \n";  
    usemsg += "\n";  
    usemsg += "Converts a GE PFile to a DICOM MRS object. The default behavior is to load the entire raw data set.\n";  
    usemsg += "\n";  


    string inputFileName; 
    string outputFileName;
    bool unsuppressed = false; 
    bool suppressed = false; 
    bool average = false; 
    svkImageWriterFactory::WriterType dataTypeOut = svkImageWriterFactory::DICOM_MRS;
    int oneTimePtPerFile = false; 
    svkDcmHeader::PHIType deidType = svkDcmHeader::PHI_IDENTIFIED; 
    string deidPatId = ""; 
    string deidStudyId = ""; 
    float temp = UNDEFINED_TEMP; 
    bool dcCorrection = true; 

    string cmdLine = svkProvenance::GetCommandLineString( argc, argv ); 


    enum FLAG_NAME {
        FLAG_ONE_TIME_PT = 0, 
        FLAG_DC_CORRECTION, 
        FLAG_DEID_TYPE, 
        FLAG_DEID_PAT_ID, 
        FLAG_DEID_STUDY_ID, 
        FLAG_TEMP
    }; 


    static struct option long_options[] =
    {
        /* This option sets a flag. */
        {"one_time_pt",      no_argument,       NULL,  FLAG_ONE_TIME_PT},
        {"no_dc_correction", no_argument,       NULL,  FLAG_DC_CORRECTION},
        {"deid_type",        required_argument, NULL,  FLAG_DEID_TYPE},
        {"deid_pat_id",      required_argument, NULL,  FLAG_DEID_PAT_ID},
        {"deid_study_id",    required_argument, NULL,  FLAG_DEID_STUDY_ID},
        {"temp",             required_argument, NULL,  FLAG_TEMP},
        {0, 0, 0, 0}
    };

    // ===============================================  
    //  Process flags and arguments
    // ===============================================  
    int i;
    int option_index = 0; 
    while ( ( i = getopt_long(argc, argv, "i:o:t:usah", long_options, &option_index) ) != EOF) {
        switch (i) {
            case 'i':
                inputFileName.assign( optarg );
                break;
            case 'o':
                outputFileName.assign(optarg);
                break;
            case 't':
                dataTypeOut = static_cast<svkImageWriterFactory::WriterType>( atoi(optarg) );
                break;
            case 'u':
                unsuppressed = true;  
                break;
            case 's':
                suppressed = true;  
                break;
            case 'a':
                average = true;  
                break;
            case FLAG_ONE_TIME_PT:
                oneTimePtPerFile = true; 
                break;
            case FLAG_DC_CORRECTION:
                dcCorrection = false; 
                break;
            case FLAG_DEID_TYPE:
                deidType = static_cast<svkDcmHeader::PHIType>(atoi( optarg));  
                break;
            case FLAG_DEID_PAT_ID:
                deidPatId.assign( optarg ); 
                break;
            case FLAG_DEID_STUDY_ID:
                deidStudyId.assign( optarg ); 
                break;
            case FLAG_TEMP:
                temp = atof( optarg ); 
                break;
            case 'h':
                cout << usemsg << endl;
                exit(1);  
                break;
            default:
                ;
        }
    }

    argc -= optind;
    argv += optind;

    // ===============================================  
    //  validate that: 
    //      an output name was supplied
    //      that not suppresses and unsuppressed were both specified 
    //      that only the supported output types was requested. 
    //      
    // ===============================================  
    if ( 
        argc != 0 ||  inputFileName.length() == 0  
        || outputFileName.length() == 0 
        || ( suppressed && unsuppressed ) 
        || ( dataTypeOut != svkImageWriterFactory::DICOM_MRS && dataTypeOut != svkImageWriterFactory::DDF ) 
    ) {
        cout << usemsg << endl;
        exit(1); 
    }

    // ===============================================  
    //  validate that if oneTimePtPerFile was specified 
    //  that the output type must be ddf 
    // ===============================================  
    if ( oneTimePtPerFile && dataTypeOut != svkImageWriterFactory::DDF ) {
        cout << "Error: -n flag and -t flag conflict. " << endl;
        cout << "        num time points option only supported for ddf output" << endl;
        cout << usemsg << endl;
        exit(1); 
    }

    // ===============================================  
    //  validate deidentification args:  
    // ===============================================  
    if ( deidType != svkDcmHeader::PHI_DEIDENTIFIED &&
         deidType != svkDcmHeader::PHI_LIMITED &&
         deidType != svkDcmHeader::PHI_IDENTIFIED) 
    {
        cout << "Error: invalid deidentificatin type: " << deidType <<  endl;
        cout << usemsg << endl;
        exit(1); 
    } else {
        cout << "Deidentify data: type = " << deidType << endl; 
    }

    cout << "file name: " << inputFileName << endl;

    // ===============================================  
    //  Use a reader factory to get the correct reader  
    //  type . 
    // ===============================================  
    vtkSmartPointer< svkImageReaderFactory > readerFactory = vtkSmartPointer< svkImageReaderFactory >::New(); 
    svkGEPFileReader* reader = svkGEPFileReader::SafeDownCast( readerFactory->CreateImageReader2(inputFileName.c_str()) );

    if (reader == NULL) {
        cerr << "Can not determine appropriate reader for: " << inputFileName << endl;
        exit(1);
    }


    // ===============================================  
    //  Use the reader to read the data into an 
    //  svkMrsImageData object and set any reading
    //  behaviors (e.g. average suppressed data). 
    // ===============================================  
    reader->SetFileName( inputFileName.c_str() );

    //  Set the svkGEPFileReader's behavior if not default
    if ( unsuppressed ) { 
        reader->SetMapperBehavior( svkGEPFileMapper::LOAD_RAW_UNSUPPRESSED ); 
        if ( average ) {
            reader->SetMapperBehavior( svkGEPFileMapper::LOAD_AVG_UNSUPPRESSED ); 
        }
    } else if ( suppressed ) { 
        reader->SetMapperBehavior( svkGEPFileMapper::LOAD_RAW_SUPPRESSED ); 
        if ( average ) {
            reader->SetMapperBehavior( svkGEPFileMapper::LOAD_AVG_SUPPRESSED ); 
        }
    } else {
        reader->SetMapperBehavior( svkGEPFileMapper::LOAD_RAW ); 
    }

    // ===============================================  
    //  Set up deidentification options: 
    // ===============================================  
    if ( deidType != svkDcmHeader::PHI_IDENTIFIED ) { 
        if ( deidPatId.compare("") != 0 && deidStudyId.compare("") != 0 ) { 
            reader->SetDeidentify( deidType, deidPatId, deidStudyId ); 
        } else if ( deidPatId.compare("") != 0 ) {
            reader->SetDeidentify( deidType, deidPatId ); 
        } else if ( deidStudyId.compare("") != 0 ) {
            reader->SetDeidentify( deidType, deidStudyId ); 
        } else {
            reader->SetDeidentify( deidType ); 
        }
    }

    if ( temp != UNDEFINED_TEMP ) { 
        reader->SetTemperature( temp ); 
    }
    
    reader->Update(); 

    svkImageData* currentImage = svkMrsImageData::SafeDownCast( reader->GetOutput() ); 

    // ===============================================  
    //  Optionally perform the DC offset correction
    // ===============================================  
    vtkSmartPointer< svkCorrectDCOffset > dc = vtkSmartPointer< svkCorrectDCOffset >::New(); 
    if ( dcCorrection ) {
        dc->SetInput( reader->GetOutput() ); 
        dc->Update(); 

        //  if this step is included, then reset the current algo to be 
        //  passed to the writer. 
        currentImage = svkMrsImageData::SafeDownCast( dc->GetOutput() ); 
    }

    // ===============================================  
    //  Write the data out to the specified file type.  
    //  Use an svkImageWriterFactory to obtain the
    //  correct writer type. 
    // ===============================================  
    vtkSmartPointer< svkImageWriterFactory > writerFactory = vtkSmartPointer< svkImageWriterFactory >::New(); 
    svkImageWriter* writer = static_cast<svkImageWriter*>( writerFactory->CreateImageWriter( dataTypeOut ) ); 

    if ( writer == NULL ) {
        cerr << "Can not determine writer of type: " << dataTypeOut << endl;
        exit(1);
    }

    writer->SetFileName( outputFileName.c_str() );
    writer->SetInput( svkMrsImageData::SafeDownCast( currentImage ) );
    if ( oneTimePtPerFile ) { 
        svkDdfVolumeWriter::SafeDownCast( writer )->SetOneTimePointsPerFile();
    }

    // ===============================================  
    //  Set the input command line into the data set 
    //  provenance: 
    // ===============================================  
    reader->GetOutput()->GetProvenance()->SetApplicationCommand( cmdLine );

    // ===============================================  
    //  Write data to file: 
    // ===============================================  
    writer->Write();

    // ===============================================  
    //  Clean up: 
    // ===============================================  
    writer->Delete();
    reader->Delete();

    return 0; 
}
