// Do not include directly, syntax is not complete! This file is both
// Rust and C++ compatible!

// Keep in sync with `point_kind_name` in evobench.cpp! XX better solution?

pub enum PointKind {
   /// Point at process init -- XX necessary?
   TStart,
   /// Individual (unpaired) point
   T,
   /// Point at the start of a scope
   TS,
   /// Point at the end of a scope
   TE,
   /// Point at thread start
   TThreadStart,
   /// Point at thread exit
   TThreadEnd,
   /// Point at process exit (benchmark always end with this
   /// message, except if there was an IO error).  XXX
   TEnd,
   /// Point directly after flushing the buffer for the current thread.
   TIO,
}
