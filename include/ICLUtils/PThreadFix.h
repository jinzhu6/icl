/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2012 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : include/ICLUtils/PThreadFix.h                          **
** Module : ICLUtils                                               **
** Authors: Christof Elbrechter                                    **
**                                                                 **
**                                                                 **
** Commercial License                                              **
** ICL can be used commercially, please refer to our website       **
** www.iclcv.org for more details.                                 **
**                                                                 **
** GNU General Public License Usage                                **
** Alternatively, this file may be used under the terms of the     **
** GNU General Public License version 3.0 as published by the      **
** Free Software Foundation and appearing in the file LICENSE.GPL  **
** included in the packaging of this file.  Please review the      **
** following information to ensure the GNU General Public License  **
** version 3.0 requirements will be met:                           **
** http://www.gnu.org/copyleft/gpl.html.                           **
**                                                                 **
** The development of this software was supported by the           **
** Excellence Cluster EXC 277 Cognitive Interaction Technology.    **
** The Excellence Cluster EXC 277 is a grant of the Deutsche       **
** Forschungsgemeinschaft (DFG) in the context of the German       **
** Excellence Initiative.                                          **
**                                                                 **
*********************************************************************/


#ifndef ICL_PTHREAD_FIX_H
#define ICL_PTHREAD_FIX_H

#if 0
#if   defined(HAVE_IPP) || defined(HAVE_MKL)
#include <ICLUtils/Timer.h>
namespace icl{
  static icl::Timer ICL_STATIC_FIX_TO_AVOID_UNDEFINED_REFERENCE_TO_PTHREAD_ATFORK;
  struct ICL_PThreadAtForkFix{
    ICL_PThreadAtForkFix();
    static ICL_PThreadAtForkFix fix;
  };
}

// for some reason, this symbol is missing, and we cannot
// tell the linker to find it ??
#define EXPLICITLY_INSTANTIATE_PTHREAD_AT_FORK                  \
  extern "C"{                                                   \
    extern int pthread_atfork(void (*)(void), void (*)(void),   \
                              void (*)(void)){                  \
    }                                                           \
  }
#else
#define EXPLICITLY_INSTANTIATE_PTHREAD_AT_FORK
#endif

#endif

#endif