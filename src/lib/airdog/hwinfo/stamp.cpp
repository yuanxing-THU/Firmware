#include <systemlib/otp.h>

#include "stamp.hpp"
#include "version.hpp"

namespace AirDog
{
namespace HardwareInfo
{

auto & factory_stamp = *(const FactoryStamp *)ADDR_OTP_START;

uint32_t
version_otp() { return factory_stamp.version_tag; }

uint32_t
version_hw()
{
	switch (factory_stamp.version_tag)
	{
	case 1:
		return factory_stamp.header.v1.version_hw;
	default:
		return 0xFFffFFff; /* let it look like uninitialized OTP; */
	}
}

uint32_t
signature_key_id()
{
	switch (factory_stamp.version_tag)
	{
	case 1:
		return factory_stamp.header.v1.sign_key_id;
	default:
		return 0xFFffFFff; /* let it look like uninitialized OTP; */
	}
}

}
// end of namespace HardwareInfo
}
// end of namespace AirDog
