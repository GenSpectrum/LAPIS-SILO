#include <silo/file_reader/file_reader.h>

void silo::FileReader::reset() {
   in_file.getInputStream().clear();                  // clear fail and eof bits
   in_file.getInputStream().seekg(0, std::ios::beg);  // g pointer back to the start
}
