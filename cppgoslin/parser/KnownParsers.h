#ifndef KNOWN_PARSERS_H
#define KNOWN_PARSERS_H


#include "cppgoslin/parser/GoslinFragmentParserEventHandler.h"
#include "cppgoslin/parser/GoslinParserEventHandler.h"

class GoslinParser : public Parser<LipidAdduct*> {
public:
    GoslinParser();
    ~GoslinParser();
};


class GoslinFragmentParser : public Parser<LipidAdduct*> {
public:
    GoslinFragmentParser();
    ~GoslinFragmentParser();
};

#endif /* KNOWN_PARSERS_H */
