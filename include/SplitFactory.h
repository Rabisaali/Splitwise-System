#ifndef SPLIT_FACTORY_H
#define SPLIT_FACTORY_H

#include "SplitStrategy.h"
#include "SplitType.h"

using namespace std;

class SplitFactory {
public:
    static SplitStrategy* getSplitStrategy(SplitType type);
};

#endif
