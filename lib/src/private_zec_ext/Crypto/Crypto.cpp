#include <zec_ext/Crypto/Crypto.hpp>
#include <mbedtls/ssl.h>
#include <mbedtls/base64.h>
#include <mbedtls/md5.h>

#include <iostream>
using namespace std;

ZEC_NS
{
	static std::string StripString(const char* Source, size_t Length)
	{
		std::string Stripped;
		Stripped.reserve(Length);
		for (size_t i = 0; i < Length; ++i) {
			/// !!! NOTE: null-string-terminator is not considered
			if (isspace(Source[i])) {
				continue;
			}
			Stripped.push_back(Source[i]);
		}
		return Stripped;
	}

	std::string EncodeBase64(const void* Source, size_t Length)
	{
		assert(Length);
		size_t Outlen = 0;
		mbedtls_base64_encode(nullptr, 0, &Outlen, reinterpret_cast<const uint8_t*>(Source), Length);

		std::string Out;
		Out.resize(Outlen);
		mbedtls_base64_encode(reinterpret_cast<uint8_t*>(const_cast<char*>(Out.data())), Out.size(),
			&Outlen, reinterpret_cast<const uint8_t*>(Source), Length);
		// Actual output might be shorter than test result, so the output string shoud be resized again:
		Out.resize(Outlen);
		return Out;
	}

	std::string EncodeBase64URL(const void* Source, size_t Length)
	{
		auto Encoded = EncodeBase64(Source, Length);
		for (auto & c : Encoded) {
			switch(c){
			case '+':
				c = '-';
				break;
			case '/':
				c = '_';
				break;
			default:
				break;
			}
		}
		return Encoded;
	}

	std::string DecodeBase64(const char* Source, size_t Length)
	{
		assert(Length);
		size_t Outlen = 0;
		std::string Stripped = StripString(Source, Length);
		if (MBEDTLS_ERR_BASE64_INVALID_CHARACTER == mbedtls_base64_decode(
				nullptr, 0, &Outlen,
				reinterpret_cast<const uint8_t*>(Stripped.data()), Stripped.size())) {
			return {};
		}

		std::string Out;
		Out.resize(Outlen);
		cout << "OutLen:" << Outlen << endl;
		mbedtls_base64_decode(reinterpret_cast<uint8_t*>(const_cast<char*>(Out.data())), Out.size(),
			&Outlen, reinterpret_cast<const uint8_t*>(Stripped.data()), Stripped.size());
		return Out;
	}

	void DigestMd5(void * Output16, const void * Source, size_t Length)
	{
		mbedtls_md5_ret((const unsigned char *)Source, Length, (unsigned char *)(Output16));
	}

	void HmacMd5(void * Output16, const void * Source, size_t Length, const void * Key, size_t KeyLength)
	{
		mbedtls_md_context_t ctx;
		mbedtls_md_init(&ctx);
		mbedtls_md_setup(&ctx, mbedtls_md_info_from_type(MBEDTLS_MD_MD5), true);
		mbedtls_md_hmac_starts(&ctx,
				reinterpret_cast<const unsigned char *>(Key), KeyLength);
		mbedtls_md_hmac_update(&ctx,
				reinterpret_cast<const unsigned char *>(Source),
				Length);
		mbedtls_md_hmac_finish(&ctx, static_cast<unsigned char *>(Output16));
		mbedtls_md_free(&ctx);
	}

	void HmacSha256(void * Output32, const void * Source, size_t Length, const void * Key, size_t KeyLength)
	{
		mbedtls_md_context_t ctx;
		mbedtls_md_init(&ctx);
		mbedtls_md_setup(&ctx, mbedtls_md_info_from_type(MBEDTLS_MD_SHA256), true);
		mbedtls_md_hmac_starts(&ctx,
				reinterpret_cast<const unsigned char *>(Key), KeyLength);
		mbedtls_md_hmac_update(&ctx,
				reinterpret_cast<const unsigned char *>(Source),
				Length);
		mbedtls_md_hmac_finish(&ctx, static_cast<unsigned char *>(Output32));
		mbedtls_md_free(&ctx);
	}

}
