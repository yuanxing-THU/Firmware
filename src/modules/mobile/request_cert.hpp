#pragma once

#include <airdog/hwinfo/version.hpp>
#include <airdog/hwinfo/stamp.hpp>
#include <stm32f4/flash.hpp>

#include "request_base.hpp"

template <>
struct Request< CMD_INFO_CERT >
{
	using value_type = void;
};

errcode_t
verify_request(Request< CMD_INFO_CERT >)
{
	using namespace AirDog::HardwareInfo;
	bool ok = otp_is_valid();
	return ok ? ERRCODE_OK : ERRCODE_OTP_ERROR;
}

template <typename Device>
void
reply(Request< CMD_INFO_CERT >, Device & dev)
{
	using namespace AirDog::HardwareInfo;

	board_id_t board_id;
	fill(board_id);

	InfoCertReply r;
	static_assert(sizeof r.doc == sizeof board_id,
			"Signature document size mismatch.");

	memcpy(r.doc, &board_id, sizeof r.doc);
	signature_copy(r.sig);
	r.cert_no = signature_key_id();

	write(dev, &r, sizeof r);
}
