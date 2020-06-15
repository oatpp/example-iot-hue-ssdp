#ifndef GenericResponseDto_hpp
#define GenericResponseDto_hpp

#include "oatpp/core/macro/codegen.hpp"
#include "oatpp/core/Types.hpp"

#include OATPP_CODEGEN_BEGIN(DTO)

/**
 *  The "success" and "error" messages by hue hubs are extremely bad engineered.
 *  This DTO is a mere try to type their structure...
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
