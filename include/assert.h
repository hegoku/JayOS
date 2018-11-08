#ifndef	_ASSERT_H_
#define	_ASSERT_H_

#define ASSERT
#ifdef ASSERT
#define assert(exp)  if (exp)
#else
#define assert(exp)
#endif

#endif