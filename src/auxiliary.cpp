#include "LipidSpace/auxiliary.h"
#include <random>
#include <algorithm>
#pragma warning(push, 0)
#include <ogdf/planarity/BoyerMyrvold.h>
#include <ogdf/basic/simple_graph_alg.h>
#pragma warning(pop)



LaWeCSE::LaWeCSE()
{
    numlabels = static_cast<labelType>(simpleLabelToString.size());
    
    m_labelFunction.sameNodeLabel = 1.0;
    m_labelFunction.sameEdgeLabel = 1.0;
    
    m_labelFunction.differentNodeLabel = 0.01;
    m_labelFunction.differentEdgeLabel = 0.01;
}



InputGraph* LaWeCSE::makeGraph(string SMILES){
    const Molecule &mol = callback.molecule;
    Parser<MoleculeSmilesCallback> parser(callback);
    
    InputGraph *newIG=new InputGraph();
    Graph *newGraph = &newIG->graph;
    NodeArray<labelType> *newNASimple = new NodeArray<labelType>(*newGraph);
    newIG->nodeLabel=newNASimple;
    EdgeArray<labelType> *newEASimple = new EdgeArray<labelType>(*newGraph, 0);
    newIG->edgeLabel=newEASimple;
    
    parser.parse(SMILES);
    
    node n,nQueue;
    adjEntry adj;
    edge e;
    
    cout << endl << mol.atoms.size() << endl;
    node *fognode = new node[mol.atoms.size()];
    
    for (int i = 0; i < mol.atoms.size(); ++i){
        n = fognode[i] = newGraph->newNode();
        string node_edge_label = std::to_string(mol.atoms[i].element);
        auto it = stringLabelToSimpleLabel.find(node_edge_label);
        if (it == stringLabelToSimpleLabel.end())
        {
            stringLabelToSimpleLabel.insert(pair<string,labelType> (node_edge_label,numlabels));
            (*newNASimple)[n]=numlabels;
            ++numlabels;
            simpleLabelToString.push_back(node_edge_label);
        }
        else
            (*newNASimple)[n] = it->second;
    }
    
    for (int i = 0; i < mol.bonds.size(); ++i)
    {
        e = newGraph->newEdge(fognode[mol.bonds[i].source], fognode[mol.bonds[i].target]);

        string node_edge_label = std::to_string(mol.bonds[i].order);
        auto it = stringLabelToSimpleLabel.find(node_edge_label);
        if (it == stringLabelToSimpleLabel.end())
        {
            stringLabelToSimpleLabel.insert(pair<string,labelType> (node_edge_label,numlabels));
            (*newEASimple)[e]=numlabels;
            ++numlabels;
            simpleLabelToString.push_back(node_edge_label);
        }
        else
            (*newEASimple)[e]=it->second;

    }
    delete[]fognode;
    makeSimpleUndirected(*newGraph);
    
    if (newGraph->numberOfNodes() > 1){
        unsigned unconnectedParts=0;
        NodeArray<bool> bfs(*newGraph,false);
        queue<node> bfsQ;
        node firstNode = newGraph->firstNode();
        bfsQ.push(firstNode);
        forall_nodes(n,*newGraph)
        {
        // all the vertices of one connection component
            while (!bfsQ.empty())
            {
                nQueue=bfsQ.front();
                bfs[nQueue]=true;
                forall_adj(adj,nQueue)
                    if (bfs[adj->twinNode()]==false)
                        bfsQ.push(adj->twinNode());
                bfsQ.pop();
            }
            if (bfs[n]==false) // new unconnected vertex
            {
                // connect it with first node, and mark edge as not compatible
                bfsQ.push(n);
                e=newGraph->newEdge(firstNode,n);
                (*newEASimple)[e]=LABEL_NOT_COMPATIBLE;
                ++unconnectedParts;
            }
        }
    }
    
    forall_nodes(n, *newGraph){
        auto it = m_labelFunction.weightTable.find(LABEL_MULTIPLYER * (*newNASimple)[n] + (*newNASimple)[n]);
        if (it == m_labelFunction.weightTable.end()) // individual entry not found
            newIG->size += m_labelFunction.sameNodeLabel;
        else
            newIG->size += it->second; // individual entry
    }
    
    forall_edges(e, *newGraph){
        auto it = m_labelFunction.weightTable.find(LABEL_MULTIPLYER * (*newEASimple)[e] + (*newEASimple)[e]);
        if (it == m_labelFunction.weightTable.end()) // individual entry not found
            newIG->size += m_labelFunction.sameEdgeLabel;
        else
            newIG->size += it->second; // individual entry
    }
    
    return newIG;
}


void LaWeCSE::computeSimilarity(InputGraph* firstGraph, InputGraph* secondGraph, int* values){
    {
        BBP_MCSI compC(m_labelFunction, *firstGraph, *secondGraph, false, false, &simpleLabelToString, -1, WEIGHT_NOT_COMPATIBLE);
        compC.computeIsomorphism();
        values[0] = firstGraph->size + secondGraph->size - compC.getSize();
        values[1] = compC.getSize();
    }
}
