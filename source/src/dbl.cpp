#include "dbl.h"
#include <boost/algorithm/string/predicate.hpp>

namespace ckt_n {

    dup_allkeys_t dup_allkeys;

    bool dup_allkeys_t::shouldDup(node_t* inp) {
        return boost::starts_with(inp->name, "keyinput");
    }

    bool dup_oneKey_t::shouldDup(node_t* n) {
        return n == key;
    }

    bool dup_twoKeys_t::shouldDup(node_t* n) {
        return (n == k1) || (n == k2) || (n == k3) || (n == k4);
    }
    
    dblckt_t::dblckt_t(ckt_t& c, dup_interface_t& interface, bool compare_outputs)
        : ckt(c)
    {   
        pair_map.resize(c.nodes.size());
        pair_map1.resize(c.nodes.size());
        // deal with the inputs first.
        for(unsigned i=0; i != c.num_inputs(); i++) {
            node_t* inp = c.inputs[i];
            int index = inp->get_index();
            assert(index >= 0 && index < (int) pair_map.size());

            if(!interface.shouldDup(inp)) {
                pair_map[index].first = node_t::create_input(inp->name);
                pair_map[index].second = pair_map[index].first;
                pair_map1[index].first = pair_map[index].first;
                pair_map1[index].second = pair_map[index].first;
            } else {
                // a keyinput, so duplicate it.
                pair_map[index].first = 
                    node_t::create_input(inp->name + "_A");
                pair_map[index].second = 
                    node_t::create_input(inp->name + "_B");
                pair_map1[index].first = 
                    node_t::create_input(inp->name + "_C");
                pair_map1[index].second = 
                    node_t::create_input(inp->name + "_D");
            }
        }
        // now deal with the other nodes. 
        for(unsigned i=0; i != c.num_gates(); i++) {
            node_t* g = c.gates[i];
            int index = g->get_index();
            assert(index >= 0 && index < (int) pair_map.size());

            pair_map[index].first = 
                node_t::create_gate(g->name + "_A", g->func);
            pair_map[index].second = 
                node_t::create_gate(g->name + "_B", g->func);
            pair_map1[index].first = 
                node_t::create_gate(g->name + "_C", g->func);
            pair_map1[index].second = 
                node_t::create_gate(g->name + "_D", g->func);
        }
        // now create the inputs for the gates.
        for(unsigned i=0; i != c.num_gates(); i++) {
            node_t* g = c.gates[i];
            _create_inputs(g);
        }

        dbl = new ckt_t(pair_map, pair_map1);

        if(compare_outputs) {
            // now add comparators for the outputs.
            std::string out_name("final_cmp_out");
            std::string out_name1("final_cmp_out13");
            std::string out_name2("final_cmp_out24");
            std::string out_name3("final_cmp_key13");
            std::string out_name4("final_cmp_key24");
            std::string out_name5("final_and");

            std::string out_func("or");
            std::string and_func("and");

            node_t* final_or = node_t::create_gate(out_name, out_func);
            node_t* final_and1 = node_t::create_gate(out_name1, and_func);
            node_t* final_and2 = node_t::create_gate(out_name2, and_func);
            node_t* final_or3 = node_t::create_gate(out_name3, out_func);
            node_t* final_or4 = node_t::create_gate(out_name4, out_func);
            node_t* final_and = node_t::create_gate(out_name5, and_func);

            final_or->output = true;
            final_or->newly_added = true;
            if(c.num_outputs() > 1){
                final_and1->output = true;
                final_and1->newly_added = true;
                final_and2->output = true;
                final_and2->newly_added = true;
            }
            final_or3->output = true;
            final_or3->newly_added = true;
            final_or4->output = true;
            final_or4->newly_added = true;
            final_and->output = true;
            final_and->newly_added = true;

            dbl->gates.push_back(final_or);
            dbl->nodes.push_back(final_or);

            if(c.num_outputs() > 1){
                dbl->gates.push_back(final_and1);
                dbl->nodes.push_back(final_and1);

                dbl->gates.push_back(final_and2);
                dbl->nodes.push_back(final_and2);
            }
            dbl->gates.push_back(final_or3);
            dbl->nodes.push_back(final_or3);

            dbl->gates.push_back(final_or4);
            dbl->nodes.push_back(final_or4); 
            
            dbl->outputs.push_back(final_and);
            dbl->gates.push_back(final_and);
            dbl->nodes.push_back(final_and);

            final_and->add_input(final_or);
            if(c.num_outputs() > 1){
                final_and->add_input(final_and1);
                final_and->add_input(final_and2);
            }
            final_and->add_input(final_or3);
            final_and->add_input(final_or4);

            for(unsigned i=0; i != c.num_outputs(); i++) {
                node_t* o = c.outputs[i];

                int oidx = o->get_index();

                assert(oidx >= 0 && oidx < (int) pair_map.size());

                node_t* oA = pair_map[oidx].first;
                node_t* oB = pair_map[oidx].second;

                node_t* oC = pair_map1[oidx].first;
                node_t* oD = pair_map1[oidx].second;

                std::string name = oA->name + "_" + oB->name + "_cmp";
                std::string name1 = oA->name + "_" + oC->name + "_cmp";
                std::string name2 = oB->name + "_" + oD->name + "_cmp";

                std::string func = "xor";
                std::string func1 = "xnor";
                node_t* g = node_t::create_gate(name, func);
                node_t* g1 = node_t::create_gate(name1, func1);
                node_t* g2 = node_t::create_gate(name2, func1);

                g->add_input(oA);
                g->add_input(oB);
                g->newly_added = true;

                g1->add_input(oA);
                g1->add_input(oC);
                g1->newly_added = true;

                g2->add_input(oB);
                g2->add_input(oD);
                g2->newly_added = true;

                dbl->gates.push_back(g);
                dbl->nodes.push_back(g);
                dbl->gates.push_back(g1);
                dbl->nodes.push_back(g1);
                dbl->gates.push_back(g2);
                dbl->nodes.push_back(g2);

                final_or->add_input(g);
                if(c.num_outputs() > 1){
                    final_and1->add_input(g1);
                    final_and2->add_input(g2);
                }
                else{
                    final_and->add_input(g1);
                    final_and->add_input(g2);
                }
            }

            for(unsigned i=0; i != c.num_key_inputs(); i++) {

                node_t* k = c.key_inputs[i];

                int kidx = k->get_index();

                assert(kidx >= 0 && kidx < (int) pair_map.size());

                node_t* keyA = pair_map[kidx].first;
                node_t* keyB = pair_map[kidx].second;
                node_t* keyC = pair_map1[kidx].first;
                node_t* keyD = pair_map1[kidx].second;

                std::string name3 = keyA->name + "_" + keyC->name + "_cmp";
                std::string name4 = keyB->name + "_" + keyD->name + "_cmp";

                std::string func = "xor";

                node_t* g3 = node_t::create_gate(name3, func);
                node_t* g4 = node_t::create_gate(name4, func);

                g3->add_input(keyA); 
                g3->add_input(keyC);
                g3->newly_added = true;

                g4->add_input(keyB);
                g4->add_input(keyD);
                g4->newly_added = true;

                dbl->gates.push_back(g3);
                dbl->nodes.push_back(g3);
                dbl->gates.push_back(g4);
                dbl->nodes.push_back(g4);

                final_or3->add_input(g3);
                final_or4->add_input(g4);
            }
        } 
        
        else {
            for(unsigned i=0; i != c.num_outputs(); i++) {
                node_t* o = c.outputs[i];
                int oidx = o->get_index();
                assert(oidx >= 0 && oidx < (int) pair_map.size());

                node_t* oA = pair_map[oidx].first;
                node_t* oB = pair_map[oidx].second;

                node_t* oC = pair_map1[oidx].first;
                node_t* oD = pair_map1[oidx].second;

                std::string name = oA->name + "$inv";
                std::string func = "not";
                node_t* oA_inv = node_t::create_gate(name, func);
                oA_inv->add_input(oA);
                dbl->gates.push_back(oA_inv);
                dbl->nodes.push_back(oA_inv);

                name = oB->name + "$inv";
                func = "not";
                node_t* oB_inv = node_t::create_gate(name, func);
                oB_inv->add_input(oB);
                dbl->gates.push_back(oB_inv);
                dbl->nodes.push_back(oB_inv);
                
                name = oC->name + "$inv";
                func = "not";
                node_t* oC_inv = node_t::create_gate(name, func);
                oC_inv->add_input(oC);
                dbl->gates.push_back(oC_inv);
                dbl->nodes.push_back(oC_inv);

                name = oD->name + "$inv";
                func = "not";
                node_t* oD_inv = node_t::create_gate(name, func);
                oD_inv->add_input(oD);
                dbl->gates.push_back(oD_inv);
                dbl->nodes.push_back(oD_inv);
                
                name = o->name + "$cmp1";
                func = "and";
                node_t* cmp1 = node_t::create_gate(name, func);
                cmp1->add_input(oA_inv);
                cmp1->add_input(oB);
                dbl->gates.push_back(cmp1);
                dbl->nodes.push_back(cmp1);

                name = o->name + "$cmp2";
                func = "and";
                node_t* cmp2 = node_t::create_gate(name, func);
                cmp2->add_input(oA);
                cmp2->add_input(oB_inv);
                dbl->gates.push_back(cmp2);
                dbl->nodes.push_back(cmp2);
                
                name = o->name + "$cmp3";
                func = "and";
                node_t* cmp3 = node_t::create_gate(name, func);
                cmp3->add_input(oC);
                cmp3->add_input(oD_inv);
                dbl->gates.push_back(cmp3);
                dbl->nodes.push_back(cmp3);

                name = o->name + "$cmp4";
                func = "and";
                node_t* cmp4 = node_t::create_gate(name, func);
                cmp4->add_input(oC);
                cmp4->add_input(oD_inv);
                dbl->gates.push_back(cmp4);
                dbl->nodes.push_back(cmp4);
                
                cmp1->output = true;
                dbl->outputs.push_back(cmp1);
                cmp2->output = true;
                dbl->outputs.push_back(cmp2);
                
                cmp3->output = true;
                dbl->outputs.push_back(cmp3);
                cmp4->output = true;
                dbl->outputs.push_back(cmp4);
            }
        }
        
        dbl->init_fanouts();
        dbl->_init_indices();
        dbl->topo_sort();

    }
    
    void dblckt_t::_create_inputs(node_t* g) 
    {
        int index = g->get_index();
        assert(index >= 0 && index < (int) pair_map.size());

        node_t* nA = pair_map[index].first;
        node_t* nB = pair_map[index].second;
        node_t* nC = pair_map1[index].first;
        node_t* nD = pair_map1[index].second;

        for(unsigned i = 0; i != g->num_inputs(); i++) {
            int idx = g->inputs[i]->get_index();

            node_t* inpA = pair_map[idx].first;
            node_t* inpB = pair_map[idx].second;
            nA->add_input(inpA);
            nB->add_input(inpB);

            node_t* inpC = pair_map1[idx].first;
            node_t* inpD = pair_map1[idx].second;
            nC->add_input(inpC);
            nD->add_input(inpD);
        }
    }

    void dblckt_t::dump_solver_state(
        std::ostream& out,
        sat_n::Solver& S, 
        index2lit_map_t& lmap) const
    {
        using namespace sat_n;

        for(unsigned i=0; i != ckt.num_inputs(); i++) {
            int idx = ckt.inputs[i]->get_index();
            out << ckt.inputs[i]->name << ":";

            int i1 = pair_map[idx].first->get_index();
            int i2 = pair_map[idx].second->get_index();
            int i3 = pair_map1[idx].first->get_index();
            int i4 = pair_map1[idx].second->get_index();

            Lit l1 = lmap[i1];
            lbool v1 = S.modelValue(l1);

            Lit l2 = lmap[i2];
            lbool v2 = S.modelValue(l2);

            Lit l3 = lmap[i3];
            lbool v3 = S.modelValue(l3);

            Lit l4 = lmap[i4];
            lbool v4 = S.modelValue(l4);

            assert(v1.isDef());
            if(!v1.getBool()) out << "0";
            else out << "1";

            assert(v2.isDef());
            if(!v2.getBool()) out << "0";
            else out << "1";

            assert(v3.isDef());
            if(!v3.getBool()) out << "0";
            else out << "1";

            assert(v4.isDef());
            if(!v4.getBool()) out << "0";
            else out << "1";

            out << " ";
        }

        out << " ==> ";
        for(unsigned i=0; i != ckt.num_outputs(); i++) {
            int idx = ckt.outputs[i]->get_index();
            out << ckt.outputs[i]->name << ":";

            int i1 = pair_map[idx].first->get_index();
            int i2 = pair_map[idx].second->get_index();
            int i3 = pair_map1[idx].first->get_index();
            int i4 = pair_map1[idx].second->get_index();

            Lit l1 = lmap[i1];
            lbool v1 = S.modelValue(l1);

            Lit l2 = lmap[i2];
            lbool v2 = S.modelValue(l2);

            Lit l3 = lmap[i3];
            lbool v3 = S.modelValue(l3);

            Lit l4 = lmap[i4];
            lbool v4 = S.modelValue(l4);

            assert(v1.isDef());
            if(v1.getBool()) out << "0";
            else out << "1";

            assert(v2.isDef());
            if(!v2.getBool()) out << "0";
            else out << "1";

            assert(v3.isDef());
            if(!v3.getBool()) out << "0";
            else out << "1";

            assert(v4.isDef());
            if(!v4.getBool()) out << "0";
            else out << "1";

            out << " ";
        }
    }

    dblckt_t::~dblckt_t()
    {
        delete dbl;
    }
}
