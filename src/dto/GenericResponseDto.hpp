#ifndef GenericResponseDto_hpp
#define GenericResponseDto_hpp

#include "oatpp/core/macro/codegen.hpp"
#include "oatpp/core/Types.hpp"

#include OATPP_CODEGEN_BEGIN(DTO)

/**
 *  Data Transfer Object. Object containing fields only.
 *  Used in API for serialization/deserialization and validation
 */

class ResponseTypeDto : public oatpp::DTO {

  DTO_INIT(ResponseTypeDto, DTO);

  // ONLY ONE AT A TIME
  DTO_FIELD(Fields<Any>, success);
  DTO_FIELD(Fields<Any>, error);

};

typedef oatpp::List<oatpp::Object<ResponseTypeDto>> GenericResponseDto;

#include OATPP_CODEGEN_END(DTO)

#endif /* GenericResponseDto_hpp */
