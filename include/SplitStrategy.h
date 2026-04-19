#ifndef SPLIT_STRATEGY_H
#define SPLIT_STRATEGY_H

#include "Split.h"

#include <string>
#include <vector>

using namespace std;

class SplitStrategy {
public:
    virtual vector<Split> calculateSplit(double totalAmount, vector<string> userIDs, vector<double> values = {}) = 0;
    virtual ~SplitStrategy() = default;
};

class EqualSplit : public SplitStrategy {
public:
    vector<Split> calculateSplit(double totalAmount, vector<string> userIDs, vector<double> values = {}) override;
};

class ExactSplit : public SplitStrategy {
public:
    vector<Split> calculateSplit(double totalAmount, vector<string> userIDs, vector<double> values = {}) override;
};

class PercentageSplit : public SplitStrategy {
public:
    vector<Split> calculateSplit(double totalAmount, vector<string> userIDs, vector<double> values = {}) override;
};

#endif
