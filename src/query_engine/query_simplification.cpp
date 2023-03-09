#include <silo/query_engine/query_engine.h>
#include <syncstream>

using namespace silo;

std::unique_ptr<BoolExpression> NucleotideSymbolEqualsExpression::simplify(
   const Database& /*db*/,
   const DatabasePartition& dbp
) const {
   std::unique_ptr<NucleotideSymbolEqualsExpression> ret =
      std::make_unique<NucleotideSymbolEqualsExpression>(position, value);
   if (value == GENOME_SYMBOL::N && !dbp.seq_store.positions[position].N_indexed) {
      return std::make_unique<PositionHasNucleotideSymbolNExpression>(position);
   }
   if (!individualized && dbp.seq_store.positions[position - 1].flipped_bitmap == value) {  /// Bitmap
                                                                                            /// of
                                                                                            /// position
                                                                                            /// is
                                                                                            /// flipped!
                                                                                            /// Introduce
                                                                                            /// Neg
      ret->individualized = true;
      return std::make_unique<NegatedExpression>(std::move(ret));
   } else {
      return ret;
   }
}
ExpressionType NucleotideSymbolEqualsExpression::type() const {
   return ExpressionType::INDEX_FILTER;
}
NucleotideSymbolEqualsExpression::NucleotideSymbolEqualsExpression() {}
NucleotideSymbolEqualsExpression::NucleotideSymbolEqualsExpression(
   unsigned int position,
   GENOME_SYMBOL value
)
    : position(position),
      value(value) {}
std::string NucleotideSymbolEqualsExpression::toString(const Database& database) {
   std::string res = std::to_string(position) + SYMBOL_REPRESENTATION[value];
   return res;
}

std::unique_ptr<BoolExpression> NucleotideSymbolMaybeExpression::simplify(
   const Database& /*db*/,
   const DatabasePartition& dbp
) const {
   std::unique_ptr<NucleotideSymbolMaybeExpression> ret =
      std::make_unique<NucleotideSymbolMaybeExpression>(position, value);
   if (dbp.seq_store.positions[position - 1].flipped_bitmap == value) {  /// Bitmap of reference is
                                                                         /// flipped! Introduce Neg
      ret->negated = true;
   }
   return ret;
}
ExpressionType NucleotideSymbolMaybeExpression::type() const {
   return ExpressionType::INDEX_FILTER;
}
NucleotideSymbolMaybeExpression::NucleotideSymbolMaybeExpression() {}
NucleotideSymbolMaybeExpression::NucleotideSymbolMaybeExpression(
   unsigned int position,
   GENOME_SYMBOL value
)
    : position(position),
      value(value) {}
std::string NucleotideSymbolMaybeExpression::toString(const Database& database) {
   std::string res = "?" + std::to_string(position) + SYMBOL_REPRESENTATION[value];
   return res;
}

std::unique_ptr<BoolExpression> PangoLineageExpression::simplify(
   const Database& /*db*/,
   const DatabasePartition& dbp
) const {
   if (lineageKey == UINT32_MAX) {
      return std::make_unique<EmptyExpression>();
   }
   if (include_sublineages && dbp.meta_store.sublineage_bitmaps[lineageKey].isEmpty()) {
      return std::make_unique<EmptyExpression>();
   } else if (!include_sublineages && dbp.meta_store.lineage_bitmaps[lineageKey].isEmpty()) {
      return std::make_unique<EmptyExpression>();
   } else {
      return std::make_unique<PangoLineageExpression>(lineageKey, include_sublineages);
   }
}
ExpressionType PangoLineageExpression::type() const {
   return ExpressionType::INDEX_FILTER;
}
PangoLineageExpression::PangoLineageExpression(uint32_t lineage_key, bool include_sublineages)
    : lineageKey(lineage_key),
      include_sublineages(include_sublineages) {}
std::string PangoLineageExpression::toString(const Database& database) {
   std::string res = database.dict->get_pango(lineageKey);
   if (include_sublineages) {
      res += ".*";
   }
   return res;
}

std::unique_ptr<BoolExpression> CountryExpression::simplify(
   const Database& /*db*/,
   const DatabasePartition& dbp
) const {
   if (country_key == UINT32_MAX || dbp.meta_store.country_bitmaps[country_key].isEmpty()) {
      return std::make_unique<EmptyExpression>();
   } else {
      return std::make_unique<CountryExpression>(country_key);
   }
}
CountryExpression::CountryExpression(uint32_t country_key)
    : country_key(country_key) {}
std::string CountryExpression::toString(const Database& database) {
   std::string res = "Country=" + database.dict->get_country(country_key);
   return res;
}

std::unique_ptr<BoolExpression> RegionExpression::simplify(
   const Database& /*db*/,
   const DatabasePartition& dbp
) const {
   if (region_key == UINT32_MAX || dbp.meta_store.region_bitmaps[region_key].isEmpty()) {
      return std::make_unique<EmptyExpression>();
   } else {
      return std::make_unique<RegionExpression>(region_key);
   }
}
ExpressionType RegionExpression::type() const {
   return ExpressionType::INDEX_FILTER;
}
RegionExpression::RegionExpression(uint32_t regionKey)
    : region_key(regionKey) {}
std::string RegionExpression::toString(const Database& database) {
   std::string res = "Region=" + database.dict->get_region(region_key);
   return res;
}

std::unique_ptr<BoolExpression> AndExpression::simplify(
   const Database& database,
   const DatabasePartition& database_partition
) const {
   std::vector<std::unique_ptr<BoolExpression>> new_children;
   std::transform(
      children.begin(), children.end(), std::back_inserter(new_children),
      [&](const std::unique_ptr<BoolExpression>& c) {
         return c->simplify(database, database_partition);
      }
   );
   std::unique_ptr<AndExpression> ret = std::make_unique<AndExpression>();
   for (unsigned i = 0; i < new_children.size(); i++) {
      auto& child = new_children[i];
      if (child->type() == FULL) {
         continue;
      } else if (child->type() == EMPTY) {
         return std::make_unique<EmptyExpression>();
      } else if (child->type() == AND) {
         AndExpression* and_child = dynamic_cast<AndExpression*>(child.get());
         std::transform(
            and_child->children.begin(), and_child->children.end(),
            std::back_inserter(new_children),
            [&](std::unique_ptr<BoolExpression>& c) { return std::move(c); }
         );
         std::transform(
            and_child->negated_children.begin(), and_child->negated_children.end(),
            std::back_inserter(ret->negated_children),
            [&](std::unique_ptr<BoolExpression>& c) { return std::move(c); }
         );
      } else if (child->type() == NEG) {
         NegatedExpression* negated_child = dynamic_cast<NegatedExpression*>(child.get());
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
AndExpression::AndExpression() {}
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
      [&](const std::unique_ptr<BoolExpression>& c) {
         return c->simplify(database, database_partition);
      }
   );
   std::unique_ptr<OrExpression> ret = std::make_unique<OrExpression>();
   for (unsigned i = 0; i < new_children.size(); i++) {
      auto& child = new_children[i];
      if (child->type() == EMPTY) {
         continue;
      } else if (child->type() == FULL) {
         return std::make_unique<FullExpression>();
      } else if (child->type() == OR) {
         OrExpression* or_child = dynamic_cast<OrExpression*>(child.get());
         std::transform(
            or_child->children.begin(), or_child->children.end(), std::back_inserter(new_children),
            [&](std::unique_ptr<BoolExpression>& c) { return std::move(c); }
         );
      } else {
         ret->children.push_back(std::move(child));
      }
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
   } else {
      return ret;
   }
}
OrExpression::OrExpression() {}
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
NegatedExpression::NegatedExpression() {}
NegatedExpression::NegatedExpression(std::unique_ptr<BoolExpression> child)
    : child(std::move(child)) {}
std::string NegatedExpression::toString(const Database& database) {
   std::string res = "!" + child->toString(database);
   return res;
}

std::unique_ptr<BoolExpression> NOfExpression::simplify(
   const Database& db,
   const DatabasePartition& dbp
) const {
   std::vector<std::unique_ptr<BoolExpression>> new_children;
   std::transform(
      children.begin(), children.end(), std::back_inserter(new_children),
      [&](const std::unique_ptr<BoolExpression>& c) { return c->simplify(db, dbp); }
   );
   std::unique_ptr<NOfExpression> ret = std::make_unique<NOfExpression>(n, impl, exactly);
   for (unsigned i = 0; i < new_children.size(); i++) {
      auto& child = new_children[i];
      if (child->type() == EMPTY) {
         continue;
      } else if (child->type() == FULL) {
         if (ret->n == 0) {
            if (ret->exactly) {
               return std::make_unique<EmptyExpression>();
            } else {
               return std::make_unique<FullExpression>();
            }
         }
         ret->n--;
      } else {
         ret->children.push_back(std::move(child));
      }
   }
   if (ret->n > ret->children.size()) {
      return std::make_unique<EmptyExpression>();
   }
   if (ret->n == ret->children.size()) {
      auto new_ret = std::make_unique<AndExpression>();
      for (auto& child : ret->children) {
         new_ret->children.emplace_back(std::move(child));
      }
      return new_ret->simplify(db, dbp);
   }
   if (ret->n == 0) {
      if (ret->exactly) {
         auto new_ret = std::make_unique<AndExpression>();
         for (auto& child : ret->children) {
            new_ret->children.emplace_back(std::make_unique<NegatedExpression>(std::move(child)));
         }
         return new_ret->simplify(db, dbp);
      } else {
         return std::make_unique<FullExpression>();
      }
   }
   if (ret->n == 1 && !ret->exactly) {
      auto new_ret = std::make_unique<OrExpression>();
      for (auto& child : ret->children) {
         new_ret->children.emplace_back(std::move(child));
      }
      return new_ret;
   }
   if (ret->children.empty()) {
      std::cerr << "NOf simplification bug: children empty, n>0, but no not children.size() < n?"
                << std::endl;
      return std::make_unique<EmptyExpression>();
   }
   if (ret->children.size() == 1) {
      std::cerr << "NOf simplification bug: 0 < n < children.size(), but children.size() == 1?"
                << std::endl;
      return std::move(ret->children[0]);
   }
   return ret;
}
ExpressionType NOfExpression::type() const {
   return ExpressionType::NOF;
}
NOfExpression::NOfExpression(unsigned int n, bool exactly, unsigned int impl)
    : n(n),
      impl(impl),
      exactly(exactly) {}

std::string NOfExpression::toString(const Database& database) {
   std::string res;
   if (exactly) {
      res = "[exactly-" + std::to_string(n) + "-of:";
   } else {
      res = "[" + std::to_string(n) + "-of:";
   }
   for (auto& child : children) {
      res += child->toString(database);
      res += ", ";
   }
   res += "]";
   return res;
}
