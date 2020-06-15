#ifndef UserRegisterDto_hpp
#define UserRegisterDto_hpp

#include "oatpp/core/macro/codegen.hpp"
#include "oatpp/core/Types.hpp"

#include OATPP_CODEGEN_BEGIN(DTO)

/**
 *  The Object send by connecting devices (i.E. Alexa) to initially register them on this 'hub'
 */
class UserRegisterDto : public oatpp::DTO {

  DTO_INIT(UserRegisterDto, DTO);

  // User values
  DTO_FIELD(String, username);
  DTO_FIELD(String, devicetype);

};

#include OATPP_CODEGEN_END(DTO)

#endif /* UserRegisterDto_hpp */
