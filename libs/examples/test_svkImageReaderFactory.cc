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
 */


#include <svkImageReaderFactory.h>
#include <svkImageData.h>
using namespace svk;


int main (int argc, char** argv)
{
    string fname(argv[1]);
    double magRange[2] = { VTK_DOUBLE_MAX, VTK_DOUBLE_MIN };
    //  Turn debuggin on (1) or of (0):
    vtkObject::SetGlobalWarningDisplay(0);
    svkImageReaderFactory* readerFactory = svkImageReaderFactory::New();
    svkImageReader2* reader = readerFactory->CreateImageReader2( fname.c_str() );
    readerFactory->Delete();
    reader->SetFileName( fname.c_str() );
    reader->Update();
    svkImageData* id = reader->GetOutput(); 
    id->Register(NULL);
    //vtkImageData* tmpImage = reader->vtkImageAlgorithm::GetOutput();
    vtkImageData* tmpImage = id;
    tmpImage->Update();

    reader->Delete();
    cout<<"id reference count = "<<id->GetReferenceCount()<<endl;
    reader = NULL;
    cout<<"tmpImage: "<<*tmpImage<<endl;
    cout<<"Header: "<<*static_cast<svkImageData*>(tmpImage)->GetDcmHeader()<<endl;
    vtkUnstructuredGrid* uGrid = vtkUnstructuredGrid::New();
    static_cast<svkMrsImageData*>(tmpImage)->GenerateSelectionBox( uGrid );
    cout << *uGrid << endl;
    cout<<"GOODBYE TEST !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!"<<endl;
    cout << "MAGNITUDE RANGE IS: " << magRange[0] << " " << magRange[1] << endl;
    id->Delete(); 
    id = NULL;
    
    return 0; 
}


