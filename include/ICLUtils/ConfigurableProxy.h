/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2012 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : include/ICLUtils/ConfigurableProxy.h                   **
** Module : ICLUtils                                               **
** Authors: Viktor Richter                                         **
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

#pragma once

#include <ICLUtils/Configurable.h>

namespace icl{
  namespace utils{

    /// This class provides the getter and setter methods of an internally set Configurable.
    class ConfigurableProxy{
      private:
        Mutex m_configurableLock;
        Configurable* m_intConfigurable;

      public:

        /// virtual destructor
        virtual ~ConfigurableProxy(){}

        /// Constructor with passed internal Configurable
        ConfigurableProxy(Configurable* c=NULL) : m_intConfigurable(c){}

        /// sets the internally used Configurable to the passed one
        void setInternalConfigurable(Configurable* c=NULL){
          Mutex::Locker l(m_configurableLock);
          m_intConfigurable = c;
        }

        /// returns the internally used Configurable
        Configurable* getInternalConfigurable(){
          Mutex::Locker l(m_configurableLock);
          ICLASSERT_THROW(m_intConfigurable,ICLException("ConfigurableProxy: internal Configurable is null"));
          return m_intConfigurable;
        }

        /// sets a property value
        void setPropertyValue(const std::string &propertyName, const Any &value) throw (ICLException){
          getInternalConfigurable() -> setPropertyValue(propertyName, value);
        }

        /// returns a list of All properties, that can be set using setProperty
        std::vector<std::string> getPropertyList(){
          return getInternalConfigurable() -> getPropertyList();
        }

        /// base implementation for property check (seaches in the property list)
        bool supportsProperty(const std::string &propertyName){
          return getInternalConfigurable() -> supportsProperty(propertyName);
        }

        /// writes all available properties into a file
        void saveProperties(const std::string &filename, const std::vector<std::string> &propertiesToSkip=Configurable::EMPTY_VEC){
          getInternalConfigurable() -> saveProperties(filename, propertiesToSkip);
        }

        /// reads a camera config file from disc
        void loadProperties(const std::string &filename,const std::vector<std::string> &propertiesToSkip=Configurable::EMPTY_VEC){
          getInternalConfigurable() -> loadProperties(filename, propertiesToSkip);
        }

        /// get type of property
        std::string getPropertyType(const std::string &propertyName){
          return getInternalConfigurable() -> getPropertyType(propertyName);
        }

        /// get information of a properties valid values
        std::string getPropertyInfo(const std::string &propertyName){
          return getInternalConfigurable() -> getPropertyInfo(propertyName);

        }

        /// returns the current value of a property or a parameter
        Any getPropertyValue(const std::string &propertyName){
          return getInternalConfigurable() -> getPropertyValue(propertyName);
        }

        /// returns the tooltip description for a given property
        std::string getPropertyToolTip(const std::string &propertyName) {
          return getInternalConfigurable() -> getPropertyToolTip(propertyName);
        }

        /// Returns whether this property may be changed internally
        int getPropertyVolatileness(const std::string &propertyName){
          return getInternalConfigurable() -> getPropertyVolatileness(propertyName);
        }
    };
  } // namespace utils
}

