#pragma once
#include <zec/Common.hpp>
#include <rapidjson/document.h>
#include <string>

ZEC_NS
{

	ZEC_STATIC_INLINE xOptional<int64_t> GetInt(const rapidjson::Value & JsonValue, const char * Key)
	{
		auto NotFound = JsonValue.MemberEnd();
		auto Iter = JsonValue.FindMember(Key);
		if (Iter == NotFound || !Iter->value.IsInt64()) {
			return {};
		}
		return { Iter->value.GetInt64() };
	}

	ZEC_STATIC_INLINE xOptional<double> GetDouble(const rapidjson::Value & JsonValue, const char * Key)
	{
		auto NotFound = JsonValue.MemberEnd();
		auto Iter = JsonValue.FindMember(Key);
		if (Iter == NotFound || !Iter->value.IsDouble()) {
			return {};
		}
		return { Iter->value.GetDouble() };
	}

	ZEC_STATIC_INLINE xOptional<std::string> GetString(const rapidjson::Value & JsonValue, const char * Key)
	{
		auto NotFound = JsonValue.MemberEnd();
		auto Iter = JsonValue.FindMember(Key);
		if (Iter == NotFound || !Iter->value.IsString()) {
			return {};
		}
		return std::string{ Iter->value.GetString(), Iter->value.GetStringLength() };
	}

	ZEC_STATIC_INLINE xOptional<rapidjson::Value::ConstArray> GetArray(const rapidjson::Value & JsonValue, const char * Key)
	{
		auto NotFound = JsonValue.MemberEnd();
		auto Iter = JsonValue.FindMember(Key);
		if (Iter == NotFound || !Iter->value.IsArray()) {
			return {};
		}
		return Iter->value.GetArray();
	}

	ZEC_STATIC_INLINE xOptional<rapidjson::Value::ConstObject> GetObject(const rapidjson::Value & JsonValue, const char * Key)
	{
		auto NotFound = JsonValue.MemberEnd();
		auto Iter = JsonValue.FindMember(Key);
		if (Iter == NotFound || !Iter->value.IsObject()) {
			return {};
		}
		return Iter->value.GetObject();
	}

}
