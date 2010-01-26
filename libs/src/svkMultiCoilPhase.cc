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
 */



#include <svkMultiCoilPhase.h>


using namespace svk;


vtkCxxRevisionMacro(svkMultiCoilPhase, "$Rev$");
vtkStandardNewMacro(svkMultiCoilPhase);


svkMultiCoilPhase::svkMultiCoilPhase()
{

#if VTK_DEBUG_ON
    this->DebugOn();
#endif

    vtkDebugMacro(<<this->GetClassName() << "::" << this->GetClassName() << "()");
    this->phaseAlgo = svkPhaseSpec::New();
}


svkMultiCoilPhase::~svkMultiCoilPhase()
{
    if (this->phaseAlgo != NULL) {
        this->phaseAlgo->Delete(); 
        this->phaseAlgo = NULL; 
    }
}



/*! 
 *
 */
int svkMultiCoilPhase::RequestInformation( vtkInformation* request, vtkInformationVector** inputVector, vtkInformationVector* outputVector )
{
    return 1;
}


/*! 
 *  Algorithm needs to 
 *      1.  a)either find the optimal phase from the central voxels in the volume from each coil and average it 
 *          or
 *          b) average the central voxels.. no, destructive interference prevents addition prior to phasing!.. do a)
 *      2.  to find the phase we first need to find the peak:  
 *          Phase using the water peak (predefined ppm range)
 *      3.  method for phasing = TBD, see Sarah Code. 
 *      4.  use phasing algorithm already implemented to apply the phases to the specified coil (bundle the vtkPhaseSpec call inside 
 *          this class.
 *
 *      for each of central voxels: 
 *      Find peak from mag spec and calc its phase: use smoothed magnitude spectrum to find maximum point (sub est_peakmagmax)
 *              get phase angle at max pos: a0 = 180.0 * atan2(aimag(r(imax)),real(r(imax)))/3.14159
 *              now estimate the peak sym: 
 *              call est_peaksym(ncol,r,rback,s1pos,s2pos,imax,a1,nsmooth) -> see csi_subs_v6.f
 *      
 */
int svkMultiCoilPhase::RequestData( vtkInformation* request, vtkInformationVector** inputVector, vtkInformationVector* outputVector )
{

    //  Iterate through spectral data from all cells.  Eventually for performance I should do this by visible
    svkImageData* data = this->GetImageDataInput(0); 

    int numFrequencyPoints = data->GetCellData()->GetNumberOfTuples();
    int numComponents = data->GetCellData()->GetNumberOfComponents();
    int numChannels  = data->GetDcmHeader()->GetNumberOfCoils();

    int numVoxels[3]; 
    data->GetNumberOfVoxels(numVoxels); 

    this->phaseAlgo->SetInput( data );
    
    for( int channel = 0; channel < numChannels; channel++ ) { 
        for (int z = 0; z < numVoxels[2]; z++) {
            for (int y = 0; y < numVoxels[1]; y++) {
                for (int x = 0; x < numVoxels[0]; x++) {

                    vtkFloatArray* spectrum = static_cast<vtkFloatArray*>(
                                            svkMrsImageData::SafeDownCast(data)->GetSpectrum( x, y, z, 0, channel ) );

                    int h2oPeakPos = this->FindMagnitudeSpecPeak( spectrum ); 

                    //  Need to implement dynamic peak width determiniation.   hardcode for now: 
                    float phase = this->PhaseBySymmetry( spectrum, h2oPeakPos, h2oPeakPos - 10, h2oPeakPos + 10 ); 

                    //cout << "PEAK MAX PT: " << x << " " << y << " " << z << " " << channel << " -> " 
                    ////        << h2oPeakPos << " Phase: " << phase << endl; 

                    //  Apply algo to data from reader:
                    int index[3]; 
                    index[0] = x; 
                    index[1] = y; 
                    index[2] = z; 

                    svkPhaseSpec* pa = svkPhaseSpec::New();
                    pa->SetInput(data); 
                    pa->SetUpdateExtent(index, index); 
                    pa->SetChannel( channel ); 
                    pa->SetPhase0( phase ); 
                    pa->Update( ); 
                    pa->Delete(); 

                    // SetPhase is relative to the previous value.  Probably need to add a SetPhase, vs 
                    // SetAbsolutePhase method to get this to work in this context. otherwise adjacent
                    //  voxels only get a relative phase applied. 
                    //this->phaseAlgo->SetUpdateExtent( index, index );
                    //this->phaseAlgo->SetChannel( channel );
                    //this->phaseAlgo->SetPhase0( 0. );
                    //this->phaseAlgo->SetPhase0( phase );
                    //this->phaseAlgo->Update();
                }
            }
        }
    }

    //  Trigger observer update via modified event:
    this->GetInput()->Modified();
    this->GetInput()->Update();
    return 1; 
} 



/*!
 *  Returns the pt location of the peak in the smoothed magnitude spectrum. 
 */
int svkMultiCoilPhase::FindMagnitudeSpecPeak( vtkFloatArray* spectrum, int smoothSize ) 
{

    svkImageData* data = this->GetImageDataInput(0); 
    int numFrequencyPoints  = data->GetCellData()->GetNumberOfTuples();

    //  Create a magnitude spectrum from which the peak will be found 
    float* mag = new float[numFrequencyPoints]; 
    for( int i = 0; i < numFrequencyPoints; i++ ) {
        mag[i] = svkSpecUtils::GetMagnigutude(spectrum, i);
    }

    //  Smooth it:    
        //todo
        //csmoothdiff(n,c,cback,csmooth,nsmooth)

    //

    int peakPtPos = 0;  
    float peakHtMax = mag[0]; 
    for( int i = 1; i < numFrequencyPoints; i++ ) {
        if ( mag[i] > peakHtMax ) {
            peakHtMax = mag[i]; 
            peakPtPos = i;  
        }
    }
    //cout << "MAG MAX: " << peakHtMax << " pt: " << peakPtPos << endl;

    delete [] mag; 

    return peakPtPos;
}


/*!
 *  Returns the phase of the peak at the specified position determined by
 *  maximizing the real component symmetry: left and right side should have equivalent
 *  positive integrals. 
 */
float svkMultiCoilPhase::PhaseBySymmetry( vtkFloatArray* spectrum, int peakMaxPtIn, int peakStartPtIn, int peakStopPtIn) 
{

    svkImageData* data = this->GetImageDataInput(0); 
    int numFrequencyPoints  = data->GetCellData()->GetNumberOfTuples();

    float phase = 0.; 

    //  Smooth the data (todo):
    //  call csmoothdiff(n,c,cback,csmooth,nsmooth)
    //  Get peak ht of smoothed spectrum
    
    int peakMaxPt = peakMaxPtIn; 
    float peakMagMax = svkSpecUtils::GetMagnigutude(spectrum, peakMaxPtIn);
    for (int i = peakStartPtIn; i < peakStopPtIn; i++ ) { 
        float magValue = svkSpecUtils::GetMagnigutude(spectrum, i);
        if ( magValue > peakMagMax ) {
            peakMagMax = magValue; 
            peakMaxPt = i;  
        } 
    } 
    int peakShift = peakMaxPt - peakMaxPtIn; 
    int peakStartPt = peakStartPtIn + peakShift;  
    int peakStopPt  = peakStopPtIn  + peakShift;  
    //cout << "peak range: " << peakStartPt << " - " << peakStopPt << endl;

    float leftIntegral = 0.;  
    float rightIntegral = 0.;  

    //  Vary the phase, theta, from -180 to 180 and 
    //  recalculate the peak symmetry at each step: 
    float rlAssymetry = 1e30; 

    if ( peakStartPt > 0 && peakStopPt < numFrequencyPoints ) {

        for (int theta = -180; theta < 180; theta++) {

            leftIntegral = 0.0; 
            rightIntegral = 0.0; 
            float phasedPoint[2]; 

            for ( int i = peakStartPt; i < peakMaxPt; i++ ) { 
                svkSpecUtils::PhaseSpectrum(spectrum, (float)theta, i, phasedPoint);
                leftIntegral += phasedPoint[0]; 
            } 

            for ( int i = peakMaxPt+ 1; i < peakStopPt; i++ ) { 
                svkSpecUtils::PhaseSpectrum(spectrum, (float)theta, i, phasedPoint);
                rightIntegral += phasedPoint[0]; 
            } 

            svkSpecUtils::PhaseSpectrum(spectrum, (float)theta, peakMaxPt, phasedPoint);
            float phasedPeakHt = phasedPoint[0]; 

            if( phasedPeakHt > 0.0 ) {
                if( fabs(leftIntegral - rightIntegral) < rlAssymetry &&
                    leftIntegral > 0.0 && rightIntegral > 0.0 )
                {
                    rlAssymetry = fabs(leftIntegral - rightIntegral); 
                    phase = theta;  
                } 
            } 
            //cout << "RLA: " << theta << " " << rlAssymetry << endl;
        }
    }

    return phase; 
}


/*!
 *
 */
int svkMultiCoilPhase::FillInputPortInformation( int vtkNotUsed(port), vtkInformation* info )
{
    info->Set(vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(), "svkMrsImageData"); 
    return 1;
}


