#ifndef AUXILIARY_H
#define AUXILIARY_H
#include "LipidSpace/global.h"
#include "LipidSpace/bbpmcsi.h"
#pragma warning(push, 0)
#include <ogdf/fileformats/GraphIO.h>
#pragma warning(pop)
#include <thread>
#include <mutex>
#include <condition_variable>
#include <list>
#pragma warning(push, 0)
#include <ogdf/basic/Thread.h>
#pragma warning(pop)
#include "LipidSpace/smiley.h"
#include "LipidSpace/molecule.h"
using namespace Smiley;


class LaWeCSE
{
private:
	LabelFunction m_labelFunction;
	map<string,labelType> stringLabelToSimpleLabel;
	vector<string> simpleLabelToString;
    labelType numlabels;
    MoleculeSmilesCallback callback;

public:
	LaWeCSE();
	double computeSimilarity(InputGraph*, InputGraph*);
    InputGraph* makeGraph(string);
};
#endif
