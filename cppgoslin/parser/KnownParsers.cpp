#include "cppgoslin/parser/KnownParsers.h"

GoslinParser::GoslinParser() : Parser<LipidAdduct*>(new GoslinParserEventHandler(), GrammarString(goslin_grammar), DEFAULT_QUOTE){
        
}


GoslinParser::~GoslinParser(){
    delete parser_event_handler;
}


GoslinFragmentParser::GoslinFragmentParser() : Parser<LipidAdduct*>(new GoslinFragmentParserEventHandler(), GrammarString(goslin_fragment_grammar), DEFAULT_QUOTE){
        
}


GoslinFragmentParser::~GoslinFragmentParser(){
    delete parser_event_handler;
}


LipidMapsParser::LipidMapsParser() : Parser<LipidAdduct*>(new LipidMapsParserEventHandler(), GrammarString(lipid_maps_grammar), DEFAULT_QUOTE){
        
}

LipidMapsParser::~LipidMapsParser(){
    delete parser_event_handler;
}


SwissLipidsParser::SwissLipidsParser() : Parser<LipidAdduct*>(new LipidMapsParserEventHandler(), GrammarString(swiss_lipids_grammar), DEFAULT_QUOTE){
        
}

SwissLipidsParser::~SwissLipidsParser(){
    delete parser_event_handler;
}



LipidParser::LipidParser(){
    parser_list.push_back(new GoslinParser());
    parser_list.push_back(new GoslinFragmentParser());
    parser_list.push_back(new LipidMapsParser());
    parser_list.push_back(new SwissLipidsParser());
}

LipidParser::~LipidParser(){
    for (auto parser : parser_list) delete parser;
}
    
    
LipidAdduct* LipidParser::parse(string lipid_name){
    
    for (auto parser : parser_list) {
        LipidAdduct *lipid = parser->parse(lipid_name);
        if (lipid){
            return lipid;
        }
    }
    return NULL;
}
