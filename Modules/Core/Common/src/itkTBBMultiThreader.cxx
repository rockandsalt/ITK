/*=========================================================================
 *
 *  Copyright Insight Software Consortium
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *         http://www.apache.org/licenses/LICENSE-2.0.txt
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 *=========================================================================*/

#include "itkTBBMultiThreader.h"
#include "itkNumericTraits.h"
#include "itkProcessObject.h"
#include <iostream>
#include <atomic>
#include <thread>
#include "tbb/parallel_for.h"

namespace itk
{

TBBMultiThreader::TBBMultiThreader()
{
  m_SingleMethod = nullptr;
  m_SingleData = nullptr;

  m_NumberOfThreads = std::max(1u, GetGlobalDefaultNumberOfThreads());
}

TBBMultiThreader::~TBBMultiThreader()
{
}

void TBBMultiThreader::SetSingleMethod(ThreadFunctionType f, void *data)
{
  m_SingleMethod = f;
  m_SingleData   = data;
}

void TBBMultiThreader::SingleMethodExecute()
{
  if( !m_SingleMethod )
    {
    itkExceptionMacro(<< "No single method set!");
    }

  //we request grain size of 1 and simple_partitioner to ensure there is no chunking
  tbb::parallel_for(tbb::blocked_range<int>(0, m_NumberOfThreads, 1), [&](tbb::blocked_range<int>r)
    {
    // Make sure that TBB did not call us with a block of "threads"
    // but rather with only one "thread" to handle
    itkAssertInDebugAndIgnoreInReleaseMacro(r.begin() + 1 == r.end());

    ThreadInfoStruct ti;
    ti.ThreadID = r.begin();
    ti.UserData = m_SingleData;
    ti.NumberOfThreads = m_NumberOfThreads;
    m_SingleMethod(&ti); //TBB takes care of properly propagating exceptions
    },
      tbb::simple_partitioner());
}

void TBBMultiThreader::PrintSelf(std::ostream & os, Indent indent) const
{
  Superclass::PrintSelf(os, indent);
}

void
TBBMultiThreader
::ParallelizeArray(
  SizeValueType firstIndex,
  SizeValueType lastIndexPlus1,
  ArrayThreadingFunctorType aFunc,
  ProcessObject* filter )
{
  if (filter)
    {
    filter->UpdateProgress(0.0f);
    }

  if ( firstIndex + 1 < lastIndexPlus1 )
    {
    unsigned count = lastIndexPlus1 - firstIndex;
    std::atomic< SizeValueType > progress = 0;
    std::thread::id callingThread = std::this_thread::get_id();
    //we request grain size of 1 and simple_partitioner to ensure there is no chunking
    tbb::parallel_for(
        tbb::blocked_range<SizeValueType>(firstIndex, lastIndexPlus1, 1),
        [&](tbb::blocked_range<SizeValueType>r)
        {
        // Make sure that TBB did not call us with a block of "threads"
        // but rather with only one "thread" to handle
        itkAssertInDebugAndIgnoreInReleaseMacro(r.begin() + 1 == r.end());
        if ( filter && filter->GetAbortGenerateData() )
          {
          std::string msg;
          ProcessAborted e( __FILE__, __LINE__ );
          msg += "AbortGenerateData was called in " + std::string( filter->GetNameOfClass() )
              + " during multi-threaded part of filter execution";
          e.SetDescription(msg);
          throw e;
          }

        aFunc( r.begin() ); //invoke the function

        if ( filter )
          {
          ++progress;
          //make sure we are updating progress only from the thead which invoked us
          if ( callingThread == std::this_thread::get_id() )
            {
            filter->UpdateProgress( float( progress ) / count );
            }
          }
        },
        tbb::simple_partitioner());
    }
  else if ( firstIndex + 1 == lastIndexPlus1 )
    {
    aFunc( firstIndex );
    }
  // else nothing needs to be executed

  if (filter)
    {
    filter->UpdateProgress(1.0f);
    if (filter->GetAbortGenerateData())
      {
      std::string msg;
      ProcessAborted e(__FILE__, __LINE__);
      msg += "AbortGenerateData was called in " + std::string(filter->GetNameOfClass() )
          + " during multi-threaded part of filter execution";
      e.SetDescription(msg);
      throw e;
      }
    }
}

}

namespace
{
struct TBBImageRegionSplitter : public itk::ImageIORegion
{
  static const bool is_splittable_in_proportion = true;
  TBBImageRegionSplitter(const TBBImageRegionSplitter&) = default;
  TBBImageRegionSplitter(const itk::ImageIORegion& region)
      :itk::ImageIORegion(region) //use itk::ImageIORegion's copy constructor
  {}
  TBBImageRegionSplitter(TBBImageRegionSplitter& region, tbb::split)
      :TBBImageRegionSplitter(region, tbb::proportional_split(1, 1)) //delegate to proportional split
  {}

  TBBImageRegionSplitter(TBBImageRegionSplitter& region, tbb::proportional_split p)
  {
    *this = region; //most things will be the same
    for (int d = int(this->GetImageDimension()) - 1; d >= 0; d--) //prefer to split along highest dimension
      {
      if (this->GetSize(d) > 1) //split along this dimension
        {
        size_t myP = (this->GetSize(d) * p.right()) / (p.left() + p.right());
        if (myP == 0)
          {
          ++myP;
          }
        else if (myP == this->GetSize(d))
          {
          --myP;
          }
        this->SetSize(d, myP);
        region.SetSize(d, region.GetSize(d) - myP);
        region.SetIndex(d, myP + region.GetIndex(d));
        return;
        }
      }
    itkGenericExceptionMacro("An ImageIORegion could not be split. Region: " << region);
  }

  bool empty() const
  {
    for (unsigned d = 0; d < this->GetImageDimension(); d++)
      {
      if (this->GetSize(d) == 0)
        {
        return true;
        }
      }
    return false;
  }

  bool is_divisible() const
  {
    for (unsigned d = 0; d < this->GetImageDimension(); d++)
      {
      if (this->GetSize(d) > 1)
        {
        return true;
        }
      }
    return false;
  }

};//TBBImageRegionSplitter struct definition
}//anonymous namespace

namespace itk
{
void TBBMultiThreader
::ParallelizeImageRegion(
    unsigned int dimension,
    const IndexValueType index[],
    const SizeValueType size[],
    ThreadingFunctorType funcP,
    ProcessObject* filter)
{
  if (filter)
    {
    filter->UpdateProgress(0.0f);
    }

  if (m_NumberOfThreads == 1) //no multi-threading wanted
    {
    funcP(index, size);
    }
  else //normal multi-threading
    {
    ImageIORegion region(dimension);
    for (unsigned d = 0; d < dimension; d++)
      {
      region.SetIndex(d, index[d]);
      region.SetSize(d, size[d]);
      }
    TBBImageRegionSplitter regionSplitter = region; //use copy constructor

    std::atomic<SizeValueType> pixelProgress = { 0 };
    SizeValueType totalCount = region.GetNumberOfPixels();
    std::thread::id callingThread = std::this_thread::get_id();

    tbb::parallel_for(regionSplitter, [&](TBBImageRegionSplitter regionToProcess)
      {
      if (filter && filter->GetAbortGenerateData())
        {
        std::string msg;
        ProcessAborted e(__FILE__, __LINE__);
        msg += "AbortGenerateData was called in " + std::string(filter->GetNameOfClass() )
            + " during multi-threaded part of filter execution";
        e.SetDescription(msg);
        throw e;
        }
      funcP(&regionToProcess.GetIndex()[0], &regionToProcess.GetSize()[0]);
      if (filter) //filter is provided, update progress
        {
        SizeValueType pixelCount = regionToProcess.GetNumberOfPixels();
        pixelProgress += pixelCount;
        //make sure we are updating progress only from the thead which invoked filter->Update();
        if (callingThread == std::this_thread::get_id())
          {
          filter->UpdateProgress(float(pixelProgress) / totalCount);
          }
        }
      }); //we implicitly use auto_partitioner for load balancing
    }

  if (filter)
    {
    filter->UpdateProgress(1.0f);
    if (filter->GetAbortGenerateData())
      {
      std::string msg;
      ProcessAborted e(__FILE__, __LINE__);
      msg += "AbortGenerateData was called in " + std::string(filter->GetNameOfClass() )
          + " during multi-threaded part of filter execution";
      e.SetDescription(msg);
      throw e;
      }
    }
}
}
