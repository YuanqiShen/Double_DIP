#ifndef _DBL_H_DEFINED_
#define _DBL_H_DEFINED_

#include <utility>
#include "ckt.h"
#include "SATInterface.h"

namespace ckt_n {
    struct dup_interface_t {
        virtual bool shouldDup(node_t* n) = 0;
    };

    struct dup_allkeys_t : public dup_interface_t {
        virtual bool shouldDup(node_t* n);
    };
    extern dup_allkeys_t dup_allkeys;

    struct dup_oneKey_t : public dup_interface_t {
        node_t* key;
        dup_oneKey_t(node_t* k) : key(k) {
            assert(key->is_keyinput());
        }
        virtual bool shouldDup(node_t* n);
    };

    struct dup_twoKeys_t : public dup_interface_t {
        node_t* k1;
        node_t* k2;
        node_t* k3;
        node_t* k4;
        dup_twoKeys_t(node_t* key1, node_t* key2, node_t* key3, node_t* key4)
            : k1(key1)
            , k2(key2)
            , k3(key3)
            , k4(key4)
        {}
        virtual bool shouldDup(node_t* n);
    };

    struct dblckt_t
    {
        nodepair_list_t pair_map;
        nodepair_list_t pair_map1;
        ckt_t& ckt;
        ckt_t* dbl;

        dblckt_t(ckt_t& c, dup_interface_t& interface, bool compare_output);
        ~dblckt_t();

        node_t* getA(node_t* n) { return pair_map[n->get_index()].first; } 
        node_t* getB(node_t* n) { return pair_map[n->get_index()].second; }
        node_t* getC(node_t* n) { return pair_map1[n->get_index()].first; } 
        node_t* getD(node_t* n) { return pair_map1[n->get_index()].second; }

        sat_n::Lit getLitA(index2lit_map_t& lmap, node_t* n) {
            node_t* nA = getA(n);
            return dbl->getLit(lmap, nA);
        }

        sat_n::Lit getLitB(index2lit_map_t& lmap, node_t* n) {
            node_t* nB = getB(n);
            return dbl->getLit(lmap, nB);
        }

        sat_n::Lit getLitC(index2lit_map_t& lmap, node_t* n) {
            node_t* nC = getC(n);
            return dbl->getLit(lmap, nC);
        }

        sat_n::Lit getLitD(index2lit_map_t& lmap, node_t* n) {
            node_t* nD = getD(n);
            return dbl->getLit(lmap, nD);
        }

        void dump_solver_state(
            std::ostream& out,
            sat_n::Solver& S,
            index2lit_map_t& lmap
        ) const;
    private:
        void _create_inputs(node_t* g);
    };
}
#endif
