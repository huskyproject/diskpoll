/* This circumvents some problems with egcs pgcc from Andrew Zabolotny */
/* His standard C++ library is missing the following two functions.    */
/* As they are only important when using multithreading, I simply re-  */
/* placed them with stub functions. */

extern "C" void _IO_flockfile(void)
{}

extern "C" void _IO_funlockfile(void)
{}
