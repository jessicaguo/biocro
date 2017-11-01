#include <stdexcept>
#include <sstream>
#include <Rinternals.h>
#include "BioCro.h"
#include "modules.h"
#include "R_helper_functions.h"

using std::string;
using std::vector;

extern "C" {

SEXP R_Gro(SEXP initial_state,
        SEXP invariate_parameters,
        SEXP varying_parameters,
        SEXP canopy_photosynthesis_module,
        SEXP soil_evaporation_module,
        SEXP growth_module,
        SEXP senescence_module)
{
    try {
        state_map s = map_from_list(initial_state);
        state_map ip = map_from_list(invariate_parameters);
        state_vector_map vp = map_vector_from_list(varying_parameters);

        if (vp.begin()->second.size() == 0) {
            return R_NilValue;
        }

        //output_map(s);
        //output_map(ip);
        std::unique_ptr<IModule> canopy;
        std::unique_ptr<IModule> soil_evaporation;
        std::unique_ptr<IModule> growth;
        std::unique_ptr<IModule> senescence;

        canopy = ModuleFactory()(CHAR(STRING_ELT(canopy_photosynthesis_module, 0)));
        soil_evaporation = ModuleFactory()(CHAR(STRING_ELT(soil_evaporation_module, 0)));
        growth = ModuleFactory()(CHAR(STRING_ELT(growth_module, 0)));
        senescence = ModuleFactory()(CHAR(STRING_ELT(senescence_module, 0)));

        vector<string> required_state = {"iSp", "doy", "SpD", "Leaf",
            "LeafN_0", "vmax_n_intercept", "vmax1", "alphab1",
            "alpha1", "TTc", "temp", "tbase", "timestep",
            "mrc1", "seneLeaf", "Stem", "seneStem",
            "mrc2", "Root", "seneRoot", "Rhizome", "seneRhizome", "kln", "growth_respiration_fraction"};

        state_map all_state = combine_state(s, combine_state(ip, at(vp, 0)));

        vector<string> missing_state;
        for (auto it = required_state.begin(); it != required_state.end(); ++it) {
            if (all_state.find(*it) == all_state.end()) {
                missing_state.push_back(*it);
            }
        }

        if (!missing_state.empty()) {
            std::ostringstream message;
            message << "The following state variables are required but are missing: ";
            for(vector<string>::iterator it = missing_state.begin(); it != missing_state.end() - 1; ++it) {
                message << *it << ", ";
            }
            message << missing_state.back() << ".";
            error(message.str().c_str());
        }

        state_vector_map result;
        try {
            result = Gro(s, ip, vp, canopy, soil_evaporation, growth, senescence, biomass_leaf_nitrogen_limitation);
        } catch (std::exception const &e) {
            std::ostringstream message;
            message << "Exception caught in R_Gro.cpp. " << e.what();
            error(message.str().c_str());
        }

        return (list_from_map(result));
    } catch (std::exception const &e) {
        error(std::string(std::string("Caught exception in R_Gro: ") + e.what()).c_str());
    } catch (...) {
        error("Caught unhandled exception in R_Gro.");
    }
}

SEXP R_Gro_ode(SEXP initial_state,
        SEXP invariate_parameters,
        SEXP varying_parameters,
        SEXP steady_state_modules_list,
        SEXP derivative_modules_list)
{
Rprintf("Start of R_Gro_ode.\n");
    try {
        state_map s = map_from_list(initial_state);
        state_map ip = map_from_list(invariate_parameters);
        state_vector_map vp = map_vector_from_list(varying_parameters);

        if (vp.begin()->second.size() == 0) {
            return R_NilValue;
        }

        //output_map(s);
        //output_map(ip);
        std::vector<std::unique_ptr<IModule>> steady_state_modules;
        std::vector<std::unique_ptr<IModule>> derivative_modules;

        vector<string> steady_state_names_vector = make_vector(steady_state_modules_list);
        for (auto it = steady_state_names_vector.begin(); it != steady_state_names_vector.end(); ++it) {
            steady_state_modules.push_back(ModuleFactory()(*it));
        }

        vector<string> derivative_names_vector = make_vector(derivative_modules_list);
        for (auto it = derivative_names_vector.begin(); it != derivative_names_vector.end(); ++it) {
            derivative_modules.push_back(ModuleFactory()(*it));
        }

        state_map all_state = combine_state(s, combine_state(ip, at(vp, 0)));


Rprintf("Before Gro call.\n");

        state_vector_map result;
        state_map state = combine_state(s, combine_state(ip, at(vp, 0)));
        append_state_to_vector(Gro(state, steady_state_modules, derivative_modules), result);
        return (list_from_map(result));

    } catch (std::exception const &e) {
        error(std::string(std::string("Caught exception in R_Gro_ode: ") + e.what()).c_str());
    } catch (...) {
        error("Caught unhandled exception in R_Gro_ode.");
    }
}

} // extern "C"

