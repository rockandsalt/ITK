/*=========================================================================

  Program:   Insight Segmentation & Registration Toolkit
  Module:    ShapeDetectionLevelSetFilter.cxx
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

  Copyright (c) 2002 Insight Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

// Software Guide : BeginLatex
//
// The following example illustrates the use of the
// \doxygen{ShapeDetectionLevelSetImageFilter}.  The implementation of this filter
// in ITK is based on the paper by Malladi et al\cite{Malladi1995}.
// In this implementation, the governing differential equation has an additional
// curvature based term. This term act as a smoothing term, where areas of
// high curvature, assumed to be due to noise, is smoothed out. Scaling parameters
// are used to control the tradeoff between the expansion term and smoothing term.
// One consequent of this additional curvature term is that the fast marching
// algorithm is no longer applicable because the contour is no longer guaranteed
// to be always expanding. Instead, the level set function is updated iteratively.
//
// The \doxygen{ShapeDetectionLevelSetImageFilter} expects two inputs, 
// the first is an initial Level Set in the form of an
// \doxygen{Image}, the second input is an feature image. For this algorithm,
// the feature image is an edge potential image that basically
// follows the same rules applicable to the speed image used for the
// \doxygen{FastMarchingImageFilter} discussed in
// section~\ref{sec:FastMarchingImageFilter}. 
//
// In this example we use an \doxygen{FastMarchingImageFilter} for producing
// the initial LevelSet as the the distance funcition to a set of user-provided
// seeds. The \doxygen{FastMarchingImageFilter} is run with its option for a
// constant speed value which allows to use this filter as a distance map
// calculator.
//
// \begin{figure} \center
// \includegraphics[width=15cm]{ShapeDetectionCollaborationDiagram1.eps}
// \caption[ShapeDetectionLevelSetImageFilter collaboration diagram]{Collaboration
// diagram of the ShapeDetectionLevelSetImageFilter applied to a segmentation task.}
// \label{fig:ShapeDetectionCollaborationDiagram}
// \end{figure}
//
// Figure~\ref{fig:ShapeDetectionCollaborationDiagram} shows the major
// components involved in the application of the
// \doxygen{ShapeDetectionLevelSetImageFilter} to a segmentation task. It involves a
// first stage of smoothing using the
// \doxygen{CurvatureAnisotropicDiffusionImageFilter}. The smoothed image is
// passed as the input for the
// \doxygen{GradientMagnitudeRecursiveGaussianImageFilter} and then to the
// \doxygen{SigmoidImageFilter} in order to produce the Edge Potential image.
// A set of user-provided seeds are passed to a
// \doxygen{FastMarchingImageFilter} in order to compute their distance map. A
// constant value is subtracted from this map in order to obtain a Level Set in
// which the \emph{Zero Set} represents the initial contour. This level set is
// also passed as input to the \doxygen{ShapeDetectionLevelSetImageFilter}.
// 
// Finally the LevelSet at the output of the
// \doxygen{ShapeDetectionLevelSetImageFilter} is passed to a
// \doxygen{BinaryThresholdImageFilter} in order to produce a binary mask
// representing the segmented object.
//
// Let's start by including the headers of the main filters involved in the
// preprocessing. 
//
// Software Guide : EndLatex 

// Software Guide : BeginCodeSnippet
#include "itkCurvatureAnisotropicDiffusionImageFilter.h"
#include "itkGradientMagnitudeRecursiveGaussianImageFilter.h"
#include "itkSigmoidImageFilter.h"
// Software Guide : EndCodeSnippet

//  Software Guide : BeginLatex
//
// The edge potential map is generated using these filter as per the previous 
// example.
//
//  Software Guide : EndLatex 


//  Software Guide : BeginLatex
//  
//  We will need the \doxygen{Image} class, the
//  \doxygen{FastMarchingImageFilter} class and the
//  \doxygen{ShapeDetectionLevelSetImageFilter} class. Hence we include their
//  headers here.
//
//  Software Guide : EndLatex 

// Software Guide : BeginCodeSnippet
#include "itkImage.h"
#include "itkFastMarchingImageFilter.h"
#include "itkShapeDetectionLevelSetImageFilter.h"
// Software Guide : EndCodeSnippet



//  Software Guide : BeginLatex
//  
//  The LevelSet resulting from the \doxygen{ShapeDetectionLevelSetImageFilter} will
//  be thresholded at the Zero level in order to get a binary image
//  representing the segmented object. The \doxygen{BinaryThresholdImageFilter}
//  is used for this purpose.
//
//  Software Guide : EndLatex 

// Software Guide : BeginCodeSnippet
#include "itkBinaryThresholdImageFilter.h"
// Software Guide : EndCodeSnippet




//  
//  Reading and writing images will be done with the \doxygen{FileImageReader}
//  and \doxygen{FileImageWriter}.
//

#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"


// 
//  The RescaleIntensityImageFilter is used to renormailize the output 
//  of filters before sending them to files. 
// 
#include "itkRescaleIntensityImageFilter.h"


int main( int argc, char **argv )
{


  if( argc < 10 )
    {
    std::cerr << "Missing Parameters " << std::endl;
    std::cerr << "Usage: " << argv[0];
    std::cerr << " inputImage  outputImage";
    std::cerr << " seedX seedY InitialDistance";
    std::cerr << " Sigma SigmoidAlpha SigmoidBeta curvatureScaling" << std::endl;
    return 1;
    }




  //  Software Guide : BeginLatex
  //  
  //  We declare now the image type using a pixel type and a particular
  //  dimension. In this case the \code{float} type is used for the pixels due
  //  to the requirements of the smoothing filter. 
  //
  //  Software Guide : EndLatex 

  // Software Guide : BeginCodeSnippet
  typedef   float           InternalPixelType;
  const     unsigned int    Dimension = 2;
  
  typedef itk::Image< InternalPixelType, Dimension >  InternalImageType;
  // Software Guide : EndCodeSnippet





  //  Software Guide : BeginLatex
  //  
  //  The output image, on the other hand, is selected to be binary.
  //
  //  Software Guide : EndLatex 

  // Software Guide : BeginCodeSnippet
  typedef unsigned char OutputPixelType;

  typedef itk::Image< OutputPixelType, Dimension > OutputImageType;
  // Software Guide : EndCodeSnippet





  //  Software Guide : BeginLatex
  //  
  //  The type of the \doxygen{BinaryThresholdImageFilter} filter is
  //  instantiated below using the internal image type and the output image
  //  type.
  //
  //  Software Guide : EndLatex 

  // Software Guide : BeginCodeSnippet
  typedef itk::BinaryThresholdImageFilter< 
                        InternalImageType, 
                        OutputImageType    >    ThresholdingFilterType;
  
  ThresholdingFilterType::Pointer thresholder = ThresholdingFilterType::New();
  // Software Guide : EndCodeSnippet
                        



  //  Software Guide : BeginLatex
  //
  //  The upper threshold of the \doxygen{BinaryThresholdImageFilter} is set up
  //  to $0.0$ in order to display the zero set of the resulting level
  //  set. The lower threshold is set to a large negative number in order to
  //  ensure that all the interior of the segmented object will appear in the
  //  inside of the binary region.
  //
  //  Software Guide : EndLatex 

  // Software Guide : BeginCodeSnippet
  thresholder->SetLowerThreshold( -1000.0 );
  thresholder->SetUpperThreshold(     0.0 );

  thresholder->SetOutsideValue(  0  );
  thresholder->SetInsideValue(  255 );
  // Software Guide : EndCodeSnippet




  //  
  // We instantiate reader and writer types in the following lines.
  //

  typedef  itk::ImageFileReader< InternalImageType > ReaderType;
  typedef  itk::ImageFileWriter<  OutputImageType  > WriterType;


  ReaderType::Pointer reader = ReaderType::New();
  WriterType::Pointer writer = WriterType::New();

  reader->SetFileName( argv[1] );
  writer->SetFileName( argv[2] );



  //
  //  The RescaleIntensityImageFilter type is declared below. This filter will
  //  renormalize image before sending them to writers.
  //
  typedef itk::RescaleIntensityImageFilter< 
                               InternalImageType, 
                               OutputImageType >   CastFilterType;

  //  Software Guide : BeginLatex
  //  
  //  
  //  The \doxygen{CurvatureAnisotropicDiffusionImageFilter} type is
  //  instantiated using the internal image type. 
  //
  //  Software Guide : EndLatex 

  // Software Guide : BeginCodeSnippet
  typedef   itk::CurvatureAnisotropicDiffusionImageFilter< 
                               InternalImageType, 
                               InternalImageType >  SmoothingFilterType;
  // Software Guide : EndCodeSnippet




  //  Software Guide : BeginLatex
  //  
  //  Then, the filter is created by invoking the \code{New()} method and
  //  assigning the result to a \doxygen{SmartPointer}.
  //
  //  Software Guide : EndLatex 

  // Software Guide : BeginCodeSnippet
  SmoothingFilterType::Pointer smoothing = SmoothingFilterType::New();
  // Software Guide : EndCodeSnippet


  //  Software Guide : BeginLatex
  //  
  //  The types of the \doxygen{GradientMagnitudeRecursiveGaussianImageFilter} and
  //  \doxygen{SigmoidImageFilter} are instantiated using the internal
  //  image type. 
  //
  //  Software Guide : EndLatex 

  // Software Guide : BeginCodeSnippet
  typedef   itk::GradientMagnitudeRecursiveGaussianImageFilter< 
                               InternalImageType, 
                               InternalImageType >  GradientFilterType;

  typedef   itk::SigmoidImageFilter<                               
                               InternalImageType, 
                               InternalImageType >  SigmoidFilterType;
  // Software Guide : EndCodeSnippet




  //  Software Guide : BeginLatex
  //  
  //  The corresponding filter objects are created with the method
  //  \code{New()}.
  //
  //  Software Guide : EndLatex 

  // Software Guide : BeginCodeSnippet
  GradientFilterType::Pointer  gradientMagnitude = GradientFilterType::New();

  SigmoidFilterType::Pointer sigmoid = SigmoidFilterType::New();
  // Software Guide : EndCodeSnippet


  //  Software Guide : BeginLatex
  //
  //  The minimum and maximum values of the \doxygen{SigmoidImageFilter} output
  //  are defined with the methods \code{SetOutputMinimum()} and
  //  \code{SetOutputMaximum()}. In our case, we want these two values to be
  //  $0.0$ and $1.0$ respectively in order to get a nice speed image to feed
  //  the \code{FastMarchingImageFilter}. Additional details on the user of the
  //  \doxygen{SigmoidImageFilter} are presented in
  //  section~\ref{sec:IntensityNonLinearMapping}.
  //
  //  Software Guide : EndLatex 

  // Software Guide : BeginCodeSnippet
  sigmoid->SetOutputMinimum(  0.0  );
  sigmoid->SetOutputMaximum(  1.0  );
  // Software Guide : EndCodeSnippet




  //  Software Guide : BeginLatex
  //  
  //  We declare now the type of the \doxygen{FastMarchingImageFilter} that
  //  will be used to generate the initial level set in the form of a distance
  //  map.
  //
  //  Software Guide : EndLatex 

  // Software Guide : BeginCodeSnippet
  typedef  itk::FastMarchingImageFilter< 
                              InternalImageType, 
                              InternalImageType >    FastMarchingFilterType;
  // Software Guide : EndCodeSnippet





  //  Software Guide : BeginLatex
  //  
  //  then, we  construct one filter of this class using the \code{New()} method. 
  //
  //  Software Guide : EndLatex 

  // Software Guide : BeginCodeSnippet
  FastMarchingFilterType::Pointer  fastMarching = FastMarchingFilterType::New();
  // Software Guide : EndCodeSnippet



  //  Software Guide : BeginLatex
  //  
  //  In the following lines we instantiate the type of the
  //  \doxygen{ShapeDetectionLevelSetImageFilter} and create an object of this type
  //  using the \code{New()} method.
  //
  //  Software Guide : EndLatex 

  // Software Guide : BeginCodeSnippet
  typedef  itk::ShapeDetectionLevelSetImageFilter< 
                              InternalImageType, 
                              InternalImageType >    ShapeDetectionFilterType;

  ShapeDetectionFilterType::Pointer shapeDetection = ShapeDetectionFilterType::New();                              
  // Software Guide : EndCodeSnippet




  //  Software Guide : BeginLatex
  //  
  //  The filters are now connected in a pipeline indicated in
  //  Figure~\ref{fig:ShapeDetectionCollaborationDiagram} using the following
  //  lines. 
  //
  //  Software Guide : EndLatex 

  // Software Guide : BeginCodeSnippet
  smoothing->SetInput( reader->GetOutput() );

  gradientMagnitude->SetInput( smoothing->GetOutput() );

  sigmoid->SetInput( gradientMagnitude->GetOutput() );

  
  shapeDetection->SetInput( fastMarching->GetOutput() );
  shapeDetection->SetFeatureImage( sigmoid->GetOutput() );

  thresholder->SetInput( shapeDetection->GetOutput() );

  writer->SetInput( thresholder->GetOutput() );
  // Software Guide : EndCodeSnippet




  //  Software Guide : BeginLatex
  //
  //  The \doxygen{CurvatureAnisotropicDiffusionImageFilter} requires a couple
  //  of parameter to be defined. The following are typical values for $2D$
  //  images. However they may have to be adjusted depending on the amount of
  //  noise present in the input image. This filter has been discussed in
  //  section~\ref{sec:GradientAnisotropicDiffusionImageFilter}.
  //
  //  Software Guide : EndLatex 

  // Software Guide : BeginCodeSnippet
  smoothing->SetTimeStep( 0.125 );
  smoothing->SetNumberOfIterations(  5 );
  smoothing->SetConductanceParameter( 3.0 );
  // Software Guide : EndCodeSnippet




  //  Software Guide : BeginLatex
  //
  //  The \doxygen{GradientMagnitudeRecursiveGaussianImageFilter} performs the
  //  equivalent of a convolution with a Gaussian kernel, followed by a
  //  derivative operator. The sigma of this Gaussian can be used to control
  //  the range of influence of the image edges. This filter has been discussed
  //  in section~\ref{sec:GradientMagnitudeRecursiveGaussianImageFilter}
  //
  //  \index{itk::GradientMagnitudeRecursiveGaussianImageFilter!SetSigma()}
  //
  //  Software Guide : EndLatex 

  const double sigma = atof( argv[6] );

  // Software Guide : BeginCodeSnippet
  gradientMagnitude->SetSigma(  sigma  );
  // Software Guide : EndCodeSnippet



  //  Software Guide : BeginLatex
  //
  //  The \doxygen{SigmoidImageFilter} requires two parameters that define the
  //  linear transformation to be applied to the sigmoid argument. This
  //  parameters have been discussed in sections~\ref{sec:IntensityNonLinearMapping}
  //  and \ref{sec:FastMarchingImageFilter}.
  //
  //  Software Guide : EndLatex 

  const double alpha =  atof( argv[7] );
  const double beta  =  atof( argv[8] );


  // Software Guide : BeginCodeSnippet
  sigmoid->SetAlpha( alpha );
  sigmoid->SetBeta(  beta  );
  // Software Guide : EndCodeSnippet



  
  //  Software Guide : BeginLatex
  //
  //  The \doxygen{FastMarchingImageFilter} requires the user to provide a seed
  //  point from which the level set will be generated. The user can actually
  //  pass not only one seed point but a set of them. Note the the
  //  \doxygen{FastMarchingImageFilter} is used here only as a helper in the
  //  determination of an initial Level Set. We could have used the
  //  \doxygen{DanielssonDistanceMapImageFilter} in the same way.
  //
  //  \index{itk::FastMarchingImageFilter!Multiple seeds}
  //
  //  The seeds are passed stored in a container. The type of this
  //  container is defined as \code{NodeContainer} among the
  //  \doxygen{FastMarchingImageFilter} traits.
  //
  //  \index{itk::FastMarchingImageFilter!Nodes}
  //  \index{itk::FastMarchingImageFilter!NodeContainer}
  //  \index{itk::FastMarchingImageFilter!NodeType}
  //
  //  Software Guide : EndLatex 

  //  Software Guide : BeginCodeSnippet
  typedef FastMarchingFilterType::NodeContainer           NodeContainer;
  typedef FastMarchingFilterType::NodeType                NodeType;

  NodeContainer::Pointer seeds = NodeContainer::New();
  //  Software Guide : EndCodeSnippet 
  

  InternalImageType::IndexType  seedPosition;
  
  seedPosition[0] = atoi( argv[3] );
  seedPosition[1] = atoi( argv[4] );


  //  Software Guide : BeginLatex
  //
  //  Nodes are created as stack variables and initialized with a value and an
  //  \doxygen{Index} position. Note that here we assign the value of minus the
  //  user-provided distance to the unique node of the seeds passed to the
  //  \doxygen{FastMarchingImageFilter}. In this way, the value will increment
  //  as the front is propagated, until it reaches the zero value corresponding
  //  to the contour. After this, the front will continue propagating until it
  //  fills up the entire image. The initial distance is taken here from the
  //  command line arguments. The rule of thumb for the user is to select this
  //  value as the distance from the seed points at which he want the initial
  //  contour to be.
  //
  //  \index{itk::FastMarchingImageFilter!Seed initialization}
  //
  //  Software Guide : EndLatex 

  const double initialDistance = atof( argv[5] );

  // Software Guide : BeginCodeSnippet
  NodeType node;

  const double seedValue = - initialDistance;
  
  node.SetValue( seedValue );
  node.SetIndex( seedPosition );
  // Software Guide : EndCodeSnippet



  //  Software Guide : BeginLatex
  //
  //  The list of nodes is initialized and then every node is inserted using
  //  the \code{InsertElement()}.
  //
  //  Software Guide : EndLatex 

  //  Software Guide : BeginCodeSnippet
  seeds->Initialize();

  seeds->InsertElement( 0, node );
  //  Software Guide : EndCodeSnippet 




  //  Software Guide : BeginLatex
  //
  //  The set of seed nodes is passed now to the
  //  \doxygen{FastMarchingImageFilter} with the method
  //  \code{SetTrialPoints()}.
  //
  //  \index{itk::FastMarchingImageFilter!SetTrialPoints()}
  //
  //  Software Guide : EndLatex 

  // Software Guide : BeginCodeSnippet
  fastMarching->SetTrialPoints(  seeds  );
  // Software Guide : EndCodeSnippet




  //  Software Guide : BeginLatex
  //  
  //  Since the \doxygen{FastMarchingImageFilter} is used here just as a
  //  distance map generator. It does not require a speed image as input.
  //  Instead the constant value $1.0$ is passed using the
  //  \code{SetSpeedConstant()} method.
  //
  //  Software Guide : EndLatex 

  //  Software Guide : BeginCodeSnippet
  fastMarching->SetSpeedConstant( 1.0 );
  //  Software Guide : EndCodeSnippet 


  //
  //  Here we configure all the writers required to see the intermediate
  //  outputs of the pipeline. This is added here only for
  //  pedagogical/debugging purposes. These intermediate output are normaly not
  //  required. Only the output of the final thresholding filter should be
  //  relevant.  Observing intermediate output is helpful in the process of
  //  fine tunning the parameters of filters in the pipeline. 
  //
  //
  CastFilterType::Pointer caster1 = CastFilterType::New();
  CastFilterType::Pointer caster2 = CastFilterType::New();
  CastFilterType::Pointer caster3 = CastFilterType::New();
  CastFilterType::Pointer caster4 = CastFilterType::New();

  WriterType::Pointer writer1 = WriterType::New();
  WriterType::Pointer writer2 = WriterType::New();
  WriterType::Pointer writer3 = WriterType::New();
  WriterType::Pointer writer4 = WriterType::New();

  caster1->SetInput( smoothing->GetOutput() );
  writer1->SetInput( caster1->GetOutput() );
  writer1->SetFileName("ShapeDetectionLevelSetFilterOutput1.png");
  caster1->SetOutputMinimum(   0 );
  caster1->SetOutputMaximum( 255 );
  writer1->Update();

  caster2->SetInput( gradientMagnitude->GetOutput() );
  writer2->SetInput( caster2->GetOutput() );
  writer2->SetFileName("ShapeDetectionLevelSetFilterOutput2.png");
  caster2->SetOutputMinimum(   0 );
  caster2->SetOutputMaximum( 255 );
  writer2->Update();

  caster3->SetInput( sigmoid->GetOutput() );
  writer3->SetInput( caster3->GetOutput() );
  writer3->SetFileName("ShapeDetectionLevelSetFilterOutput3.png");
  caster3->SetOutputMinimum(   0 );
  caster3->SetOutputMaximum( 255 );
  writer3->Update();

  caster4->SetInput( fastMarching->GetOutput() );
  writer4->SetInput( caster4->GetOutput() );
  writer4->SetFileName("ShapeDetectionLevelSetFilterOutput4.png");
  caster4->SetOutputMinimum(   0 );
  caster4->SetOutputMaximum( 255 );
  



  //  Software Guide : BeginLatex
  //
  //  The \doxygen{FastMarchingImageFilter} requires the user to specify the
  //  size of the image to be produced as output. This is done using the
  //  \code{SetOutputSize()}. Note that the size is obtained here from the
  //  output image of the smoothing filter. The size of this image is valid
  //  only after the \code{Update()} methods of this filter has been called
  //  directly or indirectly.
  //
  //  Software Guide : EndLatex 

  // Software Guide : BeginCodeSnippet
  fastMarching->SetOutputSize( 
           reader->GetOutput()->GetBufferedRegion().GetSize() );
  // Software Guide : EndCodeSnippet


  //  Software Guide : BeginLatex
  //  
  //  \doxygen{ShapeDetectionLevelSetImageFilter} provides two parameters to
  //  control the competition between the propagation or expansion term and the curvature
  //  smoothing term. The methods \code{SetPropagationScaling()} and
  //  \code{SetCurvatureScaling()} defines the relative weighting between
  //  the two terms. In this example, we will set the propagation scaling to one
  //  and let the curvature scaling be an input argument. The larger the 
  //  the curvature scaling parameter the smoother the resulting segmentation.
  //  However, the curvature scaling parameter should not be set too large either
  //  as it will draw the contour away from the shape boundaries.
  //
  //  \index{itk::ShapeDetectionLevelSetImageFilter!SetPropagationScaling()}
  //  \index{itk::SegmentationLevelSetImageFilter!SetPropagationScaling()}
  //  \index{itk::ShapeDetectionLevelSetImageFilter!SetCurvatureScaling()}
  //  \index{itk::SegmentationLevelSetImageFilter!SetCurvatureScaling()}
  //
  //  Software Guide : EndLatex 

  const double curvatureScaling = atof( argv[ 9] ); 
  //  Software Guide : BeginCodeSnippet
  shapeDetection->SetPropagationScaling(  1.0 );
  shapeDetection->SetCurvatureScaling( curvatureScaling ); 
  //  Software Guide : EndCodeSnippet 




  //  Software Guide : BeginLatex
  //  
  //  Once activiated the level set evolution stop if the convergence criteria has
  //  been reached or if the maximum number of iterations has elasped.
  //  The convergence criteria is defined in terms of the root mean squared (RMS)
  //  change in the level set function. The evolution is said to have converged
  //  if the RMS change is below a user specified threshold.
  //  In a real application is desirable to couple the evolution of the
  //  zero set to a visualization module allowing the user to follow the
  //  evolution of the zero set. With this feedback, the user may decide when
  //  to stop the algorithm before the zero set leaks through the regions of
  //  low gradient in the contour of the anatomical structure to be segmented.
  //
  //  \index{itk::ShapeDetectionLevelSetImageFilter!SetMaximumRMSError()}
  //  \index{itk::ShapeDetectionLevelSetImageFilter!SetMaximumIterations()}
  //  \index{itk::SegmentationLevelSetImageFilter!SetMaximumRMSError()}
  //  \index{itk::SegmentationLevelSetImageFilter!SetMaximumIterations()}
  //
  //  Software Guide : EndLatex 

  //  Software Guide : BeginCodeSnippet
  shapeDetection->SetMaximumRMSError( 0.02 );
  shapeDetection->SetMaximumIterations( 800 );
  //  Software Guide : EndCodeSnippet 


  
  //  Software Guide : BeginLatex
  //  
  //  The invokation of the \code{Update()} method on the writer triggers the
  //  execution of the pipeline.  As usual, the call is placed in a
  //  \code{try/catch} block should any errors ocurr and exceptions are thrown.
  //
  //  Software Guide : EndLatex 

  // Software Guide : BeginCodeSnippet
  try
    {
    writer->Update();
    }
  catch( itk::ExceptionObject & excep )
    {
    std::cerr << "Exception caught !" << std::endl;
    std::cerr << excep << std::endl;
    }
  // Software Guide : EndCodeSnippet

  // Print out some useful information 
  std::cout << std::endl;
  std::cout << "Max. no. iterations: " << shapeDetection->GetMaximumIterations() << std::endl;
  std::cout << "Max. RMS error: " << shapeDetection->GetMaximumRMSError() << std::endl;
  std::cout << std::endl;
  std::cout << "No. elpased iterations: " << shapeDetection->GetElapsedIterations() << std::endl;
  std::cout << "RMS change: " << shapeDetection->GetRMSChange() << std::endl;

  writer4->Update();

  //
  // The following writer type is used to save the output of the time-crossing
  // map in a file with apropiate pixel representation. The advantage of saving
  // this image in native format is that it can be used with a viewer to help
  // determine an appropriate threshold to be used on the output of the
  // fastmarching filter.
  //
  typedef itk::ImageFileWriter< InternalImageType > InternalWriterType;

  InternalWriterType::Pointer mapWriter = InternalWriterType::New();
  mapWriter->SetInput( fastMarching->GetOutput() );
  mapWriter->SetFileName("ShapeDetectionLevelSetFilterOutput4.mha");
  mapWriter->Update();

  InternalWriterType::Pointer speedWriter = InternalWriterType::New();
  speedWriter->SetInput( sigmoid->GetOutput() );
  speedWriter->SetFileName("ShapeDetectionLevelSetFilterOutput3.mha");
  speedWriter->Update();


  InternalWriterType::Pointer gradientWriter = InternalWriterType::New();
  gradientWriter->SetInput( gradientMagnitude->GetOutput() );
  gradientWriter->SetFileName("ShapeDetectionLevelSetFilterOutput2.mha");
  gradientWriter->Update();



  //  Software Guide : BeginLatex
  //
  //  Let's now run this example using as input the image
  //  \code{BrainProtonDensitySlice.png} provided in the directory
  //  \code{Insight/Examples/Data}. We can easily segment the major anatomical
  //  structures by providing seeds in the appropriate locations. The following
  //  table presents the parameters used for some structures.
  //
  //  \begin{center}
  //  \begin{tabular}{|l|c|c|c|c|c|c|c|}
  //  \hline
  //  Structure    & Seed Index & Distance & $\sigma$ & $\alpha$ & $\beta$ & Curvature Scaling & Output Image \\
  //  \hline
  //  Left Ventricle  & $(81,114)$ & 5.0 & 1.0 & -0.5 & 3.0  & 0.05 & First  in Figure \ref{fig:ShapeDetectionLevelSetFilterOutput2} \\ 
  //  Right Ventricle & $(99,114)$ & 5.0 & 1.0 & -0.5 & 3.0  & 0.05 & Second in Figure \ref{fig:ShapeDetectionLevelSetFilterOutput2} \\ 
  //  White matter    & $(56, 92)$ & 5.0 & 1.0 & -0.3 & 2.0  & 0.05 & Third  in Figure \ref{fig:ShapeDetectionLevelSetFilterOutput2} \\ 
  //  Gray matter     & $(40, 90)$ & 5.0 & 0.5 & -0.3 & 2.0  & 0.05 & Fourth in Figure \ref{fig:ShapeDetectionLevelSetFilterOutput2} \\ 
  //  \hline
  //  \end{tabular}
  //  \end{center}
  //
  //  Figure~\ref{fig:ShapeDetectionLevelSetFilterOutput} presents the
  //  intermediate outputs of the pipeline illustrated in
  //  Figure~\ref{fig:ShapeDetectionCollaborationDiagram}. They are from left
  //  to right: the output of the anisotropic diffusing filter, the gradient
  //  magnitude of the smoothed image and the sigmoid of the gradient magnitude
  //  which is finally used as the edge potential for the
  //  \doxygen{ShapeDetectionLevelSetImageFilter}.
  //
  //  Notice that in Figure~\ref{fig:ShapeDetectionLevelSetFilterOutput2} the
  //  segmented shapes are rounder than the results in
  //  Figure~\ref{fig:FastMarchingImageFilterOutput2} due to the effects of the
  //  curvature term in the driving equation. As with the previous example,
  //  segmentation of the gray matter is still problematic.
  //
  // \begin{figure} \center
  // \includegraphics[width=6cm]{BrainProtonDensitySlice.eps}
  // \includegraphics[width=6cm]{ShapeDetectionLevelSetFilterOutput1.eps}
  // \includegraphics[width=6cm]{ShapeDetectionLevelSetFilterOutput2.eps}
  // \includegraphics[width=6cm]{ShapeDetectionLevelSetFilterOutput3.eps}
  // \caption[ShapeDetectionLevelSetImageFilter intermediate output]{Images generated by
  // the segmentation process based on the ShapeDetectionLevelSetImageFilter. From left
  // to right and top to bottom: Input image to be segmented, image smoothed with an
  // edge-preserving smoothing filter, gradient magnitude of the smoothed
  // image, sigmoid of the gradient magnitude. This last image, the sigmoid, is
  // used to compute the speed term for the front propagation }
  // \label{fig:ShapeDetectionLevelSetFilterOutput}
  // \end{figure}
  //
  //  A larger number of iterations is reguired for segmenting large structures
  //  since it takes longer for the front to propagate and cover the region to
  //  be segmented. This drawback can be easily mitigated by setting
  //  many seed points in the initialization of the
  //  \doxygen{FastMarchingImageFilter}. This will generate an initial level
  //  set much closer in shape to the object to be segmented and hence
  //  requiring less iterations to fill in and reach out the edges of the
  //  anatomical structure.
  //
  //
  // \begin{figure} \center
  // \includegraphics[width=4cm]{ShapeDetectionLevelSetFilterOutput5.eps}
  // \includegraphics[width=4cm]{ShapeDetectionLevelSetFilterOutput6.eps}
  // \includegraphics[width=4cm]{ShapeDetectionLevelSetFilterOutput7.eps}
  // \includegraphics[width=4cm]{ShapeDetectionLevelSetFilterOutput8.eps}
  // \caption[ShapeDetectionLevelSetImageFilter segmentations]{Images generated by the
  // segmentation process based on the ShapeDetectionLevelSetImageFilter. From left to
  // right: Segmentation of the left ventricle, segmentation of the right
  // ventricle, segmentation of the white matter, attempt of segmentation of
  // the gray matter.}
  // \label{fig:ShapeDetectionLevelSetFilterOutput2}
  // \end{figure}
  //
  //  Software Guide : EndLatex 




  return 0;

}




