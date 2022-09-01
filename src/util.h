//
// Created by Alexander Taepper on 01.09.22.
//
#include <string>
#include <iostream>
#include <sstream>

[[maybe_unused]] static std::string getPangoPrefix(const std::string &pango_lineage){
   std::string pangoPref;
   if(pango_lineage.size() > 2){
      std::stringstream ss(pango_lineage);
      if(!getline(ss, pangoPref, '.')){
         std::cerr << "Non-covered case of pango lineage!" << std::endl;
         return "Not-recognized";
      }
   }
   else{
      pangoPref = pango_lineage;
   }
   return pangoPref;
}
