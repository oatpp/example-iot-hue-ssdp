#ifndef UserRegisterDto_hpp
#define UserRegisterDto_hpp

#include "oatpp/core/macro/codegen.hpp"
#include "oatpp/core/Types.hpp"

#include OATPP_CODEGEN_BEGIN(DTO)

/**
 *  Data Transfer Object. Object containing fields only.
 *  Used in API for serialization/deserialization and validation
 */
class UserRegisterDto : public oatpp::DTO {

  DTO_INIT(UserRegisterDto, DTO);

  // User values
  DTO_FIELD(String, username);
  DTO_FIELD(String, devicetype);

};

#include OATPP_CODEGEN_END(DTO)

#endif /* UserRegisterDto_hpp */
