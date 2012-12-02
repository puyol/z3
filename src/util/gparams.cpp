/*++
Copyright (c) 2012 Microsoft Corporation

Module Name:

    gparams.cpp

Abstract:

    Global parameter management.

Author:

    Leonardo (leonardo) 2012-11-29

Notes:

--*/
#include"gparams.h"
#include"dictionary.h"
#include"trace.h"

extern void gparams_register_modules();

char const * g_old_params_names[] = {
    "arith_adaptive","arith_adaptive_assertion_threshold","arith_adaptive_gcd","arith_adaptive_propagation_threshold","arith_add_binary_bounds","arith_blands_rule_threshold","arith_branch_cut_ratio","arith_dump_lemmas","arith_eager_eq_axioms","arith_eager_gcd","arith_eq_bounds","arith_euclidean_solver","arith_expand_eqs","arith_force_simplex","arith_gcd_test","arith_ignore_int","arith_lazy_adapter","arith_lazy_pivoting","arith_max_lemma_size","arith_process_all_eqs","arith_propagate_eqs","arith_propagation_mode","arith_propagation_threshold","arith_prop_strategy","arith_random_initial_value","arith_random_lower","arith_random_seed","arith_random_upper","arith_reflect","arith_skip_big_coeffs","arith_small_lemma_size","arith_solver","arith_stronger_lemmas","array_always_prop_upward","array_canonize","array_cg","array_delay_exp_axiom","array_extensional","array_laziness","array_lazy_ieq","array_lazy_ieq_delay","array_solver","array_weak","async_commands","at_labels_cex","auto_config","bb_eager","bb_ext_gates","bb_quantifiers","bin_clauses","bit2int","bv2int_distribute","bv_blast_max_size","bv_cc","bv_enable_int2bv_propagation","bv_lazy_le","bv_max_sharing","bv_reflect","bv_solver","case_split","check_at_labels","check_proof","cnf_factor","cnf_mode","context_simplifier","dack","dack_eq","dack_factor","dack_gc","dack_gc_inv_decay","dack_threshold","default_qid","default_table","default_table_checked","delay_units","delay_units_threshold","der","display_config","display_dot_proof","display_error_for_visual_studio","display_features","display_proof","display_unsat_core","distribute_forall","dt_lazy_splits","dump_goal_as_smt","elim_and","elim_bounds","elim_nlarith_quantifiers","elim_quantifiers","elim_term_ite","ematching","engine","eq_propagation","hi_div0","ignore_bad_patterns","ignore_setparameter","instruction_max","inst_gen","interactive","internalizer_nnf","lemma_gc_factor","lemma_gc_half","lemma_gc_initial","lemma_gc_new_clause_activity","lemma_gc_new_clause_relevancy","lemma_gc_new_old_ratio","lemma_gc_old_clause_activity","lemma_gc_old_clause_relevancy","lemma_gc_strategy","lift_ite","lookahead_diseq","macro_finder","max_conflicts","max_counterexamples","mbqi","mbqi_force_template","mbqi_max_cexs","mbqi_max_cexs_incr","mbqi_max_iterations","mbqi_trace","minimize_lemmas","model","model_compact","model_completion","model_display_arg_sort","model_hide_unused_partitions","model_on_final_check","model_on_timeout","model_partial","model_v1","model_v2","model_validate","new_core2th_eq","ng_lift_ite","nl_arith","nl_arith_branching","nl_arith_gb","nl_arith_gb_eqs","nl_arith_gb_perturbate","nl_arith_gb_threshold","nl_arith_max_degree","nl_arith_rounds","nnf_factor","nnf_ignore_labels","nnf_mode","nnf_sk_hack","order","order_var_weight","order_weights","phase_selection","pi_arith","pi_arith_weight","pi_avoid_skolems","pi_block_looop_patterns","pi_max_multi_patterns","pi_non_nested_arith_weight","pi_nopat_weight","pi_pull_quantifiers","pi_use_database","pi_warnings","pp_bounded","pp_bv_literals","pp_bv_neg","pp_decimal","pp_decimal_precision","pp_fixed_indent","pp_flat_assoc","pp_max_depth","pp_max_indent","pp_max_num_lines","pp_max_ribbon","pp_max_width","pp_min_alias_size","pp_simplify_implies","pp_single_line","precedence","precedence_gen","pre_demodulator","pre_simplifier","pre_simplify_expr","profile_res_sub","progress_sampling_freq","proof_mode","propagate_booleans","propagate_values","pull_cheap_ite_trees","pull_nested_quantifiers","qi_conservative_final_check","qi_cost","qi_eager_threshold","qi_lazy_instantiation","qi_lazy_quick_checker","qi_lazy_threshold","qi_max_eager_multi_patterns","qi_max_instances","qi_max_lazy_multi_pattern_matching","qi_new_gen","qi_profile","qi_profile_freq","qi_promote_unsat","qi_quick_checker","quasi_macros","random_case_split_freq","random_initial_activity","random_seed","recent_lemma_threshold","reduce_args","refine_inj_axiom","relevancy","relevancy_lemma","rel_case_split_order","restart_adaptive","restart_agility_threshold","restart_factor","restart_initial","restart_strategy","restricted_quasi_macros","simplify_clauses","smtlib2_compliant","smtlib_category","smtlib_dump_lemmas","smtlib_logic","smtlib_source_info","smtlib_trace_path","soft_timeout","solver","spc_bs","spc_es","spc_factor_subsumption_index_opt","spc_initial_subsumption_index_opt","spc_max_subsumption_index_features","spc_min_func_freq_subsumption_index","spc_num_iterations","spc_trace","statistics","strong_context_simplifier","tick","trace","trace_file_name","type_check","user_theory_persist_axioms","user_theory_preprocess_axioms","verbose","warning","well_sorted_check","z3_solver_ll_pp","z3_solver_smt_pp", 0 };

bool is_old_param_name(symbol const & name) {
    char const * const * it = g_old_params_names;
    while (*it) {
        if (name == *it)
            return true;
        it++;
    }
    return false;
}

struct gparams::imp {
    dictionary<param_descrs*> m_module_param_descrs;
    dictionary<char const *>  m_module_descrs;
    param_descrs              m_param_descrs;
    dictionary<params_ref *>  m_module_params;
    params_ref                m_params;
public:
    imp() {
    }

    ~imp() {
        {
            dictionary<param_descrs*>::iterator it  = m_module_param_descrs.begin();
            dictionary<param_descrs*>::iterator end = m_module_param_descrs.end();
            for (; it != end; ++it) {
                dealloc(it->m_value);
            }
        }
        {
            dictionary<params_ref*>::iterator it  = m_module_params.begin();
            dictionary<params_ref*>::iterator end = m_module_params.end();
            for (; it != end; ++it) {
                dealloc(it->m_value);
            }
        }
    }

    void register_global(param_descrs & d) {
        #pragma omp critical (gparams)
        {
           m_param_descrs.copy(d);
        }
    }

    void register_module(char const * module_name, param_descrs * d) {
        #pragma omp critical (gparams)
        {
            symbol s(module_name);
            param_descrs * old_d;
            if (m_module_param_descrs.find(s, old_d)) {
                old_d->copy(*d);
                dealloc(d);
            }
            else {
                m_module_param_descrs.insert(s, d);
            }
        }
    }

    void register_module_descr(char const * module_name, char const * descr) {
        #pragma omp critical (gparams)
        {
            m_module_descrs.insert(symbol(module_name), descr);
        }
    }

    void display(std::ostream & out, unsigned indent, bool smt2_style) {
        #pragma omp critical (gparams)
        {
            out << "Global parameters\n";
            m_param_descrs.display(out, indent + 4, smt2_style);
            out << "\n";
            dictionary<param_descrs*>::iterator it  = m_module_param_descrs.begin();
            dictionary<param_descrs*>::iterator end = m_module_param_descrs.end();
            for (; it != end; ++it) {
                out << "[module] " << it->m_key;
                char const * descr = 0;
                if (m_module_descrs.find(it->m_key, descr)) {
                    out << ", description: " << descr;
                }
                out << "\n";
                it->m_value->display(out, indent + 4, smt2_style);
            }
        }
    }

    void normalize(char const * name, /* out */ symbol & mod_name, /* out */ symbol & param_name) {
        if (*name == ':')
            name++;
        std::string tmp = name;
        unsigned n = tmp.size();
        for (unsigned i = 0; i < n; i++) {
            if (tmp[i] >= 'A' && tmp[i] <= 'Z')
                tmp[i] = tmp[i] - 'A' + 'a';
            else if (tmp[i] == '-')
                tmp[i] = '_';
        }
        for (unsigned i = 0; i < n; i++) {
            if (tmp[i] == '.') {
                param_name = tmp.substr(i+1).c_str();
                tmp.resize(i);
                mod_name   = tmp.c_str();
                return;
            }
        }
        param_name = tmp.c_str();
        mod_name   = symbol::null;
    }

    params_ref & get_params(symbol const & mod_name) {
        if (mod_name == symbol::null) {
            return m_params;
        }
        else {
            params_ref * p = 0;
            if (!m_module_params.find(mod_name, p)) {
                p = alloc(params_ref);
                m_module_params.insert(mod_name, p);
            }
            SASSERT(p != 0);
            return *p;
        }
    }

    void throw_unknown_parameter(symbol const & param_name, symbol const & mod_name) {
        if (mod_name == symbol::null) {
            if (is_old_param_name(param_name)) {
                throw exception("unknown parameter '%s', this is an old parameter name, invoke 'z3 -ps' to obtain the new parameter list", 
                                param_name.bare_str());
            }
            else {
                throw exception("unknown parameter '%s'", param_name.bare_str());
            }
        }
        else {
            throw exception("unknown parameter '%s' at module '%s'", param_name.bare_str(), mod_name.bare_str());
        }
    }

    void set(param_descrs const & d, symbol const & param_name, char const * value, symbol const & mod_name) {
        param_kind k = d.get_kind(param_name);
        params_ref & ps = get_params(mod_name);
        if (k == CPK_INVALID) {
            throw_unknown_parameter(param_name, mod_name);
        }
        else if (k == CPK_UINT) {
            long val = strtol(value, 0, 10);
            ps.set_uint(param_name, static_cast<unsigned>(val));
        }
        else if (k == CPK_BOOL) {
            if (strcmp(value, "true") == 0) {
                ps.set_bool(param_name, true);
            }
            else if (strcmp(value, "false") == 0) {
                ps.set_bool(param_name, false);
            }
            else {
                if (mod_name == symbol::null)
                    throw exception("invalid value '%s' for Boolean parameter '%s'", value, param_name.bare_str());
                else
                    throw exception("invalid value '%s' for Boolean parameter '%s' at module '%s'", value, param_name.bare_str(), mod_name.bare_str());
            }
        }
        else if (k == CPK_SYMBOL) {
            ps.set_sym(param_name, symbol(value));
        }
        else if (k == CPK_STRING) {
            ps.set_str(param_name, value);
        }
        else {
            if (mod_name == symbol::null)
                throw exception("unsupported parameter type '%s'", param_name.bare_str());
            else
                throw exception("unsupported parameter type '%s' at module '%s'", param_name.bare_str(), mod_name.bare_str());
        }
    }

    void set(char const * name, char const * value) {
        bool error = false;
        std::string error_msg;
        #pragma omp critical (gparams)
        {
            try {
                symbol m, p;
                normalize(name, m, p);
                if (m == symbol::null) {
                    set(m_param_descrs, p, value, m);
                }
                else {
                    param_descrs * d;
                    if (m_module_param_descrs.find(m, d)) {
                        set(*d, p, value, m);
                    }
                    else {
                        throw exception("invalid parameter, unknown module '%s'", m.bare_str());
                    }
                }
            }
            catch (exception & ex) {
                // Exception cannot cross critical section boundaries.
                error = true;
                error_msg = ex.msg();
            }
        }
        if (error)
            throw exception(error_msg);
    }

    std::string get_value(params_ref const & ps, symbol const & p) {
        std::ostringstream buffer;
        ps.display(buffer, p);
        return buffer.str();
    }

    std::string get_default(param_descrs const & d, symbol const & p, symbol const & m) {
        if (!d.contains(p)) {
            throw_unknown_parameter(p, m);
        }
        char const * r = d.get_default(p);
        if (r == 0) 
            return "default";
        return r;
    }

    std::string get_value(char const * name) {
        std::string r;
        bool error = false;
        std::string error_msg;
        #pragma omp critical (gparams)
        {
            try {
                symbol m, p;
                normalize(name, m, p);
                if (m == symbol::null) {
                    if (m_params.contains(p)) {
                        r = get_value(m_params, p);
                    }
                    else {
                        r = get_default(m_param_descrs, p, m);
                    }
                }
                else {
                    params_ref * ps = 0;
                    if (m_module_params.find(m, ps) && ps->contains(p)) {
                        r = get_value(*ps, p);
                    }
                    else {
                        param_descrs * d;
                        if (m_module_param_descrs.find(m, d)) {
                            r = get_default(*d, p, m);
                        }
                        else {
                            throw exception("unknown module '%s'", m.bare_str());
                        }
                    }
                }
            }
            catch (exception & ex) {
                // Exception cannot cross critical section boundaries.
                error = true;
                error_msg = ex.msg();
            }
        }
        if (error)
            throw exception(error_msg);
        return r;
    }

    params_ref get_module(symbol const & module_name) {
        params_ref result;
        params_ref * ps = 0;
        #pragma omp critical (gparams)
        {
            if (m_module_params.find(module_name, ps)) {
                result = *ps;
            }
        }
        return result;
    }
    
    params_ref get() { 
        params_ref result;
        TRACE("gparams", tout << "get() m_params: " << m_params << "\n";);
        #pragma omp critical (gparams)
        {
            result = m_params;
        }
        return result;
    }

};

gparams::imp * gparams::g_imp = 0;

void gparams::set(char const * name, char const * value) {
    TRACE("gparams", tout << "setting [" << name << "] <- '" << value << "'\n";);
    SASSERT(g_imp != 0);
    g_imp->set(name, value);
}

void gparams::set(symbol const & name, char const * value) {
    SASSERT(g_imp != 0);
    g_imp->set(name.bare_str(), value);
}

std::string gparams::get_value(char const * name) {
    SASSERT(g_imp != 0);
    return g_imp->get_value(name);
}

std::string gparams::get_value(symbol const & name) {
    SASSERT(g_imp != 0);
    return g_imp->get_value(name.bare_str());
}

void gparams::register_global(param_descrs & d) {
    SASSERT(g_imp != 0);
    g_imp->register_global(d);
}

void gparams::register_module(char const * module_name, param_descrs * d) {
    SASSERT(g_imp != 0);
    g_imp->register_module(module_name, d);
}

void gparams::register_module_descr(char const * module_name, char const * descr) {
    SASSERT(g_imp != 0);
    g_imp->register_module_descr(module_name, descr);
}

params_ref gparams::get_module(char const * module_name) {
    return get_module(symbol(module_name));
}

params_ref gparams::get_module(symbol const & module_name) {
    SASSERT(g_imp != 0);
    return g_imp->get_module(module_name);
}

params_ref gparams::get() {
    TRACE("gparams", tout << "gparams::get()\n";);
    SASSERT(g_imp != 0);
    return g_imp->get();
}

void gparams::display(std::ostream & out, unsigned indent, bool smt2_style) {
    SASSERT(g_imp != 0);
    g_imp->display(out, indent, smt2_style);
}

void gparams::init() {
    TRACE("gparams", tout << "gparams::init()\n";);
    g_imp = alloc(imp);
    gparams_register_modules();
}

void gparams::finalize() {
    TRACE("gparams", tout << "gparams::finalize()\n";);
    if (g_imp != 0) {
        dealloc(g_imp);
        g_imp = 0;
    }
}


