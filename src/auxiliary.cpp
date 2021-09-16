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
    
    labelFunction.sameNodeLabel = 1.0;
    labelFunction.sameEdgeLabel = 1.0;
    
    labelFunction.differentNodeLabel = 0.01;
    labelFunction.differentEdgeLabel = 0.01;
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
    
    return newIG;
}


// enum simCoefficients { SC_WallisEtAl, SC_BunkeShearer, SC_Asymmetric, SC_NormalizedJohnson, SC_Johnson, SC_SokalSneath, SC_Kulczynski, SC_McConnaughey };
wType ReadGraphDB::computeSimilarity(wType wMCS, wType wG, wType wH, SimCoefficient simCoefficient)
{
    switch(simCoefficient)
    {
    case SimCoefficient::SC_WallisEtAl:
        return (wMCS / (wG + wH - wMCS));
    case SimCoefficient::SC_BunkeShearer:
        return (wMCS / max(wG, wH));
    case SimCoefficient::SC_Asymmetric:
        return (wMCS / min(wG, wH));
    case SimCoefficient::SC_NormalizedJohnson:
        return (2 * wMCS / (wG + wH));
    case SimCoefficient::SC_Johnson:
        return (wMCS * wMCS / (wG * wH));
    case SimCoefficient::SC_SokalSneath:
        return (wMCS / (2 * wG + 2 * wH - 3 * wMCS));
    case SimCoefficient::SC_Kulczynski:
        return ((wMCS * (wG + wH)) / (2 * wG * wH ));
    case SimCoefficient::SC_McConnaughey:
        return ((wG * wMCS + wH * wMCS - wG * wH) / (wG * wH));
    default:
        cerr << "Similarity Coefficient unknown.\n";
        return 0;
    }
}


int ReadGraphDB::readFile(const char* filename)
{
    
    
    node n,nQueue;
    adjEntry adj;
    edge e;
    //size_t graphIndex=0
    //size_t graphIndex=inputGraph.size();
    unsigned graphDBIndex=0; // index in current DB
    unsigned smallgraphs=0;
    InputGraph* oneToX_First=nullptr;
    unsigned totalbbpcomps=0;
    random_device rd;
    mt19937 rng(rd());
    //size_t graph_removal=graphIndexStart;

    InputGraph* firstGraph = nullptr;
    InputGraph* secondGraph = nullptr;
    bool firstGraphToDelete = false;

    unsigned num_notConnected=0;
    labelType numlabels=static_cast<labelType>(simpleLabelToString.size());
    //streampos streamp=0;
    if (verbose || !comparedGraphs.is_open())
        cout << "Reading database "<<filename<<"..\n";
    clock_t timestart=clock();
    ofstream fogDB;
    /*if (outerplanarOutputFilename!=nullptr && checkOuterplanarity)
    {
        outerplanarDB.open(outerplanarOutputFilename,ios_base::out);
        if (!outerplanarDB.is_open())
            cout << "Could not open outerplanar database "<<outerplanarOutputFilename<<" for output."<<endl;
    }*/
    if (fogOutputFilename!=nullptr  && fogOutputFilename[0]!=0)
    {
        fogDB.open(fogOutputFilename,ios_base::out);
        if (!fogDB.is_open())
            cerr << "Could not open fog database "<<fogOutputFilename<<" for output."<<endl;
    }
    if ((verbose || !comparedGraphs.is_open()) && m_computationType == ComputationType::COMP_I2) // 2nd file with -i
        cout << "Columns: Position and label of pattern, position and label of molecular graph, |pattern|, |Mol. Gr.|, |MCS|, similarity=|MCS|/(|pattern|+|Mol. Gr.|-|MCS|)" << endl;

    char startofline;
    int numNodes,numEdges,bracketCtr=0,bracketCtrNew=0;
    string gr_label="";
    //while (!graphDB.eof())
    while (!graphDB.eof())// && graphDBIndex < 5)
    //while (!graphDB.eof() && (graphDBIndex<6500 || m_computationType != COMP_OneToX))
    {
        bracketCtr=bracketCtrNew;
        graphDB.getline(line,10999);
        bracketCtrNew=bracketCtr;

        // check bracket structure for gml format
        for (int i=0; line[i]!='\0' ; ++i)
        {
            if (line[i]=='[')
                ++bracketCtrNew;
            else if (line[i]==']')
                --bracketCtrNew;
        }


        if (bracketCtrNew==1) // gml graph area
        {
            // search for "label"
            string linestr=line;
            size_t labelpos = linestr.find("label ");
            if (labelpos != std::string::npos)
            {
                gr_label=linestr.substr(labelpos+6);
                continue;
            }
        }
        sstr << line<<endl;
        if ((bracketCtrNew==0 && bracketCtr>0) || line[0]=='#') // End of graph in gml-Format or start of graph in fog format
        {
            // the graph and the labels
            InputGraph *newIG=new InputGraph();
            // conditions to store the graph in the inputGraph vector
            if ( m_computationType == ComputationType::COMP_I1 // -i pattern file
                || (m_computationType == ComputationType::COMP_I2 && computationThreads==0) ) // -i graph file single threaded
            {
                m_inputGraph.push_back(newIG);
            }
            Graph *newGraph = &newIG->graph;
            NodeArray<labelType> *newNASimple=new NodeArray<labelType>(*newGraph);
            newIG->nodeLabel=newNASimple;
            EdgeArray<labelType> *newEASimple=new EdgeArray<labelType>(*newGraph,0);
            newIG->edgeLabel=newEASimple;
            //if (line[0]==']') // GML
            if (bracketCtrNew==0 && bracketCtr>0) // GML
            {
                graphAttributes.init(*newGraph,GraphAttributes::nodeLabel | GraphAttributes::edgeLabel);
                GraphIO::readGML(graphAttributes,*newGraph,sstr);
                numNodes=newGraph->numberOfNodes();
                newIG->simpleLabel=false;
                if (m_removeMultiEdgesAndSelfLoops) //(removeMultiEdgesAndSelfLoops)
                {
                    makeSimpleUndirected(*newGraph);
                }
                sstr.clear();
                sstr.str(std::string());
            }
            else // fog
            {
                sstr >> startofline;
                if (sstr.eof())
                {
                    cerr << "Fog graph "<<graphDBIndex+1<<" first line incomplete!\n";
                    newGraph->clear();
                    numEdges=0;
                    numNodes=0;
                    goto firstLineIncomplete;
                }
                sstr >> gr_label;
                if (sstr.eof())
                {
                    cerr << "Fog graph "<<graphDBIndex+1<<" first line incomplete!\n";
                    newGraph->clear();
                    numEdges=0;
                    numNodes=0;
                    goto firstLineIncomplete;
                }
                sstr >> numNodes;
                if (sstr.eof())
                {
                    cerr << "Fog graph "<<graphDBIndex+1<<" first line incomplete!\n";
                    newGraph->clear();
                    numEdges=0;
                    numNodes=0;
                    goto firstLineIncomplete;
                }
                sstr >> numEdges;
                if (sstr.eof())
                {
                    cerr << "Fog graph "<<graphDBIndex+1<<" first line incomplete!\n";
                    newGraph->clear();
                    numEdges=0;
                    numNodes=0;
                    goto firstLineIncomplete;
                }
                firstLineIncomplete:
                node *fognode=new node[numNodes];
                graphDB.getline(line,10999);
                sstr.clear();
                sstr.str(std::string());
                sstr <<line;

                string node_edge_label="";
                for (int i=0; i<numNodes; ++i)
                {
                    if (sstr.eof())
                    {
                        cerr << "Fog graph "<<graphDBIndex+1<<"("<<gr_label<<") contains too few vertices!\n";
                        newGraph->clear();
                        numEdges=0;
                        break;
                    }
                    n=fognode[i]=newGraph->newNode();
                    sstr >> node_edge_label;
                    auto it = stringLabelToSimpleLabel.find(node_edge_label);
                    if (it == stringLabelToSimpleLabel.end())
                    {
                        stringLabelToSimpleLabel.insert(pair<string,labelType> (node_edge_label,numlabels));
                        (*newNASimple)[n]=numlabels;
                        ++numlabels;
                        simpleLabelToString.push_back(node_edge_label);
                    }
                    else
                        (*newNASimple)[n]=it->second;
                }
                int source,target;
                edge e;
                graphDB.getline(line,10999);
                sstr.clear();
                sstr.str(std::string());
                sstr <<line;
                for (int i=0; i<numEdges; ++i)
                {
                    if (sstr.eof())
                    {
                        cerr << "Fog graph "<<graphDBIndex+1<<"("<<gr_label<<") contains too few edges!\n";
                        newGraph->clear();
                        break;
                    }
                    sstr>> source;
                    if (source<1 || source>numNodes)
                    {
                        cerr << "Fog graph "<<graphDBIndex+1<<"("<<gr_label<<") edge "<<i+1<<" source out of range!\n";
                        newGraph->clear();
                        break;
                    }
                    if (sstr.eof())
                    {
                        cerr << "Fog graph "<<graphDBIndex+1<<"("<<gr_label<<") contains too few edges!\n";
                        newGraph->clear();
                        break;
                    }
                    sstr>> target;
                    if (target<1 || target>numNodes)
                    {
                        cerr << "Fog graph "<<graphDBIndex+1<<"("<<gr_label<<") edge "<<i+1<<" target out of range!\n";
                        newGraph->clear();
                        break;
                    }
                    if (sstr.eof())
                    {
                        cerr << "Fog graph "<<graphDBIndex+1<<"("<<gr_label<<") contains too few edges!\n";
                        newGraph->clear();
                        break;
                    }
                    e=newGraph->newEdge(fognode[source-1],fognode[target-1]);
                    //sstr>>(*newEASimple)[e];

                    sstr >> node_edge_label;
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
                newIG->simpleLabel=false;
                if (m_removeMultiEdgesAndSelfLoops)
                {
                    makeSimpleUndirected(*newGraph);
                }
                sstr.clear();
                sstr.str(std::string());
                newIG->fogNodeEdgeOffset=1;
            }

            // connect unconnected components
            if (newGraph->numberOfNodes()>1)
            {
                unsigned unconnectedParts=0;
                NodeArray<bool> bfs(*newGraph,false);
                queue<node> bfsQ;
                node firstNode=newGraph->firstNode();
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
                if (unconnectedParts>0)
                    ++num_notConnected;
            }

            newIG->graphIndexToDBIndex=graphDBIndex;

            // store labels, if graph in GML format
            if (line[0]==']')
            {
                forall_nodes(n,*newGraph)
                {
                    auto it= stringLabelToSimpleLabel.find(graphAttributes.label(n));
                    if (it == stringLabelToSimpleLabel.end())
                    {
                        stringLabelToSimpleLabel.insert(pair<string,labelType> (graphAttributes.label(n),numlabels));
                        (*newNASimple)[n]=numlabels;
                        ++numlabels;
                        simpleLabelToString.push_back(graphAttributes.label(n));
                    }
                    else
                        (*newNASimple)[n]=it->second;
                }
                forall_edges(e,*newGraph)
                {
                    if ((*newEASimple)[e]!=LABEL_NOT_COMPATIBLE)
                    {
                        auto it= stringLabelToSimpleLabel.find(graphAttributes.label(e));
                        if (it == stringLabelToSimpleLabel.end())
                        {
                            stringLabelToSimpleLabel.insert(pair<string,labelType> (graphAttributes.label(e),numlabels));
                            (*newEASimple)[e]=numlabels;
                            ++numlabels;
                            simpleLabelToString.push_back(graphAttributes.label(e));
                        }
                        else
                            (*newEASimple)[e]=it->second;
                    }
                }
            }
            // write to fog DB
            if (fogDB.is_open())
            {
        /*		bool keep=true;
                if (shrink>1 && uniform_int_distribution<>(1,shrink)(rng)!=1)
                    keep=false;

                if (keep)
                {
                    if (convertToBC)
                    {
                        BCTree bct(*newGraph);
                        const Graph &BC=bct.bcTree();
                        if (BC.numberOfNodes()>40)
                        {
                            fogDB<< "# "<<graphIndex<<" "<<BC.numberOfNodes()<<" "<<BC.numberOfEdges()<<endl;
                            forall_nodes(n,BC)
                                fogDB << "0 ";
                            fogDB <<endl;
                            forall_edges(e,BC)
                                fogDB << e->source()->index()+1<<" "<<e->target()->index()+1<<" 1 ";
                            fogDB <<endl;
                        }
                    }
                    else
                    {
                        fogDB<< "# "<<graphIndex<<" "<<newGraph->numberOfNodes()<<" "<<newGraph->numberOfEdges()-unconnectedParts<<endl;
                        forall_nodes(n,*newGraph)
                            fogDB << (*newNASimple)[n]<< " ";
                        fogDB <<endl;
                        forall_edges(e,*newGraph)
                            if ((*newEASimple)[e]!=LABEL_NOT_COMPATIBLE)
                                fogDB << e->source()->index()+1<<" "<<e->target()->index()+1<<" "<<(*newEASimple)[e]<< " ";
                        fogDB <<endl;
                    }
                }*/
            }

            if (gr_label=="")
                gr_label="(NO_LABEL)";
            newIG->graphLabel=gr_label;
            gr_label="";

            // Delete empty graphs (and omit calculations)
            if (newGraph->numberOfNodes()==0)
            {
                // check if this empty graph was added to the inputGraph vector
                if (m_inputGraph.size()>0 && m_inputGraph.back()==newIG)
                    m_inputGraph.pop_back();
                delete newIG;
                ++smallgraphs;
            }
            else
            {
                // weight of input graph
                //newIG->size=newGraph->numberOfEdges()+newGraph->numberOfNodes(); // old: vertices+edges
                forall_nodes(n,*newGraph)
                {
                    auto it = m_labelFunction.weightTable.find(LABEL_MULTIPLYER * (*newNASimple)[n] + (*newNASimple)[n]);
                    if (it == m_labelFunction.weightTable.end()) // individual entry not found
                        newIG->size += m_labelFunction.sameNodeLabel;
                    else
                        newIG->size += it->second; // individual entry
                }
                forall_edges(e,*newGraph)
                {
                    auto it = m_labelFunction.weightTable.find(LABEL_MULTIPLYER * (*newEASimple)[e] + (*newEASimple)[e]);
                    if (it == m_labelFunction.weightTable.end()) // individual entry not found
                        newIG->size += m_labelFunction.sameEdgeLabel;
                    else
                        newIG->size += it->second; // individual entry
                }

                if (newGraph->numberOfNodes()==1) // transform single vertex graphs so that BC computation does not fail
                {
                    node nFirst=newGraph->firstNode();
                    node nNotCom=newGraph->newNode();
                    (*newNASimple)[nNotCom]=LABEL_NOT_COMPATIBLE;
                    edge eNotCom=newGraph->newEdge(nFirst,nNotCom);
                    (*newEASimple)[eNotCom]=LABEL_NOT_COMPATIBLE;
                }
                // initialize the graphs of the pattern (first) file; -i parameter
                if (m_computationType == ComputationType::COMP_I1)
                    initVbcAndSingleParts(*newIG);

                // direct compare of two graphs
                else if (m_computationType == ComputationType::COMP_C)
                {
                    // save the graph, if it's one of the two to compare
                    if (graphDBIndex == (unsigned) m_argument1)
                    {
                        m_inputGraph.push_back(newIG);
                    }
                    if (graphDBIndex == (unsigned) m_argument2)
                    {
                        m_inputGraph.push_back(newIG);
                    }
                    if  (graphDBIndex != (unsigned) m_argument1 &&  graphDBIndex != (unsigned) m_argument2) // otherwise delete the graph
                    {
                        delete newIG;
                    }
                    // both graph ids found
                    if (m_inputGraph.size()==2)
                    {
                        {
                            // ensure first graph in vector is first graph specified
                            if (m_inputGraph[0]->graphIndexToDBIndex == (unsigned) m_argument2)
                                swap(m_inputGraph[0],m_inputGraph[1]);
                            BBP_MCSI compC(m_labelFunction,*m_inputGraph[0],*m_inputGraph[1],m_argument3,m_argument4, &simpleLabelToString,m_maxCliqueTime, m_distance_penalty);
                            compC.computeIsomorphism();
                            cout << "Similarity: "<<computeSimilarity(compC.getSize(), m_inputGraph[0]->size ,m_inputGraph[1]->size,m_simCoefficient)<<" between "<<m_inputGraph[0]->graphLabel<<" and "<<m_inputGraph[1]->graphLabel<<endl;
                        }
                        delete m_inputGraph.back();
                        if (m_argument1 != m_argument2)
                        {
                            m_inputGraph.pop_back();
                            delete m_inputGraph.back();
                        }
                        m_inputGraph.clear();
                        graphDB.close();
                        return 0;
                    }

                }

                // one to X comparison, just single threaded currently
                else if (m_computationType == ComputationType::COMP_OneToX)
                {
                    if (graphDBIndex % (m_argument1+1) == 0) // first graph of one to X block
                    {
                        oneToX_First = newIG;
                    }
                    else // not first graph of one to X block
                    {
                        wType weight=0;

                        bbpl.push_back(new BBP_MCSI(m_labelFunction,*oneToX_First,*newIG,false,false, &simpleLabelToString, m_maxCliqueTime, m_distance_penalty));
                        weight=bbpl.back()->computeSize();
                        if (comparedGraphs.is_open())
                        {
// DEBUG
//if (bbpl.back()->getNumberOfUnfinishedCliqueComputations()>0)
//{
                            comparedGraphs << oneToX_First->graphIndexToDBIndex+1 << " "<< oneToX_First->graphLabel << " " << graphDBIndex+1 << " "<< newIG->graphLabel << " "
                                           << oneToX_First->size << " " << newIG->size << " " << weight << " "
                                           //<< weight / (oneToX_First->size + newIG->size -weight);
                                           << computeSimilarity(weight, oneToX_First->size ,newIG->size,m_simCoefficient);
                            if (m_appendCliqueComputationTimeouts)
                                comparedGraphs << " " << bbpl.back()->getNumberOfUnfinishedCliqueComputations();
                            comparedGraphs << endl;
//}
                        }
                        else
                        {
                            cout << oneToX_First->graphIndexToDBIndex+1 << " "<< oneToX_First->graphLabel << " " << graphDBIndex+1 << " "<< newIG->graphLabel << " "
                                 << oneToX_First->size << " " << newIG->size << " " << weight << " "
                                 //<< weight / (oneToX_First->size + newIG->size -weight);
                                 << computeSimilarity(weight, oneToX_First->size ,newIG->size,m_simCoefficient);
                            if (m_appendCliqueComputationTimeouts)
                                cout << " " << bbpl.back()->getNumberOfUnfinishedCliqueComputations();
                            cout << endl;
                        }
                        ++totalbbpcomps;
                        // Delete mcs computations if there are at least maxInMemory in memory
                        if (bbpl.size() >= maxMCSInMemory)
                        {
                            for (auto bbpIt=bbpl.begin(); bbpIt != bbpl.end(); ++bbpIt)
                            {
                                firstGraph=(*bbpIt)->get_ig_G();
                                secondGraph=(*bbpIt)->get_ig_H();
                                delete *bbpIt;
                                // whole block of MCS computations deleted, then also delete first graph of block
                                if (firstGraph->graphIndexToDBIndex+m_argument1 == secondGraph->graphIndexToDBIndex)
                                {
//cout << " F"<< firstGraph->graphIndexToDBIndex+1<<" ";
                                    delete firstGraph;
                                    firstGraphToDelete = false;
                                }
                                else
                                {
                                    firstGraphToDelete = true;
                                }
//cout << " S"<< secondGraph->graphIndexToDBIndex+1<<" ";
                                delete secondGraph;
                            }
                            bbpl.clear();
                        }
                    }
                }
                // -i second file
                else if (m_computationType == ComputationType::COMP_I2)
                {

                    if (computationThreads == 0)
                    {
                        // Compare it to all graphs of the first file
                        for (size_t index1=0; index1<graphIndexStart; ++index1)
                        {
                            wType weight=0;

                            bbpl.push_back(new BBP_MCSI(m_labelFunction,*m_inputGraph[index1],*newIG,false,false, &simpleLabelToString, m_maxCliqueTime, m_distance_penalty));
                            weight=bbpl.back()->computeSize();
            // debug: display non outerplanar graphs
            //if (!newIG->mOuterplanar || !inputGraph[index1]->mOuterplanar)
                //bbpv.back()->computeIsomorphism();
                            if (comparedGraphs.is_open())
                            {
                                comparedGraphs << m_inputGraph[index1]->graphIndexToDBIndex+1 << " "<< m_inputGraph[index1]->graphLabel << " " << graphDBIndex+1 << " "<< newIG->graphLabel << " "
                                               << m_inputGraph[index1]->size << " " << newIG->size << " " << weight << " "
                                               //<< weight / (m_inputGraph[index1]->size + newIG->size -weight) << endl;
                                               << computeSimilarity(weight, m_inputGraph[index1]->size ,newIG->size,m_simCoefficient);
                                if (m_appendCliqueComputationTimeouts)
                                    comparedGraphs << " " << bbpl.back()->getNumberOfUnfinishedCliqueComputations();
                                comparedGraphs << endl;


                            }
                            else
                            {
                                cout << m_inputGraph[index1]->graphIndexToDBIndex+1 << " "<< m_inputGraph[index1]->graphLabel << " " << graphDBIndex+1 << " "<< newIG->graphLabel << " "
                                     << m_inputGraph[index1]->size << " " << newIG->size << " " << weight << " "
                                     //<< weight / (m_inputGraph[index1]->size + newIG->size -weight) << endl;
                                     << computeSimilarity(weight, m_inputGraph[index1]->size ,newIG->size,m_simCoefficient);
                                if (m_appendCliqueComputationTimeouts)
                                    cout << " " << bbpl.back()->getNumberOfUnfinishedCliqueComputations();
                                cout << endl;

                            }
                            // Delete mcs computations if there are at least maxInMemory in memory
                            if (bbpl.size() >= maxMCSInMemory)
                            {
                                for (auto bbpIt=bbpl.begin(); bbpIt != bbpl.end(); ++bbpIt)
                                {
                                    firstGraph=(*bbpIt)->get_ig_G();
                                    secondGraph=(*bbpIt)->get_ig_H();
                                    delete *bbpIt;
                                    // delete second input graph if it was compared to all graphs of pattern file
                                    if (firstGraph->graphIndexToDBIndex == graphIndexStart-1)
                                    {
//cout << " S"<< secondGraph->graphIndexToDBIndex+1<<" ";
                                        delete secondGraph;
                                    }
                                }
                                bbpl.clear();
                            }
                        }
                        totalbbpcomps += graphIndexStart;
                    }
                    else // threaded computing
                    {
                        unique_lock<mutex> lock_cv_input_modify(threadInfo.mut_input_modify);
                        threadInfo.cv_input_modify.wait(lock_cv_input_modify, [this] {return threadInfo.graphsInQueue->size() < readQueueSize; });
                        threadInfo.graphsInQueue->push(newIG);
                        lock_cv_input_modify.unlock();
                        threadInfo.cv_input_modify.notify_one();
                        totalbbpcomps += graphIndexStart;
                    }
                }
                ++graphDBIndex;
            }
        }
    }

    if (m_computationType == ComputationType::COMP_I2 && computationThreads > 0)
    {
        threadInfo.readFinished = true;
        threadInfo.cv_input_modify.notify_all();
        //m_threadInfo.mut_input.unlock();
        for (unsigned t = 0; t < computationThreads; ++t)
            bbp_thread[t].join();
        delete[] bbp_thread;
        delete threadInfo.graphsInQueue;
        // delete pattern vector
/*        for (auto patternIt = threadInfo.patternsV->begin(); patternIt != threadInfo.patternsV->end(); patternIt++)
        {
            delete *patternIt;
        }*/

       /* while (!threadInfo.graphsToDeleteFromMainThread.empty())
        {
            delete threadInfo.graphsToDeleteFromMainThread.front();
            threadInfo.graphsToDeleteFromMainThread.pop();
        }
        OGDF_ALLOCATOR::flushPool();*/
    }

    graphDB.close();
    if (fogDB.is_open())
        fogDB.close();

     // delete remaining graphs;
    if (m_computationType == ComputationType::COMP_OneToX)
    {
        // cout << "OneToX";
        // all computations in bbpl deleted, but pattern graph not
        if (bbpl.empty())
        {
            if (firstGraphToDelete)
                delete firstGraph;
        }
        else
        {
            for (auto bbpIt=bbpl.begin(); bbpIt != bbpl.end(); ++bbpIt)
            {
                firstGraph=(*bbpIt)->get_ig_G();
                secondGraph=(*bbpIt)->get_ig_H();
                delete *bbpIt;
                // whole block of MCS computations deleted or last pair of graphs
                if (firstGraph->graphIndexToDBIndex+m_argument1 == secondGraph->graphIndexToDBIndex
                    || graphDBIndex-1 == secondGraph->graphIndexToDBIndex)
                {
//	cout << " F"<< firstGraph->graphIndexToDBIndex+1<<" ";
                    delete firstGraph;
                }
//	cout << " S"<< secondGraph->graphIndexToDBIndex+1<<" ";
                delete secondGraph;
            }
            bbpl.clear();
        }
        if (graphDBIndex  % (m_argument1 + 1) == 1) // Only the 'one' of oneToX has been read. Now delete it.
        {
            delete oneToX_First;
        }
    }
    else if (m_computationType == ComputationType::COMP_I2)
    {
        if (computationThreads == 0)
        {
            for (auto bbpIt=bbpl.begin(); bbpIt != bbpl.end(); ++bbpIt)
            {
                firstGraph=(*bbpIt)->get_ig_G();
                secondGraph=(*bbpIt)->get_ig_H();
                delete *bbpIt;
                // delete second input graph if it was compared to all graphs of pattern file
                if (firstGraph->graphIndexToDBIndex == graphIndexStart-1)
                {
//cout << " S"<< secondGraph->graphIndexToDBIndex+1<<" ";
                    delete secondGraph;
                }
            }
            bbpl.clear();

        }
        // delete pattern graphs
        for (size_t index1=0; index1<graphIndexStart; ++index1)
        {
            delete m_inputGraph[index1];
        }
        m_inputGraph.clear();
    }

    if (verbose || !comparedGraphs.is_open())
    {
        cout << "All "<<graphDBIndex<< " graphs read from DB.\n";
        if (smallgraphs>0)
            cout << smallgraphs << " graphs were empty or erroneous and therefore omitted.\n";
    }
    //<<num_notConnected<<" graphs are not connected.\n";
    //if (checkOuterplanarity)
    //	cout <<graphIndex-numOuterPlanar<< " graphs are not outerplanar.\n";
    //if (bcheckPlanarity)
    //	cout << graphIndex-numPlanar << " graphs are not planar.\n";
    //else
    //	cout << "Graphs were not checked for outerplanarity. Make sure database contains only outerplanar graphs!\n";
    if (verbose || !comparedGraphs.is_open())
    {
        cout <<"Converted " <<stringLabelToSimpleLabel.size()<<" string labels into simple labels. Total time for reading DB and computations: ";
        showtime(clock()-timestart);
        // showtime(static_cast<time_t>(clock()-timestart));
    }
    if (m_computationType == ComputationType::COMP_I1)
    {
        if (m_inputGraph.size()==0)
        {
            cerr << "File "<<filename<< " contains no graphs.\n";
            return 1;
        }
        else
            m_computationType = ComputationType::COMP_I2;
    }
    else if (m_computationType == ComputationType::COMP_C)
    {
        cerr << "Graph indices out of range. "<<filename<<" contains only "<<graphDBIndex<< " graphs.\n";
        return 1;
    }
    return 0;
}

void createFogStarGraphFile(int start, int end, int step)
{
    ofstream stargraph;
    stargraph.open("stargraph.fog",ios_base::out);
    int index=0;
    while (start<=end)
    {
        if (start<2)
        {
            stargraph << "# "<<index++<<" 2 1\n0 0\n1 2 1\n";
        }
        else
        {
            stargraph << "# "<<index++<<" "<<start<<" "<<start-1<<endl;
            for (int i=0; i<start; ++i) // nodes
                stargraph << "0 ";
            stargraph<<endl;
            for (int i=2; i<=start; ++i) // edges
                stargraph << "1 "<<i<<" 1 ";
            stargraph<<endl;
        }
        start+=step;
    }
    stargraph.close();
}

void createRandomTreeFile(unsigned numTrees, unsigned minsize, unsigned maxsize, unsigned numlabels, unsigned maxdegree, bool twoEvenSizedSets)
{
    if (numlabels<1)
        numlabels=1;
    if (minsize<1)
        minsize=1;
    if (minsize>maxsize && !twoEvenSizedSets)
        maxsize=minsize;
    if (maxdegree<2)
        maxdegree=max(maxsize,minsize);

    random_device rd;
    mt19937 rng(rd());
    ofstream randomtrees;
    randomtrees.open("randomtree.fog",ios_base::out);
    for (unsigned i=0; i<numTrees; ++i)
    {
        vector<unsigned> degree;
        vector<unsigned> nodeid;
        unsigned numNodes;
        if (twoEvenSizedSets)
        {
            i<numTrees/2?numNodes=minsize:numNodes=maxsize;
        }
        else
            numNodes=uniform_int_distribution<>(minsize,maxsize)(rng);

        randomtrees << "# "<<i<<" "<<numNodes<<" "<<numNodes-1<<endl;
        for (unsigned j=0; j<numNodes; ++j) // nodes
        {
            unsigned label=uniform_int_distribution<>(1,numlabels)(rng);
            randomtrees << label << " ";
        }
        randomtrees << endl;
        degree.push_back(0);
        nodeid.push_back(1);
        for (unsigned j=2; j<= numNodes; ++j) // nodes
        {
            // select random node with degree<maxdegree
            unsigned src = uniform_int_distribution<>(0, static_cast<int>(nodeid.size()) - 1)(rng);
            //unsigned src=uniform_int_distribution<>(1,j-1)(rng);
            // randomtrees << src << " "<< j<< " ";
            randomtrees << nodeid[src] << " "<< j<< " ";
            degree.push_back(1);
            nodeid.push_back(j);
            if (degree[src]==maxdegree-1) // no additional vertex allowed
            {
                degree[src]=degree.back();
                nodeid[src]=nodeid.back();
                degree.pop_back();
                nodeid.pop_back();
            }
            else
                ++degree[src];
            unsigned label=uniform_int_distribution<>(1,numlabels)(rng);
            randomtrees << label << " ";
        }
        degree.clear();
        nodeid.clear();
        randomtrees << endl;
    }
    randomtrees.close();
}

void createRandomOuterplanarFile(unsigned numGraphs, unsigned minsize, unsigned maxsize, unsigned numlabels, bool twoEvenSizedSets, int maxBridgeLength, int maxComponentSize, int bridgeProbabilityPercent, int edgeExtendPercent)
{/* OGDF2017
    Graph G;
    random_device rd;
    mt19937 rng(rd());
    ofstream randomOuterplanar;
    randomOuterplanar.open("randomOuterplanar.fog",ios_base::out);

    if (numlabels<1)
        numlabels=1;
    if (minsize<1)
        minsize=1;
    if (minsize>maxsize && !twoEvenSizedSets)
        maxsize=minsize;
    if (numGraphs<1)
        numGraphs=1;

    long totalNodes=0;
    long totalBlockNodes=0;
    long totalEdges=0;
    long totalBlocks=0;


    for (unsigned g=0; g<numGraphs; ++g)
    {
        int numNodes;
        if (twoEvenSizedSets)
        {
            g<numGraphs/2?numNodes=minsize:numNodes=maxsize;
        }
        else
            numNodes=uniform_int_distribution<>(minsize,maxsize)(rng);

        int freeVert = numNodes;
        if (freeVert<1)
            freeVert=1;

        int compSize = maxComponentSize;
        if(compSize < 3)
            compSize = freeVert;

        vector<ogdf::edge> edgeCandidates;
        vector<bool> edgeCandidatesIsBridge;

        G.clear();

        while(freeVert > 0)
        {
            // Decide whether to build a bridge or a component:
            // Build a component, i.e. a ring which can either be attached to the end of a bridge or
            // connected to another component with one common edge:
            if(uniform_int_distribution<>(1,100)(rng) > bridgeProbabilityPercent)
            {
                int compSize = uniform_int_distribution<>(3,max(3,maxComponentSize))(rng);
                // Nothing generated yet:
                ogdf::node start;
                if(edgeCandidates.empty() && G.empty())
                {
                    // Component size
                    if (compSize>freeVert)
                        compSize=max(3,freeVert);

                    start = G.newNode();
                    ogdf::node cur = start;

                    for(int j = 1; j < compSize; j++)
                    {
                        ogdf::node next = G.newNode();
                        edgeCandidates.push_back(G.newEdge(cur, next));
                        edgeCandidatesIsBridge.push_back(false);

                        cur = next;
                    }

                    edgeCandidates.push_back(G.newEdge(cur, start));
                    edgeCandidatesIsBridge.push_back(false);
                    freeVert -= compSize;
                    ++totalBlocks;
                    totalBlockNodes += compSize;
                }
                else
                {
                    // Decide whether to choose an edge or a node as root:
                    if (uniform_int_distribution<>(1,100)(rng) > edgeExtendPercent) //true -> node select
                    {
                        // Component size
                        if (compSize>freeVert+1)
                            compSize=max(3,freeVert+1);

                        start = G.chooseNode();
                        ogdf::node cur = start;

                        for(int j = 1; j < compSize; j++)
                        {
                            ogdf::node next = G.newNode();
                            edgeCandidates.push_back(G.newEdge(cur, next));
                            edgeCandidatesIsBridge.push_back(false);

                            cur = next;
                        }

                        edgeCandidates.push_back(G.newEdge(cur, start));
                        edgeCandidatesIsBridge.push_back(false);

                        freeVert -= compSize-1;
                        ++totalBlocks;
                        totalBlockNodes += compSize;
                    }
                    else // edge extend
                    {
                        // Component size
                        if (compSize>freeVert+2)
                            compSize=max(3,freeVert+2);

                        int index =  uniform_int_distribution<>(0,edgeCandidates.size()-1)(rng);

                        start = edgeCandidates[index]->source();
                        ogdf::node cur = start;

                        for(int j = 2; j < compSize; j++)
                        {
                            ogdf::node next = G.newNode();
                            edgeCandidates.push_back(G.newEdge(cur, next));
                            edgeCandidatesIsBridge.push_back(false);

                            cur = next;
                        }

                        edgeCandidates.push_back(G.newEdge(cur, edgeCandidates[index]->target()));
                        edgeCandidatesIsBridge.push_back(false);

                        if (edgeCandidatesIsBridge[index])
                        {
                            edgeCandidatesIsBridge[index]=false;
                            ++totalBlocks;
                            totalBlockNodes += compSize;
                        }
                        else
                        {
                            edgeCandidates[index]=edgeCandidates.back();
                            edgeCandidatesIsBridge[index]=edgeCandidatesIsBridge.back();
                            edgeCandidates.pop_back();
                            edgeCandidatesIsBridge.pop_back();
                            totalBlockNodes += compSize-2;
                        }
                        //edgeCandidates.del(edgeCandidates.get(index));
                        freeVert -= compSize-2;
                    } //if(r < G.numberOfNodes())
                } // else(edgeCandidates.empty() && G.empty())
            } // if(freeVert > 3 && rand() % 2)
            else
            {
                int bridgeLength = uniform_int_distribution<>(1,max(1,maxBridgeLength))(rng);
                if(G.empty())
                {
                    G.newNode();
                    freeVert--;
                }
                if (bridgeLength>freeVert)
                    bridgeLength=freeVert;

                ogdf::node start = G.chooseNode();

                ogdf::node cur = start;
                for(int j = 0; j < bridgeLength; j++)
                {
                    ogdf::node next = G.newNode();
                    edgeCandidates.push_back(G.newEdge(cur, next));
                    edgeCandidatesIsBridge.push_back(true);

                    cur = next;
                }
                freeVert -= bridgeLength;
            } //else(freeVert > 3 && rand() % 2)
        } // while
        // write graph to disk
        randomOuterplanar << "# "<<g<<" "<<G.numberOfNodes()<<" "<<G.numberOfEdges()<<endl;
        for (int j=0; j<G.numberOfNodes(); ++j) // nodes
            randomOuterplanar << uniform_int_distribution<>(1,numlabels)(rng) << " ";
        randomOuterplanar << endl;
        edge e;
        forall_edges(e,G)
            randomOuterplanar << e->source()->index()+1<< " "<< e->target()->index()+1 << " "<<uniform_int_distribution<>(1,numlabels)(rng)<< " ";
        randomOuterplanar << endl;

        totalNodes += G.numberOfNodes();
        totalEdges += G.numberOfEdges();
    } // for
    if (totalBlocks==0)
        totalBlocks=-1;
    cout << "Generated "<< numGraphs << " outerplanar graphs with average edge density "<<1.0*totalEdges/totalNodes<< " and an average block size of " << int(10.0*totalBlockNodes/totalBlocks+0.5)/10.0 <<endl;
    //cout << "totalNodes="<< totalNodes<<" totalBlockNodes="<< totalBlockNodes<<" totalEdges="<<totalEdges <<" totalBlocks="<< totalBlocks<< endl;
    randomOuterplanar.close();*/
}





void transform(const char* filenameinput1, const char* filenameinput2, const char* filenameoutput)
{
    cout << "Transforming outputs into single file for graphical display .. ";
    ifstream if1,if2;
    ofstream output;
    if1.open(filenameinput1,ios_base::in);
    if (!if1.is_open())
    {
        cout << "Could not input file "<<filenameinput1<<endl;
        return;
    }
    if2.open(filenameinput2,ios_base::in);
    if (!if2.is_open())
    {
        cout << "Could not input file "<<filenameinput2<<endl;
        return;
    }
    output.open(filenameoutput,ios_base::out);
    if (!output.is_open())
    {
        cout << "Could not open output file "<<filenameoutput<<endl;
        return;
    }
    char line[256];
    stringstream sstr;
    int runs1,runs2;
    if1.getline(line,255); // some information, not needed here
    if1.getline(line,255); // number of lines
    sstr<< line<<endl;
    sstr>> runs1;
    if2.getline(line,255); // some information, not needed here
    if2.getline(line,255); // number of lines
    sstr<< line<<endl;
    sstr>> runs2;
    if (runs1!=runs2)
    {
        cout << "Files contain different number of MCS computations"<<endl;
        return;
    }
    vector <double> factor;
    vector <int64_t> timet1,timet2;
    factor.reserve(runs1);
    int64_t notrelevant,time1,time2,mcssize1,mcssize2,mcs1bigger=0,mcs2bigger=0,graph1,graph2,avg1=0,avg2=0;
    double avgfactor=0,maxfactor=0,var1=0,var2=0,var3=0;
    int sizetree2tline1=0;
    for (int run=0; run<runs1; ++run)
    {
        if1.getline(line,255);
        sstr << line<<endl;
        sstr>> graph1; sstr>> graph2; sstr>> notrelevant;
        if (run==0)
            sstr >> sizetree2tline1;
        else
            sstr>> notrelevant;
        sstr >> mcssize1;
        sstr>> time1;
        timet1.push_back(time1);
        avg1+=time1;
        if2.getline(line,255);
        sstr << line<<endl;
        sstr>> notrelevant; sstr>> notrelevant; sstr>> notrelevant; sstr>> notrelevant;
        sstr >> mcssize2;
        if (mcssize1<mcssize2)
        {
            ++mcs2bigger;
            //cout << "Induced smaller than not induced: Run="<<run<<" G,H="<<graph1<<" "<<graph2<<" |MCSI|="<<mcssize1<<" |MCSE|="<<mcssize2<<endl;
        }
        else if (mcssize1>mcssize2)
        {
            ++mcs1bigger;
            cout << "Induced larger than not induced: Run="<<run+1<<" G,H="<<graph1<<" "<<graph2<<" |MCSI|="<<mcssize1<<" |MCSE|="<<mcssize2<<endl;
        }
        sstr>> time2;
        output << time1<<" "<<time2<<endl;
        maxfactor=max(maxfactor, 1.0*time2/time1);
        factor.push_back(1.0*time2/time1);
        timet2.push_back(time2);
        avg2+=time2;
        avgfactor+=1.0*time2/time1;
    }
    avgfactor/=runs1;
    avg1 /= runs1;
    avg2 /= runs1;
    cout << "done."<<endl;
    if (mcs1bigger>0)
        cout << "In "<<mcs1bigger<< " of "<<runs1<<" computations first MCS was bigger."<<endl;
    if (mcs2bigger>0)
        cout << "In "<<mcs2bigger<< " of "<<runs1<<" computations second MCS was bigger."<<endl;
    sort (timet1.begin(), timet1.end());
    sort (timet2.begin(), timet2.end());
    for (int run=0; run<runs1; ++run)
    {
        var1+=1.0*(timet1[run]-avg1)*(timet1[run]-avg1);
        var2+=1.0*(timet2[run]-avg2)*(timet2[run]-avg2);
        var3+=1.0*(factor[run]-avgfactor)*(factor[run]-avgfactor);
    }
    var1 /= -1LL + runs1;
    var2 /= -1LL + runs1;
    var3 /= -1LL + runs1;
    var1 = sqrt(var1);
    var2 = sqrt(var2);
    var3 = sqrt(var3);

    cout << "MCS. Average time: "<<avg1<<", Standardabw.: "<<var1<<", Maximum: "<<timet1[-1LL + runs1]<<", 95%: "<<timet1[static_cast<size_t> (runs1*0.95)]<<", Median: "<<timet1[runs1/2]<<", 5%: "<<timet1[static_cast<size_t> (runs1*0.05)]<<", Minimum: "<<timet1[0]<<endl;
    cout << "Greff. Average time: "<<avg2<<", Standardabw.: "<<var2<<", Maximum: "<<timet2[-1LL + runs1]<<", 95%: "<<timet2[static_cast<size_t> (runs1*0.95)]<<", Median: "<<timet2[runs1/2]<<", 5%: "<<timet2[static_cast<size_t> (runs1*0.05)]<<", Minimum: "<<timet2[0]<<endl;
    sort (factor.begin(), factor.end());
    cout << "Compare. Average speed increase: "<<avgfactor<<", Standardabw.: "<<var3<<", Maximum: "<<factor[-1LL + runs1]<<", 95%: "<<factor[static_cast<size_t> (runs1*0.95)]<<", Median: "<<factor[runs1/2]<<", 5%: "<<factor[static_cast<size_t> (runs1*0.05)]<<", Minimum: "<<factor[0]<<endl;
    cout << endl;
    // 20&$1.3\pm 17\%$&$40\pm 4\%$&30\\ Latex style
    //cout << sizetree2tline1 << "&$"<<(avg1/100)/10.0<<"\\pm"<<int(100*var1)/avg1<<"\\%$&$"<<avg2/1000<<"\\pm"<<int(100*var2)/avg2<<"\\%$&"<<int(avgfactor*10)/10.0<<"\\\\"<<endl<<"\\hline"<<endl;
    cout << "MCS.   $"<<int64_t((avg1+50)/100)/10.0<<"\\pm"<<int64_t((var1+50)/100)/10.0<<"$ ms     fog.   $"<<int64_t((avg2+500)/1000)<<"\\pm"<<int64_t((var2+500)/1000)<<"$ ms"<<endl;
// Varianze = Summe( (X_mittel -Xi)^2 )
//		StdAbw = Sqrt(Varianze) / (n-1)
}
