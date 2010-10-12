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


#ifndef SVK_MRS_IOD_H
#define SVK_MRS_IOD_H


#include <vtkObjectFactory.h>
#include <svkIOD.h>


namespace svk {


using namespace std;


/*! 
 *  Base class of static methods for default MRSIOD initialization 
 *  using svkDcmHeader adapter interface. 
 */
class svkMRSIOD : public svkIOD 
{

    public:

        static svkMRSIOD* New();
        vtkTypeRevisionMacro( svkMRSIOD, svkIOD);

        //  Methods:
        virtual void  InitDcmHeader();
        void          InitMREchoMacro(float TE); 
        void          InitMRAveragesMacro(int numAverages); 
        void          InitMRSpectroscopyFrameTypeMacro(); 
        void          InitFrameAnatomyMacro(); 
        void          InitMRTimingAndRelatedParametersMacro(float tr, float flipAngle); 
        void          InitMRModifierMacro(float inversionTime); 
        void          InitMRTransmitCoilMacro(string coilName, string coilMfg, string coilType); 
        void          InitVolumeLocalizationSeq(float size[3], float center[3], float dcos[3][3]); 


    protected:

        svkMRSIOD();
        ~svkMRSIOD();

        //  Methods:
        void            InitMRSpectroscopyModule();
        void            InitMRSpectroscopyPulseSequenceModule();
        void            InitMRSpectroscopyDataModule();
        virtual void    InitSOPCommonModule(); 

};


}   //svk


#endif //SVK_MRS_IOD_H
