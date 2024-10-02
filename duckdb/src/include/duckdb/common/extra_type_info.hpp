//===----------------------------------------------------------------------===//
//                         DuckDB
//
// duckdb/common/extra_type_info.hpp
//
//
//===----------------------------------------------------------------------===//

#pragma once

#include "duckdb/common/common.hpp"
#include "duckdb/common/types/vector.hpp"

namespace duckdb {

//! Extra Type Info Type
enum class ExtraTypeInfoType : uint8_t {
	INVALID_TYPE_INFO = 0,
	GENERIC_TYPE_INFO = 1,
	DECIMAL_TYPE_INFO = 2,
	STRING_TYPE_INFO = 3,
	LIST_TYPE_INFO = 4,
	STRUCT_TYPE_INFO = 5,
	ENUM_TYPE_INFO = 6,
	USER_TYPE_INFO = 7,
	AGGREGATE_STATE_TYPE_INFO = 8,
	ARRAY_TYPE_INFO = 9,
	ANY_TYPE_INFO = 10,
	INTEGER_LITERAL_TYPE_INFO = 11
};

struct ExtraTypeInfo {
	explicit ExtraTypeInfo(ExtraTypeInfoType type);
	explicit ExtraTypeInfo(ExtraTypeInfoType type, string alias);
	virtual ~ExtraTypeInfo();

	ExtraTypeInfoType type;
	string alias;
	vector<Value> modifiers;

public:
	bool Equals(ExtraTypeInfo *other_p) const;

	static shared_ptr<ExtraTypeInfo> Deserialize(Deserializer &source);
	virtual shared_ptr<ExtraTypeInfo> Copy() const;

	template <class TARGET>
	TARGET &Cast() {
		DynamicCastCheck<TARGET>(this);
		return reinterpret_cast<TARGET &>(*this);
	}
	template <class TARGET>
	const TARGET &Cast() const {
		DynamicCastCheck<TARGET>(this);
		return reinterpret_cast<const TARGET &>(*this);
	}

protected:
	virtual bool EqualsInternal(ExtraTypeInfo *other_p) const;
};

struct DecimalTypeInfo : public ExtraTypeInfo {
	DecimalTypeInfo(uint8_t width_p, uint8_t scale_p);

	uint8_t width;
	uint8_t scale;

public:
	static shared_ptr<ExtraTypeInfo> Deserialize(Deserializer &source);
	shared_ptr<ExtraTypeInfo> Copy() const override;

protected:
	bool EqualsInternal(ExtraTypeInfo *other_p) const override;

private:
	DecimalTypeInfo();
};

struct StringTypeInfo : public ExtraTypeInfo {
	explicit StringTypeInfo(string collation_p);

	string collation;

public:
	static shared_ptr<ExtraTypeInfo> Deserialize(Deserializer &source);
	shared_ptr<ExtraTypeInfo> Copy() const override;

protected:
	bool EqualsInternal(ExtraTypeInfo *other_p) const override;

private:
	StringTypeInfo();
};

struct ListTypeInfo : public ExtraTypeInfo {
	explicit ListTypeInfo(LogicalType child_type_p);

	LogicalType child_type;

public:
	static shared_ptr<ExtraTypeInfo> Deserialize(Deserializer &source);
	shared_ptr<ExtraTypeInfo> Copy() const override;

protected:
	bool EqualsInternal(ExtraTypeInfo *other_p) const override;

private:
	ListTypeInfo();
};

struct StructTypeInfo : public ExtraTypeInfo {
	explicit StructTypeInfo(child_list_t<LogicalType> child_types_p);

	child_list_t<LogicalType> child_types;

public:
	static shared_ptr<ExtraTypeInfo> Deserialize(Deserializer &deserializer);
	shared_ptr<ExtraTypeInfo> Copy() const override;

protected:
	bool EqualsInternal(ExtraTypeInfo *other_p) const override;

private:
	StructTypeInfo();
};

struct AggregateStateTypeInfo : public ExtraTypeInfo {
	explicit AggregateStateTypeInfo(aggregate_state_t state_type_p);

	aggregate_state_t state_type;

public:
	static shared_ptr<ExtraTypeInfo> Deserialize(Deserializer &source);
	shared_ptr<ExtraTypeInfo> Copy() const override;

protected:
	bool EqualsInternal(ExtraTypeInfo *other_p) const override;

private:
	AggregateStateTypeInfo();
};

struct UserTypeInfo : public ExtraTypeInfo {
	explicit UserTypeInfo(string name_p);
	UserTypeInfo(string name_p, vector<Value> modifiers_p);
	UserTypeInfo(string catalog_p, string schema_p, string name_p, vector<Value> modifiers_p);

	string catalog;
	string schema;
	string user_type_name;
	vector<Value> user_type_modifiers;

public:
	static shared_ptr<ExtraTypeInfo> Deserialize(Deserializer &source);
	shared_ptr<ExtraTypeInfo> Copy() const override;

protected:
	bool EqualsInternal(ExtraTypeInfo *other_p) const override;

private:
	UserTypeInfo();
};

// If this type is primarily stored in the catalog or not. Enums from Pandas/Factors are not in the catalog.
enum EnumDictType : uint8_t { INVALID = 0, VECTOR_DICT = 1 };

struct ArrayTypeInfo : public ExtraTypeInfo {
	LogicalType child_type;
	uint32_t size;
	explicit ArrayTypeInfo(LogicalType child_type_p, uint32_t size_p);

public:
	static shared_ptr<ExtraTypeInfo> Deserialize(Deserializer &reader);
	shared_ptr<ExtraTypeInfo> Copy() const override;

protected:
	bool EqualsInternal(ExtraTypeInfo *other_p) const override;
};

struct AnyTypeInfo : public ExtraTypeInfo {
	AnyTypeInfo(LogicalType target_type, idx_t cast_score);

	LogicalType target_type;
	idx_t cast_score;

public:
	static shared_ptr<ExtraTypeInfo> Deserialize(Deserializer &source);
	shared_ptr<ExtraTypeInfo> Copy() const override;

protected:
	bool EqualsInternal(ExtraTypeInfo *other_p) const override;

private:
	AnyTypeInfo();
};

struct IntegerLiteralTypeInfo : public ExtraTypeInfo {
	explicit IntegerLiteralTypeInfo(Value constant_value);

	Value constant_value;

public:
	static shared_ptr<ExtraTypeInfo> Deserialize(Deserializer &source);
	shared_ptr<ExtraTypeInfo> Copy() const override;

protected:
	bool EqualsInternal(ExtraTypeInfo *other_p) const override;

private:
	IntegerLiteralTypeInfo();
};

} // namespace duckdb
