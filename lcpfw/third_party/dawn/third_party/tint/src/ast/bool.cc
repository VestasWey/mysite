// Copyright 2020 The Tint Authors.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "src/ast/bool.h"

#include "src/program_builder.h"

TINT_INSTANTIATE_TYPEINFO(tint::ast::Bool);

namespace tint {
namespace ast {

Bool::Bool(ProgramID program_id, const Source& source)
    : Base(program_id, source) {}

Bool::Bool(Bool&&) = default;

Bool::~Bool() = default;

std::string Bool::type_name() const {
  return "__bool";
}

std::string Bool::FriendlyName(const SymbolTable&) const {
  return "bool";
}

Bool* Bool::Clone(CloneContext* ctx) const {
  auto src = ctx->Clone(source());
  return ctx->dst->create<Bool>(src);
}

}  // namespace ast
}  // namespace tint
