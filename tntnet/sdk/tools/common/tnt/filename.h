/*  tnt/filename.h
   Copyright (C) 2006 Tommi Maekitalo

This file is part of tntnet.

Tntnet is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

Tntnet is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with tntnet; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330,
Boston, MA  02111-1307  USA
*/

#ifndef TNT_FILENAME_H
#define TNT_FILENAME_H

#include <string>

namespace tnt
{
  class Filename
  {
      std::string path;
      std::string basename;
      std::string ext;

    public:
      Filename(const std::string& filename);

      void setPath(const std::string& p)
      {
        path = p;
        if (p.size() == 0 || p.at(p.size()) != '/')
          path += '/';
      }

      const std::string& getPath() const       { return path; }

      void setBasename(const std::string& b)   { basename = b; }
      const std::string& getBasename() const   { return basename; }

      void setExt(const std::string& e)
      {
        ext = e;
        if (e.size() > 0 && e.at(0) != '.')
          ext.insert(0, 1, '.');
      }
      const std::string& getExt() const        { return ext; }

      std::string getFilename() const          { return basename + ext; }
      std::string getFullPath() const          { return path + basename + ext; }
  };
}

#endif // TNT_FILENAME_H
