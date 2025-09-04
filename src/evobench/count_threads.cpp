#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

namespace evobench {

int count_threads() {
   char path[256];
   snprintf(path, sizeof(path), "/proc/%d/task", getpid());

   int count = 0;
   DIR* dir = opendir(path);
   if (!dir) {
      perror("count_threads: opendir");
      abort();
   }
   struct dirent* entry;

   while ((entry = readdir(dir)) != NULL) {
      if (entry->d_name[0] != '.')
         count++;
   }

   closedir(dir);
   return count;
}
}  // namespace evobench
