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

#ifndef SVK_OVERLAY_VIEW_H
#define SVK_OVERLAY_VIEW_H


#include <vtkImageViewer2.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkRenderer.h>
#include <vtkCamera.h>
#include <vtkImageMapToWindowLevelColors.h>
#include <vtkProp3DCollection.h>
#include <vtkAreaPicker.h>
#include <vtkImageActor.h>
#include <vtkObjectFactory.h>
#include <vtkExtractEdges.h>
#include <vtkCellData.h>
#include <vtkPointData.h>
#include <vtkPlane.h>
#include <vtkColorTransferFunction.h>
#include <vtkLookupTable.h>
#include <vtkScalarBarActor.h>

#include <svkOverlayViewController.h>
#include <svkImageViewer2.h>
#include <svkDataView.h>
#include <svkMrsImageData.h>
#include <svkDataValidator.h>
#include <svkObliqueReslice.h>
#include <svkImageMapToColors.h>
#include <svkLookupTable.h>
#include <svkSatBandSet.h>


namespace svk {


// Determines how close to the voxels things are clipped
#define CLIP_TOLERANCE 0.001

class svkDataViewController;
class svkOverlayViewController;

/*!
 *   An implementation of DataView, this class is designed to create a
 *   visualization of the spectroscopy spatial information overlayed on the
 *   anatomical image. It uses a vtkImageViewer2 to present the anatamical data
 *   then the spectroscopy data is added.
 */
class svkOverlayView : public svkDataView
{
    friend class svkOverlayViewController;
    
    public:
        // vtk type revision macro
        vtkTypeRevisionMacro( svkOverlayView, svkDataView );

        static svkOverlayView*       New();
    
        svkOverlayView();
        ~svkOverlayView();

        // Methods:
        virtual void        SetInput( svkImageData* data, int index = 0);
        virtual void        SetSlice(int slice);
        virtual void        SetSlice(int slice, int imageNum);
        virtual void        SetRWInteractor( vtkRenderWindowInteractor* );    
        virtual void        Refresh();
              string        GetDataCompatibility( svkImageData* data, int targetIndex );
        void                TurnOrthogonalImagesOn();
        void                TurnOrthogonalImagesOff();


        //! Enum represents input indecies
        enum DataInputs { 
            MRI = 0, 
            MRS = 1, 
            OVERLAY = 2
        };

        //! Enum represents objects in the scene
        enum PropType {
            VOL_SELECTION = 0, 
            SAT_BANDS,
            SAT_BANDS_OUTLINE,
            SAT_BANDS_PERP1,
            SAT_BANDS_PERP2,
            SAT_BANDS_PERP1_OUTLINE,
            SAT_BANDS_PERP2_OUTLINE,
            PLOT_GRID,
            OVERLAY_IMAGE,
            OVERLAY_IMAGE_BACK,
            COORDINATES,
            COLOR_BAR,
            LAST_PROP = COLOR_BAR
        };

        //! Enum represents interpolation methods for the overlay 
        enum InterpolationType {
            NEAREST = 0,
            LINEAR,
            SINC 
        };

        //! Enum represents renderers in the window
        enum RendererType {
            PRIMARY = 0, 
            MOUSE_LOCATION,
            LAST_RENDERER = MOUSE_LOCATION
        };

        //! Enum represents color schema, used for printing
        enum {
            LIGHT_ON_DARK = 0,
            DARK_ON_LIGHT
        } ColorSchema;



    protected: 
        
        //! the vtkImageViewer2 object used to display the image 
        svkImageViewer2*                imageViewer;   

        //! the top left, bottom right corners of the current view 
        int*                            tlcBrc;

        //! the slice of the current view
        int                             slice;

        //! the render window in which the view is to be displayed 
        vtkRenderWindow*                myRenderWindow;

        //! Object used to window livel the overlay 
        svkImageMapToColors* windowLeveler;

        // Transfer function for rendering overlays
        svkLookupTable*                 colorTransfer;


        // Methods:
        void                            SetupMrInput( bool firstInput );
        void                            SetupMsInput( bool firstInput );
        void                            SetCenterImageSlice( );
        void                            SetSelection( double* selectionArea, bool isWorldCords = 0 );
        void                            SetOverlayOpacity( double opacity );
        void                            SetOverlayThreshold( double threshold );
        void                            SelectActors( int* tlcBrc );
        int*                            HighlightSelectionVoxels();
        void                            GenerateClippingPlanes();
        void                            SetupOverlay();
        void                            SetInterpolationType( int interpolationType );
        void                            SetLUT( svkLookupTable::svkLookupTableType type );
        void                            ResetWindowLevel();
        int                             FindCenterImageSlice( int spectraSlice, int orientation );
        int                             FindSpectraSlice( int imageSlice, int orientation );

    private:

        void                            SetSliceOverlay();
        
        //! Stores the interpolation method of the overlay
        InterpolationType               interpolationType; 

        //! Manipulates the actor that represents the sat bands
        svkSatBandSet*                 satBands;
        svkSatBandSet*                 satBandsPerp1;
        svkSatBandSet*                 satBandsPerp2;

        void                            ResliceImage(svkImageData* input, svkImageData* target, int targetIndex);
        
};


}   //svk


#endif //SVK_OVERLAY_VIEW_H
