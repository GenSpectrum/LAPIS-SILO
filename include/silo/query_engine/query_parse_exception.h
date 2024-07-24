#pragma once

#include <stdexcept>
#include <string>

#define CHECK_SILO_QUERY(condition, message)    \
   if (!(condition)) {                          \
      throw silo::QueryParseException(message); \
   }

namespace silo {

class [[maybe_unused]] QueryException : public std::runtime_error {
  protected:
   explicit QueryException(const std::string& error_message);

  public:
   /// A short string describing the phase or similar of query
   /// exception.
   [[nodiscard]] virtual std::string_view duringString() const = 0;
};

class [[maybe_unused]] QueryParseException : public QueryException {
  public:
   [[maybe_unused]] explicit QueryParseException(const std::string& error_message);
   [[nodiscard]] std::string_view duringString() const override;
};

class [[maybe_unused]] QueryEvaluationException : public QueryException {
  public:
   [[maybe_unused]] explicit QueryEvaluationException(const std::string& error_message);
   [[nodiscard]] std::string_view duringString() const override;
};

}  // namespace silo
