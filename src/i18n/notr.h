#ifndef NOTR_H_
#define NOTR_H_

#define notr(value) (value)
#define qnotr(value) QString (value)
#define qnotrUtf8(value) QString::fromUtf8 (value)

// TODO There should be qnotrCount(count, singular, plural[, none]) function,
// but scripts/find_missing_tr does not handle multi-argument functions

#endif
