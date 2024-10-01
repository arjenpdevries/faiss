//===----------------------------------------------------------------------===//
//                         DuckDB
//
// duckdb/catalog/dependency.hpp
//
//
//===----------------------------------------------------------------------===//

#pragma once

#include "duckdb/common/common.hpp"
#include "duckdb/common/string_util.hpp"
#include "duckdb/common/unordered_set.hpp"

namespace duckdb {
class CatalogEntry;

struct DependencyFlags {
public:
	DependencyFlags() : value(0) {
	}
	DependencyFlags(const DependencyFlags &other) : value(other.value) {
	}
	virtual ~DependencyFlags() = default;
	DependencyFlags &operator=(const DependencyFlags &other) {
		value = other.value;
		return *this;
	}
	bool operator==(const DependencyFlags &other) const {
		return other.value == value;
	}
	bool operator!=(const DependencyFlags &other) const {
		return !(*this == other);
	}

public:
	virtual string ToString() const = 0;

protected:
	template <uint8_t BIT>
	bool IsSet() const {
		static const uint8_t FLAG = (1 << BIT);
		return (value & FLAG) == FLAG;
	}
	template <uint8_t BIT>
	void Set() {
		static const uint8_t FLAG = (1 << BIT);
		value |= FLAG;
	}
	void Merge(uint8_t other) {
		value |= other;
	}
	uint8_t Value() {
		return value;
	}

private:
	uint8_t value;
};

struct DependencySubjectFlags : public DependencyFlags {
private:
	static constexpr uint8_t OWNERSHIP = 0;

public:
	DependencySubjectFlags &Apply(DependencySubjectFlags other) {
		Merge(other.Value());
		return *this;
	}

public:
	bool IsOwnership() const {
		return IsSet<OWNERSHIP>();
	}

public:
	DependencySubjectFlags &SetOwnership() {
		Set<OWNERSHIP>();
		return *this;
	}

public:
	string ToString() const override {
		string result;
		if (IsOwnership()) {
			result += "OWNS";
		}
		return result;
	}
};

struct DependencyDependentFlags : public DependencyFlags {
private:
	static constexpr uint8_t BLOCKING = 0;
	static constexpr uint8_t OWNED_BY = 1;

public:
	DependencyDependentFlags &Apply(DependencyDependentFlags other) {
		Merge(other.Value());
		return *this;
	}

public:
	bool IsBlocking() const {
		return IsSet<BLOCKING>();
	}
	bool IsOwnedBy() const {
		return IsSet<OWNED_BY>();
	}

public:
	DependencyDependentFlags &SetBlocking() {
		Set<BLOCKING>();
		return *this;
	}
	DependencyDependentFlags &SetOwnedBy() {
		Set<OWNED_BY>();
		return *this;
	}

public:
	string ToString() const override {
		string result;
		if (IsBlocking()) {
			result += "REGULAR";
		} else {
			result += "AUTOMATIC";
		}
		result += " | ";
		if (IsOwnedBy()) {
			result += "OWNED BY";
		}
		return result;
	}
};

struct CatalogEntryInfo {
public:
	string schema;
	string name;

public:
	bool operator==(const CatalogEntryInfo &other) const {
		if (!StringUtil::CIEquals(other.schema, schema)) {
			return false;
		}
		if (!StringUtil::CIEquals(other.name, name)) {
			return false;
		}
		return true;
	}

public:
	void Serialize(Serializer &serializer) const;
	static CatalogEntryInfo Deserialize(Deserializer &deserializer);
};

struct Dependency {
	Dependency(CatalogEntry &entry, // NOLINT: Allow implicit conversion from `CatalogEntry`
	           DependencyDependentFlags flags = DependencyDependentFlags().SetBlocking())
	    : flags(std::move(flags)) {
	}

	//! The type of dependency
	DependencyDependentFlags flags;
};

struct DependencyHashFunction {
	uint64_t operator()(const Dependency &a) const {
		return 0;
	}
};

struct DependencyEquality {
	bool operator()(const Dependency &a, const Dependency &b) const {
		return false;
	}
};
using dependency_set_t = unordered_set<Dependency, DependencyHashFunction, DependencyEquality>;

} // namespace duckdb
