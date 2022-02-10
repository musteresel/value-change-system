#pragma once

namespace Values {

/// \brief Template definition for the default ChangeManager.
///
/// Please provide a template specialization to a single template argument
/// in your user code in order to select a default ChangeManager for your code
/// base!
///
template <typename X, typename...> struct DefaultChangeManager {
  using type = typename X::PLEASE_IMPLEMENT_SPEC_IN_USER_CODE;
};

} // namespace Values
