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


LipidMapsParser::LipidMapsParser() : Parser<LipidAdduct*>(new LipidMapsParserEventHandler(), GrammarString(lipid_maps_grammar), Parser<LipidAdduct*>::DEFAULT_QUOTE){
        
}

LipidMapsParser::~LipidMapsParser(){
    delete parser_event_handler;
}



LipidParser::LipidParser(){
    parser_list.push_back(GoslinParser());
    parser_list.push_back(GoslinFragmentParser());
    parser_list.push_back(LipidMapsParser());
}
    
    
LipidAdduct* LipidParser::parse(string lipid_name){
    LipidAdduct* lipid = NULL;
    
    for (auto parser : parser_list) {
        lipid = parser.parse(lipid_name);
        if (lipid){
            break;
        }
    }
    return lipid;
}
