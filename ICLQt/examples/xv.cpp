/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2012 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLQt/examples/xv.cpp                                  **
** Module : ICLQt                                                  **
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

#include <iostream>
#include <iterator>
#include <ICLIO/FileGrabber.h>
#include <ICLQuick/Common.h>
#include <QtGui/QDesktopWidget>

//#include <QScreen>

GUI gui;

Size compute_image_size(const std::vector<const ImgBase*> &is, QDesktopWidget *desktop){
  Size s;
  for(unsigned int i=0;i<is.size();++i){
    s.width = iclMax(s.width,is[i]->getWidth());
    s.height = iclMax(s.height,is[i]->getHeight());
  }
  QRect r = desktop->availableGeometry();

  return Size(iclMin(s.width,r.width()-20),iclMin(s.height,r.height()-20));
}

int main (int n, char **ppc){
  pasethelp("icl-xv is ICL's simple image viewer.\n"
            "It can display images of all supported file formats.\n"
            "Furthermore, icl-xv is used for ICLQuick's\n"
            "global icl::show(image) function. Here, the -delete\n"
            "flag is used in order to delete a temporary image\n"
            "after loading it.");
  QApplication app(n,ppc);
  paex
  ("-input","define image to read")
  ("-delete","delete image file after reading")
  ("-roi","if set, image roi is visualized");
  painit(n,ppc,"-input|-i(filename) -delete|-d -roi|-r",true);

  const ImgBase *image = 0;
  if(pa("-input")){
    string imageName = pa("-input");

    try{
      static FileGrabber w(imageName);
      image = w.grab();
      if(pa("-delete")){
        if(imageName.length()){
          int errorCode = system((string("rm -rf ")+imageName).c_str());
          if ( errorCode != 0 )
            WARNING_LOG( "Error code of system call unequal 0!" );
        }
      }
    }catch(ICLException e){
      static ImgQ o = ones(320,240,1)*100;
      fontsize(15);
      text(o, 90,90,"image not found!");
      image = &o;
    }
    if(pacount()){
      std::cout << "Warning if called with -input, all extra given filenames are omitted!" << std::endl;
    }

    Size size = compute_image_size(std::vector<const ImgBase*>(1,image),QApplication::desktop());
    gui << Draw().handle("draw").size(size/20);
    gui.show();

    gui["draw"] = image;
    gui["draw"].render();

  }else if(pacount()){
    if(pa("-delete")){
      std::cout << "-delete flag is not supported when running in multi image mode" << std::endl;
      std::cout << "call iclxv -input ImageName -delete instead (for single images only)" << std::endl;
    }
    std::string imageList = "";
    std::vector<const ImgBase*> imageVec;
    std::vector<std::string> imageVecStrs;
    
    Size maxSize;
    for(unsigned int i=0;i<pacount();++i){
      std::string s = pa(i);
      try{
        FileGrabber grabber(s,false,true);
        const ImgBase *image = grabber.grab();
        if(!image) throw ICLException("");
        maxSize.width = iclMax(image->getWidth(),maxSize.width);
        maxSize.height = iclMax(image->getHeight(),maxSize.height);
        imageVec.push_back(image->deepCopy());

        std::replace_if(s.begin(),s.end(),std::bind2nd(std::equal_to<char>(),','),'-');
        imageList += (imageList.length() ? ",": "") +s;
        imageVecStrs.push_back(s);
      }catch(const ICLException &ex){
        std::cout << "unable to load image: " << s << std::endl;
        std::cout << "(skipping!)" << std::endl;
      }
    }
    Size size = compute_image_size(imageVec,QApplication::desktop());
    Tab t(imageList);

    for(size_t i=0;i<imageVecStrs.size();++i){
      t << Image().handle("image-"+str(i));
    }
    
    gui << t << Show();
    
    for(size_t i=0;i<imageVecStrs.size();++i){
      gui["image-"+str(i)] = imageVec[i];
      ICL_DELETE(imageVec[i]);
    }
  }else{
    throw ICLException("no image given, or none of the given images was found!");
  }

  if(pa("-roi")){
    if(pa("-input")){
      static DrawHandle draw = gui["draw"];
      draw->color(255,0,0);
      draw->fill(0,0,0,0);
      draw->rect(image->getROI().x,image->getROI().y,image->getROI().width,image->getROI().height);
      draw.render();
    }else{
      std::cout << "roi visualization is not supported in multi image mode!" << std::endl;
    }
  }

  return app.exec();
}
