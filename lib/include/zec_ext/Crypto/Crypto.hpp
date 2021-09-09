#pragma once
#include <zec/Common.hpp>
#include <string_view>
#include <string>

ZEC_NS
{
	ZEC_API std::string EncodeBase64(const void* Source, size_t Length);
	ZEC_API std::string EncodeBase64URL(const void* Source, size_t Length);
	// DecodeBase64 can decode both normal and url version
	ZEC_API std::string DecodeBase64(const char* Source, size_t Length);

	// shortcut
	ZEC_INLINE std::string EncodeBase64(const std::string_view &Source) {
		return EncodeBase64(Source.data(), Source.length());
	}
	ZEC_INLINE std::string EncodeBase64URL(const std::string_view &Source) {
		return EncodeBase64URL(Source.data(), Source.length());
	}
	ZEC_INLINE std::string DecodeBase64(const std::string_view &String) {
		return DecodeBase64(String.data(), String.length());
	}

	ZEC_API void DigestMd5(void * Output16, const void * Source, size_t Length);
	ZEC_API void HmacMd5(void * Output16, const void * Source, size_t Length, const void * Key, size_t KeyLength);
	ZEC_API void HmacSha256(void * Output32, const void * Source, size_t Length, const void * Key, size_t KeyLength);


	ZEC_INLINE void DigestMd5(void * Output16, const std::string_view & Source)	{
		return DigestMd5(Output16, Source.data(), Source.length());
	}
	ZEC_INLINE void HmacMd5(void * Output16, const std::string_view & Source, const std::string_view & Key) {
		return HmacMd5(Output16, Source.data(), Source.length(), Key.data(), Key.length());
	}
	ZEC_INLINE void HmacSha256(void * Output32, const std::string_view & Source, const std::string_view & Key) {
		return HmacSha256(Output32, Source.data(), Source.length(), Key.data(), Key.length());
	}
}
