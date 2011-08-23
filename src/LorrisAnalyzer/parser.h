#ifndef PARSER_H
#define PARSER_H

#include "lorrisanalyzer.h"

class Parser
{
public:
    Parser();
    ~Parser();

    void setStructure(analyzer_packet structure) { m_structure = structure; }

private:
    analyzer_packet m_structure;
};

#endif // PARSER_H
