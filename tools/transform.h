#pragma once

// Copyright (c) 2018-present The Alive2 Authors.
// Distributed under the MIT license that can be found in the LICENSE file.

#include "ir/function.h"
#include "ir/state.h"
#include "smt/solver.h"
#include "util/errors.h"
#include <memory>
#include <set>
#include <ostream>
#include <string>
#include <unordered_map>

class Cache;

namespace tools {

struct TransformPrintOpts {
  bool print_fn_header = true;
  bool skip_tgt = false;
};


struct Transform {
  std::string name;
  IR::Function src, tgt;
  IR::Predicate *precondition = nullptr;

  void preprocess();
  void print(std::ostream &os, const TransformPrintOpts &opt = {}) const;
  friend std::ostream& operator<<(std::ostream &os, const Transform &t);
};


class TypingAssignments {
  smt::Solver s, sneg;
  smt::Result r;
  bool has_only_one_solution = false;
  bool is_unsat = false;
  TypingAssignments(const smt::expr &e);

public:
  bool operator!() const { return !(bool)*this; }
  operator bool() const;
  void operator++(void);
  bool hasSingleTyping() const { return has_only_one_solution; }

  friend class TransformVerify;
};


class TransformVerify {
  Transform &t;
  std::unordered_map<std::string, const IR::Instr*> tgt_instrs;
  bool check_each_var;

public:
  TransformVerify(Transform &t, bool check_each_var);
  std::pair<std::unique_ptr<IR::State>,std::unique_ptr<IR::State>> exec() const;
  util::Errors verify() const;
  TypingAssignments getTypings() const;
  void fixupTypes(const TypingAssignments &ty);
};


void print_model_val(std::ostream &os, const IR::State &st, const smt::Model &m,
                     const IR::Value *var, const IR::Type &type,
                     const IR::StateValue &val, unsigned child = 0);

smt::expr preprocess(const Transform &t, const std::set<smt::expr> &qvars0,
                const std::set<smt::expr> &undef_qvars, smt::expr && e);


using print_var_val_ty = std::function<void(std::ostream&, const smt::Model&)>;

bool error(util::Errors &errs, const IR::State &src_state, const IR::State &tgt_state,
           const smt::Result &r, const IR::Value *var, const char *msg, bool check_each_var,
           print_var_val_ty print_var_val);

}
