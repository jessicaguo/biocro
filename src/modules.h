#ifndef MODULES_H
#define MODULES_H

#include <unordered_map>
#include <string>
#include <vector>
#include <memory>
#include "BioCro.h"
#include "c3photo.h"

using std::vector;
using std::string;
using std::unordered_map;

typedef unordered_map<string, double> state_map;
typedef unordered_map<string, vector<double>> state_vector_map;

class IModule {
    public:
        IModule(vector<string> const required_state, vector<string> const modified_state) :
            _required_state(required_state),
            _modified_state(modified_state)
    {}
        vector<string> list_required_state() const;
        vector<string> list_modified_state() const;
        state_map run (state_map const &state) const;
        state_map run (state_vector_map const &state_history, state_vector_map const &deriv_history, state_map const &parameters) const;
        vector<string> state_requirements_are_met(state_map const &state) const;
        virtual ~IModule() = 0;  // Make the destructor a pure virtual function so that no objects can be made directly from this class.
    private:
        vector<string> const _required_state;
        vector<string> const _modified_state; 
        virtual state_map do_operation(state_map const &state) const {state_map derivs; return derivs;};
        virtual state_map do_operation(state_vector_map const &state_history, state_vector_map const &deriv_history, state_map const &parameters) const;
        bool requirements_are_met;
};

inline IModule::~IModule() {};  // A destructor must be defined, and since the default is overwritten when defining it as pure virtual, add an inline one in the header.

class Leaf_photosynthesis_module : public IModule {
    public:
        Leaf_photosynthesis_module(const vector<string> required_state, const vector<string> modified_state)
            : IModule(required_state, modified_state)
        {}
        virtual struct c3_str assimilation(struct Model_state) = 0;
};

class c3_leaf : public Leaf_photosynthesis_module {
    public:
        c3_leaf()
            : Leaf_photosynthesis_module(vector<string> {"Qp", "Tleaf"} , vector<string> {})
        {} 
        double run (state_map s);
        struct c3_str assimilation(state_map s);
};

class ICanopy_photosynthesis_module : public IModule {
    public:
        ICanopy_photosynthesis_module(const vector<string> required_state, const vector<string> modified_state)
            : IModule(required_state, modified_state)
        {}
};

class c4_canopy : public ICanopy_photosynthesis_module {
    public:
        c4_canopy()
            : ICanopy_photosynthesis_module(vector<string> {"lai", "doy", "hour", "solar", "temp",
                    "rh", "windspeed", "lat", "nlayers", "vmax1",
                    "alpha1", "kparm", "beta", "Rd", "Catm",
                    "b0", "b1", "theta", "kd", "chil",
                    "heightf", "LeafN", "kpLN", "lnb0", "lnb1",
                    "lnfun", "upperT", "lowerT", "leafwidth",
                    "et_equation", "StomataWS", "ws",
                    "ileafN", "kln", "Vmaxb0", "Vmaxb1", "alphab1",
                    "Rdb1", "Rdb0", "kpLN"}, 
                    vector<string> {})
        {}
    private:
        virtual state_map do_operation (state_map const &s) const;
};

class c3_canopy : public ICanopy_photosynthesis_module {
    public:
        c3_canopy()
            : ICanopy_photosynthesis_module(vector<string> {"lai", "doy", "hour", "solarr", "temp",
                    "rh", "windspeed", "lat", "nlayers", "vmax",
                    "jmax", "rd", "catm", "o2", "b0",
                    "b1", "theta", "kd", "heightf", "leafn",
                    "kpln", "lnb0", "lnb1", "lnfun", "stomataws",
                    "ws"},
                    vector<string> {})
        {}
    private:
        virtual state_map do_operation (state_map const &s) const;
};

class ISoil_evaporation_module : public IModule {
    public:
        ISoil_evaporation_module(const vector<string> required_state, const vector<string> modified_state)
            : IModule(required_state, modified_state)
        {}
};

class one_layer_soil_profile : public ISoil_evaporation_module {
    public:
        one_layer_soil_profile()
            : ISoil_evaporation_module(vector<string> {"lai", "temp", "solar", "waterCont",
                    "FieldC", "WiltP", "windspeed", "rh", "rsec"},
                    vector<string> {})
        {}
    private:
        virtual state_map do_operation(state_map const &s) const;
};

class ISenescence_module : public IModule {
    public:
        ISenescence_module(const vector<string> required_state, const vector<string> modified_state)
            : IModule(required_state, modified_state)
        {}
};

class thermal_time_senescence_module : public ISenescence_module {
    public:
        thermal_time_senescence_module()
            : ISenescence_module(vector<string> {"TTc", "seneLeaf", "seneStem", "seneRoot", "seneRhizome",
                "kLeaf", "kStem", "kRoot", "kRhizome", "kGrain",
                "newLeafcol", "newStemcol", "newRootcol", "newRhizomecol",
                "senesced_leaf_index", "senesced_stem_index", "senesced_root_index", "senesced_rhizome_index",
                "mrc1", "mrc2"},
                vector<string> {})
        {}
    private:
        virtual state_map do_operation(state_vector_map const &s_history, state_vector_map const &d_history, state_map const &parameters) const;
};

class IGrowth_module : public IModule {
    public:
        IGrowth_module(const vector<string> required_state, const vector<string> modified_state)
            : IModule(required_state, modified_state)
        {}
};

class partitioning_growth_module : public IGrowth_module {
    public:
        partitioning_growth_module()
            : IGrowth_module(vector<string> {"TTc", "LeafWS", "temp", "CanopyA",
                "Leaf", "Stem", "Root", "Rhizome", "Grain",
                "kLeaf", "kStem", "kRoot", "kRhizome", "kGrain",
                "mrc1", "mrc2"},
                vector<string> {})
        {}
    private:
        virtual state_map do_operation(state_vector_map const &s_history, state_vector_map const &d_history, state_map const &parameters) const;
};

state_map combine_state(state_map const &state_a, state_map const &state_b);

state_vector_map Gro(
        state_map const &initial_state,
        state_map const &invariant_parameters,
        state_vector_map const &varying_parameters,
        std::unique_ptr<IModule> const &canopy_photosynthesis_module,
        std::unique_ptr<IModule> const &soil_evaporation_module,
		double (*leaf_n_limitation)(state_map const &model_state));

double biomass_leaf_nitrogen_limitation(state_map const &model_state);

void output_map(state_map const &m);

state_map replace_state(state_map const &state, state_map const &newstate);

state_vector_map allocate_state(state_map const &m, int n);

void append_state_to_vector(state_map const &state, state_vector_map &state_vector);

std::unique_ptr<IModule> make_module(string const &module_name);

state_map at(state_vector_map const vector_map, vector<double>::size_type n);

state_map& operator+=(state_map &lhs, state_map const &rhs);

# endif

