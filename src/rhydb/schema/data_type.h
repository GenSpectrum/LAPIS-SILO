#pragma once

#include <compare>
#include <cstdint>
#include <initializer_list>
#include <optional>
#include <string_view>

#include <boost/serialization/access.hpp>
#include <boost/serialization/split_member.hpp>

#include "rhydb/common/panic.h"

namespace rhydb::schema {

/// The type of a column or value, composed like Arrow's data types: every kind but
/// `DICTIONARY_ENCODED` is a complete type on its own, a dictionary-encoded type has the type of
/// its dictionary values as child (see `dictionaryEncoded`).
class DataType {
  public:
   /// The discriminator of a type (like clang's `Decl::Kind`). Code that handles each type
   /// differently switches over `getKind()` and reads the parameters of parameterized kinds from
   /// the `DataType` itself.
   enum class Kind : uint8_t {
      STRING,
      DICTIONARY_ENCODED,
      DATE32,
      BOOL,
      INT32,
      INT64,
      FLOAT,
      AMINO_ACID_SEQUENCE,
      NUCLEOTIDE_SEQUENCE,
      ZSTD_COMPRESSED_STRING,
   };

  private:
   Kind kind = Kind::STRING;
   /// The child type of a parameterized kind (the dictionary values of `DICTIONARY_ENCODED`), or
   /// nullptr. Children are canonical instances (see `canonical`), which keeps a `DataType` a
   /// cheap `constexpr` value.
   const DataType* child = nullptr;

   constexpr DataType(Kind kind, const DataType* child)
       : kind(kind),
         child(child) {}

   explicit constexpr DataType(Kind kind)
       : kind(kind) {}

   /// The canonical instance of the non-parameterized `kind`
   [[nodiscard]] static constexpr const DataType& canonical(Kind kind);

  public:
   static const DataType STRING;
   static const DataType DATE32;
   static const DataType BOOL;
   static const DataType INT32;
   static const DataType INT64;
   static const DataType FLOAT;
   static const DataType AMINO_ACID_SEQUENCE;
   static const DataType NUCLEOTIDE_SEQUENCE;
   static const DataType ZSTD_COMPRESSED_STRING;

   /// Only needed to deserialize into; yields `STRING`
   constexpr DataType() = default;

   [[nodiscard]] static constexpr bool canBeDictionaryEncoded(const DataType& value_type) {
      return value_type.kind == Kind::STRING;
   }

   /// A column storing each distinct value of type `value_type` once in a sorted dictionary
   [[nodiscard]] static constexpr DataType dictionaryEncoded(const DataType& value_type);

   [[nodiscard]] constexpr Kind getKind() const { return kind; }

   [[nodiscard]] constexpr bool isDictionaryEncoded() const {
      return kind == Kind::DICTIONARY_ENCODED;
   }

   /// The type of the dictionary values of a `DICTIONARY_ENCODED` type
   [[nodiscard]] constexpr DataType getDictionaryValueType() const {
      RHYDB_ASSERT(isDictionaryEncoded());
      return *child;
   }

   /// The type of a single value of this column once read: the dictionary value type for a
   /// dictionary-encoded type, the type itself otherwise
   [[nodiscard]] constexpr DataType getPlainType() const {
      return isDictionaryEncoded() ? *child : *this;
   }

   // NOLINTNEXTLINE(misc-no-recursion)
   constexpr bool operator==(const DataType& other) const {
      if (kind != other.kind || (child == nullptr) != (other.child == nullptr)) {
         return false;
      }
      return child == nullptr || *child == *other.child;
   }

   // NOLINTNEXTLINE(misc-no-recursion)
   constexpr std::strong_ordering operator<=>(const DataType& other) const {
      if (const auto ordering = kind <=> other.kind; ordering != 0) {
         return ordering;
      }
      // A type without child sorts first
      if (child == nullptr || other.child == nullptr) {
         if (child == other.child) {
            return std::strong_ordering::equal;
         }
         return child == nullptr ? std::strong_ordering::less : std::strong_ordering::greater;
      }
      return *child <=> *other.child;
   }

  private:
   friend class boost::serialization::access;

   template <class Archive>
   void save(Archive& archive, const uint32_t /*version*/) const {
      // clang-format off
      archive & kind;
      if (child != nullptr) {
         archive & child->kind;
      }
      // clang-format on
   }

   template <class Archive>
   void load(Archive& archive, const uint32_t /*version*/) {
      // clang-format off
      archive & kind;
      child = nullptr;
      if (kind == Kind::DICTIONARY_ENCODED) {
         Kind child_kind;
         archive & child_kind;
         child = &canonical(child_kind);
      }
      // clang-format on
   }

   BOOST_SERIALIZATION_SPLIT_MEMBER()
};

inline constexpr DataType DataType::STRING{Kind::STRING};
inline constexpr DataType DataType::DATE32{Kind::DATE32};
inline constexpr DataType DataType::BOOL{Kind::BOOL};
inline constexpr DataType DataType::INT32{Kind::INT32};
inline constexpr DataType DataType::INT64{Kind::INT64};
inline constexpr DataType DataType::FLOAT{Kind::FLOAT};
inline constexpr DataType DataType::AMINO_ACID_SEQUENCE{Kind::AMINO_ACID_SEQUENCE};
inline constexpr DataType DataType::NUCLEOTIDE_SEQUENCE{Kind::NUCLEOTIDE_SEQUENCE};
inline constexpr DataType DataType::ZSTD_COMPRESSED_STRING{Kind::ZSTD_COMPRESSED_STRING};

constexpr const DataType& DataType::canonical(Kind kind) {
   switch (kind) {
      case Kind::STRING:
         return STRING;
      case Kind::DATE32:
         return DATE32;
      case Kind::BOOL:
         return BOOL;
      case Kind::INT32:
         return INT32;
      case Kind::INT64:
         return INT64;
      case Kind::FLOAT:
         return FLOAT;
      case Kind::AMINO_ACID_SEQUENCE:
         return AMINO_ACID_SEQUENCE;
      case Kind::NUCLEOTIDE_SEQUENCE:
         return NUCLEOTIDE_SEQUENCE;
      case Kind::ZSTD_COMPRESSED_STRING:
         return ZSTD_COMPRESSED_STRING;
      case Kind::DICTIONARY_ENCODED:
         break;
   }
   RHYDB_UNREACHABLE();
}

constexpr DataType DataType::dictionaryEncoded(const DataType& value_type) {
   RHYDB_ASSERT(canBeDictionaryEncoded(value_type));
   return DataType{Kind::DICTIONARY_ENCODED, &canonical(value_type.kind)};
}

/// The name of a non-parameterized type in a schema definition (database config, `createTable`)
constexpr std::string_view toSchemaTypeName(DataType type) {
   switch (type.getKind()) {
      case DataType::Kind::STRING:
         return "string";
      case DataType::Kind::DATE32:
         return "date";
      case DataType::Kind::BOOL:
         return "boolean";
      case DataType::Kind::INT32:
         return "int32";
      case DataType::Kind::INT64:
         return "int64";
      case DataType::Kind::FLOAT:
         return "float";
      case DataType::Kind::NUCLEOTIDE_SEQUENCE:
         return "nucleotideSequence";
      case DataType::Kind::AMINO_ACID_SEQUENCE:
         return "aminoAcidSequence";
      case DataType::Kind::ZSTD_COMPRESSED_STRING:
         return "zstdCompressedString";
      case DataType::Kind::DICTIONARY_ENCODED:
         break;
   }
   RHYDB_UNREACHABLE();
}

/// The non-parameterized type named `name` in a schema definition (see `toSchemaTypeName`), which
/// also accepts the alias `int`
constexpr std::optional<DataType> fromSchemaTypeName(std::string_view name) {
   if (name == "int") {
      return DataType::INT32;
   }
   for (const DataType type : {
           DataType::STRING,
           DataType::DATE32,
           DataType::BOOL,
           DataType::INT32,
           DataType::INT64,
           DataType::FLOAT,
           DataType::NUCLEOTIDE_SEQUENCE,
           DataType::AMINO_ACID_SEQUENCE,
           DataType::ZSTD_COMPRESSED_STRING,
        }) {
      if (toSchemaTypeName(type) == name) {
         return type;
      }
   }
   return std::nullopt;
}

}  // namespace rhydb::schema
