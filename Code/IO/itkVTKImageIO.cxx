/*=========================================================================

  Program:   Insight Segmentation & Registration Toolkit
  Module:    itkVTKImageIO.cxx
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

Copyright (c) 2001 Insight Consortium
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

 * Redistributions of source code must retain the above copyright notice,
   this list of conditions and the following disclaimer.

 * Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

 * The name of the Insight Consortium, nor the names of any consortium members,
   nor of any contributors, may be used to endorse or promote products derived
   from this software without specific prior written permission.

  * Modified source versions must be plainly marked as such, and must not be
    misrepresented as being the original software.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDER AND CONTRIBUTORS ``AS IS''
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHORS OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

=========================================================================*/
#include "itkVTKImageIO.h"
#include "png.h"

namespace itk
{

bool VTKImageIO::CanReadFile(const char* file) 
{ 
  return true;
}
  
const std::type_info& VTKImageIO::GetPixelType() const
{
  switch(m_VTKPixelType)
    {
    case UCHAR:
      return typeid(unsigned char);
    case USHORT:
      return typeid(unsigned short);
    case CHAR:
    case SHORT:
    case UINT:
    case INT:
    case ULONG:
    case LONG:
    case FLOAT:
    case DOUBLE:
      {
      itkErrorMacro ("Invalid type: " << m_VTKPixelType << ", only unsigned char and unsigned short are allowed.");
      return this->ConvertToTypeInfo(m_VTKPixelType);      
      }
    default:
      return typeid(unsigned char);
    }
}

  
unsigned int VTKImageIO::GetComponentSize() const
{
  switch(m_VTKPixelType)
    {
    case UCHAR:
      return sizeof(unsigned char);
    case USHORT:
      return sizeof(unsigned short);
    case CHAR:
    case SHORT:
    case UINT:
    case INT:
    case ULONG:
    case LONG:
    case FLOAT:
    case DOUBLE:
      {
      itkErrorMacro ("Invalid type: " << m_VTKPixelType << ", only unsigned char and unsigned short are allowed.");
      return 0;
      }
    }
  return 1;
}

  
void VTKImageIO::Read(void* buffer)
{
}


const double* 
VTKImageIO::GetOrigin() const
{
  return m_Origin;
}


const double* 
VTKImageIO::GetSpacing() const
{
  return m_Spacing;
}


VTKImageIO::VTKImageIO()
{
  this->SetNumberOfDimensions(2);
  m_VTKPixelType = UCHAR;
}

VTKImageIO::~VTKImageIO()
{
}

void VTKImageIO::PrintSelf(std::ostream& os, Indent indent) const
{
  Superclass::PrintSelf(os, indent);
  os << indent << "VTKPixelType " << m_VTKPixelType << "\n";
}

  
  
void VTKImageIO::ReadImageInformation()
{
}


} // end namespace itk
