#ifndef DEBUG_H
#define DEBUG_H

// Comment out this line to bring in or out debugging log statements
// By making it a preprocessor - it includes it excludes or includes the code at build
#define DEBUG 1

// https://stackoverflow.com/a/3371577/774741
#ifdef DEBUG
#define DEBUG_MSG(str) do { std::cout << str; } while( false )
#else
#define DEBUG_MSG(str) do { } while ( false )
#endif

#endif