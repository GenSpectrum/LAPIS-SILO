#include <silo/query_engine/query_engine.h>
#include <syncstream>

namespace silo {
std::unique_ptr<BoolExpression> NucleotideSymbolEqualsExpression::simplify(
   const Database& /*database*/,
   const DatabasePartition& database_partition
) const {
   std::unique_ptr<NucleotideSymbolEqualsExpression> result =
      std::make_unique<NucleotideSymbolEqualsExpression>(position, value);
   if (value == GENOME_SYMBOL::N && !database_partition.seq_store.positions[position].nucleotide_symbol_n_indexed) {
      return std::make_unique<PositionHasNucleotideSymbolNExpression>(position);
   }
   if (!individualized && database_partition.seq_store.positions[position - 1].flipped_bitmap == value) {
      result->individualized = true;
      return std::make_unique<NegatedExpression>(std::move(result));
   }
   return result;
}
ExpressionType NucleotideSymbolEqualsExpression::type() const {
   return ExpressionType::INDEX_FILTER;
}
NucleotideSymbolEqualsExpression::NucleotideSymbolEqualsExpression() = default;
NucleotideSymbolEqualsExpression::NucleotideSymbolEqualsExpression(
   unsigned int position,
   GENOME_SYMBOL value
)
    : position(position),
      value(value) {}
std::string NucleotideSymbolEqualsExpression::toString(const Database& /*database*/) {
   std::string res = std::to_string(position) + SYMBOL_REPRESENTATION[value];
   return res;
}

std::unique_ptr<BoolExpression> NucleotideSymbolMaybeExpression::simplify(
   const Database& /*database*/,
   const DatabasePartition& database_partition
) const {
   std::unique_ptr<NucleotideSymbolMaybeExpression> ret =
      std::make_unique<NucleotideSymbolMaybeExpression>(position, value);
   if (database_partition.seq_store.positions[position - 1].flipped_bitmap == value) {  /// Bitmap
                                                                                        /// of
                                                                                        /// reference
                                                                                        /// is
      /// flipped! Introduce Neg
      ret->negated = true;
   }
   return ret;
}
ExpressionType NucleotideSymbolMaybeExpression::type() const {
   return ExpressionType::INDEX_FILTER;
}
NucleotideSymbolMaybeExpression::NucleotideSymbolMaybeExpression() = default;
NucleotideSymbolMaybeExpression::NucleotideSymbolMaybeExpression(
   unsigned int position,
   GENOME_SYMBOL value
)
    : position(position),
      value(value) {}
std::string NucleotideSymbolMaybeExpression::toString(const Database& /*database*/) {
   std::string res = "?" + std::to_string(position) + SYMBOL_REPRESENTATION[value];
   return res;
}

std::unique_ptr<BoolExpression> PangoLineageExpression::simplify(
   const Database& /*database*/,
   const DatabasePartition& database_partition
) const {
   if (lineageKey == UINT32_MAX) {
      return std::make_unique<EmptyExpression>();
   }
   if (include_sublineages && database_partition.meta_store.sublineage_bitmaps[lineageKey].isEmpty()) {
      return std::make_unique<EmptyExpression>();
   }
   if (!include_sublineages && database_partition.meta_store.lineage_bitmaps[lineageKey].isEmpty()) {
      return std::make_unique<EmptyExpression>();
   }
   return std::make_unique<PangoLineageExpression>(lineageKey, include_sublineages);
}
ExpressionType PangoLineageExpression::type() const {
   return ExpressionType::INDEX_FILTER;
}
PangoLineageExpression::PangoLineageExpression(uint32_t lineage_key, bool include_sublineages)
    : lineageKey(lineage_key),
      include_sublineages(include_sublineages) {}
std::string PangoLineageExpression::toString(const Database& database) {
   std::string res = database.dict->getPangoLineage(lineageKey);
   if (include_sublineages) {
      res += ".*";
   }
   return res;
}

std::unique_ptr<BoolExpression> CountryExpression::simplify(
   const Database& /*database*/,
   const DatabasePartition& database_partition
) const {
   if (country_key == UINT32_MAX || database_partition.meta_store.country_bitmaps[country_key].isEmpty()) {
      return std::make_unique<EmptyExpression>();
   }
   return std::make_unique<CountryExpression>(country_key);
}
CountryExpression::CountryExpression(uint32_t country_key)
    : country_key(country_key) {}
std::string CountryExpression::toString(const Database& database) {
   std::string res = "Country=" + database.dict->getCountry(country_key);
   return res;
}
ExpressionType CountryExpression::type() const {
   return ExpressionType::INDEX_FILTER;
}

std::unique_ptr<BoolExpression> RegionExpression::simplify(
   const Database& /*database*/,
   const DatabasePartition& database_partition
) const {
   if (region_key == UINT32_MAX || database_partition.meta_store.region_bitmaps[region_key].isEmpty()) {
      return std::make_unique<EmptyExpression>();
   }
   return std::make_unique<RegionExpression>(region_key);
}
ExpressionType RegionExpression::type() const {
   return ExpressionType::INDEX_FILTER;
}
RegionExpression::RegionExpression(uint32_t regionKey)
    : region_key(regionKey) {}
std::string RegionExpression::toString(const Database& database) {
   std::string res = "Region=" + database.dict->getRegion(region_key);
   return res;
}

std::unique_ptr<BoolExpression> AndExpression::simplify(
   const Database& database,
   const DatabasePartition& database_partition
) const {
   std::vector<std::unique_ptr<BoolExpression>> new_children;
   std::transform(
      children.begin(), children.end(), std::back_inserter(new_children),
      [&](const std::unique_ptr<BoolExpression>& expression) {
         return expression->simplify(database, database_partition);
      }
   );
   std::unique_ptr<AndExpression> ret = std::make_unique<AndExpression>();
   for (unsigned i = 0; i < new_children.size(); i++) {
      auto& child = new_children[i];
      if (child->type() == FULL) {
         continue;
      }
      if (child->type() == EMPTY) {
         return std::make_unique<EmptyExpression>();
      }
      if (child->type() == AND) {
         auto* and_child = dynamic_cast<AndExpression*>(child.get());
         std::transform(
            and_child->children.begin(), and_child->children.end(),
            std::back_inserter(new_children),
            [&](std::unique_ptr<BoolExpression>& expression) { return std::move(expression); }
         );
         std::transform(
            and_child->negated_children.begin(), and_child->negated_children.end(),
            std::back_inserter(ret->negated_children),
            [&](std::unique_ptr<BoolExpression>& expression) { return std::move(expression); }
         );
      } else if (child->type() == NEG) {
         auto* negated_child = dynamic_cast<NegatedExpression*>(child.get());
         ret->negated_children.emplace_back(std::move(negated_child->child));
      } else {
         ret->children.push_back(std::move(child));
      }
   }
   if (ret->children.empty() && ret->negated_children.empty()) {
      return std::make_unique<FullExpression>();
   }
   if (ret->children.size() == 1 && ret->negated_children.empty()) {
      return std::move(ret->children[0]);
   }
   if (ret->negated_children.size() == 1 && ret->children.empty()) {
      return std::make_unique<NegatedExpression>(std::move(ret->negated_children[0]));
   }
   if (ret->children.empty()) {
      std::unique_ptr<OrExpression> or_ret = std::make_unique<OrExpression>();
      for (auto& child : ret->negated_children) {
         or_ret->children.push_back(std::move(child));
      }
      return std::make_unique<NegatedExpression>(std::move(or_ret));
   }
   return ret;
}
AndExpression::AndExpression() = default;
ExpressionType AndExpression::type() const {
   return ExpressionType::AND;
}
std::string AndExpression::toString(const Database& database) {
   std::string res = "(";
   for (auto& child : children) {
      res += " & ";
      res += child->toString(database);
   }
   for (auto& child : negated_children) {
      res += " &! ";
      res += child->toString(database);
   }
   res += ")";
   return res;
}

std::unique_ptr<BoolExpression> OrExpression::simplify(
   const Database& database,
   const DatabasePartition& database_partition
) const {
   std::vector<std::unique_ptr<BoolExpression>> new_children;
   std::transform(
      children.begin(), children.end(), std::back_inserter(new_children),
      [&](const std::unique_ptr<BoolExpression>& expression) {
         return expression->simplify(database, database_partition);
      }
   );
   std::unique_ptr<OrExpression> ret = std::make_unique<OrExpression>();
   for (unsigned i = 0; i < new_children.size(); i++) {
      auto& child = new_children[i];
      if (child->type() == EMPTY) {
         continue;
      }
      if (child->type() == FULL) {
         return std::make_unique<FullExpression>();
      }
      if (child->type() == OR) {
         auto* or_child = dynamic_cast<OrExpression*>(child.get());
         std::transform(
            or_child->children.begin(), or_child->children.end(), std::back_inserter(new_children),
            [&](std::unique_ptr<BoolExpression>& expression) { return std::move(expression); }
         );
      }
      ret->children.push_back(std::move(child));
   }
   if (ret->children.empty()) {
      return std::make_unique<EmptyExpression>();
   }
   if (ret->children.size() == 1) {
      return std::move(ret->children[0]);
   }
   if (std::any_of(
          new_children.begin(), new_children.end(),
          [](const std::unique_ptr<BoolExpression>& child) { return child->type() == NEG; }
       )) {
      std::unique_ptr<AndExpression> and_ret = std::make_unique<AndExpression>();
      for (auto& child : ret->children) {
         if (child->type() == NEG) {
            and_ret->negated_children.emplace_back(
               std::move(dynamic_cast<NegatedExpression*>(child.get())->child)
            );
         } else {
            and_ret->children.push_back(std::move(child));
         }
      }
      return and_ret;
   }
   return ret;
}

OrExpression::OrExpression() = default;

ExpressionType OrExpression::type() const {
   return ExpressionType::OR;
}

std::string OrExpression::toString(const Database& database) {
   std::string res = "(";
   for (auto& child : children) {
      res += child->toString(database);
      res += " | ";
   }
   res += ")";
   return res;
}

std::unique_ptr<BoolExpression> NegatedExpression::simplify(
   const Database& database,
   const DatabasePartition& database_partition
) const {
   std::unique_ptr<NegatedExpression> ret =
      std::make_unique<NegatedExpression>(child->simplify(database, database_partition));
   if (ret->child->type() == ExpressionType::NEG) {
      return std::move(dynamic_cast<NegatedExpression*>(ret->child.get())->child);
   }
   return ret;
}

NegatedExpression::NegatedExpression() = default;

NegatedExpression::NegatedExpression(std::unique_ptr<BoolExpression> child)
    : child(std::move(child)) {}

std::string NegatedExpression::toString(const Database& database) {
   std::string res = "!" + child->toString(database);
   return res;
}

ExpressionType NegatedExpression::type() const {
   return ExpressionType::NEG;
}

// TODO(someone): reduce cognitive complexity
// NOLINTNEXTLINE(readability-function-cognitive-complexity)
std::unique_ptr<BoolExpression> NOfExpression::simplify(
   const Database& database,
   const DatabasePartition& database_partition
) const {
   std::vector<std::unique_ptr<BoolExpression>> new_children;
   std::transform(
      children.begin(), children.end(), std::back_inserter(new_children),
      [&](const std::unique_ptr<BoolExpression>& expression) {
         return expression->simplify(database, database_partition);
      }
   );
   std::unique_ptr<NOfExpression> result =
      std::make_unique<NOfExpression>(number_of_matchers, match_exactly, implementation);
   for (auto& child : new_children) {
      if (child->type() == EMPTY) {
         continue;
      }
      if (child->type() == FULL) {
         if (result->number_of_matchers == 0) {
            if (result->match_exactly) {
               return std::make_unique<EmptyExpression>();
            }
            return std::make_unique<FullExpression>();
         }
         result->number_of_matchers--;
      } else {
         result->children.push_back(std::move(child));
      }
   }
   if (result->number_of_matchers > result->children.size()) {
      return std::make_unique<EmptyExpression>();
   }
   if (result->number_of_matchers == result->children.size()) {
      auto new_ret = std::make_unique<AndExpression>();
      for (auto& child : result->children) {
         new_ret->children.emplace_back(std::move(child));
      }
      return new_ret->simplify(database, database_partition);
   }
   if (result->number_of_matchers == 0) {
      if (result->match_exactly) {
         auto new_ret = std::make_unique<AndExpression>();
         for (auto& child : result->children) {
            new_ret->children.emplace_back(std::make_unique<NegatedExpression>(std::move(child)));
         }
         return new_ret->simplify(database, database_partition);
      }
      return std::make_unique<FullExpression>();
   }
   if (result->number_of_matchers == 1 && !result->match_exactly) {
      auto new_ret = std::make_unique<OrExpression>();
      for (auto& child : result->children) {
         new_ret->children.emplace_back(std::move(child));
      }
      return new_ret;
   }
   if (result->children.empty()) {
      std::cerr << "NOf simplification bug: children empty, n>0, but no not children.size() < n?"
                << std::endl;
      return std::make_unique<EmptyExpression>();
   }
   if (result->children.size() == 1) {
      std::cerr << "NOf simplification bug: 0 < n < children.size(), but children.size() == 1?"
                << std::endl;
      return std::move(result->children[0]);
   }
   return result;
}
ExpressionType NOfExpression::type() const {
   return ExpressionType::NOF;
}
NOfExpression::NOfExpression(
   unsigned int number_of_matchers,
   bool match_exactly,
   NOfExpressionImplementation implementation
)
    : number_of_matchers(number_of_matchers),
      implementation(implementation),
      match_exactly(match_exactly) {}

std::string NOfExpression::toString(const Database& database) {
   std::string res;
   if (match_exactly) {
      res = "[exactly-" + std::to_string(number_of_matchers) + "-of:";
   } else {
      res = "[" + std::to_string(number_of_matchers) + "-of:";
   }
   for (auto& child : children) {
      res += child->toString(database);
      res += ", ";
   }
   res += "]";
   return res;
}

}  // namespace silo