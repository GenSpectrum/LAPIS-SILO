#pragma once

// This wrapper is required because roaring is currently not well-behaved when #include'd.
// Importantly, it includes system headers and previously sets preprocessor macro's
// to ensure portability of the included headers.
// However, this prevents other libraries to use features of the detected platform.
// See https://github.com/RoaringBitmap/CRoaring/issues/690

#include <netdb.h>
#include <netinet/in.h>

#include <roaring/roaring.hh>
