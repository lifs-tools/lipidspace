#include "cppgoslin/parser/KnownParsers.h"


GoslinParser::GoslinParser() : Parser<LipidAdduct*>(new GoslinParserEventHandler(), GrammarString(goslin_grammar), Parser<LipidAdduct*>::DEFAULT_QUOTE){
        
}

GoslinParser::~GoslinParser(){
    delete parser_event_handler;
}

GoslinFragmentParser::GoslinFragmentParser() : Parser<LipidAdduct*>(new GoslinFragmentParserEventHandler(), GrammarString(goslin_fragment_grammar), Parser<LipidAdduct*>::DEFAULT_QUOTE){
        
}

GoslinFragmentParser::~GoslinFragmentParser(){
    delete parser_event_handler;
}
